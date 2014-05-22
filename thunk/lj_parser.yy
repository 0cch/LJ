%skeleton "lalr1.cc"
%require "3.0"
%defines
%define api.namespace {LJ}
%define parser_class_name {Parser}
%define api.value.type variant
%define parse.assert
%define api.token.constructor
%code requires
{
#include <string>

#include "lj_ast.h"

extern LJ::location loc;

class LJ_Driver;
}

// The parsing context.
%param { LJ_Driver& driver }

%locations
%initial-action
{
  // Initialize the initial location.
  @$.begin.filename = @$.end.filename = &driver.file_;
};

%define parse.trace
%define parse.error verbose

%code
{
# include "lj_driver.hpp"
}

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  LP				"(" 
  RP				")" 
  LC				"{" 
  RC				"}" 
  LB				"[" 
  RB				"]" 
  SEMICOLON			";" 
  COLON				":" 
  COMMA				"," 
  LOGICAL_AND		"&&"
  LOGICAL_OR		"||"
  ASSIGN			"=" 
  EQ				"=="
  NE				"!="
  GT				">" 
  GE				">="
  LT				"<" 
  LE				"<="
  ADD				"+" 
  SUB				"-" 
  MUL				"*" 
  DIV				"/" 
  MOD				"%" 
  BIT_AND			"&" 
  BIT_OR			"|" 
  BIT_XOR			"^" 
  BIT_NOT			"~" 
  ADD_ASSIGN		"+="
  SUB_ASSIGN		"-="
  MUL_ASSIGN		"*="
  DIV_ASSIGN		"/="
  MOD_ASSIGN		"%="
  INCREMENT			"++"
  DECREMENT			"--"
  EXCLAMATION		"!" 
  DOT				"." 
  IF				"if"      
  ELSE				"else"    
  ELSEIF			"elseif"  
  WHILE				"while"   
  DO				"do"      
  FOR				"for"     
  FOREACH			"foreach" 
  RETURN			"return"  
  BREAK				"break"   
  CONTINUE			"continue"
  NULL				"null"    
  TRUE				"true"    
  FALSE				"false"
  GLOBAL			"global"
  FUNCTION			"function"
;

%token <__int64>     INT_LITERAL
%token <double>     DOUBLE_LITERAL
%token <std::string>     STRING_LITERAL
%token <std::string>      IDENTIFIER

%type   <std::list<std::string> *> parameter_list
%type   <ArgumentList *> argument_list
%type   <Expression *> expression expression_opt
        logical_and_expression logical_or_expression
        equality_expression relational_expression
        additive_expression multiplicative_expression
        unary_expression primary_expression
%type   <Statement *> statement global_statement
        if_statement while_statement for_statement
        return_statement break_statement continue_statement
%type   <StatementList *> statement_list
%type   <Block *> block
%type	<Elseif *> elseif
%type   <ElseifList *> elseif_list
%type   <std::list<std::string> *> identifier_list


%printer { debug_stream () << $$; } <*>;

%%

%right "=";
%left "+" "-";
%left "*" "/" "%";

%start translation_unit;
translation_unit
        : definition_or_statement
        | translation_unit definition_or_statement
        ;
definition_or_statement
        : function_definition
        | statement
        {
			MAKE_STATEMENT_LIST(driver.statement_list_, $1);	
        }
        ;

function_definition
        : FUNCTION IDENTIFIER LP parameter_list RP block
        {
            MAKE_FUNCTION_DEF($2, $4, $6, loc);
        }
        | FUNCTION IDENTIFIER LP RP block
        {
            MAKE_FUNCTION_DEF($2, NULL, $5, loc);
        }
        ;
parameter_list
        : IDENTIFIER
        {
			MAKE_PARAMETER_LIST($$, $1);
        }
        | parameter_list COMMA IDENTIFIER
        {
            ADD_PARAMETER_LIST($$, $1, $3);
        }
        ;
