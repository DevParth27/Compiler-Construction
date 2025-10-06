#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STACK 200
#define MAX_INPUT 200

// Token types (terminals first)
typedef enum {
    T_ID = 0,     // identifier (id)  -- terminal
    T_PLUS = 1,   // +
    T_MUL = 2,    // *
    T_LPAREN = 3, // (
    T_RPAREN = 4, // )
    T_END = 5,    // $ (end marker)
    T_E = 6       // Nonterminal E (NOT part of precedence table)
} TokenType;

// Operator precedence table (terminals only: rows/cols 0..5)
int precedence_table[6][6] = {
    //       id   +    *    (    )    $
    /*id*/  { 0,   2,   2,   0,   2,   2 },
    /*+ */  { 1,   2,   1,   1,   2,   2 },
    /** */  { 1,   2,   2,   1,   2,   2 },
    /*( */  { 1,   1,   1,   1,   3,   0 },
    /*) */  { 0,   2,   2,   0,   2,   2 },
    /*$ */  { 1,   1,   1,   1,   0,   4 }
};

const char* token_names[] = {"id", "+", "*", "(", ")", "$", "E"};
const char* relation_symbols[] = {"ERR", "<", ">", "=", "ACC"};

char input[MAX_INPUT];
int input_pos = 0;

int stack_[MAX_STACK];
int top_ = -1;

// Prototypes
int parse_expression();
void push(TokenType t);
TokenType pop();
TokenType top();
TokenType get_next_token();
void print_stack();

// Return the topmost TERMINAL on the stack (skip T_E). If none, return T_END (behaves like $).
TokenType top_terminal() {
    for (int i = top_; i >= 0; --i) {
        if (stack_[i] != T_E) return (TokenType)stack_[i];
    }
    return T_END;
}

// Get next token from input string
TokenType get_next_token() {
    while (input_pos < (int)strlen(input) && isspace((unsigned char)input[input_pos])) input_pos++;
    if (input_pos >= (int)strlen(input)) return T_END;

    char c = input[input_pos++];
    switch (c) {
        case '+': return T_PLUS;
        case '*': return T_MUL;
        case '(': return T_LPAREN;
        case ')': return T_RPAREN;
        default:
            if (isalpha((unsigned char)c) || c == '_') {
                while (input_pos < (int)strlen(input) &&
                       (isalnum((unsigned char)input[input_pos]) || input[input_pos] == '_')) {
                    input_pos++;
                }
                return T_ID;
            }
            // Unknown char → treat as end/error sentinel
            return T_END;
    }
}

// Stack ops
void push(TokenType t) {
    if (top_ >= MAX_STACK - 1) {
        printf("Stack overflow!\n");
        exit(1);
    }
    stack_[++top_] = t;
}
TokenType pop() {
    if (top_ < 0) {
        printf("Stack underflow!\n");
        exit(1);
    }
    return (TokenType)stack_[top_--];
}
TokenType top() {
    if (top_ < 0) return T_END;
    return (TokenType)stack_[top_];
}

void print_stack() {
    printf("[");
    for (int i = 0; i <= top_; i++) {
        printf("%s", token_names[stack_[i]]);
        if (i < top_) printf(" ");
    }
    printf("]");
}

// Try to reduce once; return 1 if reduced, 0 if no handle found
int reduce_once() {
    // Patterns to check (from top downward):
    // E + E
    if (top_ >= 2 &&
        stack_[top_] == T_E &&
        stack_[top_ - 1] == T_PLUS &&
        stack_[top_ - 2] == T_E) {
        printf("     Reducing E + E → E\n");
        pop(); pop(); pop();
        push(T_E);
        return 1;
    }
    // E * E
    if (top_ >= 2 &&
        stack_[top_] == T_E &&
        stack_[top_ - 1] == T_MUL &&
        stack_[top_ - 2] == T_E) {
        printf("     Reducing E * E → E\n");
        pop(); pop(); pop();
        push(T_E);
        return 1;
    }
    // ( E )
    if (top_ >= 2 &&
        stack_[top_] == T_RPAREN &&
        stack_[top_ - 1] == T_E &&
        stack_[top_ - 2] == T_LPAREN) {
        printf("     Reducing ( E ) → E\n");
        pop(); pop(); pop();
        push(T_E);
        return 1;
    }
    // id → E  (single terminal id at top)
    if (top_ >= 0 && stack_[top_] == T_ID) {
        printf("     Reducing id → E\n");
        stack_[top_] = T_E; // replace in place
        return 1;
    }
    return 0;
}

int parse_expression() {
    TokenType a;               // current input symbol
    int step = 1;

    printf("Parsing Expression: \"%s\"\n", input);
    printf("===================");
    for (size_t i = 0; i < strlen(input); i++) printf("=");
    printf("\n\n");

    // init
    push(T_END);               // $
    a = get_next_token();

    printf("Step | Stack                      | Input | Relation | Action\n");
    printf("-----|----------------------------|-------|----------|------------------\n");

    while (1) {
        TokenType s = top_terminal();
        int action = precedence_table[s][a];

        printf("%-4d | ", step++);
        print_stack();
        printf("%*s | %-5s | %-8s | ",
               (int)(26 - 2 * (top_ + 1)), "", token_names[a],
               relation_symbols[action]);

        if (action == 1 || action == 3) {
            // SHIFT (<) or MATCH (=)
            printf(action == 1 ? "SHIFT\n" : "MATCH\n");
            push(a);
            a = get_next_token();
        } else if (action == 2) {
            // REDUCE (>)
            printf("REDUCE\n");
            if (!reduce_once()) {
                printf("     ERROR: No valid handle to reduce.\n");
                printf("\n*** PARSING FAILED! ***\n");
                return 0;
            }
        } else if (action == 4) {
            // ACC from table ($,$) usually
            printf("ACCEPT\n");
            printf("\n*** PARSING SUCCESSFUL! ***\n");
            printf("The expression \"%s\" is syntactically correct.\n", input);
            return 1;
        } else {
            // ERR
            // Also allow explicit accept check: stack = $ E and a = $
            if (a == T_END && top_ == 1 && stack_[0] == T_END && stack_[1] == T_E) {
                printf("ACCEPT (by configuration)\n");
                printf("\n*** PARSING SUCCESSFUL! ***\n");
                printf("The expression \"%s\" is syntactically correct.\n", input);
                return 1;
            }
            printf("ERROR\n");
            printf("\n*** PARSING FAILED! ***\n");
            return 0;
        }

        // Hard safety against runaway loops
        if (step > 1000) {
            printf("ERROR: Too many steps, possible infinite loop\n");
            return 0;
        }
    }
}

int main() {
    printf("=== Fixed Bottom-Up Operator Precedence Parser ===\n");
    printf("===================================================\n\n");

    printf("Grammar: E → E + E | E * E | ( E ) | id\n");
    printf("Precedence: * has higher precedence than +\n\n");

    printf("Operator Precedence Table (terminals):\n");
    printf("     ");
    for (int j = 0; j < 6; j++) printf("%4s", token_names[j]);
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

    while (1) {
        printf("Enter arithmetic expression (or 'quit' to exit): ");
        if (!fgets(input, MAX_INPUT, stdin)) break;
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "quit") == 0) break;
        if ((int)strlen(input) == 0) continue;

        // Reset state
        input_pos = 0;
        top_ = -1;

        parse_expression();

        printf("\n");
        for (int i = 0; i < 50; i++) printf("-");
        printf("\n\n");
    }

    printf("Goodbye!\n");
    return 0;
}
