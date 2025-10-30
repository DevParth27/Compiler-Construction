#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_EXPR_LEN 200
#define MAX_STACK_SIZE 100
#define MAX_CODE_LINES 500
#define MAX_VARS 50
#define MAX_NAME_LEN 20

// Data structures for intermediate code generation
typedef struct {
    char op[10];
    char arg1[20];
    char arg2[20];
    char result[20];
} ThreeAddressCode;

typedef struct {
    char name[MAX_NAME_LEN];
    int temp_count;
} Variable;

// Global variables
ThreeAddressCode code[MAX_CODE_LINES];
Variable variables[MAX_VARS];
int code_count = 0;
int temp_count = 0;
int var_count = 0;
int label_count = 0;

// Stack for operators and operands
char op_stack[MAX_STACK_SIZE][10];
char operand_stack[MAX_STACK_SIZE][20];
int op_top = -1;
int operand_top = -1;

// Function prototypes
void menu();
void generate_postfix_and_3ac();
void generate_conditional_3ac();
void generate_loop_3ac();
void generate_array_3ac();
void display_generated_code();
void clear_code();

// Postfix and 3AC generation functions
void infix_to_postfix(char* infix, char* postfix);
void postfix_to_3ac(char* postfix);
int precedence(char op);
bool is_operator(char c);
void push_op(char* op);
void push_operand(char* operand);
char* pop_op();
char* pop_operand();
char* new_temp();
char* new_label();
void add_3ac(char* op, char* arg1, char* arg2, char* result);

// Utility functions
void trim_whitespace(char* str);
bool is_valid_identifier(char* str);

int main() {
    printf("=== INTERMEDIATE CODE GENERATION ===\n");
    printf("Practical 07: Postfix Expression & 3-Address Code Generator\n");
    printf("Features:\n");
    printf("1. Infix to Postfix conversion\n");
    printf("2. 3-Address Code generation\n");
    printf("3. Conditional statements (if-else)\n");
    printf("4. Loop constructs (while, for)\n");
    printf("5. Array operations\n\n");
    
    menu();
    return 0;
}

void menu() {
    int choice;
    
    while (1) {
        printf("\n=== MENU ===\n");
        printf("1. Generate Postfix & 3-Address Code for Expression\n");
        printf("2. Generate 3-Address Code for Conditional Statement\n");
        printf("3. Generate 3-Address Code for Loop\n");
        printf("4. Generate 3-Address Code for Array Operations\n");
        printf("5. Display Generated Code\n");
        printf("6. Clear Code\n");
        printf("7. Exit\n");
        printf("Enter choice: ");
        
        scanf("%d", &choice);
        getchar(); // consume newline
        
        switch (choice) {
            case 1:
                generate_postfix_and_3ac();
                break;
            case 2:
                generate_conditional_3ac();
                break;
            case 3:
                generate_loop_3ac();
                break;
            case 4:
                generate_array_3ac();
                break;
            case 5:
                display_generated_code();
                break;
            case 6:
                clear_code();
                break;
            case 7:
                printf("Exiting...\n");
                return;
            default:
                printf("Invalid choice!\n");
        }
    }
}

void generate_postfix_and_3ac() {
    char infix[MAX_EXPR_LEN];
    char postfix[MAX_EXPR_LEN];
    
    printf("\n=== POSTFIX & 3-ADDRESS CODE GENERATION ===\n");
    printf("Enter infix expression (e.g., a + b * c - d): ");
    fgets(infix, sizeof(infix), stdin);
    trim_whitespace(infix);
    
    printf("\nInput Expression: %s\n", infix);
    
    // Convert to postfix
    infix_to_postfix(infix, postfix);
    printf("Postfix Expression: %s\n", postfix);
    
    // Generate 3-address code
    printf("\n3-Address Code:\n");
    postfix_to_3ac(postfix);
    
    printf("\nCode generation completed!\n");
}

