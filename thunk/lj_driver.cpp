#include "lj_driver.hpp"
#include "lj_parser.hpp"

#include <math.h>

namespace LJ {

	LJ_Driver::LJ_Driver()
		: trace_scanning_(false), trace_parsing_(false), statement_list_(NULL)
	{

	}

	LJ_Driver::~LJ_Driver()
	{
	}

	int LJ_Driver::Parse(const std::string &f)
	{
		file_ = f;
		ScanBegin();
		Parser parser(*this);
		parser.set_debug_level(trace_parsing_);
		int res = parser.parse();
		ScanEnd();
		return res;
	}

	void LJ_Driver::Error(const location& l, const std::string& m)
	{
		std::cerr << l << ": " << m << std::endl;
		exit(0);
	}

	void LJ_Driver::Error(const std::string& m)
	{
		std::cerr << m << std::endl;
		exit(0);
	}

	void LJ_Driver::AddFunction(FunctionDefinition *f)
	{
		function_list_.push_back(f);
	}



	void LJ_Driver::Dump()
	{
		for (auto &i : *statement_list_) {
			i->Dump(0);
		}
	}

	void LJ_Driver::EvalBooleanExpression(boolean boolean_value)
	{
		BooleanValue *v = NEW_BOOLEAN_VALUE();
		v->value_ = boolean_value;
		value_stack_.push((ValueBase *)v);
	}

	void LJ_Driver::EvalIntExpression(__int64 int_value)
	{
		IntValue *v = NEW_INT_VALUE();
		v->value_ = int_value;
		value_stack_.push((ValueBase *)v);
	}

	void LJ_Driver::EvalDoubleExpression(double double_value)
	{
		DoubleValue *v = NEW_DOUBLE_VALUE();
		v->value_ = double_value;
		value_stack_.push((ValueBase *)v);
	}

	void LJ_Driver::EvalStringExpression(const std::string &string_value)
	{
		StringValue *v = NEW_STRING_VALUE();
		v->value_ = string_value;
		value_stack_.push((ValueBase *)v);
	}

	void LJ_Driver::EvalNullExpression()
	{
		NullValue *v = NEW_NULL_VALUE();
		value_stack_.push((ValueBase *)v);
	}

	void LJ_Driver::EvalIdentifierExpression(Expression *expr)
	{
		ValueBase *v;

		std::map<std::string, ValueBase *>::iterator it;
		it = local_value_stack_.top().find(*((std::string *)expr->GetValue(0)));
		if (it != local_value_stack_.top().end()) {
			v = it->second;
		}
		else {
			it = global_value_.find(*((std::string *)expr->GetValue(0)));
			if (it != global_value_.end()) {
				v = it->second;
			}
			else {
				Error(expr->GetLocation(), "EvalIdentifierExpression error");
			}
		}


		value_stack_.push(v);
	}

	ValueBase ** LJ_Driver::GetIdentifierLValue(const std::string &identifier)
	{
		std::map<std::string, ValueBase *>::iterator it;
		it = local_value_stack_.top().find(identifier);
		if (it == local_value_stack_.top().end()) {
			return &it->second;
		}
		else {
			it = global_value_.find(identifier);
			if (it != global_value_.end()) {
				return &it->second;
			}
		}

		std::pair<const std::string, ValueBase *> v(identifier, NULL);

		if (local_value_stack_.size() == 0) {
			return &global_value_.insert(v).first->second;
		}
		else {
			return &local_value_stack_.top().insert(v).first->second;
		}
	}

	ValueBase ** LJ_Driver::GetLValue(Expression *expr)
	{
		if (expr->GetType() == IDENTIFIER_EXPRESSION) {
			return GetIdentifierLValue(*((std::string *)expr->GetValue(0)));
		}
		else {
			Error(expr->GetLocation(), "GetLValue error");
			return NULL;
		}
	}

