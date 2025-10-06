%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STACK 100
#define MAX_INPUT 100

typedef enum {
    T_ID = 0,     // identifier (id)
    T_PLUS = 1,   // +
    T_MUL = 2,    // *
    T_LPAREN = 3, // (
    T_RPAREN = 4, // )
    T_END = 5     // $ (end marker)
} TokenType;

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

int stack[MAX_STACK];
int stack_top = -1;
TokenType token_stream[MAX_INPUT];
int token_count = 0;
int token_pos = 0;
int step_count = 1;

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

// Find handle to reduce
int find_handle(TokenType next_token) {
    // (E)
    if (stack_top >= 2 &&
        stack[stack_top-2] == T_LPAREN &&
        stack[stack_top-1] == T_ID &&
        stack[stack_top] == T_RPAREN) {
        return 3;
    }
    // E+E or E*E
    if (stack_top >= 2 &&
        stack[stack_top-2] == T_ID &&
        stack[stack_top] == T_ID &&
        (stack[stack_top-1] == T_PLUS || stack[stack_top-1] == T_MUL)) {
        return 3;
    }
    // Single id, only if next token is not an operator
    if (stack_top >= 0 && stack[stack_top] == T_ID) {
        if (next_token == T_PLUS || next_token == T_MUL || next_token == T_ID || next_token == T_LPAREN) {
            return 0;
        }
        return 1;
    }
    return 0;
}

// Reduce the handle
void reduce_handle(TokenType next_token) {
    int handle_size = find_handle(next_token);
    if (handle_size == 3) {
        if (stack[stack_top-2] == T_LPAREN && stack[stack_top] == T_RPAREN) {
            printf("     Reducing: (E) → E\n");
        } else if (stack[stack_top-1] == T_PLUS) {
            printf("     Reducing: E + E → E\n");
        } else if (stack[stack_top-1] == T_MUL) {
            printf("     Reducing: E * E → E\n");
        }
        pop(); pop(); pop();
        push(T_ID);
    } else if (handle_size == 1) {
        printf("     Reducing: id → E\n");
        pop();
        push(T_ID);
    }
}

// Main parsing function
int parse_expression() {
    TokenType current_token;
    TokenType stack_symbol;
    int action;
    int step = 1;

    printf("Step | Stack          | Input | Relation | Action\n");
    printf("-----|----------------|-------|----------|------------------\n");

    stack_top = -1;
    push(T_END);
    token_pos = 0;
    current_token = token_stream[token_pos++];

    while (1) {
        stack_symbol = top();
        action = precedence_table[stack_symbol][current_token];

        printf("%-4d | ", step++);
        print_stack();
        printf("%*s | %-5s | %-8s | ",
               (int)(14 - strlen("[]") - stack_top * 2), "",
               token_names[current_token],
               relation_symbols[action]);

        switch (action) {
            case 1: // Shift (<)
                printf("SHIFT\n");
                push(current_token);
                current_token = token_stream[token_pos++];
                break;
            case 2: // Reduce (>)
                printf("REDUCE\n");
                reduce_handle(current_token);
                break;
            case 3: // Equal (=)
                printf("MATCH\n");
                push(current_token);
                current_token = token_stream[token_pos++];
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
        if (step > 50) {
            printf("ERROR: Too many steps, stopping\n");
            return 0;
        }
    }
}

// Tokenizer for input line
void tokenize(const char* line) {
    token_count = 0;
    int i = 0;
    while (line[i]) {
        if (isspace(line[i])) {
            i++;
            continue;
        }
        if (isalpha(line[i]) || line[i] == '_') {
            int start = i;
            while (isalnum(line[i]) || line[i] == '_') i++;
            token_stream[token_count++] = T_ID;
            continue;
        }
        if (line[i] == '+') {
            token_stream[token_count++] = T_PLUS;
            i++;
            continue;
        }
        if (line[i] == '*') {
            token_stream[token_count++] = T_MUL;
            i++;
            continue;
        }
        if (line[i] == '(') {
            token_stream[token_count++] = T_LPAREN;
            i++;
            continue;
        }
        if (line[i] == ')') {
            token_stream[token_count++] = T_RPAREN;
            i++;
            continue;
        }
        // Unknown character
        printf("Unknown character: %c\n", line[i]);
        i++;
    }
    token_stream[token_count++] = T_END;
}

%}

%%

[a-zA-Z_][a-zA-Z0-9_]*  { /* handled in main, not used here */ }
\+                      { /* handled in main, not used here */ }
\*                      { /* handled in main, not used here */ }
\(                      { /* handled in main, not used here */ }
\)                      { /* handled in main, not used here */ }
[ \t]+                  { /* skip whitespace */ }
\n                      { /* end of input */ }
.                       { /* unknown character */ }

%%

int yywrap() { return 1; }

int main() {
    char line[MAX_INPUT];
    printf("=== Operator Precedence Parser (Lex version) ===\n");
    printf("Grammar: E → E + E | E * E | (E) | id\n");
    printf("Precedence: * has higher precedence than +\n\n");

    while (1) {
        printf("Enter arithmetic expression (or 'quit' to exit): ");
        if (!fgets(line, MAX_INPUT, stdin)) break;
        line[strcspn(line, "\n")] = 0;
        if (strcmp(line, "quit") == 0) break;
        if (strlen(line) == 0) continue;

        tokenize(line);
        step_count = 1;
        parse_expression();

        printf("\n");
        for (int i = 0; i < 50; i++) printf("-");
        printf("\n\n");
    }
    printf("Goodbye!\n");
    return 0;
}
