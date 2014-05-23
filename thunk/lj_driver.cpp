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
		Value<boolean, BOOLEAN_VALUE> *v = new Value<boolean, BOOLEAN_VALUE>;
		v->value_ = boolean_value;
		value_stack_.push((ValueBase *)v);
	}

	void LJ_Driver::EvalIntExpression(__int64 int_value)
	{
		Value<__int64, INT_VALUE> *v = new Value<__int64, INT_VALUE>;
		v->value_ = int_value;
		value_stack_.push((ValueBase *)v);
	}

	void LJ_Driver::EvalDoubleExpression(double double_value)
	{
		Value<double, DOUBLE_VALUE> *v = new Value<double, DOUBLE_VALUE>;
		v->value_ = double_value;
		value_stack_.push((ValueBase *)v);
	}

	void LJ_Driver::EvalStringExpression(char *string_value)
	{
		Value<std::string, STRING_VALUE> *v = new Value<std::string, STRING_VALUE>;
		v->value_ = string_value;
		value_stack_.push((ValueBase *)v);
	}

	void LJ_Driver::EvalNullExpression()
	{
		Value<NullType, NULL_VALUE> *v = new Value<NullType, NULL_VALUE>;
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

	ValueBase ** LJ_Driver::GetIdentifierLValue(const std::string &identifier, bool is_global_value)
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

		if (is_global_value) {
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
		Value<boolean, BOOLEAN_VALUE> *v = new Value<boolean, BOOLEAN_VALUE>;

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

		Value<__int64, INT_VALUE> *int_value = NULL;
		Value<boolean, BOOLEAN_VALUE> *boolean_value = NULL;

		if (IsMathop(op)) {
			int_value = new Value<__int64, INT_VALUE>;
			v = int_value;
		} else if (IsCompareop(op)) {
			boolean_value = new Value<boolean, BOOLEAN_VALUE>;
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

		Value<double, DOUBLE_VALUE> *double_value = NULL;
		Value<boolean, BOOLEAN_VALUE> *boolean_value = NULL;

		if (IsMathop(op)) {
			double_value = new Value<double, DOUBLE_VALUE>;
			v = double_value;
		} else if (IsCompareop(op)) {
			boolean_value = new Value<boolean, BOOLEAN_VALUE>;
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
		Value<boolean, BOOLEAN_VALUE> *v = new Value<boolean, BOOLEAN_VALUE>;
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
		Value<boolean, BOOLEAN_VALUE> *v = new Value<boolean, BOOLEAN_VALUE>;

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
		Value<std::string, STRING_VALUE> *v = new Value<std::string, STRING_VALUE>;
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
		left_val = *(value_stack_.end() - 2);

		if (left_val->GetType() == INT_VALUE && right_val->GetType() == INT_VALUE) {
			result = EvalBinaryInt(op, dynamic_cast<Value<__int64, INT_VALUE> *>(left_val).value_, 
				dynamic_cast<Value<__int64, INT_VALUE> *>(right_val).value_, left->GetLocation());
		} 
		else if (left_val->GetType() == DOUBLE_VALUE && right_val->GetType() == DOUBLE_VALUE) {
			result = EvalBinaryDouble(op, dynamic_cast<Value<double, DOUBLE_VALUE> *>(left_val).value_, 
				dynamic_cast<Value<double, DOUBLE_VALUE> *>(right_val).value_, left->GetLocation());
		} 
		else if (left_val->GetType() == INT_VALUE && right_val->GetType() == DOUBLE_VALUE) {
			result = EvalBinaryDouble(op, (double)dynamic_cast<Value<__int64, INT_VALUE> *>(left_val).value_, 
				dynamic_cast<Value<double, DOUBLE_VALUE> *>(right_val).value_, left->GetLocation());
		} 
		else if (left_val->GetType() == DOUBLE_VALUE && right_val->GetType() == INT_VALUE) {
			result = EvalBinaryDouble(op, dynamic_cast<Value<double, DOUBLE_VALUE> *>(left_val).value_, 
				(double)dynamic_cast<Value<__int64, INT_VALUE> *>(right_val).value_, left->GetLocation());
		} 
		else if (left_val->GetType() == BOOLEAN_VALUE && right_val->GetType() == BOOLEAN_VALUE) {

			result = EvalBinaryBoolean(op, dynamic_cast<Value<boolean, BOOLEAN_VALUE> *>(left_val).value_, 
				dynamic_cast<Value<boolean, BOOLEAN_VALUE> *>(right_val).value_, left->GetLocation());
		} 
		else if (left_val->GetType() == STRING_VALUE && op == ADD_EXPRESSION) {
			result = ChainString(dynamic_cast<Value<std::string, STRING_VALUE> *>(left_val).value_, 
				dynamic_cast<Value<std::string, STRING_VALUE> *>(right_val).value_);
		} 
		else if (left_val->GetType() == STRING_VALUE && right_val->GetType() == STRING_VALUE) {
			result = EvalCompareString(op, left_val, right_val, left->GetLocation());
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
		
		Value<boolean, BOOLEAN_VALUE> v = new Value<boolean, BOOLEAN_VALUE>;

		EvalExpression(left);
		left_val = value_stack_.top();
		value_stack_.pop();

		if (left_val.GetType() != BOOLEAN_VALUE) {
			Error(left->GetLocation(), "EvalLogicalAndOrExpression error");
		}
		if (op == LOGICAL_AND_EXPRESSION) {
			if (!dynamic_cast<Value<boolean, BOOLEAN_VALUE> *>(left_val).value_) {
				v.value_ = 0;
				goto FUNC_END;
			}
		} 
		else if (op == LOGICAL_OR_EXPRESSION) {
			if (dynamic_cast<Value<boolean, BOOLEAN_VALUE> *>(left_val).value_) {
				v.value_ = 1;
				goto FUNC_END;
			}
		} else {
			__asm int 3;
		}

		EvalExpression(right);
		right_val = value_stack_.top();
		value_stack_.pop();

		v.value_ = dynamic_cast<Value<boolean, BOOLEAN_VALUE> *>(right_val).value_;

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
			Value<__int64, INT_VALUE> *int_value = new Value<__int64, INT_VALUE>;
			int_value->value_ = -dynamic_cast<Value<__int64, INT_VALUE> *>(v)->value_;
			result = int_value;
		} else if (v->GetType() == DOUBLE_VALUE) {
			Value<double, DOUBLE_VALUE> *double_value = new Value<double, DOUBLE_VALUE>;
			double_value->value_ = -dynamic_cast<Value<double, DOUBLE_VALUE> *>(v)->value_;
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
		Statement *result;
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

			local_value_stack_.top()[*param_p] = arg_val
		}

		if (param_p != func->GetParamList()->end()) {
			Error(expr->GetLocation(), "CallFunction error");
		}
// 		result = ExecuteStatementList(func->GetBlock()->GetStatementList());
// 		if (result.GetType() == RETURN_STATEMENT_RESULT) {
// 			value = result.u.return_value;
// 		} else {
// 			v = new Value<NullType, NULL_VALUE>;
// 		}

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
			EvalIdentifierExpression(*(std::string *)expr->GetValue(0));
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
}