%{                                            /* -*- C++ -*- */
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <string>
#include <io.h>

#include "lj_driver.hpp"
#include "lj_parser.hpp"

#define YY_NO_UNISTD_H

#undef yywrap
#define yywrap() 1

// The location of the current token.
static LJ::location loc;
%}

%option noyywrap nounput batch debug

id    [a-zA-Z][a-zA-Z_0-9]*
int   [0-9]+
blank [ \t]

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns(yyleng);
%}
%%
%{
  // Code run each time yylex is called.
  loc.step();
%}
{blank}+   loc.step();
[\n]+      loc.lines (yyleng); loc.step();

"-"      return LJ::LJ_Parser::make_MINUS(loc);
"+"      return LJ::LJ_Parser::make_PLUS(loc);
"*"      return LJ::LJ_Parser::make_STAR(loc);
"/"      return LJ::LJ_Parser::make_SLASH(loc);
"("      return LJ::LJ_Parser::make_LPAREN(loc);
")"      return LJ::LJ_Parser::make_RPAREN(loc);
":="     return LJ::LJ_Parser::make_ASSIGN(loc);

{int}	{
	errno = 0;
	long n = strtol(yytext, NULL, 10);
	if (!(INT_MIN <= n && n <= INT_MAX && errno != ERANGE)) {
		driver.Error(loc, "integer is out of range");
	}
	return LJ::LJ_Parser::make_NUMBER(n, loc);
}
{id}       return LJ::LJ_Parser::make_IDENTIFIER(yytext, loc);
.          driver.Error(loc, "invalid character");
<<EOF>>    return LJ::LJ_Parser::make_END(loc);
%%

void LJ_Driver::ScanBegin()
{
	yy_flex_debug = trace_scanning_;
	if (file_ == "-")
		yyin = stdin;
	else if (!(yyin = fopen(file_.c_str (), "r")))
	{
		Error(std::string ("cannot open ") + file_ + ": " + strerror(errno));
		exit(1);
	}
}

void LJ_Driver::ScanEnd()
{
	fclose(yyin);
}
