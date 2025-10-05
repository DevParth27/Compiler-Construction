%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STACK 100

// Token types
typedef enum {
    T_ID = 0,     // identifier (id)
    T_PLUS = 1,   // +
    T_MUL = 2,    // *
    T_LPAREN = 3, // (
    T_RPAREN = 4, // )
    T_END = 5     // $ (end marker)
} TokenType;

// Operator precedence table
int precedence_table[6][6] = {
    //     id  +   *   (   )   $
    /*id*/ {0,  2,  2,  0,  2,  2},  // id
    /*+ */ {1,  2,  1,  1,  2,  2},  // +
    /** */ {1,  2,  2,  1,  2,  2},  // *
    /*( */ {1,  1,  1,  1,  3,  0},  // (
    /*) */ {0,  2,  2,  0,  2,  2},  // )
    /*$ */ {1,  1,  1,  1,  0,  4}   // $
};

const char* token_names[] = {"id", "+", "*", "(", ")", "$"};
const char* relation_symbols[] = {"ERR", "<", ">", "=", "ACC"};

// Global variables
int stack[MAX_STACK];
int stack_top = -1;
TokenType current_token;
int step_count = 1;

// Function prototypes
void push(TokenType token);
TokenType pop();
TokenType top();
void print_stack();
void simple_reduce();
int parse_expression();
TokenType get_token_type();

// Stack operations
void push(TokenType token) {
    if (stack_top >= MAX_STACK - 1) {
        printf("Stack overflow!\n");
        exit(1);
    }
    stack[++stack_top] = token;
}

TokenType pop() {
    if (stack_top < 0) {
        printf("Stack underflow!\n");
        exit(1);
    }
    return stack[stack_top--];
}

TokenType top() {
    if (stack_top < 0) return T_END;
    return stack[stack_top];
}

void print_stack() {
    printf("[");
    for (int i = 0; i <= stack_top; i++) {
        printf("%s", token_names[stack[i]]);
        if (i < stack_top) printf(" ");
    }
    printf("]");
}

// Simple reduction
void simple_reduce() {
    if (stack_top >= 0) {
        TokenType popped = pop();
        printf("     Reducing %s → E\n", token_names[popped]);
        push(T_ID); // Push E (represented as ID)
    }
}

// Get token type from yytext
TokenType get_token_type() {
    if (strcmp(yytext, "+") == 0) return T_PLUS;
    if (strcmp(yytext, "*") == 0) return T_MUL;
    if (strcmp(yytext, "(") == 0) return T_LPAREN;
    if (strcmp(yytext, ")") == 0) return T_RPAREN;
    if (strcmp(yytext, "$") == 0 || strcmp(yytext, "\n") == 0) return T_END;
    return T_ID; // Default to identifier
}

// Main parsing function
int parse_expression() {
    TokenType stack_symbol;
    int action;
    
    printf("Step | Stack          | Input | Relation | Action\n");
    printf("-----|----------------|-------|----------|------------------\n");
    
    // Initialize
    push(T_END);  // Push $ onto stack
    current_token = yylex() ? get_token_type() : T_END;
    
    while (1) {
        stack_symbol = top();
        action = precedence_table[stack_symbol][current_token];
        
        printf("%-4d | ", step_count++);
        print_stack();
        printf("%*s | %-5s | %-8s | ", 
               (int)(14 - strlen("[]") - stack_top * 2), "", 
               token_names[current_token],
               relation_symbols[action]);
        
        switch (action) {
            case 1: // Shift (<)
                printf("SHIFT\n");
                push(current_token);
                current_token = yylex() ? get_token_type() : T_END;
                break;
                
            case 2: // Reduce (>)
                printf("REDUCE\n");
                simple_reduce();
                break;
                
            case 3: // Equal (=)
                printf("MATCH\n");
                push(current_token);
                current_token = yylex() ? get_token_type() : T_END;
                break;
                
            case 4: // Accept
                printf("ACCEPT\n");
                printf("\n*** PARSING SUCCESSFUL! ***\n");
                printf("Expression is syntactically correct.\n");
                return 1;
                
            default: // Error
                printf("ERROR\n");
                printf("\n*** PARSING FAILED! ***\n");
                printf("Syntax error in expression\n");
                return 0;
        }
        
        // Safety check
        if (step_count > 50) {
            printf("ERROR: Too many steps, possible infinite loop\n");
            return 0;
        }
    }
}

%}

%%

[a-zA-Z][a-zA-Z0-9_]*  { return T_ID; }
\+                     { return T_PLUS; }
\*                     { return T_MUL; }
\(                     { return T_LPAREN; }
\)                     { return T_RPAREN; }
[ \t]+                 { /* skip whitespace */ }
\n                     { return T_END; }
.                      { printf("Unknown character: %s\n", yytext); }

%%

int yywrap() {
    return 1;
}

int main() {
    printf("=== Lex-based Bottom-Up Operator Precedence Parser ===\n");
    printf("=======================================================\n\n");
    
    printf("Grammar: E → E + E | E * E | (E) | id\n");
    printf("Precedence: * has higher precedence than +\n\n");
    
    printf("Operator Precedence Table:\n");
    printf("     ");
    for (int j = 0; j < 6; j++) {
        printf("%4s", token_names[j]);
    }
    printf("\n");
    
    for (int i = 0; i < 6; i++) {
        printf("%3s: ", token_names[i]);
        for (int j = 0; j < 6; j++) {
            switch (precedence_table[i][j]) {
                case 0: printf("   -"); break;
                case 1: printf("   <"); break;
                case 2: printf("   >"); break;
                case 3: printf("   ="); break;
                case 4: printf(" ACC"); break;
            }
        }
        printf("\n");
    }
    printf("\n");
    
    printf("Enter arithmetic expression (end with Enter):\n");
    printf("Examples: id+id*id, (id+id)*id, id*id+id\n");
    printf("Expression: ");
    
    // Reset globals
    stack_top = -1;
    step_count = 1;
    
    // Parse the input
    parse_expression();
    
    return 0;
}