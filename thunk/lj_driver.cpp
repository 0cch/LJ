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
	}

	void LJ_Driver::Error(const std::string& m)
	{
		std::cerr << m << std::endl;
	}

	void LJ_Driver::AddFunction(FunctionDefiniton *f)
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
		value_stack_.pop();

		dest = GetLValue(left);
		*dest = src;
	}


	boolean LJ_Driver::EvalBinaryBoolean(ExpressionType op, boolean left, boolean right, location &l)
	{
		boolean result;

		if (op == EQ_EXPRESSION) {
			result = left == right;
		} else if (op == NE_EXPRESSION) {
			result = left != right;
		} else {
			Error(l, "EvalBinaryBoolean error");
		}

		return result;
	}

	ValueBase* LJ_Driver::EvalBinaryInt(ExpressionType op, __int64 left, __int64 right, location &l)
	{
		ValueBase *v;

		Value<__int64, INT_VALUE> *int_value = NULL;
		Value<boolean, BOOLEAN_VALUE> *boolean_value = NULL;

		if (IsMathOperator(op)) {
			int_value = new Value<__int64, INT_VALUE>;
			v = int_value;
		} else if (IsCompareOperator(op)) {
			boolean_value = new Value<boolean, BOOLEAN_VALUE>;
			v = boolean_value;
		} else {
			__asm int 3
		}

		switch (operator) {
		case BOOLEAN_EXPRESSION:    /* FALLTHRU */
		case INT_EXPRESSION:        /* FALLTHRU */
		case DOUBLE_EXPRESSION:     /* FALLTHRU */
		case STRING_EXPRESSION:     /* FALLTHRU */
		case IDENTIFIER_EXPRESSION: /* FALLTHRU */
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
		case LOGICAL_AND_EXPRESSION:        /* FALLTHRU */
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
		case MINUS_EXPRESSION:              /* FALLTHRU */
		case FUNCTION_CALL_EXPRESSION:      /* FALLTHRU */
		case NULL_EXPRESSION:				/* FALLTHRU */
		default:
			__asm int 3;
		}

		return v;
	}

	ValueBase* LJ_Driver::EvalBinaryDouble(ExpressionType op, double left, double right, location &l)
	{
		ValueBase *v;

		Value<double, DOUBLE_VALUE> *double_value = NULL;
		Value<boolean, BOOLEAN_VALUE> *boolean_value = NULL;

		if (IsMathOperator(op)) {
			double_value = new Value<double, DOUBLE_VALUE>;
			v = double_value;
		} else if (IsCompareOperator(op)) {
			boolean_value = new Value<boolean, BOOLEAN_VALUE>;
			v = boolean_value;
		} else {
			__asm int 3
		}

		switch (op) {
		case BOOLEAN_EXPRESSION:    /* FALLTHRU */
		case INT_EXPRESSION:        /* FALLTHRU */
		case DOUBLE_EXPRESSION:     /* FALLTHRU */
		case STRING_EXPRESSION:     /* FALLTHRU */
		case IDENTIFIER_EXPRESSION: /* FALLTHRU */
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
		case LOGICAL_AND_EXPRESSION:        /* FALLTHRU */
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
		case MINUS_EXPRESSION:              /* FALLTHRU */
		case FUNCTION_CALL_EXPRESSION:      /* FALLTHRU */
		case NULL_EXPRESSION:               /* FALLTHRU */

		default:
			__asm int 3;
		}

		return v;
	}
}