void generate_conditional_3ac() {
    char condition[MAX_EXPR_LEN];
    char true_stmt[MAX_EXPR_LEN];
    char false_stmt[MAX_EXPR_LEN];
    char* label1 = new_label();
    char* label2 = new_label();
    
    printf("\n=== CONDITIONAL STATEMENT 3-ADDRESS CODE ===\n");
    printf("Enter condition (e.g., a > b): ");
    fgets(condition, sizeof(condition), stdin);
    trim_whitespace(condition);
    
    printf("Enter true statement (e.g., x = a + b): ");
    fgets(true_stmt, sizeof(true_stmt), stdin);
    trim_whitespace(true_stmt);
    
    printf("Enter false statement (e.g., x = a - b): ");
    fgets(false_stmt, sizeof(false_stmt), stdin);
    trim_whitespace(false_stmt);
    
    printf("\n3-Address Code for if-else:\n");
    
    // Generate condition check
    char* temp_cond = new_temp();
    add_3ac("eval", condition, "", temp_cond);
    add_3ac("if_false", temp_cond, label1, "");
    
    // True block
    char postfix_true[MAX_EXPR_LEN];
    if (strchr(true_stmt, '=')) {
        char* eq_pos = strchr(true_stmt, '=');
        char var[20], expr[MAX_EXPR_LEN];
        strncpy(var, true_stmt, eq_pos - true_stmt);
        var[eq_pos - true_stmt] = '\0';
        strcpy(expr, eq_pos + 1);
        trim_whitespace(var);
        trim_whitespace(expr);
        
        infix_to_postfix(expr, postfix_true);
        postfix_to_3ac(postfix_true);
        add_3ac("=", code[code_count-1].result, "", var);
    }
    
    add_3ac("goto", label2, "", "");
    add_3ac("label", label1, "", "");
    
    // False block
    char postfix_false[MAX_EXPR_LEN];
    if (strchr(false_stmt, '=')) {
        char* eq_pos = strchr(false_stmt, '=');
        char var[20], expr[MAX_EXPR_LEN];
        strncpy(var, false_stmt, eq_pos - false_stmt);
        var[eq_pos - false_stmt] = '\0';
        strcpy(expr, eq_pos + 1);
        trim_whitespace(var);
        trim_whitespace(expr);
        
        infix_to_postfix(expr, postfix_false);
        postfix_to_3ac(postfix_false);
        add_3ac("=", code[code_count-1].result, "", var);
    }
    
    add_3ac("label", label2, "", "");
    
    printf("Conditional code generation completed!\n");
}

void generate_loop_3ac() {
    char condition[MAX_EXPR_LEN];
    char body[MAX_EXPR_LEN];
    char* label_start = new_label();
    char* label_end = new_label();
    
    printf("\n=== LOOP 3-ADDRESS CODE ===\n");
    printf("Enter loop condition (e.g., i < 10): ");
    fgets(condition, sizeof(condition), stdin);
    trim_whitespace(condition);
    
    printf("Enter loop body statement (e.g., sum = sum + i): ");
    fgets(body, sizeof(body), stdin);
    trim_whitespace(body);
    
    printf("\n3-Address Code for while loop:\n");
    
    // Loop start
    add_3ac("label", label_start, "", "");
    
    // Condition check
    char* temp_cond = new_temp();
    add_3ac("eval", condition, "", temp_cond);
    add_3ac("if_false", temp_cond, label_end, "");
    
    // Loop body
    if (strchr(body, '=')) {
        char* eq_pos = strchr(body, '=');
        char var[20], expr[MAX_EXPR_LEN];
        strncpy(var, body, eq_pos - body);
        var[eq_pos - body] = '\0';
        strcpy(expr, eq_pos + 1);
        trim_whitespace(var);
        trim_whitespace(expr);
        
        char postfix_body[MAX_EXPR_LEN];
        infix_to_postfix(expr, postfix_body);
        postfix_to_3ac(postfix_body);
        add_3ac("=", code[code_count-1].result, "", var);
    }
    
    // Jump back to start
    add_3ac("goto", label_start, "", "");
    add_3ac("label", label_end, "", "");
    
    printf("Loop code generation completed!\n");
}