	void LJ_Driver::EvalAssignExpression(Expression *left, Expression *right)
	{
		ValueBase *src;
		ValueBase **dest;

		EvalExpression(right);
		src = value_stack_.top();

		dest = GetLValue(left);
		*dest = src;
	}


	ValueBase* LJ_Driver::EvalBinaryBoolean(ExpressionType op, boolean left, boolean right, const location &l)
	{
		BooleanValue *v = NEW_BOOLEAN_VALUE();

		if (op == EQ_EXPRESSION) {
			v->value_ = left == right;
		} else if (op == NE_EXPRESSION) {
			v->value_ = left != right;
		} else {
			Error(l, "EvalBinaryBoolean error");
		}

		return v;
	}

	ValueBase* LJ_Driver::EvalBinaryInt(ExpressionType op, __int64 left, __int64 right, const location &l)
	{
		ValueBase *v;

		IntValue *int_value = NULL;
		BooleanValue *boolean_value = NULL;

		if (IsMathOperator(op)) {
			int_value = NEW_INT_VALUE();
			v = int_value;
		} else if (IsCompareOperator(op)) {
			boolean_value = NEW_BOOLEAN_VALUE();
			v = boolean_value;
		} else {
			__asm int 3
		}

		switch (op) {
		case BOOLEAN_EXPRESSION:
		case INT_EXPRESSION:
		case DOUBLE_EXPRESSION:
		case STRING_EXPRESSION:
		case IDENTIFIER_EXPRESSION:
		case ASSIGN_EXPRESSION:
			__asm int 3
			break;
		case ADD_EXPRESSION:
			int_value->value_ = left + right;
			break;
		case SUB_EXPRESSION:
			int_value->value_ = left - right;
			break;
		case MUL_EXPRESSION:
			int_value->value_ = left * right;
			break;
		case DIV_EXPRESSION:
			int_value->value_ = left / right;
			break;
		case MOD_EXPRESSION:
			int_value->value_ = left % right;
			break;
		case LOGICAL_AND_EXPRESSION:
		case LOGICAL_OR_EXPRESSION:
			__asm int 3
			break;
		case EQ_EXPRESSION:
			boolean_value->value_ = left == right;
			break;
		case NE_EXPRESSION:
			boolean_value->value_ = left != right;
			break;
		case GT_EXPRESSION:
			boolean_value->value_ = left > right;
			break;
		case GE_EXPRESSION:
			boolean_value->value_ = left >= right;
			break;
		case LT_EXPRESSION:
			boolean_value->value_ = left < right;
			break;
		case LE_EXPRESSION:
			boolean_value->value_ = left <= right;
			break;
		case MINUS_EXPRESSION:
		case FUNCTION_CALL_EXPRESSION:
		case NULL_EXPRESSION:
		default:
			__asm int 3;
		}

		return v;
	}

	ValueBase* LJ_Driver::EvalBinaryDouble(ExpressionType op, double left, double right, const location &l)
	{
		ValueBase *v;

		DoubleValue *double_value = NULL;
		BooleanValue *boolean_value = NULL;

		if (IsMathOperator(op)) {
			double_value = NEW_DOUBLE_VALUE();
			v = double_value;
		} else if (IsCompareOperator(op)) {
			boolean_value = NEW_BOOLEAN_VALUE();
			v = boolean_value;
		} else {
			__asm int 3
		}

		switch (op) {
		case BOOLEAN_EXPRESSION:
		case INT_EXPRESSION:
		case DOUBLE_EXPRESSION:
		case STRING_EXPRESSION:
		case IDENTIFIER_EXPRESSION:
		case ASSIGN_EXPRESSION:
			__asm int 3;
			break;
		case ADD_EXPRESSION:
			double_value->value_ = left + right;
			break;
		case SUB_EXPRESSION:
			double_value->value_ = left - right;
			break;
		case MUL_EXPRESSION:
			double_value->value_ = left * right;
			break;
		case DIV_EXPRESSION:
			double_value->value_ = left / right;
			break;
		case MOD_EXPRESSION:
			double_value->value_ = fmod(left, right);
			break;
		case LOGICAL_AND_EXPRESSION:
		case LOGICAL_OR_EXPRESSION:
			__asm int 3;
			break;
		case EQ_EXPRESSION:
			boolean_value->value_ = left == right;
			break;
		case NE_EXPRESSION:
			boolean_value->value_ = left != right;
			break;
		case GT_EXPRESSION:
			boolean_value->value_ = left > right;
			break;
		case GE_EXPRESSION:
			boolean_value->value_ = left >= right;
			break;
		case LT_EXPRESSION:
			boolean_value->value_ = left < right;
			break;
		case LE_EXPRESSION:
			boolean_value->value_ = left <= right;
			break;
		case MINUS_EXPRESSION:
		case FUNCTION_CALL_EXPRESSION:
		case NULL_EXPRESSION:

		default:
			__asm int 3;
		}

		return v;
	}

