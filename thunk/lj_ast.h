#ifndef __LJ_AST_H__
#define __LJ_AST_H__

#include <list>
#include <set>
#include <string>
#include <iostream>
#include "location.hh"

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

	char *GetExpressionTypeString(int type);

	class Expression {
	public:
		Expression(const location &l) : loc_(l){}
		virtual ~Expression() {}

		virtual ExpressionType GetType() const = 0;
		virtual void Dump(int indent) const = 0;
		virtual void* GetValue(int index) = 0;

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

		void* GetValue(int index) override {
			return &value_;
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

		void Dump(int indent) const override {
			PrintIndent(indent);
			std::cout << GetExpressionTypeString(GetType()) << std::endl;
		}

		void* GetValue(int index) override {
			return NULL;
		}
	};

	template<ExpressionType T>
	class UnaryExpression : public Expression {
	public:
		UnaryExpression(Expression *e0, const location &l) : Expression(l), e0_(e0) {}
		~UnaryExpression() { delete e0_; }

		ExpressionType GetType() const override {
			return T;
		}

		void Dump(int indent) const override {
			PrintIndent(indent++);
			std::cout << GetExpressionTypeString(GetType()) << std::endl;
			e0_->Dump(indent);
		}

		void* GetValue(int index) override {
			return e0_;
		}

	private:
		Expression *e0_;
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
		BinaryExpression(Expression *e0, Expression *e1, const location &l) : Expression(l), e0_(e0), e1_(e1) {}
		~BinaryExpression() {
			delete e0_;
			delete e1_;
		}

		ExpressionType GetType() const override {
			return T;
		}

		void Dump(int indent) const override {
			PrintIndent(indent++);
			std::cout << GetExpressionTypeString(GetType()) << std::endl;
			e0_->Dump(indent);
			e1_->Dump(indent);
		}

		void* GetValue(int index) override {
			if (index == 0) {
				return e0_;
			}
			else {
				return e1_;
			}
		}

	private:
		Expression *e0_;
		Expression *e1_;
	};

	typedef std::list<Expression *> ArgumentList;
	template<>
	class BinaryExpression<FUNCTION_CALL_EXPRESSION> : public Expression {
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

		std::string& GetFunctionName() {return n0_;}
		ArgumentList * GetArgList() {return a1_;}

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

	private:
		location loc_;
	};

	typedef std::list<Statement *> StatementList;
	typedef std::list<std::string> IdentifierList;

	class Block {
	public:
		Block(StatementList *s) : statement_list_(s){}
		~Block() {
			DeleteElems(*statement_list_);
			delete statement_list_;
		}
		StatementList& GetStatementList() { return *statement_list_; }

		void Dump(int indent) const {
			std::cout << "BLOCK" << std::endl;
			for (StatementList::iterator it = statement_list_->begin();
				it != statement_list_->end(); ++it) {
				(*it)->Dump(indent);
			}
		}

	private:
		StatementList *statement_list_;

	};

	class Elseif {
	public:
		Elseif(Expression *e, Block *b) : e_(e), b_(b){}
		~Elseif() {
			delete b_;
		}

		void Dump(int indent) const {
			std::cout << "ELSEIF" << std::endl;
			e_->Dump(indent);
			b_->Dump(indent);
		}

	private:
		Expression *e_;
		Block *b_;
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

		std::string& GetFunctionName() {return name_;}
		ParameterList * GetParamList() {return p_;}
		Block * GetBlock() {return b_;}

	private:
		location loc_;
		ParameterList *p_;
		std::string name_;
		Block *b_;
	};
}




#endif