void generate_array_3ac() {
    char array_op[MAX_EXPR_LEN];
    
    printf("\n=== ARRAY OPERATIONS 3-ADDRESS CODE ===\n");
    printf("Choose operation:\n");
    printf("1. Array assignment (e.g., a[i] = x + y)\n");
    printf("2. Array access (e.g., x = a[i] + b[j])\n");
    printf("Enter choice: ");
    
    int choice;
    scanf("%d", &choice);
    getchar();
    
    if (choice == 1) {
        printf("Enter array assignment (e.g., arr[i] = x + y): ");
        fgets(array_op, sizeof(array_op), stdin);
        trim_whitespace(array_op);
        
        printf("\n3-Address Code for array assignment:\n");
        
        // Parse array assignment
        char* eq_pos = strchr(array_op, '=');
        if (eq_pos) {
            char left[50], right[MAX_EXPR_LEN];
            strncpy(left, array_op, eq_pos - array_op);
            left[eq_pos - array_op] = '\0';
            strcpy(right, eq_pos + 1);
            trim_whitespace(left);
            trim_whitespace(right);
            
            // Extract array name and index
            char* bracket_start = strchr(left, '[');
            char* bracket_end = strchr(left, ']');
            if (bracket_start && bracket_end) {
                char array_name[20], index[20];
                strncpy(array_name, left, bracket_start - left);
                array_name[bracket_start - left] = '\0';
                strncpy(index, bracket_start + 1, bracket_end - bracket_start - 1);
                index[bracket_end - bracket_start - 1] = '\0';
                
                // Generate code for right side expression
                char postfix[MAX_EXPR_LEN];
                infix_to_postfix(right, postfix);
                postfix_to_3ac(postfix);
                
                // Generate array assignment
                char* temp_addr = new_temp();
                add_3ac("*", index, "4", temp_addr); // Assuming 4 bytes per element
                add_3ac("[]", array_name, temp_addr, code[code_count-2].result);
            }
        }
    } else if (choice == 2) {
        printf("Enter expression with array access (e.g., x = a[i] + b[j]): ");
        fgets(array_op, sizeof(array_op), stdin);
        trim_whitespace(array_op);
        
        printf("\n3-Address Code for array access:\n");
        
        // Simplified array access code generation
        char* eq_pos = strchr(array_op, '=');
        if (eq_pos) {
            char var[20], expr[MAX_EXPR_LEN];
            strncpy(var, array_op, eq_pos - array_op);
            var[eq_pos - array_op] = '\0';
            strcpy(expr, eq_pos + 1);
            trim_whitespace(var);
            trim_whitespace(expr);
            
            // For simplicity, treat array access as variables for now
            char postfix[MAX_EXPR_LEN];
            infix_to_postfix(expr, postfix);
            postfix_to_3ac(postfix);
            add_3ac("=", code[code_count-1].result, "", var);
        }
    }
    
    printf("Array operation code generation completed!\n");
}

void infix_to_postfix(char* infix, char* postfix) {
    int i = 0, j = 0;
    char token[20];
    op_top = -1;
    
    while (infix[i] != '\0') {
        if (isspace(infix[i])) {
            i++;
            continue;
        }
        
        if (isalnum(infix[i])) {
            // Read operand
            int k = 0;
            while (isalnum(infix[i]) || infix[i] == '_') {
                token[k++] = infix[i++];
            }
            token[k] = '\0';
            
            // Add to postfix
            if (j > 0) postfix[j++] = ' ';
            strcpy(postfix + j, token);
            j += strlen(token);
        }
        else if (infix[i] == '(') {
            push_op("(");
            i++;
        }
        else if (infix[i] == ')') {
            while (op_top >= 0 && strcmp(op_stack[op_top], "(") != 0) {
                if (j > 0) postfix[j++] = ' ';
                strcpy(postfix + j, pop_op());
                j += strlen(postfix + j);
            }
            if (op_top >= 0) pop_op(); // Remove '('
            i++;
        }
        else if (is_operator(infix[i])) {
            char op[2] = {infix[i], '\0'};
            
            while (op_top >= 0 && strcmp(op_stack[op_top], "(") != 0 &&
                   precedence(op_stack[op_top][0]) >= precedence(infix[i])) {
                if (j > 0) postfix[j++] = ' ';
                strcpy(postfix + j, pop_op());
                j += strlen(postfix + j);
            }
            push_op(op);
            i++;
        }
        else {
            i++; // Skip unknown characters
        }
    }
    
    while (op_top >= 0) {
        if (j > 0) postfix[j++] = ' ';
        strcpy(postfix + j, pop_op());
        j += strlen(postfix + j);
    }
    
    postfix[j] = '\0';
}

