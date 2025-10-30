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

"int"           { print_token("KEYWORD_INT", yytext); token_count++; }
"float"         { print_token("KEYWORD_FLOAT", yytext); token_count++; }

{IDENTIFIER}    { print_token("IDENTIFIER", yytext); token_count++; }
{INTEGER}       { print_token("INTEGER", yytext); token_count++; }
{FLOAT}         { print_token("FLOAT", yytext); token_count++; }

"+"             { print_token("PLUS", yytext); token_count++; }
"-"             { print_token("MINUS", yytext); token_count++; }
"*"             { print_token("MULTIPLY", yytext); token_count++; }
"/"             { print_token("DIVIDE", yytext); token_count++; }
"="             { print_token("ASSIGN", yytext); token_count++; }

"("             { print_token("LPAREN", yytext); token_count++; }
")"             { print_token("RPAREN", yytext); token_count++; }
";"             { print_token("SEMICOLON", yytext); token_count++; }

{WHITESPACE}    { /* Ignore whitespace */ }
\n              { line_number++; print_token("NEWLINE", "\\n"); }

.               { print_token("UNKNOWN", yytext); token_count++; }

%%

void print_token(char* token_type, char* lexeme) {
    printf("Line %d: %-15s -> %s\n", line_number, token_type, lexeme);
    
    // Semantic actions based on token type
    if (strcmp(token_type, "KEYWORD_INT") == 0 || strcmp(token_type, "KEYWORD_FLOAT") == 0) {
        printf("         [Semantic] Type declaration detected\n");
    }
    else if (strcmp(token_type, "IDENTIFIER") == 0) {
        printf("         [Semantic] Variable identifier: %s\n", lexeme);
    }
    else if (strcmp(token_type, "INTEGER") == 0) {
        printf("         [Semantic] Integer literal: %s (value: %d)\n", lexeme, atoi(lexeme));
    }
    else if (strcmp(token_type, "FLOAT") == 0) {
        printf("         [Semantic] Float literal: %s (value: %.2f)\n", lexeme, atof(lexeme));
    }
    else if (strcmp(token_type, "PLUS") == 0 || strcmp(token_type, "MINUS") == 0 || 
             strcmp(token_type, "MULTIPLY") == 0 || strcmp(token_type, "DIVIDE") == 0) {
        printf("         [Semantic] Arithmetic operator for expression evaluation\n");
    }
    else if (strcmp(token_type, "ASSIGN") == 0) {
        printf("         [Semantic] Assignment operator for variable binding\n");
    }
    else if (strcmp(token_type, "UNKNOWN") == 0) {
        printf("         [Semantic] ERROR: Unrecognized token '%s'\n", lexeme);
    }
}

int main(int argc, char* argv[]) {
    printf("=== SYNTAX DIRECTED TRANSLATION - LEXICAL ANALYZER ===\n");
    printf("Practical 06: Semantic Analysis with Attribute Grammar\n");
    printf("Features:\n");
    printf("- Token recognition for type declarations\n");
    printf("- Identifier and literal analysis\n");
    printf("- Operator precedence support\n");
    printf("- Semantic actions for each token\n\n");
    
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
    
    printf("\n=== ANALYSIS SUMMARY ===\n");
    printf("Total tokens processed: %d\n", token_count);
    printf("Total lines processed: %d\n", line_number);
    
    if (argc > 1) {
        fclose(yyin);
    }
    
    return 0;
}

int yywrap() {
    return 1;
}