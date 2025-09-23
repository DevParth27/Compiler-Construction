%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_STACK 100

// Token types
#define ID 1
#define PLUS 2
#define MUL 3
#define LPAREN 4
#define RPAREN 5
#define END 6

// Operator precedence table
// 0: error, 1: <, 2: >, 3: =, 4: accept
int precedence[7][7] = {
    //   id  +   *   (   )   $
    /*id*/   {0, 2, 2, 0, 2, 2},
    /*+ */   {1, 2, 1, 1, 2, 2},
    /** */   {1, 2, 2, 1, 2, 2},
    /*( */   {1, 1, 1, 1, 3, 0},
    /*) */   {0, 2, 2, 0, 2, 2},
    /*$ */   {1, 1, 1, 1, 0, 4}
};

const char* token_names[] = {"", "id", "+", "*", "(", ")", "$"};

int stack[MAX_STACK];
int top = -1;

void push(int t) { stack[++top] = t; }
int pop() { return stack[top--]; }
int peek() { return stack[top]; }

int get_token_type(const char* yytext) {
    if (strcmp(yytext, "+") == 0) return PLUS;
    if (strcmp(yytext, "*") == 0) return MUL;
    if (strcmp(yytext, "(") == 0) return LPAREN;
    if (strcmp(yytext, ")") == 0) return RPAREN;
    if (strcmp(yytext, "$") == 0) return END;
    return ID;
}

int yylex();

void parse() {
    int a, b, action;
    int input = yylex();
    push(END); // $ on stack
    while (1) {
        a = peek();
        b = input;
        action = precedence[a][b];
        if (action == 1) { // shift
            push(input);
            input = yylex();
        } else if (action == 2) { // reduce
            // For simplicity, just pop one token (simulate reduction)
            pop();
            printf("Reduce\n");
        } else if (action == 3) { // equal (for parentheses)
            push(input);
            input = yylex();
        } else if (action == 4) { // accept
            printf("Accepted\n");
            break;
        } else {
            printf("Error in parsing\n");
            break;
        }
    }
}
%}

%%

id      { return ID; }
\+      { return PLUS; }
\*      { return MUL; }
\(      { return LPAREN; }
\)      { return RPAREN; }
\n      { return END; }
[ \t]+  { /* skip whitespace */ }
.       { printf("Unknown character: %s\n", yytext); }

%%

int main() {
    printf("Enter expression (e.g., id+id*id):\n");
    parse();
    return 0;
}
