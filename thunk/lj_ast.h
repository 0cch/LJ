#ifndef __LJ_AST_H__
#define __LJ_AST_H__

#include <list>
#include <set>
#include <string>
#include "location.hh"

#define MAKE_VALUE_EXP(t, u, v, l)		new ValueExpression<t, u>(v, l)
#define MAKE_EXP(t, l)						new NullExpression<t>(l)
#define MAKE_UNARY_EXP(t, e, l)			new UnaryExpression<t>(e, l)
#define MAKE_BIN_EXP(t, e0, e1, l)		new BinaryExpression<t>(e0, e1, l)

#define MAKE_EXP_STAT(e, l)				new ExpressionStatement(e, l)
#define MAKE_GLOBAL_STAT(e, l)			new GlobalStatement(e, l)
#define MAKE_RETURN_STAT(e, l)			new ReturnStatement(e, l)
#define MAKE_If_STAT(e, t, elif, el, l)	new IfStatement(e, t, elif, el, l)
#define MAKE_WHILE_STAT(e, b, l)		new WhileStatement(e, b, l)
#define MAKE_FOR_STAT(i, e, p, b, l)	new ForStatement(i, e, p, b, l)
#define MAKE_SIMPLE_STAT(t, l)			new SimpleStatement<t>(l)

#define MAKE_FUNCTION_DEF(n, p, b, l)	FunctionDefiniton *f = new FunctionDefiniton(n, p, b, l); driver.AddFunction(f)

#define MAKE_IDENTIFIER_LIST(r, n)		r = new IdentifierList; r->push_back(n)
#define ADD_IDENTIFIER_LIST(r, o, n)	o->push_back(n); r = o;

#define MAKE_PARAMETER_LIST(r, n)		r = new ParameterList; r->push_back(n)
#define ADD_PARAMETER_LIST(r, o, n)		o->push_back(n); r = o;

#define MAKE_ELSEIF_LIST(r, n)			r = new ElseifList; r->push_back(n)
#define ADD_ELSEIF_LIST(r, o, n)		o->push_back(n); r = o;
#define MAKE_ELSEIF(r, n, m)			r = new Elseif(n, m);

#define MAKE_BLOCK(r, m)				r = new Block(m)

#define MAKE_ARGUMENT_LIST(r, n)		r = new ArgumentList; r->push_back(n)
#define ADD_ARGUMENT_LIST(r, o, n)		o->push_back(n); r = o;

#define MAKE_STATEMENT_LIST(r, n)		r = new StatementList; r->push_back(n)
#define ADD_STATEMENT_LIST(r, o, n)		o->push_back(n); r = o;

namespace LJ {

	enum ExpressionType {

		BOOLEAN_EXPRESSION = 1,
		INT_EXPRESSION,
		DOUBLE_EXPRESSION,
		STRING_EXPRESSION,
		IDENTIFIER_EXPRESSION,
		ASSIGN_EXPRESSION,
		ADD_EXPRESSION,
		SUB_EXPRESSION,
		MUL_EXPRESSION,
		DIV_EXPRESSION,
		MOD_EXPRESSION,
		EQ_EXPRESSION,
		NE_EXPRESSION,
		GT_EXPRESSION,
		GE_EXPRESSION,
		LT_EXPRESSION,
		LE_EXPRESSION,
		LOGICAL_AND_EXPRESSION,
		LOGICAL_OR_EXPRESSION,
		MINUS_EXPRESSION,
		EXCLAMATION_EXPRESSION,
		FUNCTION_CALL_EXPRESSION,
		TRUE_EXPRESSION,
		FALSE_EXPRESSION,
		NULL_EXPRESSION,
	};

	class Expression {
	public:
		Expression(location &l) : loc_(l){}
		virtual ~Expression() {}

		virtual ExpressionType GetType() const = 0;
		location& GetLocation() { return loc_; }
		void SetLocation(location &val) { loc_ = val; }

	private:
		location &loc_;
	};

	template<ExpressionType T, class U>
	class ValueExpression : public Expression {
	public:
		ValueExpression(U value, location &l) : Expression(l), value_(value) {}
		~ValueExpression() {}

		ExpressionType GetType() const override {
			return T;
		}
	private:
		U value_;
	};

	template<ExpressionType T>
	class NullExpression : public Expression
	{
	public:
		NullExpression(location l) : Expression(l) {}
		~NullExpression() {}

		ExpressionType GetType() const override {
			return T;
		}
	};

	template<ExpressionType T>
	class UnaryExpression : public Expression {
	public:
		UnaryExpression(Expression *e0, location &l) : Expression(l), e0_(e0) {}
		~UnaryExpression() {}

		ExpressionType GetType() const override {
			return T;
		}

	private:
		Expression *e0_;
	};

	template<ExpressionType T>
	class BinaryExpression : public Expression {
	public:
	public:
		BinaryExpression(Expression *e0, Expression *e1, location &l) : Expression(l), e0_(e0), e1_(e1) {}
		~BinaryExpression() {}

		ExpressionType GetType() const override {
			return T;
		}

	private:
		Expression *e0_;
		Expression *e1_;
	};

