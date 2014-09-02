#ifndef __LJ_AST_H__
#define __LJ_AST_H__

#include <list>
#include <set>
#include <string>
#include <iostream>
#include "location.hh"

typedef unsigned char boolean;

#define MAKE_VALUE_EXP(t, u, v, l)		new ValueExpression<t, u>(v, l)
#define MAKE_EXP(t, l)					new NullExpression<t>(l)
#define MAKE_UNARY_EXP(t, e, l)			new UnaryExpression<t>(e, l)
#define MAKE_BIN_EXP(t, e0, e1, l)		new BinaryExpression<t>(e0, e1, l)

#define MAKE_EXP_STAT(e, l)				new ExpressionStatement(e, l)
#define MAKE_GLOBAL_STAT(e, l)			new GlobalStatement(e, l)
#define MAKE_RETURN_STAT(e, l)			new ReturnStatement(e, l)
#define MAKE_If_STAT(e, t, elif, el, l)	new IfStatement(e, t, elif, el, l)
#define MAKE_WHILE_STAT(e, b, l)		new WhileStatement(e, b, l)
#define MAKE_FOR_STAT(i, e, p, b, l)	new ForStatement(i, e, p, b, l)
#define MAKE_SIMPLE_STAT(t, l)			new SimpleStatement<t>(l)

#define MAKE_FUNCTION_DEF(n, p, b, l)	FunctionDefinition *f = new FunctionDefinition(n, p, b, l); driver.AddFunction(f)

#define MAKE_IDENTIFIER_LIST(r, n)		r = new IdentifierList; r->push_back(n)
#define ADD_IDENTIFIER_LIST(r, o, n)	o->push_back(n); r = o;

#define MAKE_PARAMETER_LIST(r, n)		r = new ParameterList; r->push_back(n)
#define ADD_PARAMETER_LIST(r, o, n)		o->push_back(n); r = o;

#define MAKE_ELSEIF_LIST(r, n)			r = new ElseifList; r->push_back(n)
#define ADD_ELSEIF_LIST(r, o, n)		o->push_back(n); r = o;
#define MAKE_ELSEIF(r, n, m, l)			r = new Elseif(n, m, l);