argument_list
        : expression
        {
            MAKE_ARGUMENT_LIST($$, $1);
        }
        | argument_list COMMA expression
        {
			ADD_ARGUMENT_LIST($$, $1, $3);
        }
        ;
statement_list
        : statement
        {
			MAKE_STATEMENT_LIST($$, $1);
        }
        | statement_list statement
        {
            ADD_STATEMENT_LIST($$, $1, $2);
        }
        ;
expression
        : logical_or_expression {$$ = $1;}
        | IDENTIFIER ASSIGN expression
        {
            $$ = MAKE_BIN_EXP(ASSIGN_EXPRESSION, $1, $3, loc);
        }
        ;
logical_or_expression
        : logical_and_expression {$$ = $1;}
        | logical_or_expression LOGICAL_OR logical_and_expression
        {
            $$ = MAKE_BIN_EXP(LOGICAL_OR_EXPRESSION, $1, $3, loc);
        }
        ;
logical_and_expression
        : equality_expression {$$ = $1;}
        | logical_and_expression LOGICAL_AND equality_expression
        {
            $$ = MAKE_BIN_EXP(LOGICAL_AND_EXPRESSION, $1, $3, loc);
        }
        ;
equality_expression
        : relational_expression {$$ = $1;}
        | equality_expression EQ relational_expression
        {
            $$ = MAKE_BIN_EXP(EQ_EXPRESSION, $1, $3, loc);
        }
        | equality_expression NE relational_expression
        {
            $$ = MAKE_BIN_EXP(NE_EXPRESSION, $1, $3, loc);
        }
        ;
relational_expression
        : additive_expression {$$ = $1;}
        | relational_expression GT additive_expression
        {
            $$ = MAKE_BIN_EXP(GT_EXPRESSION, $1, $3, loc);
        }
        | relational_expression GE additive_expression
        {
            $$ = MAKE_BIN_EXP(GE_EXPRESSION, $1, $3, loc);
        }
        | relational_expression LT additive_expression
        {
            $$ = MAKE_BIN_EXP(LT_EXPRESSION, $1, $3, loc);
        }
        | relational_expression LE additive_expression
        {
            $$ = MAKE_BIN_EXP(LE_EXPRESSION, $1, $3, loc);
        }
        ;
additive_expression
        : multiplicative_expression {$$ = $1;}
        | additive_expression ADD multiplicative_expression
        {
            $$ = MAKE_BIN_EXP(ADD_EXPRESSION, $1, $3, loc);
        }
        | additive_expression SUB multiplicative_expression
        {
            $$ = MAKE_BIN_EXP(SUB_EXPRESSION, $1, $3, loc);
        }
        ;
multiplicative_expression
        : unary_expression {$$ = $1;}
        | multiplicative_expression MUL unary_expression
        {
            $$ = MAKE_BIN_EXP(MUL_EXPRESSION, $1, $3, loc);
        }
        | multiplicative_expression DIV unary_expression
        {
            $$ = MAKE_BIN_EXP(DIV_EXPRESSION, $1, $3, loc);
        }
        | multiplicative_expression MOD unary_expression
        {
            $$ = MAKE_BIN_EXP(MOD_EXPRESSION, $1, $3, loc);
        }
        ;
unary_expression
        : primary_expression {$$ = $1;}
		| EXCLAMATION unary_expression
		{
			$$ = MAKE_UNARY_EXP(EXCLAMATION_EXPRESSION, $2, loc);
		}
        | SUB unary_expression
        {
            $$ = MAKE_UNARY_EXP(MINUS_EXPRESSION, $2, loc);
        }
        ;