	typedef std::list<Expression *> ArgumentList;
	template<>
	class BinaryExpression<FUNCTION_CALL_EXPRESSION> : public Expression{
	public:
	public:
		BinaryExpression(const std::string &n0, ArgumentList *a1, location &l) : Expression(l), n0_(n0), a1_(a1) {}
		~BinaryExpression() {}

		ExpressionType GetType() const override {
			return FUNCTION_CALL_EXPRESSION;
		}

	private:
		std::string n0_;
		ArgumentList *a1_;
	};

	template<>
	class BinaryExpression<ASSIGN_EXPRESSION> : public Expression{
	public:
	public:
		BinaryExpression(const std::string &n0, Expression *e1, location &l) : Expression(l), n0_(n0), e1_(e1) {}
		~BinaryExpression() {}

		ExpressionType GetType() const override {
			return ASSIGN_EXPRESSION;
		}

	private:
		std::string n0_;
		Expression *e1_;
	};

	enum StatementType {
		EXPRESSION_STATEMENT = 1,
		GLOBAL_STATEMENT,
		IF_STATEMENT,
		WHILE_STATEMENT,
		FOR_STATEMENT,
		RETURN_STATEMENT,
		BREAK_STATEMENT,
		CONTINUE_STATEMENT,
	};

	class Statement {
	public:
		Statement(location &l) : loc_(l) {}
		virtual ~Statement() {}

		virtual StatementType GetType() const = 0;
		location& GetLocation() { return loc_; }
		void SetLocation(location &val) { loc_ = val; }

	private:
		location &loc_;
	};

	typedef std::list<Statement *> StatementList;
	typedef std::list<std::string> IdentifierList;

	class Block {
	public:
		Block(StatementList *s) : statement_list_(s){}
		~Block() {}
		StatementList& GetStatementList() { return *statement_list_; }

	private:
		StatementList *statement_list_;

	};

	class Elseif {
	public:
		Elseif(Expression *e, Block *b) : e_(e), b_(b){}
		~Elseif() {}

	private:
		Expression *e_;
		Block *b_;
	};
	typedef std::list<Elseif *> ElseifList;


	class ExpressionStatement : public Statement {
	public:
		ExpressionStatement(Expression *e, location &l) : Statement(l), e_(e) {}
		~ExpressionStatement() {}

		StatementType GetType() const override {
			return EXPRESSION_STATEMENT;
		}

	private:
		Expression *e_;
	};

	class ReturnStatement : public Statement {
	public:
		ReturnStatement(Expression *e, location &l) : Statement(l), e_(e) {}
		~ReturnStatement() {}

		StatementType GetType() const override {
			return RETURN_STATEMENT;
		}

	private:
		Expression *e_;
	};

	class GlobalStatement : public Statement {
	public:
		GlobalStatement(IdentifierList *id_list, location &l) : Statement(l), identifier_list_(id_list) {}
		~GlobalStatement() {}

		StatementType GetType() const override {
			return GLOBAL_STATEMENT;
		}

		IdentifierList& GetIdentifierList() { return *identifier_list_; }

	private:
		IdentifierList *identifier_list_;
	};

	class IfStatement : public Statement {
	public:
		IfStatement(Expression *e, Block *then_b, ElseifList *elseif_list, Block *else_b, location &l) :
			Statement(l), e_(e), then_b_(then_b), elseif_list_(elseif_list), else_b_(else_b) {}
		~IfStatement() {}

		StatementType GetType() const override {
			return IF_STATEMENT;
		}

	private:
		Expression *e_;
		Block *then_b_;
		ElseifList *elseif_list_;
		Block *else_b_;
	};

	class WhileStatement : public Statement {
	public:
		WhileStatement(Expression *e, Block *b, location &l) :
			Statement(l), e_(e), b_(b) {}
		~WhileStatement() {}

		StatementType GetType() const override {
			return WHILE_STATEMENT;
		}

	private:
		Expression *e_;
		Block *b_;
	};

	class ForStatement : public Statement {
	public:
		ForStatement(Expression *init_e, Expression *condition_e, Expression *post_e, Block *b, location &l) :
			Statement(l), init_e_(init_e), condition_e_(condition_e), post_e_(post_e), b_(b) {}
		~ForStatement() {}

		StatementType GetType() const override {
			return FOR_STATEMENT;
		}

	private:
		Expression *init_e_;
		Expression *condition_e_;
		Expression *post_e_;
		Block *b_;
	};

	template<StatementType T>
	class SimpleStatement : public Statement {
	public:
		SimpleStatement(location &l) : Statement(l) {}
		~SimpleStatement() {}

		StatementType GetType() const override {
			return T;
		}
	};

	typedef std::list<std::string> ParameterList;

	class FunctionDefiniton {
	public:
		FunctionDefiniton(const std::string &name, ParameterList *p, Block *b, location l) :
			name_(name), p_(p), b_(b), loc_(l) {}
		~FunctionDefiniton() {}
		location& GetLocation() { return loc_; }
		void SetLocation(location &val) { loc_ = val; }
	private:
		location loc_;
		ParameterList *p_;
		std::string name_;
		Block *b_;
	};
}




#endif
