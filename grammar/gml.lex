%option noyywrap 
%{
  #include <string>
  using namespace std;
  #include "gml.h"
  #include "gml.tab.h"

  int line_num = 1;
%}

DIGIT [0-9]
SIGN (\-|\+)

%%
[ \t]          ;
\n { ++line_num;}
{SIGN}?{DIGIT}+\.{DIGIT}+   {yylval.dval = stod(yytext); return FLOAT; }
{SIGN}?{DIGIT}+         { yylval.ival = stoi(yytext); return INT; }
\[   { return LEFT_BRACKET; }
\]   { return RIGHT_BRACKET; }
graph { return GRAPH; }
\"([^\\\"]|\\.)*\" { yylval.sval = strdup(yytext); return STRING_LITERAL; }
[a-zA-Z0-9_]+     { yylval.sval = strdup(yytext); return ID; }
. { return *yytext; }
%%