	ValueBase* LJ_Driver::EvalCompareString(ExpressionType op, std::string &left, std::string &right, const location &l)
	{
		BooleanValue *v = NEW_BOOLEAN_VALUE();
		int cmp = left.compare(right);

		if (op == EQ_EXPRESSION) {
			v->value_ = (cmp == 0);
		} else if (op == NE_EXPRESSION) {
			v->value_ = (cmp != 0);
		} else if (op == GT_EXPRESSION) {
			v->value_ = (cmp > 0);
		} else if (op == GE_EXPRESSION) {
			v->value_ = (cmp >= 0);
		} else if (op == LT_EXPRESSION) {
			v->value_ = (cmp < 0);
		} else if (op == LE_EXPRESSION) {
			v->value_ = (cmp <= 0);
		} else {
			Error(l, "EvalBinaryBoolean error");
		}

		return v;
	}

	ValueBase* LJ_Driver::EvalBinaryNull(ExpressionType op, ValueBase *left, ValueBase *right, const location &l)
	{
		BooleanValue *v = NEW_BOOLEAN_VALUE();

		if (op == EQ_EXPRESSION) {
			v->value_ = left->GetType() == NULL_VALUE && right->GetType() == NULL_VALUE;
		} else if (op == NE_EXPRESSION) {
			v->value_ =  !(left->GetType() == NULL_VALUE && right->GetType() == NULL_VALUE);
		} else {
			Error(l, "EvalBinaryNull error");
		}

		return v;
	}

	ValueBase* LJ_Driver::ChainString(std::string &left, std::string &right)
	{
		StringValue *v = NEW_STRING_VALUE();
		v->value_ = left + right;
		return v;
	}

