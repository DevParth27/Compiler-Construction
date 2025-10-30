#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_VARS 100
#define MAX_NAME_LEN 50
#define MAX_EXPR_LEN 200
#define MAX_STACK 100

// Data types supported
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_UNDEFINED
} DataType;

// Variable symbol table entry
typedef struct {
    char name[MAX_NAME_LEN];
    DataType type;
    union {
        int int_val;
        float float_val;
    } value;
    bool is_initialized;
} Variable;

// Expression node for syntax tree
typedef struct ExprNode {
    enum { NODE_VAR, NODE_NUM_INT, NODE_NUM_FLOAT, NODE_OP } type;
    DataType data_type;
    union {
        char var_name[MAX_NAME_LEN];
        int int_val;
        float float_val;
        char op;
    } data;
    struct ExprNode* left;
    struct ExprNode* right;
    // Synthesized attributes
    union {
        int int_result;
        float float_result;
    } result;
} ExprNode;

// Global symbol table
Variable symbol_table[MAX_VARS];
int var_count = 0;

// Function prototypes
void menu();
void declare_variable();
void assign_variable();
void evaluate_expression();
void display_symbol_table();
void semantic_analysis_demo();

// Symbol table operations
int find_variable(const char* name);
void add_variable(const char* name, DataType type);
void set_variable_value(const char* name, DataType type, void* value);

// Expression parsing and evaluation
ExprNode* parse_expression(const char* expr, int* pos);
ExprNode* parse_term(const char* expr, int* pos);
ExprNode* parse_factor(const char* expr, int* pos);
ExprNode* create_node(int node_type);
void free_expression_tree(ExprNode* node);

// Semantic analysis
bool perform_semantic_analysis(ExprNode* node);
DataType get_result_type(DataType left, DataType right, char op);
void evaluate_node(ExprNode* node);
void print_expression_tree(ExprNode* node, int depth);

// Utility functions
void skip_whitespace(const char* expr, int* pos);
bool is_operator(char c);
int get_precedence(char op);
const char* type_to_string(DataType type);

int main() {
    printf("=== Syntax Directed Translation - Semantic Analysis ===\n");
    printf("This program demonstrates:\n");
    printf("1. Type checking using synthesized attributes\n");
    printf("2. Undefined variable detection\n");
    printf("3. Expression evaluation with attribute grammar\n\n");
    
    menu();
    return 0;
}

void menu() {
    int choice;
    
    while (1) {
        printf("\n=== MENU ===\n");
        printf("1. Declare Variable\n");
        printf("2. Assign Variable\n");
        printf("3. Evaluate Expression\n");
        printf("4. Display Symbol Table\n");
        printf("5. Semantic Analysis Demo\n");
        printf("6. Exit\n");
        printf("Enter choice: ");
        
        scanf("%d", &choice);
        getchar(); // consume newline
        
        switch (choice) {
            case 1:
                declare_variable();
                break;
            case 2:
                assign_variable();
                break;
            case 3:
                evaluate_expression();
                break;
            case 4:
                display_symbol_table();
                break;
            case 5:
                semantic_analysis_demo();
                break;
            case 6:
                printf("Exiting...\n");
                return;
            default:
                printf("Invalid choice!\n");
        }
    }
}

void declare_variable() {
    char name[MAX_NAME_LEN];
    int type_choice;
    
    printf("Enter variable name: ");
    scanf("%s", name);
    
    // Check if variable already exists
    if (find_variable(name) != -1) {
        printf("Error: Variable '%s' already declared!\n", name);
        return;
    }
    
    printf("Select type:\n1. int\n2. float\nChoice: ");
    scanf("%d", &type_choice);
    
    DataType type = (type_choice == 1) ? TYPE_INT : TYPE_FLOAT;
    add_variable(name, type);
    
    printf("Variable '%s' declared as %s\n", name, type_to_string(type));
}

