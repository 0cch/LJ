#ifndef __LJ_DRIVER_H__
#define __LJ_DRIVER_H__
#include <string>
#include <map>
#include <stack>

#include "lj_parser.hpp"
#include "lj_ast.h"
#include "lj_val.h"

// Tell Flex the lexer's prototype ...
# define YY_DECL LJ::LJ_Parser::symbol_type yylex(LJ_Driver& driver)
// ... and declare it for the parser's sake.
YY_DECL;

class LJ_Driver
{
public:
	LJ_Driver();
	virtual ~LJ_Driver();

	void ScanBegin();
	void ScanEnd();
	bool trace_scanning_;

	int Parse(const std::string& f);
	std::string file_;
	bool trace_parsing_;

	void Error(const LJ::location& l, const std::string& m);
	void Error(const std::string& m);

	void AddFunction(LJ::FunctionDefiniton *f);

	void EvalBooleanExpression(boolean boolean_value);
	void EvalIntExpression(__int64 int_value);
	void EvalDoubleExpression(double double_value);
	void EvalStringExpression(char *string_value);
	void EvalNullExpression();
	void EvalIdentifierExpression(Expression *expr);
	LJ::StatementList *statement_list_;

	std::stack<ValueBase *> value_stack_;

	std::map<std::string, ValueBase *> global_value_;

	std::stack<std::map<std::string, ValueBase *>> local_value_stack_;

private:
	std::list<LJ::FunctionDefiniton *> function_list_;
	
};
#endif