	void LJ_Driver::EvalBinaryExpression(ExpressionType op, Expression *left, Expression *right)
	{
		ValueBase *left_val;
		ValueBase *right_val;
		ValueBase *result;

		EvalExpression(left);
		EvalExpression(right);

		right_val = value_stack_.top();
		left_val = *(value_stack_._Get_container().end() - 2);

		if (left_val->GetType() == INT_VALUE && right_val->GetType() == INT_VALUE) {
			result = EvalBinaryInt(op, dynamic_cast<IntValue *>(left_val)->value_, 
				dynamic_cast<IntValue *>(right_val)->value_, left->GetLocation());
		} 
		else if (left_val->GetType() == DOUBLE_VALUE && right_val->GetType() == DOUBLE_VALUE) {
			result = EvalBinaryDouble(op, dynamic_cast<DoubleValue *>(left_val)->value_,
				dynamic_cast<DoubleValue *>(right_val)->value_, left->GetLocation());
		} 
		else if (left_val->GetType() == INT_VALUE && right_val->GetType() == DOUBLE_VALUE) {
			result = EvalBinaryDouble(op, (double)dynamic_cast<IntValue *>(left_val)->value_,
				dynamic_cast<DoubleValue *>(right_val)->value_, left->GetLocation());
		} 
		else if (left_val->GetType() == DOUBLE_VALUE && right_val->GetType() == INT_VALUE) {
			result = EvalBinaryDouble(op, dynamic_cast<DoubleValue *>(left_val)->value_,
				(double)dynamic_cast<IntValue *>(right_val)->value_, left->GetLocation());
		} 
		else if (left_val->GetType() == BOOLEAN_VALUE && right_val->GetType() == BOOLEAN_VALUE) {

			result = EvalBinaryBoolean(op, dynamic_cast<BooleanValue *>(left_val)->value_,
				dynamic_cast<BooleanValue *>(right_val)->value_, left->GetLocation());
		} 
		else if (left_val->GetType() == STRING_VALUE && op == ADD_EXPRESSION) {
			result = ChainString(dynamic_cast<StringValue *>(left_val)->value_,
				dynamic_cast<StringValue *>(right_val)->value_);
		} 
		else if (left_val->GetType() == STRING_VALUE && right_val->GetType() == STRING_VALUE) {
			result = EvalCompareString(op, dynamic_cast<StringValue *>(left_val)->value_, 
				dynamic_cast<StringValue *>(right_val)->value_, left->GetLocation());
		} 
		else if (left_val->GetType() == NULL_VALUE || right_val->GetType() == NULL_VALUE) {
			result = EvalBinaryNull(op, left_val, right_val, left->GetLocation());
		} 
		else {
			Error(left->GetLocation(), "EvalBinaryExpression error");
		}

		value_stack_.pop();
		value_stack_.pop();

		value_stack_.push(result);
	}

	void LJ_Driver::EvalLogicalAndOrExpression(ExpressionType op, Expression *left, Expression *right)
	{
		ValueBase *left_val;
		ValueBase *right_val;
		
		BooleanValue *v = NEW_BOOLEAN_VALUE();

		EvalExpression(left);
		left_val = value_stack_.top();
		value_stack_.pop();

		if (left_val->GetType() != BOOLEAN_VALUE) {
			Error(left->GetLocation(), "EvalLogicalAndOrExpression error");
		}
		if (op == LOGICAL_AND_EXPRESSION) {
			if (!dynamic_cast<BooleanValue *>(left_val)->value_) {
				v->value_ = 0;
				goto FUNC_END;
			}
		} 
		else if (op == LOGICAL_OR_EXPRESSION) {
			if (dynamic_cast<BooleanValue *>(left_val)->value_) {
				v->value_ = 1;
				goto FUNC_END;
			}
		} else {
			__asm int 3;
		}

		EvalExpression(right);
		right_val = value_stack_.top();
		value_stack_.pop();

		v->value_ = dynamic_cast<BooleanValue *>(right_val)->value_;

FUNC_END:
		value_stack_.push(v);
	}

	void LJ_Driver::EvalMinusExpression(Expression *expr)
	{
		ValueBase* v;
		ValueBase* result;
		EvalExpression(expr);
		v = value_stack_.top();
		value_stack_.pop();
		if (v->GetType() == INT_VALUE) {
			IntValue *int_value = NEW_INT_VALUE();
			int_value->value_ = -dynamic_cast<IntValue *>(v)->value_;
			result = int_value;
		} else if (v->GetType() == DOUBLE_VALUE) {
			DoubleValue *double_value = NEW_DOUBLE_VALUE();
			double_value->value_ = -dynamic_cast<DoubleValue *>(v)->value_;
			result = double_value;
		} else {
			Error(expr->GetLocation(), "EvalMinusExpression error");
		}

		value_stack_.push(result);
	}