primary_expression
        : IDENTIFIER LP argument_list RP
        {
            $$ = MAKE_BIN_EXP(FUNCTION_CALL_EXPRESSION, $1, $3, loc);
        }
        | IDENTIFIER LP RP
        {
            $$ = MAKE_BIN_EXP(FUNCTION_CALL_EXPRESSION, $1, NULL, loc);
        }
        | LP expression RP
        {
            $$ = $2;
        }
        | IDENTIFIER
        {
            $$ = MAKE_VALUE_EXP(IDENTIFIER_EXPRESSION, std::string, $1, loc);
        }
		| INT_LITERAL
		{
			$$ = MAKE_VALUE_EXP(INT_EXPRESSION, __int64, $1, loc);
		}
        | DOUBLE_LITERAL
		{
			$$ = MAKE_VALUE_EXP(DOUBLE_EXPRESSION, double, $1, loc);
		}
        | STRING_LITERAL
		{
			$$ = MAKE_VALUE_EXP(STRING_EXPRESSION, std::string, $1, loc);
		}
        | TRUE
        {
            $$ = MAKE_EXP(TRUE_EXPRESSION, loc);
        }
        | FALSE
        {
            $$ = MAKE_EXP(FALSE_EXPRESSION, loc);
        }
        | NULL
        {
            $$ = MAKE_EXP(NULL_EXPRESSION, loc);
        }
        ;
statement
        : expression SEMICOLON
        {
          $$ = MAKE_EXP_STAT($1, loc);
        }
		| global_statement
        | if_statement
        | while_statement
        | for_statement
        | return_statement
        | break_statement
        | continue_statement
        ;
global_statement
        : GLOBAL identifier_list SEMICOLON
        {
            $$ = MAKE_GLOBAL_STAT($2, loc);
        }
        ;

identifier_list
        : IDENTIFIER
        {
			MAKE_IDENTIFIER_LIST($$, $1);
        }
        | identifier_list COMMA IDENTIFIER
        {
            ADD_IDENTIFIER_LIST($$, $1, $3);
        }
        ;
if_statement
        : IF LP expression RP block
        {
            $$ = MAKE_If_STAT($3, $5, NULL, NULL, loc);
        }
        | IF LP expression RP block ELSE block
        {
            $$ = MAKE_If_STAT($3, $5, NULL, $7, loc);
        }
        | IF LP expression RP block elseif_list
        {
            $$ = MAKE_If_STAT($3, $5, $6, NULL, loc);
        }
        | IF LP expression RP block elseif_list ELSE block
        {
            $$ = MAKE_If_STAT($3, $5, $6, $8, loc);
        }
        ;
elseif_list
        : elseif 
		{
			MAKE_ELSEIF_LIST($$, $1);
		}
        | elseif_list elseif
        {
			ADD_ELSEIF_LIST($$, $1, $2);
        }
        ;
elseif
        : ELSEIF LP expression RP block
        {
            MAKE_ELSEIF($$, $3, $5);
        }
        ;
while_statement
        : WHILE LP expression RP block
        {
            $$ = MAKE_WHILE_STAT($3, $5, loc);
        }
        ;
for_statement
        : FOR LP expression_opt SEMICOLON expression_opt SEMICOLON
          expression_opt RP block
        {
            $$ = MAKE_FOR_STAT($3, $5, $7, $9, loc);
        }
        ;
expression_opt
        : /* empty */
        {
            $$ = NULL;
        }
        | expression {$$ = $1;}
        ;
return_statement
        : RETURN expression_opt SEMICOLON
        {
            $$ = MAKE_RETURN_STAT($2, loc);
        }
        ;
break_statement
        : BREAK SEMICOLON
        {
            $$ = MAKE_SIMPLE_STAT(BREAK_STATEMENT, loc);
        }
        ;
continue_statement
        : CONTINUE SEMICOLON
        {
            $$ = MAKE_SIMPLE_STAT(CONTINUE_STATEMENT, loc);
        }
        ;
block
        : LC statement_list RC
        {
			MAKE_BLOCK($$, $2);
        }
        | LC RC
        {
            MAKE_BLOCK($$, NULL);
        }
        ;
%%


void
LJ::Parser::error(const location_type& l, const std::string& m)
{
	driver.Error(l, m);
}