void assign_variable() {
    char name[MAX_NAME_LEN];
    char expr_str[MAX_EXPR_LEN];
    
    printf("Enter variable name: ");
    scanf("%s", name);
    getchar();
    
    int var_idx = find_variable(name);
    if (var_idx == -1) {
        printf("Error: Variable '%s' not declared!\n", name);
        return;
    }
    
    printf("Enter expression: ");
    fgets(expr_str, sizeof(expr_str), stdin);
    expr_str[strcspn(expr_str, "\n")] = 0; // remove newline
    
    int pos = 0;
    ExprNode* expr = parse_expression(expr_str, &pos);
    
    if (!expr) {
        printf("Error: Invalid expression!\n");
        return;
    }
    
    // Perform semantic analysis
    if (!perform_semantic_analysis(expr)) {
        printf("Semantic analysis failed!\n");
        free_expression_tree(expr);
        return;
    }
    
    // Type checking
    if (expr->data_type != symbol_table[var_idx].type) {
        printf("Warning: Type mismatch! Variable is %s but expression is %s\n",
               type_to_string(symbol_table[var_idx].type),
               type_to_string(expr->data_type));
    }
    
    // Evaluate expression
    evaluate_node(expr);
    
    // Assign value
    if (symbol_table[var_idx].type == TYPE_INT) {
        symbol_table[var_idx].value.int_val = (expr->data_type == TYPE_INT) ? 
            expr->result.int_result : (int)expr->result.float_result;
    } else {
        symbol_table[var_idx].value.float_val = (expr->data_type == TYPE_FLOAT) ? 
            expr->result.float_result : (float)expr->result.int_result;
    }
    
    symbol_table[var_idx].is_initialized = true;
    
    printf("Assignment successful!\n");
    printf("%s = ", name);
    if (symbol_table[var_idx].type == TYPE_INT) {
        printf("%d\n", symbol_table[var_idx].value.int_val);
    } else {
        printf("%.2f\n", symbol_table[var_idx].value.float_val);
    }
    
    free_expression_tree(expr);
}

void evaluate_expression() {
    char expr_str[MAX_EXPR_LEN];
    
    printf("Enter expression to evaluate: ");
    fgets(expr_str, sizeof(expr_str), stdin);
    expr_str[strcspn(expr_str, "\n")] = 0;
    
    int pos = 0;
    ExprNode* expr = parse_expression(expr_str, &pos);
    
    if (!expr) {
        printf("Error: Invalid expression!\n");
        return;
    }
    
    printf("\n=== SYNTAX TREE ===\n");
    print_expression_tree(expr, 0);
    
    printf("\n=== SEMANTIC ANALYSIS ===\n");
    if (!perform_semantic_analysis(expr)) {
        printf("Semantic analysis failed!\n");
        free_expression_tree(expr);
        return;
    }
    
    printf("Expression type: %s\n", type_to_string(expr->data_type));
    
    printf("\n=== EVALUATION ===\n");
    evaluate_node(expr);
    
    printf("Result: ");
    if (expr->data_type == TYPE_INT) {
        printf("%d\n", expr->result.int_result);
    } else {
        printf("%.2f\n", expr->result.float_result);
    }
    
    free_expression_tree(expr);
}

void semantic_analysis_demo() {
    printf("\n=== SEMANTIC ANALYSIS DEMONSTRATION ===\n");
    
    // Add some demo variables
    add_variable("x", TYPE_INT);
    set_variable_value("x", TYPE_INT, &(int){10});
    
    add_variable("y", TYPE_FLOAT);
    set_variable_value("y", TYPE_FLOAT, &(float){3.5});
    
    printf("Demo variables added: x (int) = 10, y (float) = 3.5\n\n");
    
    // Test expressions
    const char* test_expressions[] = {
        "x + 5",           // int + int = int
        "y * 2.0",         // float * float = float
        "x + y",           // int + float = float (type promotion)
        "z + 1",           // undefined variable error
        "x / 0"            // division by zero (runtime check)
    };
    
    int num_tests = sizeof(test_expressions) / sizeof(test_expressions[0]);
    
    for (int i = 0; i < num_tests; i++) {
        printf("Testing: %s\n", test_expressions[i]);
        
        int pos = 0;
        ExprNode* expr = parse_expression(test_expressions[i], &pos);
        
        if (!expr) {
            printf("  Parse Error: Invalid expression\n\n");
            continue;
        }
        
        printf("  Syntax Tree:\n");
        print_expression_tree(expr, 2);
        
        if (perform_semantic_analysis(expr)) {
            printf("  Semantic Analysis: PASSED\n");
            printf("  Result Type: %s\n", type_to_string(expr->data_type));
            
            evaluate_node(expr);
            printf("  Evaluated Result: ");
            if (expr->data_type == TYPE_INT) {
                printf("%d\n", expr->result.int_result);
            } else {
                printf("%.2f\n", expr->result.float_result);
            }
        } else {
            printf("  Semantic Analysis: FAILED\n");
        }
        
        printf("\n");
        free_expression_tree(expr);
    }
}