	void LJ_Driver::CallFunction(Expression *e, FunctionDefinition *func)
	{
		BinaryExpression<FUNCTION_CALL_EXPRESSION> *expr = dynamic_cast<BinaryExpression<FUNCTION_CALL_EXPRESSION> *>(e);

		ValueBase *v;
		ArgumentList::iterator arg_p;
		ParameterList::iterator param_p;

		for (arg_p = expr->GetArgList()->begin(), param_p = func->GetParamList()->begin(); 
			arg_p != expr->GetArgList()->end(); ++arg_p, ++param_p) {
			ValueBase *arg_val;

			if (param_p == func->GetParamList()->end()) {
				Error(expr->GetLocation(), "CallFunction error");
			}
			EvalExpression(*arg_p);
			arg_val = value_stack_.top();
			value_stack_.pop();

			local_value_stack_.top()[*param_p] = arg_val;
		}

		if (param_p != func->GetParamList()->end()) {
			Error(expr->GetLocation(), "CallFunction error");
		}
		StatementResult result = ExecuteStatementList((StatementList *)func->GetBlock()->GetValue(0));
		if (result.type_ == RETURN_STATEMENT_RESULT) {
			v = result.value_;
		} else {
			v = NEW_NULL_VALUE();
		}

		value_stack_.push(v);
	}

	void LJ_Driver::EvalFunctionCallExpression(Expression *e)
	{
		BinaryExpression<FUNCTION_CALL_EXPRESSION> *expr = dynamic_cast<BinaryExpression<FUNCTION_CALL_EXPRESSION> *>(e);
		FunctionDefinition *func = NULL;

		for (std::list<FunctionDefinition *>::iterator it = function_list_.begin();
			it != function_list_.end(); ++it) {

				if ((*it)->GetFunctionName() == expr->GetFunctionName()) {
					func = *it;
					break;
				}
		}

		if (func == NULL) {
			Error(expr->GetLocation(), "EvalFunctionCallExpression error");
		}

		local_value_stack_.push(std::map<std::string, ValueBase *>());

		switch (func->GetType()) {
		case FUNCTION_DEFINITION:
			CallFunction(expr, func);
			break;
		default:
			__asm int 3;
		}
		local_value_stack_.pop();
	}

	void LJ_Driver::EvalExpression(Expression *expr)
	{
		switch (expr->GetType()) {
		case BOOLEAN_EXPRESSION:
			EvalBooleanExpression(*(boolean *)expr->GetValue(0));
			break;
		case INT_EXPRESSION:
			EvalIntExpression(*(__int64 *)expr->GetValue(0));
			break;
		case DOUBLE_EXPRESSION:
			EvalDoubleExpression(*(double *)expr->GetValue(0));
			break;
		case STRING_EXPRESSION:
			EvalStringExpression(*(std::string *)expr->GetValue(0));
			break;
		case IDENTIFIER_EXPRESSION:
			EvalIdentifierExpression(expr);
			break;
		case ASSIGN_EXPRESSION:
			EvalAssignExpression((Expression *)expr->GetValue(0), (Expression *)expr->GetValue(1));
			break;
		case ADD_EXPRESSION:
		case SUB_EXPRESSION:
		case MUL_EXPRESSION:
		case DIV_EXPRESSION:
		case MOD_EXPRESSION:
		case EQ_EXPRESSION:
		case NE_EXPRESSION:
		case GT_EXPRESSION:
		case GE_EXPRESSION:
		case LT_EXPRESSION:
		case LE_EXPRESSION:
			EvalBinaryExpression(expr->GetType(), (Expression *)expr->GetValue(0), (Expression *)expr->GetValue(1));
			break;
		case LOGICAL_AND_EXPRESSION:
		case LOGICAL_OR_EXPRESSION:
			EvalLogicalAndOrExpression(expr->GetType(), (Expression *)expr->GetValue(0), (Expression *)expr->GetValue(1));
			break;
		case MINUS_EXPRESSION:
			EvalMinusExpression((Expression *)expr->GetValue(0));
			break;
		case FUNCTION_CALL_EXPRESSION:
			EvalFunctionCallExpression(expr);
			break;
		case NULL_EXPRESSION:
			EvalNullExpression();
			break;
		default:
			__asm int 3;
		}
	}

