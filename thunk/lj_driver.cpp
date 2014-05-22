#include "lj_driver.hpp"
#include "lj_parser.hpp"


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
	LJ::Parser parser(*this);
	parser.set_debug_level(trace_parsing_);
	int res = parser.parse();
	ScanEnd();
	return res;
}

void LJ_Driver::Error(const LJ::location& l, const std::string& m)
{
	std::cerr << l << ": " << m << std::endl;
}

void LJ_Driver::Error(const std::string& m)
{
	std::cerr << m << std::endl;
}

void LJ_Driver::AddFunction(LJ::FunctionDefiniton *f)
{
	function_list_.push_back(f);
}

void LJ_Driver::EvalBooleanExpression(boolean boolean_value)
{
	LJ::Value<boolean, BOOLEAN_VALUE> *v = new LJ::Value<boolean, BOOLEAN_VALUE>;
	v.value_ = boolean_value;
	value_stack_.push(v);
}

void LJ_Driver::EvalIntExpression(__int64 int_value)
{
	LJ::Value<__int64, INT_VALUE> *v = new LJ::Value<__int64, INT_VALUE>;
	v.value_ = int_value;
	value_stack_.push(v);
}

void LJ_Driver::EvalDoubleExpression(double double_value)
{
	LJ::Value<double, DOUBLE_VALUE> *v = new LJ::Value<double, DOUBLE_VALUE>;
	v.value_ = double_value;
	value_stack_.push(v);
}

void LJ_Driver::EvalStringExpression(char *string_value)
{
	LJ::Value<std::string, STRING_VALUE> *v = new LJ::Value<std::string, STRING_VALUE>;
	v.value_ = string_value;
	value_stack_.push(v);
}

void LJ_Driver::EvalNullExpression()
{
	LJ::Value<NullType, NULL_VALUE> *v = new LJ::Value<NullType, NULL_VALUE>;
	value_stack_.push(v);
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