#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STACK 100
#define MAX_INPUT 100

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
char input[MAX_INPUT];
int input_pos = 0;
int stack[MAX_STACK];
int stack_top = -1;

// Function prototypes
TokenType get_next_token();
void push(TokenType token);
TokenType pop();
TokenType top();
void print_stack();
int parse_expression();

// Get next token from input string
TokenType get_next_token() {
    while (input_pos < strlen(input) && isspace(input[input_pos])) {
        input_pos++;
    }
    
    if (input_pos >= strlen(input)) {
        return T_END;
    }
    
    char c = input[input_pos++];
    switch (c) {
        case '+': return T_PLUS;
        case '*': return T_MUL;
        case '(': return T_LPAREN;
        case ')': return T_RPAREN;
        default:
            if (isalpha(c) || c == '_') {
                while (input_pos < strlen(input) && 
                       (isalnum(input[input_pos]) || input[input_pos] == '_')) {
                    input_pos++;
                }
                return T_ID;
            }
            return T_END;
    }
}

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

// Simple reduction - just pop the top element and replace with E (id)
void simple_reduce() {
    if (stack_top >= 0) {
        TokenType popped = pop();
        printf("     Reducing %s → E\n", token_names[popped]);
        push(T_ID); // Push E (represented as ID)
    }
}

// Main parsing function
int parse_expression() {
    TokenType current_token;
    TokenType stack_symbol;
    int action;
    int step = 1;
    
    printf("Parsing Expression: \"%s\"\n", input);
    printf("===================");
    for (int i = 0; i < strlen(input); i++) printf("=");
    printf("\n\n");
    
    // Initialize
    push(T_END);  // Push $ onto stack
    current_token = get_next_token();
    
    printf("Step | Stack          | Input | Relation | Action\n");
    printf("-----|----------------|-------|----------|------------------\n");
    
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
                current_token = get_next_token();
                break;
                
            case 2: // Reduce (>)
                printf("REDUCE\n");
                simple_reduce();
                break;
                
            case 3: // Equal (=)
                printf("MATCH\n");
                push(current_token);
                current_token = get_next_token();
                break;
                
            case 4: // Accept
                printf("ACCEPT\n");
                printf("\n*** PARSING SUCCESSFUL! ***\n");
                printf("The expression \"%s\" is syntactically correct.\n", input);
                return 1;
                
            default: // Error
                printf("ERROR\n");
                printf("\n*** PARSING FAILED! ***\n");
                printf("Syntax error in expression \"%s\"\n", input);
                return 0;
        }
        
        // Safety check to prevent infinite loops
        if (step > 50) {
            printf("ERROR: Too many steps, possible infinite loop\n");
            return 0;
        }
    }
}

int main() {
    printf("=== Fixed Bottom-Up Operator Precedence Parser ===\n");
    printf("===================================================\n\n");
    
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
    
    while (1) {
        printf("Enter arithmetic expression (or 'quit' to exit): ");
        if (!fgets(input, MAX_INPUT, stdin)) break;
        
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "quit") == 0) break;
        if (strlen(input) == 0) continue;
        
        // Reset for new parse
        input_pos = 0;
        stack_top = -1;
        
        parse_expression();
        
        printf("\n");
        for (int i = 0; i < 50; i++) printf("-");
        printf("\n\n");
    }
    
    printf("Goodbye!\n");
    return 0;
}