	ValueBase *LJ_Driver::GetEvalExpression(Expression *expr)
	{
		EvalExpression(expr);
		ValueBase *v = value_stack_.top();
		value_stack_.pop();
		return v;
	}

	StatementResult LJ_Driver::ExecuteExpressionStatement(Statement *statement)
	{
		ValueBase *v;

		v = GetEvalExpression((Expression *)statement->GetValue(0));

		return StatementResult(NORMAL_STATEMENT_RESULT, v);
	}

	StatementResult	LJ_Driver::ExecuteGlobalStatement(Statement *statement)
	{
		if (local_value_stack_.size() == 0) {
			Error(statement->GetLocation(), "ExecuteGlobalStatement error");
		}

		IdentifierList * identifier_list = (IdentifierList *)statement->GetValue(0);
		for (IdentifierList::iterator it = identifier_list->begin();
			it != identifier_list->end(); ++it) {

			if (global_value_.find(*it) == global_value_.end()) {
				Error(statement->GetLocation(), "ExecuteGlobalStatement error");
			}
		}

		return StatementResult(NORMAL_STATEMENT_RESULT, NULL);
	}

	StatementResult LJ_Driver::ExecuteElseif(ElseifList *elseif_list, boolean *executed)
	{
		StatementResult result(NORMAL_STATEMENT_RESULT, NULL);
		
		*executed = 0;
		for (ElseifList::iterator it = elseif_list->begin();
			it != elseif_list->end(); ++it) {

			ValueBase *v = GetEvalExpression((Expression *)(*it)->GetValue(0));
			if (v->GetType() != BOOLEAN_VALUE) {
				Error((*it)->GetLocation(), "ExecuteElseif error");
			}

			if (dynamic_cast<BooleanValue *>(v)->value_) {
				result = ExecuteStatementList((StatementList *)((Block *)(*it)->GetValue(1))->GetValue(0));
				*executed = 1;
				if (result.type_ != NORMAL_STATEMENT_RESULT) {
					goto FUNC_END;
				}
			}
		}

	FUNC_END:
		return result;
	}

	StatementResult LJ_Driver::ExecuteIfStatement(Statement *statement)
	{
		StatementResult result(NORMAL_STATEMENT_RESULT, NULL);
	
		ValueBase *v = GetEvalExpression((Expression *)statement->GetValue(0));
		if (v->GetType() != BOOLEAN_VALUE) {
			Error(statement->GetLocation(), "ExecuteIfStatement error");
		}
		
		if (dynamic_cast<BooleanValue *>(v)->value_) {
			result = ExecuteStatementList((StatementList *)((Block *)statement->GetValue(1))->GetValue(0));
		}
		else {
			boolean elseif_executed;
			result = ExecuteElseif((ElseifList *)statement->GetValue(2), &elseif_executed);
			if (result.type_ != NORMAL_STATEMENT_RESULT) {
				goto FUNC_END;
			}
			if (!elseif_executed && statement->GetValue(3) != NULL) {
				result = ExecuteStatementList((StatementList *)((Block *)statement->GetValue(3))->GetValue(0));
			}
		}

	FUNC_END:
		return result;
	}

	StatementResult LJ_Driver::ExecuteWhileStatement(Statement *statement)
	{
		StatementResult result(NORMAL_STATEMENT_RESULT, NULL);
		
		for (;;) {
			ValueBase *v = GetEvalExpression((Expression *)statement->GetValue(0));
			if (v->GetType() != BOOLEAN_VALUE) {
				Error(statement->GetLocation(), "ExecuteWhileStatement error");
			}
			
			if (!dynamic_cast<BooleanValue *>(v)->value_) {
				break;
			}

			result = ExecuteStatementList((StatementList *)((Block *)statement->GetValue(1))->GetValue(0));
			if (result.type_ == RETURN_STATEMENT_RESULT) {
				break;
			}
			else if (result.type_ == BREAK_STATEMENT_RESULT) {
				result.type_ = NORMAL_STATEMENT_RESULT;
				break;
			}
		}

		return result;
	}

