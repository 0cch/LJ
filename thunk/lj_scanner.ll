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
LJ::location loc;
%}

%option noyywrap nounput batch debug

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns(yyleng);
%}
%%
%{
  // Code run each time yylex is called.
  loc.step();
%}
[ \t]+		loc.step();
[\n]+		loc.lines (yyleng); loc.step();

<INITIAL>"if"           return LJ::LJ_Parser::make_IF(loc);
<INITIAL>"else"         return LJ::LJ_Parser::make_ELSE(loc);
<INITIAL>"elseif"       return LJ::LJ_Parser::make_ELSEIF(loc);
<INITIAL>"while"        return LJ::LJ_Parser::make_WHILE(loc);
<INITIAL>"do"           return LJ::LJ_Parser::make_DO(loc);
<INITIAL>"for"          return LJ::LJ_Parser::make_FOR(loc);
<INITIAL>"foreach"      return LJ::LJ_Parser::make_FOREACH(loc);
<INITIAL>"return"       return LJ::LJ_Parser::make_RETURN(loc);
<INITIAL>"break"        return LJ::LJ_Parser::make_BREAK(loc);
<INITIAL>"continue"     return LJ::LJ_Parser::make_CONTINUE(loc);
<INITIAL>"null"         return LJ::LJ_Parser::make_NULL(loc);
<INITIAL>"true"         return LJ::LJ_Parser::make_TRUE(loc);
<INITIAL>"false"        return LJ::LJ_Parser::make_FALSE(loc);
<INITIAL>"global"       return LJ::LJ_Parser::make_GLOBAL(loc);
<INITIAL>"function"     return LJ::LJ_Parser::make_FUNCTION(loc);
<INITIAL>"("            return LJ::LJ_Parser::make_LP(loc);
<INITIAL>")"            return LJ::LJ_Parser::make_RP(loc);
<INITIAL>"{"            return LJ::LJ_Parser::make_LC(loc);
<INITIAL>"}"            return LJ::LJ_Parser::make_RC(loc);
<INITIAL>"["            return LJ::LJ_Parser::make_LB(loc);
<INITIAL>"]"            return LJ::LJ_Parser::make_RB(loc);
<INITIAL>";"            return LJ::LJ_Parser::make_SEMICOLON(loc);
<INITIAL>":"            return LJ::LJ_Parser::make_COLON(loc);
<INITIAL>","            return LJ::LJ_Parser::make_COMMA(loc);
<INITIAL>"&&"           return LJ::LJ_Parser::make_LOGICAL_AND(loc);
<INITIAL>"||"           return LJ::LJ_Parser::make_LOGICAL_OR(loc);
<INITIAL>"="            return LJ::LJ_Parser::make_ASSIGN(loc);
<INITIAL>"=="           return LJ::LJ_Parser::make_EQ(loc);
<INITIAL>"!="           return LJ::LJ_Parser::make_NE(loc);
<INITIAL>">"            return LJ::LJ_Parser::make_GT(loc);
<INITIAL>">="           return LJ::LJ_Parser::make_GE(loc);
<INITIAL>"<"            return LJ::LJ_Parser::make_LT(loc);
<INITIAL>"<="           return LJ::LJ_Parser::make_LE(loc);
<INITIAL>"+"            return LJ::LJ_Parser::make_ADD(loc);
<INITIAL>"-"            return LJ::LJ_Parser::make_SUB(loc);
<INITIAL>"*"            return LJ::LJ_Parser::make_MUL(loc);
<INITIAL>"/"            return LJ::LJ_Parser::make_DIV(loc);
<INITIAL>"%"            return LJ::LJ_Parser::make_MOD(loc);
<INITIAL>"&"            return LJ::LJ_Parser::make_BIT_AND(loc);
<INITIAL>"|"            return LJ::LJ_Parser::make_BIT_OR(loc);
<INITIAL>"^"            return LJ::LJ_Parser::make_BIT_XOR(loc);
<INITIAL>"~"            return LJ::LJ_Parser::make_BIT_NOT(loc);
<INITIAL>"+="           return LJ::LJ_Parser::make_ADD_ASSIGN(loc);
<INITIAL>"-="           return LJ::LJ_Parser::make_SUB_ASSIGN(loc);
<INITIAL>"*="           return LJ::LJ_Parser::make_MUL_ASSIGN(loc);
<INITIAL>"/="           return LJ::LJ_Parser::make_DIV_ASSIGN(loc);
<INITIAL>"%="           return LJ::LJ_Parser::make_MOD_ASSIGN(loc);
<INITIAL>"++"           return LJ::LJ_Parser::make_INCREMENT(loc);
<INITIAL>"--"           return LJ::LJ_Parser::make_DECREMENT(loc);
<INITIAL>"!"            return LJ::LJ_Parser::make_EXCLAMATION(loc);
<INITIAL>"."            return LJ::LJ_Parser::make_DOT(loc);


<INITIAL>[0-9]+	{
	errno = 0;
	__int64 n = 0;
	sscanf(yytext, "%I64d", &n);
	return LJ::LJ_Parser::make_INT_LITERAL(n, loc);
}

<INITIAL>"0"[xX][0-9a-fA-F]+ {
	__int64 n = 0;
    sscanf(yytext, "%I64x", &n);
    return LJ::LJ_Parser::make_INT_LITERAL(n, loc);
}

<INITIAL>[0-9]+\.[0-9]+ {
    double n = 0.0;
    sscanf(yytext, "%lf", &n);
    return LJ::LJ_Parser::make_DOUBLE_LITERAL(n, loc);
}

<INITIAL>[A-Za-z_][A-Za-z_0-9]*      return LJ::LJ_Parser::make_IDENTIFIER(yytext, loc);
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