void postfix_to_3ac(char* postfix) {
    operand_top = -1;
    char* token = strtok(postfix, " ");
    
    while (token != NULL) {
        if (is_operator(token[0]) && strlen(token) == 1) {
            if (operand_top >= 1) {
                char* arg2 = pop_operand();
                char* arg1 = pop_operand();
                char* result = new_temp();
                
                add_3ac(token, arg1, arg2, result);
                push_operand(result);
            }
        } else {
            push_operand(token);
        }
        token = strtok(NULL, " ");
    }
}

int precedence(char op) {
    switch (op) {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
        default:
            return 0;
    }
}

bool is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

void push_op(char* op) {
    if (op_top < MAX_STACK_SIZE - 1) {
        strcpy(op_stack[++op_top], op);
    }
}

void push_operand(char* operand) {
    if (operand_top < MAX_STACK_SIZE - 1) {
        strcpy(operand_stack[++operand_top], operand);
    }
}

char* pop_op() {
    if (op_top >= 0) {
        return op_stack[op_top--];
    }
    return "";
}

char* pop_operand() {
    if (operand_top >= 0) {
        return operand_stack[operand_top--];
    }
    return "";
}

char* new_temp() {
    static char temp[10];
    sprintf(temp, "t%d", temp_count++);
    return temp;
}

char* new_label() {
    static char label[10];
    sprintf(label, "L%d", label_count++);
    return label;
}

void add_3ac(char* op, char* arg1, char* arg2, char* result) {
    if (code_count < MAX_CODE_LINES) {
        strcpy(code[code_count].op, op);
        strcpy(code[code_count].arg1, arg1);
        strcpy(code[code_count].arg2, arg2);
        strcpy(code[code_count].result, result);
        
        // Print the 3AC line
        printf("%d: ", code_count);
        if (strcmp(op, "label") == 0) {
            printf("%s:\n", arg1);
        } else if (strcmp(op, "goto") == 0) {
            printf("goto %s\n", arg1);
        } else if (strcmp(op, "if_false") == 0) {
            printf("if_false %s goto %s\n", arg1, arg2);
        } else if (strcmp(op, "=") == 0) {
            printf("%s = %s\n", result, arg1);
        } else if (strcmp(op, "[]") == 0) {
            printf("%s[%s] = %s\n", arg1, arg2, result);
        } else if (strcmp(op, "eval") == 0) {
            printf("%s = %s\n", result, arg1);
        } else {
            printf("%s = %s %s %s\n", result, arg1, op, arg2);
        }
        
        code_count++;
    }
}

void display_generated_code() {
    printf("\n=== GENERATED 3-ADDRESS CODE ===\n");
    if (code_count == 0) {
        printf("No code generated yet.\n");
        return;
    }
    
    for (int i = 0; i < code_count; i++) {
        printf("%d: ", i);
        if (strcmp(code[i].op, "label") == 0) {
            printf("%s:\n", code[i].arg1);
        } else if (strcmp(code[i].op, "goto") == 0) {
            printf("goto %s\n", code[i].arg1);
        } else if (strcmp(code[i].op, "if_false") == 0) {
            printf("if_false %s goto %s\n", code[i].arg1, code[i].arg2);
        } else if (strcmp(code[i].op, "=") == 0) {
            printf("%s = %s\n", code[i].result, code[i].arg1);
        } else if (strcmp(code[i].op, "[]") == 0) {
            printf("%s[%s] = %s\n", code[i].arg1, code[i].arg2, code[i].result);
        } else if (strcmp(code[i].op, "eval") == 0) {
            printf("%s = %s\n", code[i].result, code[i].arg1);
        } else {
            printf("%s = %s %s %s\n", code[i].result, code[i].arg1, code[i].op, code[i].arg2);
        }
    }
}

void clear_code() {
    code_count = 0;
    temp_count = 0;
    label_count = 0;
    printf("Code cleared successfully!\n");
}

void trim_whitespace(char* str) {
    char* end;
    
    // Trim leading space
    while (isspace((unsigned char)*str)) str++;
    
    // All spaces?
    if (*str == 0) return;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    end[1] = '\0';
    
    // Move trimmed string to beginning
    if (str != str) {
        memmove(str - (str - str), str, strlen(str) + 1);
    }
}

bool is_valid_identifier(char* str) {
    if (!isalpha(str[0]) && str[0] != '_') return false;
    
    for (int i = 1; str[i] != '\0'; i++) {
        if (!isalnum(str[i]) && str[i] != '_') return false;
    }
    
    return true;
}