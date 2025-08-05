%{
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
int operator_count = 0;
%}

/* Regular expression definitions */
identifier    [a-zA-Z][a-zA-Z0-9]*
number        [0-9]+
whitespace    [ \t\n]
multi_op      "++"|"--"|"=="|"!="|"<="|">="|"&&"|"||"|">>"|"<<"
single_op     [+\-*/%=<>!&|^~]

%%

{multi_op}     { printf("Multi-char operator: %s\n", yytext); operator_count++; }
{single_op}    { printf("Single-char operator: %c\n", yytext[0]); operator_count++; }
{identifier}   { printf("Identifier: %s\n", yytext); }
{number}       { printf("Number: %s\n", yytext); }
{whitespace}   { /* ignore whitespace */ }
.              { printf("Other character: %c\n", yytext[0]); }

%%

int main() {
    printf("Enter code snippet (Ctrl+D to end):\n");
    yylex();
    printf("\nTotal operators found: %d\n", operator_count);
    return 0;
}

int yywrap() {
    return 1;
}