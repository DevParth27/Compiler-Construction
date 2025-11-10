%{
#include <stdio.h>
%}

IDENT    [a-zA-Z_][a-zA-Z0-9_]*
NUMBER   [0-9]+
OP       [\+\-\*/=]

%%
{IDENT}     { printf("IDENT(%s)\n", yytext); }
{NUMBER}    { printf("NUMBER(%s)\n", yytext); }
{OP}        { printf("OP(%s)\n", yytext); }
"("         { printf("LPAREN(%s)\n", yytext); }
")"         { printf("RPAREN(%s)\n", yytext); }
"["         { printf("LBRACK(%s)\n", yytext); }
"]"         { printf("RBRACK(%s)\n", yytext); }
";"         { printf("SEMI(%s)\n", yytext); }
[ \t\r\n]+  ;
.           { printf("UNKNOWN(%s)\n", yytext); }
%%

int yywrap() { return 1; }

int main() {
    printf("PRACTICAL 08 LEXER: Tokenizing input for optimization demo\n");
    yylex();
    return 0;
}