	StatementResult LJ_Driver::ExecuteForStatement(Statement *statement)
	{
		StatementResult result(NORMAL_STATEMENT_RESULT, NULL);

		if (statement->GetValue(0) != NULL) {
			GetEvalExpression((Expression *)statement->GetValue(0));
		}
		for (;;) {
			if (statement->GetValue(1) != NULL) {
				ValueBase *v = GetEvalExpression((Expression *)statement->GetValue(0));
				if (v->GetType() != BOOLEAN_VALUE) {
					Error(statement->GetLocation(), "ExecuteForStatement error");
				}
				
				if (!dynamic_cast<BooleanValue *>(v)->value_) {
					break;
				}
			}
			result = ExecuteStatementList((StatementList *)((Block *)statement->GetValue(3))->GetValue(0));
			if (result.type_ == RETURN_STATEMENT_RESULT) {
				break;
			}
			else if (result.type_ == BREAK_STATEMENT_RESULT) {
				result.type_ = NORMAL_STATEMENT_RESULT;
				break;
			}

			if (statement->GetValue(2) != NULL) {
				GetEvalExpression((Expression *)statement->GetValue(2));
			}
		}

		return result;
	}

	StatementResult LJ_Driver::ExecuteReturnStatement(Statement *statement)
	{
		StatementResult result(RETURN_STATEMENT_RESULT, NULL);

		if (statement->GetValue(0) != NULL) {
			
			ValueBase *v = GetEvalExpression((Expression *)statement->GetValue(0));
			return StatementResult(RETURN_STATEMENT_RESULT, v);
		}
		else {
			ValueBase *v = new Value < NullType, NULL_VALUE>;
			return StatementResult(RETURN_STATEMENT_RESULT, v);
		}
	}

	StatementResult LJ_Driver::ExecuteBreakStatement(Statement *statement)
	{
		return StatementResult(BREAK_STATEMENT_RESULT, NULL);
	}

	StatementResult LJ_Driver::ExecuteContinueStatement(Statement *statement)
	{
		return StatementResult(CONTINUE_STATEMENT_RESULT, NULL);
	}

	StatementResult LJ_Driver::ExecuteStatement(Statement *statement)
	{
		StatementResult result(NORMAL_STATEMENT_RESULT, NULL);

		switch (statement->GetType()) {
		case EXPRESSION_STATEMENT:
			result = ExecuteExpressionStatement(statement);
			break;
		case GLOBAL_STATEMENT:
			result = ExecuteGlobalStatement(statement);
			break;
		case IF_STATEMENT:
			result = ExecuteIfStatement(statement);
			break;
		case WHILE_STATEMENT:
			result = ExecuteWhileStatement(statement);
			break;
		case FOR_STATEMENT:
			result = ExecuteForStatement(statement);
			break;
		case RETURN_STATEMENT:
			result = ExecuteReturnStatement(statement);
			break;
		case BREAK_STATEMENT:
			result = ExecuteBreakStatement(statement);
			break;
		case CONTINUE_STATEMENT:
			result = ExecuteContinueStatement(statement);
			break;
		default:
			__asm int 3;
		}

		return result;
	}

	StatementResult LJ_Driver::ExecuteStatementList(StatementList *list)
	{
		StatementResult result(NORMAL_STATEMENT_RESULT, NULL);

		for (StatementList::iterator it = list->begin();
			it != list->end(); ++it) {
			result = ExecuteStatement(*it);
			if (result.type_ != NORMAL_STATEMENT_RESULT)
				goto FUNC_END;
		}

	FUNC_END:
		return result;
	}
}