#define MAKE_BLOCK(r, m, l)				r = new Block(m, l)

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

	char *GetExpressionTypeString(int type);

	class Expression {
	public:
		Expression(const location &l) : loc_(l){}
		virtual ~Expression() {}

		virtual ExpressionType GetType() const = 0;
		virtual void Dump(int indent) const = 0;
		virtual void Eval(LJ_Driver *driver) const = 0;

		location& GetLocation() { return loc_; }
		void SetLocation(location &val) { loc_ = val; }

		void PrintIndent(int indent) const
		{
			for (int i = 0; i < indent; i++) {
				std::cout << "  ";
			}
		}

	private:
		location loc_;
	};

	template<ExpressionType T, class U>
	class ValueExpression : public Expression {
	public:
		ValueExpression(U value, const location &l) : Expression(l), value_(value) {}
		~ValueExpression() {}

		ExpressionType GetType() const override {
			return T;
		}

		void Dump(int indent) const override {
			PrintIndent(indent);
			std::cout << GetExpressionTypeString(GetType()) << " = [" << value_ << "]" << std::endl;
		}

		U GetValue() { return value_; }

	protected:
		U value_;
	};

	class BooleanExpression : public ValueExpression < BOOLEAN_EXPRESSION, boolean > {
	public:
		BooleanExpression(boolean value, const location &l) : ValueExpression(value, l) {}
		~BooleanExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class IntExpression : public ValueExpression < INT_EXPRESSION, __int64 > {
	public:
		IntExpression(__int64 value, const location &l) : ValueExpression(value, l) {}
		~IntExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class DoubleExpression : public ValueExpression < DOUBLE_EXPRESSION, double > {
	public:
		DoubleExpression(double value, const location &l) : ValueExpression(value, l) {}
		~DoubleExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class StringExpression : public ValueExpression < STRING_EXPRESSION, std::string > {
	public:
		StringExpression(std::string &value, const location &l) : ValueExpression(value, l) {}
		~StringExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class IdentifierExpression : public ValueExpression < IDENTIFIER_EXPRESSION, std::string > {
	public:
		IdentifierExpression(std::string &value, const location &l) : ValueExpression(value, l) {}
		~IdentifierExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	template<ExpressionType T>
	class EmptyExpression : public Expression {
	public:
		EmptyExpression(const location &l) : Expression(l) {}
		~EmptyExpression() {}

		ExpressionType GetType() const override {
			return T;
		}

		void Dump(int indent) const override {
			PrintIndent(indent);
			std::cout << GetExpressionTypeString(GetType()) << std::endl;
		}
	};

	class NullExpression : public EmptyExpression < NULL_EXPRESSION > {
	public:
		NullExpression(const location &l) : EmptyExpression(l) {}
		~NullExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class TrueExpression : public EmptyExpression < TRUE_EXPRESSION > {
	public:
		TrueExpression(const location &l) : EmptyExpression(l) {}
		~TrueExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class FalseExpression : public EmptyExpression < FALSE_EXPRESSION > {
	public:
		FalseExpression(const location &l) : EmptyExpression(l) {}
		~FalseExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	template<ExpressionType T>
	class UnaryExpression : public Expression {
	public:
		UnaryExpression(Expression *expr, const location &l) : Expression(l), expr_(expr) {}
		~UnaryExpression() { delete expr_; }

		ExpressionType GetType() const override {
			return T;
		}

		void Dump(int indent) const override {
			PrintIndent(indent++);
			std::cout << GetExpressionTypeString(GetType()) << std::endl;
			expr_->Dump(indent);
		}

		Expression* GetExpression() override {
			return expr_;
		}

	protected:
		Expression *expr_;
	};

	class MinusExpression : public UnaryExpression < MINUS_EXPRESSION > {
	public:
		MinusExpression(Expression *expr, const location &l) : UnaryExpression(expr, l) {}
		~MinusExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class ExclamationExpression : public UnaryExpression < EXCLAMATION_EXPRESSION > {
	public:
		ExclamationExpression(Expression *expr, const location &l) : UnaryExpression(expr, l) {}
		~ExclamationExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	template<class T>
	void DeleteElems(T &e)
	{
		for (T::iterator it = e.begin(); it != e.end(); ++it) {
			delete *it;
		}
	}

	template<ExpressionType T>
	class BinaryExpression : public Expression {
	public:
	public:
		BinaryExpression(Expression *left, Expression *right, const location &l) : Expression(l), left_(left), right_(right) {}
		~BinaryExpression() {
			delete left_;
			delete right_;
		}

		ExpressionType GetType() const override {
			return T;
		}

		void Dump(int indent) const override {
			PrintIndent(indent++);
			std::cout << GetExpressionTypeString(GetType()) << std::endl;
			left_->Dump(indent);
			right_->Dump(indent);
		}

	protected:
		Expression *left_;
		Expression *right_;
	};

	class AssignExpression : public BinaryExpression < ASSIGN_EXPRESSION > {
	public:
		AssignExpression(Expression *left, Expression *right, const location &l) : BinaryExpression(left, right, l) {}
		~AssignExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class AddExpression : public BinaryExpression < ADD_EXPRESSION > {
	public:
		AddExpression(Expression *left, Expression *right, const location &l) : BinaryExpression(left, right, l) {}
		~AddExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class SubExpression : public BinaryExpression < SUB_EXPRESSION > {
	public:
		SubExpression(Expression *left, Expression *right, const location &l) : BinaryExpression(left, right, l) {}
		~SubExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class MulExpression : public BinaryExpression < MUL_EXPRESSION > {
	public:
		MulExpression(Expression *left, Expression *right, const location &l) : BinaryExpression(left, right, l) {}
		~MulExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class DivExpression : public BinaryExpression < DIV_EXPRESSION > {
	public:
		DivExpression(Expression *left, Expression *right, const location &l) : BinaryExpression(left, right, l) {}
		~DivExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class ModExpression : public BinaryExpression < MOD_EXPRESSION > {
	public:
		ModExpression(Expression *left, Expression *right, const location &l) : BinaryExpression(left, right, l) {}
		~ModExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class EQExpression : public BinaryExpression < EQ_EXPRESSION > {
	public:
		EQExpression(Expression *left, Expression *right, const location &l) : BinaryExpression(left, right, l) {}
		~EQExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class NEExpression : public BinaryExpression < NE_EXPRESSION > {
	public:
		NEExpression(Expression *left, Expression *right, const location &l) : BinaryExpression(left, right, l) {}
		~NEExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class GTExpression : public BinaryExpression < GT_EXPRESSION > {
	public:
		GTExpression(Expression *left, Expression *right, const location &l) : BinaryExpression(left, right, l) {}
		~GTExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class GEExpression : public BinaryExpression < GE_EXPRESSION > {
	public:
		GEExpression(Expression *left, Expression *right, const location &l) : BinaryExpression(left, right, l) {}
		~GEExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class LTExpression : public BinaryExpression < LT_EXPRESSION > {
	public:
		LTExpression(Expression *left, Expression *right, const location &l) : BinaryExpression(left, right, l) {}
		~LTExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class LEExpression : public BinaryExpression < LE_EXPRESSION > {
	public:
		LEExpression(Expression *left, Expression *right, const location &l) : BinaryExpression(left, right, l) {}
		~LEExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class LogicalAndExpression : public BinaryExpression < LOGICAL_AND_EXPRESSION > {
	public:
		LogicalAndExpression(Expression *left, Expression *right, const location &l) : BinaryExpression(left, right, l) {}
		~LogicalAndExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	class LogicalOrExpression : public BinaryExpression < LOGICAL_OR_EXPRESSION > {
	public:
		LogicalOrExpression(Expression *left, Expression *right, const location &l) : BinaryExpression(left, right, l) {}
		~LogicalOrExpression() {}

		void Eval(LJ_Driver *driver) const {}
	};

	typedef std::list<Expression *> ArgumentList;
	template<>
	class BinaryExpression<FUNCTION_CALL_EXPRESSION> : public Expression{
	public:
	public:
		BinaryExpression(const std::string &n0, ArgumentList *a1, const location &l) : Expression(l), n0_(n0), a1_(a1) {}
		~BinaryExpression() {
			DeleteElems(*a1_);
			delete a1_;
		}

		ExpressionType GetType() const override {
			return FUNCTION_CALL_EXPRESSION;
		}

		void Dump(int indent) const override {
			PrintIndent(indent++);
			std::cout << GetExpressionTypeString(GetType()) << " = [" << n0_ << "]" << std::endl;
			for (ArgumentList::iterator it = a1_->begin(); it != a1_->end(); ++it) {
				(*it)->Dump(indent);
			}
		}

		void* GetValue(int index) override {
			if (index == 0) {
				return &n0_;
			}
			else {
				return a1_;
			}
		}

		std::string& GetFunctionName() { return n0_; }
		ArgumentList * GetArgList() { return a1_; }

	private:
		std::string n0_;
		ArgumentList *a1_;
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

	char *GetStatementTypeString(int type);

	class Statement {
	public:
		Statement(const location &l) : loc_(l) {}
		virtual ~Statement() {}

		virtual StatementType GetType() const = 0;
		virtual void Dump(int indent) const = 0;
		location& GetLocation() { return loc_; }
		void SetLocation(location &val) { loc_ = val; }

		void PrintIndent(int indent) const
		{
			for (int i = 0; i < indent; i++) {
				std::cout << "  ";
			}
		}

		virtual void *GetValue(int index) = 0;

	private:
		location loc_;
	};

	typedef std::list<Statement *> StatementList;
	typedef std::list<std::string> IdentifierList;

	class Block {
	public:
		Block(StatementList *s, const location &l) : statement_list_(s), loc_(l) {}
		~Block() {
			DeleteElems(*statement_list_);
			delete statement_list_;
		}

		void Dump(int indent) const {
			std::cout << "BLOCK" << std::endl;
			for (StatementList::iterator it = statement_list_->begin();
				it != statement_list_->end(); ++it) {
				(*it)->Dump(indent);
			}
		}

		void *GetValue(int index) { return statement_list_; }

	private:
		StatementList *statement_list_;
		location loc_;

	};

	class Elseif {
	public:
		Elseif(Expression *e, Block *b, const location &l) : e_(e), b_(b), loc_(l){}
		~Elseif() {
			delete b_;
		}

		location& GetLocation() { return loc_; }
		void SetLocation(location &val) { loc_ = val; }

		void Dump(int indent) const {
			std::cout << "ELSEIF" << std::endl;
			e_->Dump(indent);
			b_->Dump(indent);
		}

		void *GetValue(int index) {
			if (index == 0) {
				return e_;
			}
			else {
				return b_;
			}

		}

	private:
		Expression *e_;
		Block *b_;
		location loc_;
	};
	typedef std::list<Elseif *> ElseifList;


	class ExpressionStatement : public Statement {
	public:
		ExpressionStatement(Expression *e, const location &l) : Statement(l), e_(e) {}
		~ExpressionStatement() {
			delete e_;
		}

		StatementType GetType() const override {
			return EXPRESSION_STATEMENT;
		}

		void Dump(int indent) const override {
			PrintIndent(indent++);
			std::cout << GetStatementTypeString(GetType()) << std::endl;
			e_->Dump(indent);
		}

		void *GetValue(int index) override { return e_; }

	private:
		Expression *e_;
	};

	class ReturnStatement : public Statement {
	public:
		ReturnStatement(Expression *e, const location &l) : Statement(l), e_(e) {}
		~ReturnStatement() {
			delete e_;
		}

		StatementType GetType() const override {
			return RETURN_STATEMENT;
		}

		void Dump(int indent) const override {
			PrintIndent(indent++);
			std::cout << GetStatementTypeString(GetType()) << std::endl;
			e_->Dump(indent);
		}

		void *GetValue(int index) override { return e_; }


	private:
		Expression *e_;
	};

	class GlobalStatement : public Statement {
	public:
		GlobalStatement(IdentifierList *id_list, const location &l) : Statement(l), identifier_list_(id_list) {}
		~GlobalStatement() {
			delete identifier_list_;
		}

		StatementType GetType() const override {
			return GLOBAL_STATEMENT;
		}

		IdentifierList& GetIdentifierList() { return *identifier_list_; }

		void Dump(int indent) const override {
			PrintIndent(indent++);
			std::cout << GetStatementTypeString(GetType()) << std::endl;
			PrintIndent(indent++);
			std::cout << "[";
			for (IdentifierList::iterator it = identifier_list_->begin();
				it != identifier_list_->end(); ++it) {
				std::cout << *it << ", ";
			}
			std::cout << "]";
		}

		void *GetValue(int index) override { return identifier_list_; }

	private:
		IdentifierList *identifier_list_;
	};

	class IfStatement : public Statement {
	public:
		IfStatement(Expression *e, Block *then_b, ElseifList *elseif_list, Block *else_b, const location &l) :
			Statement(l), e_(e), then_b_(then_b), elseif_list_(elseif_list), else_b_(else_b) {}
		~IfStatement() {
			delete e_;
			delete then_b_;
			DeleteElems(*elseif_list_);
			delete elseif_list_;
			delete else_b_;
		}

		StatementType GetType() const override {
			return IF_STATEMENT;
		}

		void Dump(int indent) const override {
			PrintIndent(indent++);
			std::cout << GetStatementTypeString(GetType()) << std::endl;
			e_->Dump(indent);
			if (then_b_ != NULL) {
				PrintIndent(indent);
				then_b_->Dump(indent);
			}

			if (elseif_list_ != NULL) {
				for (ElseifList::iterator it = elseif_list_->begin();
					it != elseif_list_->end(); ++it) {
					PrintIndent(indent);
					(*it)->Dump(indent);
				}
			}

			if (else_b_ != NULL) {
				PrintIndent(indent);
				else_b_->Dump(indent);
			}

		}

		void *GetValue(int index) override {
			if (index == 0) {
				return e_;
			}
			else if (index == 1) {
				return then_b_;
			}
			else if (index == 2) {
				return elseif_list_;
			}
			else {
				return else_b_;
			}
		}

	private:
		Expression *e_;
		Block *then_b_;
		ElseifList *elseif_list_;
		Block *else_b_;
	};

	class WhileStatement : public Statement {
	public:
		WhileStatement(Expression *e, Block *b, const location &l) :
			Statement(l), e_(e), b_(b) {}
		~WhileStatement() {
			delete e_;
			delete b_;
		}

		StatementType GetType() const override {
			return WHILE_STATEMENT;
		}

		void Dump(int indent) const override {
			PrintIndent(indent++);
			std::cout << GetStatementTypeString(GetType()) << std::endl;
			e_->Dump(indent);
			if (b_ != NULL) {
				PrintIndent(indent);
				b_->Dump(indent);
			}
		}

		void *GetValue(int index) override {
			if (index == 0) {
				return e_;
			}
			else {
				return b_;
			}
		}

	private:
		Expression *e_;
		Block *b_;
	};

	class ForStatement : public Statement {
	public:
		ForStatement(Expression *init_e, Expression *condition_e, Expression *post_e, Block *b, const location &l) :
			Statement(l), init_e_(init_e), condition_e_(condition_e), post_e_(post_e), b_(b) {}
		~ForStatement() {
			delete init_e_;
			delete condition_e_;
			delete post_e_;
			delete b_;
		}

		StatementType GetType() const override {
			return FOR_STATEMENT;
		}

		void Dump(int indent) const override {
			PrintIndent(indent++);
			std::cout << GetStatementTypeString(GetType()) << std::endl;
			if (init_e_ != NULL) {
				init_e_->Dump(indent);
			}

			if (condition_e_ != NULL) {
				condition_e_->Dump(indent);
			}

			if (post_e_ != NULL) {
				post_e_->Dump(indent);
			}

			if (b_ != NULL) {
				PrintIndent(indent);
				b_->Dump(indent);
			}
		}

		void *GetValue(int index) override {
			if (index == 0) {
				return init_e_;
			}
			else if (index == 1) {
				return condition_e_;
			}
			else if (index == 2) {
				return post_e_;
			}
			else {
				return b_;
			}
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
		SimpleStatement(const location &l) : Statement(l) {}
		~SimpleStatement() {}

		StatementType GetType() const override {
			return T;
		}

		void Dump(int indent) const override {
			PrintIndent(indent);
			std::cout << GetStatementTypeString(GetType()) << std::endl;
		}

		void *GetValue(int index) override { return NULL; }
	};

	typedef std::list<std::string> ParameterList;

	enum FunctionType {
		FUNCTION_DEFINITION = 1
	};

	class FunctionDefinition {
	public:
		FunctionDefinition(const std::string &name, ParameterList *p, Block *b, const location &l) :
			name_(name), p_(p), b_(b), loc_(l) {}
		~FunctionDefinition() {
			delete p_;
			delete b_;
		}
		location& GetLocation() { return loc_; }
		void SetLocation(location &val) { loc_ = val; }
		FunctionType GetType() {
			return FUNCTION_DEFINITION;
		}

		std::string& GetFunctionName() { return name_; }
		ParameterList * GetParamList() { return p_; }
		Block * GetBlock() { return b_; }

	private:
		location loc_;
		ParameterList *p_;
		std::string name_;
		Block *b_;
	};
}




#endif
