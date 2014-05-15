%skeleton "lalr1.cc"
%require "3.0"
%defines
%define api.namespace {LJ}
%define parser_class_name {LJ_Parser}
%define api.value.type variant
%define parse.assert
%define api.token.constructor
%code requires
{
#include <string>

#pragma warning(disable:4146 4996 4065)

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
  ASSIGN  ":="
  MINUS   "-"
  PLUS    "+"
  STAR    "*"
  SLASH   "/"
  LPAREN  "("
  RPAREN  ")"
;

%token <std::string> IDENTIFIER "identifier"
%token <int> NUMBER "number"
%type  <int> exp

%printer { debug_stream () << $$; } <*>;

%%
%start unit;
unit: assignments exp  { driver.result_ = $2; };

assignments:
  assignments assignment {}
| /* Nothing.  */        {};

assignment:
  "identifier" ":=" exp { driver.variables[$1] = $3; };

%left "+" "-";
%left "*" "/";
exp:
  exp "+" exp   { $$ = $1 + $3; }
| exp "-" exp   { $$ = $1 - $3; }
| exp "*" exp   { $$ = $1 * $3; }
| exp "/" exp   { $$ = $1 / $3; }
| "(" exp ")"   { std::swap ($$, $2); }
| "identifier"  { $$ = driver.variables[$1]; }
| "number"      { std::swap ($$, $1); };
%%

void
LJ::LJ_Parser::error(const location_type& l, const std::string& m)
{
	driver.Error(l, m);
}
