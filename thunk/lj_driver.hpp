#ifndef __LJ_DRIVER_H__
#define __LJ_DRIVER_H__
#include <string>
#include <map>
#include <stack>

typedef unsigned char boolean;

#include "lj_parser.hpp"
#include "lj_ast.h"
#include "lj_val.h"

// Tell Flex the lexer's prototype ...
# define YY_DECL Parser::symbol_type yylex(LJ_Driver& driver)
// ... and declare it for the parser's sake.
YY_DECL;

namespace LJ {
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

		void Error(const location& l, const std::string& m);
		void Error(const std::string& m);

		void Dump();

		void AddFunction(FunctionDefiniton *f);

		void EvalBooleanExpression(boolean boolean_value);
		void EvalIntExpression(__int64 int_value);
		void EvalDoubleExpression(double double_value);
		void EvalStringExpression(char *string_value);
		void EvalNullExpression();
		void EvalIdentifierExpression(Expression *expr);
		ValueBase ** GetIdentifierLValue(const std::string &identifier, bool is_global_value);
		ValueBase ** GetLValue(Expression *expr);
		void EvalAssignExpression(Expression *left, Expression *right);
		boolean EvalBinaryBoolean(ExpressionType op, boolean left, boolean right, location &l);
		ValueBase* EvalBinaryInt(ExpressionType op, __int64 left, __int64 right, location &l);
		ValueBase* EvalBinaryDouble(ExpressionType op, double left, double right, location &l);
		StatementList *statement_list_;

		std::stack<ValueBase *> value_stack_;

		std::map<std::string, ValueBase *> global_value_;

		std::stack<std::map<std::string, ValueBase *>> local_value_stack_;

	private:
		std::list<FunctionDefiniton *> function_list_;

	};

#define IsMathOperator(op) \
	((op) == ADD_EXPRESSION || (op) == SUB_EXPRESSION\
	|| (op) == MUL_EXPRESSION || (op) == DIV_EXPRESSION\
	|| (op) == MOD_EXPRESSION)

#define IsCompareOperator(op) \
	((op) == EQ_EXPRESSION || (op) == NE_EXPRESSION\
	|| (op) == GT_EXPRESSION || (op) == GE_EXPRESSION\
	|| (op) == LT_EXPRESSION || (op) == LE_EXPRESSION)

#define IsLogicalOperator(op) \
	((op) == LOGICAL_AND_EXPRESSION || (op) == LOGICAL_OR_EXPRESSION)
}




#endif
