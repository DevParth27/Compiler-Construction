%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int line_number = 1;
int token_count = 0;

void print_token(char* token_type, char* lexeme);
%}

DIGIT       [0-9]
LETTER      [a-zA-Z]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT}|_)*
INTEGER     {DIGIT}+
FLOAT       {DIGIT}+\.{DIGIT}+
WHITESPACE  [ \t]+

%%

"if"            { print_token("IF", yytext); token_count++; }
"else"          { print_token("ELSE", yytext); token_count++; }
"while"         { print_token("WHILE", yytext); token_count++; }
"for"           { print_token("FOR", yytext); token_count++; }
"int"           { print_token("INT", yytext); token_count++; }
"float"         { print_token("FLOAT", yytext); token_count++; }

{IDENTIFIER}    { print_token("IDENTIFIER", yytext); token_count++; }
{INTEGER}       { print_token("INTEGER", yytext); token_count++; }
{FLOAT}         { print_token("FLOAT_NUM", yytext); token_count++; }

"+"             { print_token("PLUS", yytext); token_count++; }
"-"             { print_token("MINUS", yytext); token_count++; }
"*"             { print_token("MULTIPLY", yytext); token_count++; }
"/"             { print_token("DIVIDE", yytext); token_count++; }
"="             { print_token("ASSIGN", yytext); token_count++; }

">="            { print_token("GTE", yytext); token_count++; }
"<="            { print_token("LTE", yytext); token_count++; }
"=="            { print_token("EQ", yytext); token_count++; }
"!="            { print_token("NEQ", yytext); token_count++; }
">"             { print_token("GT", yytext); token_count++; }
"<"             { print_token("LT", yytext); token_count++; }

"("             { print_token("LPAREN", yytext); token_count++; }
")"             { print_token("RPAREN", yytext); token_count++; }
"["             { print_token("LBRACKET", yytext); token_count++; }
"]"             { print_token("RBRACKET", yytext); token_count++; }
"{"             { print_token("LBRACE", yytext); token_count++; }
"}"             { print_token("RBRACE", yytext); token_count++; }
";"             { print_token("SEMICOLON", yytext); token_count++; }
","             { print_token("COMMA", yytext); token_count++; }

{WHITESPACE}    { /* Ignore whitespace */ }
\n              { line_number++; print_token("NEWLINE", "\\n"); }

.               { print_token("UNKNOWN", yytext); token_count++; }

%%

void print_token(char* token_type, char* lexeme) {
    printf("Line %d: %-15s -> %s\n", line_number, token_type, lexeme);
    
    // Semantic actions for intermediate code generation
    if (strcmp(token_type, "IF") == 0 || strcmp(token_type, "ELSE") == 0) {
        printf("         [ICG] Conditional construct - prepare branch code\n");
    }
    else if (strcmp(token_type, "WHILE") == 0 || strcmp(token_type, "FOR") == 0) {
        printf("         [ICG] Loop construct - prepare jump code\n");
    }
    else if (strcmp(token_type, "IDENTIFIER") == 0) {
        printf("         [ICG] Variable '%s' - 3-address code operand\n", lexeme);
    }
    else if (strcmp(token_type, "INTEGER") == 0 || strcmp(token_type, "FLOAT_NUM") == 0) {
        printf("         [ICG] Literal: %s - expression operand\n", lexeme);
    }
    else if (strcmp(token_type, "PLUS") == 0 || strcmp(token_type, "MINUS") == 0 || 
             strcmp(token_type, "MULTIPLY") == 0 || strcmp(token_type, "DIVIDE") == 0) {
        printf("         [ICG] Arithmetic op '%s' - temp variable needed\n", lexeme);
    }
    else if (strcmp(token_type, "ASSIGN") == 0) {
        printf("         [ICG] Assignment - result target\n");
    }
    else if (strcmp(token_type, "GT") == 0 || strcmp(token_type, "LT") == 0 || 
             strcmp(token_type, "GTE") == 0 || strcmp(token_type, "LTE") == 0 ||
             strcmp(token_type, "EQ") == 0 || strcmp(token_type, "NEQ") == 0) {
        printf("         [ICG] Relational op '%s' - condition for branch\n", lexeme);
    }
    else if (strcmp(token_type, "LBRACKET") == 0 || strcmp(token_type, "RBRACKET") == 0) {
        printf("         [ICG] Array indexing - address calculation\n");
    }
}

int main(int argc, char* argv[]) {
    printf("=== INTERMEDIATE CODE GENERATION - LEXICAL ANALYZER ===\n");
    printf("Practical 07: Token Analysis for Code Generation\n");
    printf("Features: Expression tokens, Control structures, Array operations\n\n");
    
    if (argc > 1) {
        FILE* file = fopen(argv[1], "r");
        if (!file) {
            fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
            return 1;
        }
        yyin = file;
        printf("Analyzing file: %s\n\n", argv[1]);
    } else {
        printf("Enter code (Ctrl+D to end):\n");
        yyin = stdin;
    }
    
    printf("=== TOKEN ANALYSIS ===\n");
    yylex();
    
    printf("\n=== SUMMARY ===\n");
    printf("Tokens: %d, Lines: %d\n", token_count, line_number);
    
    if (argc > 1) {
        fclose(yyin);
    }
    
    return 0;
}

int yywrap() {
    return 1;
}