// Symbol table operations
int find_variable(const char* name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void add_variable(const char* name, DataType type) {
    if (var_count >= MAX_VARS) {
        printf("Error: Symbol table full!\n");
        return;
    }
    
    strcpy(symbol_table[var_count].name, name);
    symbol_table[var_count].type = type;
    symbol_table[var_count].is_initialized = false;
    var_count++;
}

void set_variable_value(const char* name, DataType type, void* value) {
    int idx = find_variable(name);
    if (idx == -1) return;
    
    if (type == TYPE_INT) {
        symbol_table[idx].value.int_val = *(int*)value;
    } else {
        symbol_table[idx].value.float_val = *(float*)value;
    }
    symbol_table[idx].is_initialized = true;
}

void display_symbol_table() {
    printf("\n=== SYMBOL TABLE ===\n");
    printf("%-15s %-10s %-15s %-12s\n", "Name", "Type", "Value", "Initialized");
    printf("--------------------------------------------------------\n");
    
    for (int i = 0; i < var_count; i++) {
        printf("%-15s %-10s ", symbol_table[i].name, type_to_string(symbol_table[i].type));
        
        if (symbol_table[i].is_initialized) {
            if (symbol_table[i].type == TYPE_INT) {
                printf("%-15d", symbol_table[i].value.int_val);
            } else {
                printf("%-15.2f", symbol_table[i].value.float_val);
            }
            printf("YES\n");
        } else {
            printf("%-15s NO\n", "undefined");
        }
    }
    
    if (var_count == 0) {
        printf("No variables declared.\n");
    }
}

// Expression parsing
ExprNode* parse_expression(const char* expr, int* pos) {
    ExprNode* left = parse_term(expr, pos);
    if (!left) return NULL;
    
    skip_whitespace(expr, pos);
    
    while (expr[*pos] == '+' || expr[*pos] == '-') {
        char op = expr[*pos];
        (*pos)++;
        
        ExprNode* right = parse_term(expr, pos);
        if (!right) {
            free_expression_tree(left);
            return NULL;
        }
        
        ExprNode* op_node = create_node(NODE_OP);
        op_node->data.op = op;
        op_node->left = left;
        op_node->right = right;
        left = op_node;
        
        skip_whitespace(expr, pos);
    }
    
    return left;
}

ExprNode* parse_term(const char* expr, int* pos) {
    ExprNode* left = parse_factor(expr, pos);
    if (!left) return NULL;
    
    skip_whitespace(expr, pos);
    
    while (expr[*pos] == '*' || expr[*pos] == '/') {
        char op = expr[*pos];
        (*pos)++;
        
        ExprNode* right = parse_factor(expr, pos);
        if (!right) {
            free_expression_tree(left);
            return NULL;
        }
        
        ExprNode* op_node = create_node(NODE_OP);
        op_node->data.op = op;
        op_node->left = left;
        op_node->right = right;
        left = op_node;
        
        skip_whitespace(expr, pos);
    }
    
    return left;
}

ExprNode* parse_factor(const char* expr, int* pos) {
    skip_whitespace(expr, pos);
    
    if (expr[*pos] == '(') {
        (*pos)++;
        ExprNode* node = parse_expression(expr, pos);
        skip_whitespace(expr, pos);
        if (expr[*pos] == ')') {
            (*pos)++;
        }
        return node;
    }
    
    if (isdigit(expr[*pos])) {
        // Parse number
        int start = *pos;
        bool is_float = false;
        
        while (isdigit(expr[*pos])) (*pos)++;
        
        if (expr[*pos] == '.') {
            is_float = true;
            (*pos)++;
            while (isdigit(expr[*pos])) (*pos)++;
        }
        
        ExprNode* node = create_node(is_float ? NODE_NUM_FLOAT : NODE_NUM_INT);
        
        char num_str[50];
        strncpy(num_str, expr + start, *pos - start);
        num_str[*pos - start] = '\0';
        
        if (is_float) {
            node->data.float_val = atof(num_str);
            node->data_type = TYPE_FLOAT;
        } else {
            node->data.int_val = atoi(num_str);
            node->data_type = TYPE_INT;
        }
        
        return node;
    }
    
    if (isalpha(expr[*pos])) {
        // Parse variable
        int start = *pos;
        while (isalnum(expr[*pos]) || expr[*pos] == '_') (*pos)++;
        
        ExprNode* node = create_node(NODE_VAR);
        strncpy(node->data.var_name, expr + start, *pos - start);
        node->data.var_name[*pos - start] = '\0';
        
        return node;
    }
    
    return NULL;
}

ExprNode* create_node(int node_type) {
    ExprNode* node = (ExprNode*)malloc(sizeof(ExprNode));
    node->type = node_type;
    node->data_type = TYPE_UNDEFINED;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void free_expression_tree(ExprNode* node) {
    if (!node) return;
    free_expression_tree(node->left);
    free_expression_tree(node->right);
    free(node);
}

// Semantic analysis with synthesized attributes
bool perform_semantic_analysis(ExprNode* node) {
    if (!node) return false;
    
    switch (node->type) {
        case NODE_VAR: {
            int var_idx = find_variable(node->data.var_name);
            if (var_idx == -1) {
                printf("Error: Undefined variable '%s'\n", node->data.var_name);
                return false;
            }
            if (!symbol_table[var_idx].is_initialized) {
                printf("Warning: Variable '%s' used before initialization\n", node->data.var_name);
            }
            node->data_type = symbol_table[var_idx].type;
            return true;
        }
        
        case NODE_NUM_INT:
            node->data_type = TYPE_INT;
            return true;
            
        case NODE_NUM_FLOAT:
            node->data_type = TYPE_FLOAT;
            return true;
            
        case NODE_OP:
            if (!perform_semantic_analysis(node->left) || 
                !perform_semantic_analysis(node->right)) {
                return false;
            }
            
            // Type inference using synthesized attributes
            node->data_type = get_result_type(node->left->data_type, 
                                            node->right->data_type, 
                                            node->data.op);
            return true;
    }
    
    return false;
}

DataType get_result_type(DataType left, DataType right, char op) {
    // Type promotion rules
    if (left == TYPE_FLOAT || right == TYPE_FLOAT) {
        return TYPE_FLOAT;
    }
    return TYPE_INT;
}

void evaluate_node(ExprNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_VAR: {
            int var_idx = find_variable(node->data.var_name);
            if (var_idx != -1 && symbol_table[var_idx].is_initialized) {
                if (node->data_type == TYPE_INT) {
                    node->result.int_result = symbol_table[var_idx].value.int_val;
                } else {
                    node->result.float_result = symbol_table[var_idx].value.float_val;
                }
            }
            break;
        }
        
        case NODE_NUM_INT:
            node->result.int_result = node->data.int_val;
            break;
            
        case NODE_NUM_FLOAT:
            node->result.float_result = node->data.float_val;
            break;
            
        case NODE_OP:
            evaluate_node(node->left);
            evaluate_node(node->right);
            
            // Get operand values with type conversion
            float left_val = (node->left->data_type == TYPE_INT) ? 
                (float)node->left->result.int_result : node->left->result.float_result;
            float right_val = (node->right->data_type == TYPE_INT) ? 
                (float)node->right->result.int_result : node->right->result.float_result;
            
            float result;
            switch (node->data.op) {
                case '+': result = left_val + right_val; break;
                case '-': result = left_val - right_val; break;
                case '*': result = left_val * right_val; break;
                case '/': 
                    if (right_val == 0) {
                        printf("Error: Division by zero!\n");
                        result = 0;
                    } else {
                        result = left_val / right_val;
                    }
                    break;
                default: result = 0; break;
            }
            
            if (node->data_type == TYPE_INT) {
                node->result.int_result = (int)result;
            } else {
                node->result.float_result = result;
            }
            break;
    }
}

void print_expression_tree(ExprNode* node, int depth) {
    if (!node) return;
    
    for (int i = 0; i < depth; i++) printf("  ");
    
    switch (node->type) {
        case NODE_VAR:
            printf("VAR: %s (%s)\n", node->data.var_name, type_to_string(node->data_type));
            break;
        case NODE_NUM_INT:
            printf("INT: %d\n", node->data.int_val);
            break;
        case NODE_NUM_FLOAT:
            printf("FLOAT: %.2f\n", node->data.float_val);
            break;
        case NODE_OP:
            printf("OP: %c (%s)\n", node->data.op, type_to_string(node->data_type));
            print_expression_tree(node->left, depth + 1);
            print_expression_tree(node->right, depth + 1);
            break;
    }
}

// Utility functions
void skip_whitespace(const char* expr, int* pos) {
    while (expr[*pos] && isspace(expr[*pos])) {
        (*pos)++;
    }
}

bool is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

int get_precedence(char op) {
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

const char* type_to_string(DataType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_UNDEFINED: return "undefined";
        default: return "unknown";
    }
}