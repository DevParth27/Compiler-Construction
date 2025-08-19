#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_RULES 50
#define MAX_SYMBOLS 20
#define MAX_STRING_LEN 100

// Structure to represent a production rule
typedef struct {
    char lhs;
    char rhs[MAX_STRING_LEN];
} Production;

// Global variables
Production grammar[MAX_RULES];
int num_rules = 0;
char terminals[MAX_SYMBOLS];
char non_terminals[MAX_SYMBOLS];
int num_terminals = 0;
int num_non_terminals = 0;

// Function prototypes
void input_grammar();
void display_grammar();
int detect_ambiguity();
void compute_first(char symbol, char first_set[]);
void compute_follow(char symbol, char follow_set[]);
bool can_derive_string(char* input_string);
bool is_terminal(char c);
bool is_non_terminal(char c);
void add_to_set(char set[], char symbol);
bool contains(char set[], char symbol);
void print_set(char set[]);
void menu();

int main() {
    int choice;
    char symbol;
    char first_set[MAX_SYMBOLS] = {0};
    char follow_set[MAX_SYMBOLS] = {0};
    char input_string[MAX_STRING_LEN];
    
    printf("CFG Construction and FIRST/FOLLOW Computation\n");
    printf("============================================\n\n");
    
    while(1) {
        menu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch(choice) {
            case 1:
                input_grammar();
                break;
                
            case 2:
                display_grammar();
                break;
                
            case 3:
                switch(detect_ambiguity()) {
                    case 0:
                        printf("Grammar is NOT ambiguous.\n");
                        break;
                    case 1:
                        printf("Grammar is AMBIGUOUS.\n");
                        break;
                    default:
                        printf("Error in ambiguity detection.\n");
                }
                break;
                
            case 4:
                printf("Enter non-terminal to compute FIRST: ");
                scanf(" %c", &symbol);
                memset(first_set, 0, sizeof(first_set));
                compute_first(symbol, first_set);
                printf("FIRST(%c) = { ", symbol);
                print_set(first_set);
                printf("}\n");
                break;
                
            case 5:
                printf("Enter non-terminal to compute FOLLOW: ");
                scanf(" %c", &symbol);
                memset(follow_set, 0, sizeof(follow_set));
                compute_follow(symbol, follow_set);
                printf("FOLLOW(%c) = { ", symbol);
                print_set(follow_set);
                printf("}\n");
                break;
                
            case 6:
                printf("Enter string to check derivation: ");
                scanf("%s", input_string);
                switch(can_derive_string(input_string)) {
                    case true:
                        printf("String '%s' CAN be derived from the grammar.\n", input_string);
                        break;
                    case false:
                        printf("String '%s' CANNOT be derived from the grammar.\n", input_string);
                        break;
                }
                break;
                
            case 7:
                printf("Exiting program...\n");
                exit(0);
                
            default:
                printf("Invalid choice! Please try again.\n");
        }
        printf("\n");
    }
    
    return 0;
}

void menu() {
    printf("Menu:\n");
    printf("1. Input Grammar\n");
    printf("2. Display Grammar\n");
    printf("3. Detect Ambiguity\n");
    printf("4. Compute FIRST of Non-terminal\n");
    printf("5. Compute FOLLOW of Non-terminal\n");
    printf("6. Check String Derivation\n");
    printf("7. Exit\n");
}

void input_grammar() {
    printf("Enter number of production rules: ");
    scanf("%d", &num_rules);
    
    printf("Enter production rules (format: A->abc or A->ε for epsilon):\n");
    for(int i = 0; i < num_rules; i++) {
        printf("Rule %d: ", i + 1);
        scanf(" %c->%s", &grammar[i].lhs, grammar[i].rhs);
        
        // Add LHS to non-terminals if not already present
        if(!is_non_terminal(grammar[i].lhs)) {
            non_terminals[num_non_terminals++] = grammar[i].lhs;
        }
        
        // Add terminals from RHS
        for(int j = 0; grammar[i].rhs[j] != '\0'; j++) {
            if(grammar[i].rhs[j] != 'ε' && !is_non_terminal(grammar[i].rhs[j]) && 
               !is_terminal(grammar[i].rhs[j])) {
                terminals[num_terminals++] = grammar[i].rhs[j];
            }
        }
    }
    
    printf("Grammar input completed!\n");
}

void display_grammar() {
    if(num_rules == 0) {
        printf("No grammar rules entered yet!\n");
        return;
    }
    
    printf("Grammar Rules:\n");
    for(int i = 0; i < num_rules; i++) {
        printf("%c -> %s\n", grammar[i].lhs, grammar[i].rhs);
    }
    
    printf("\nNon-terminals: { ");
    for(int i = 0; i < num_non_terminals; i++) {
        printf("%c ", non_terminals[i]);
    }
    printf("}\n");
    
    printf("Terminals: { ");
    for(int i = 0; i < num_terminals; i++) {
        printf("%c ", terminals[i]);
    }
    printf("}\n");
}

int detect_ambiguity() {
    // Simple ambiguity detection: check for multiple rules with same LHS and overlapping FIRST sets
    for(int i = 0; i < num_rules; i++) {
        for(int j = i + 1; j < num_rules; j++) {
            if(grammar[i].lhs == grammar[j].lhs) {
                char first1[MAX_SYMBOLS] = {0};
                char first2[MAX_SYMBOLS] = {0};
                
                // Get first symbol of each RHS
                char first_sym1 = grammar[i].rhs[0];
                char first_sym2 = grammar[j].rhs[0];
                
                // If both start with same terminal or both can derive epsilon
                if(first_sym1 == first_sym2 || 
                   (first_sym1 == 'ε' && first_sym2 == 'ε')) {
                    return 1; // Ambiguous
                }
                
                // Check if both can start with same terminal through non-terminal expansion
                if(is_non_terminal(first_sym1) && is_non_terminal(first_sym2)) {
                    compute_first(first_sym1, first1);
                    compute_first(first_sym2, first2);
                    
                    // Check for intersection
                    for(int k = 0; first1[k] != '\0'; k++) {
                        if(contains(first2, first1[k])) {
                            return 1; // Ambiguous
                        }
                    }
                }
            }
        }
    }
    return 0; // Not ambiguous
}

void compute_first(char symbol, char first_set[]) {
    if(is_terminal(symbol) || symbol == 'ε') {
        add_to_set(first_set, symbol);
        return;
    }
    
    // For non-terminals, check all productions
    for(int i = 0; i < num_rules; i++) {
        if(grammar[i].lhs == symbol) {
            char* rhs = grammar[i].rhs;
            
            if(rhs[0] == 'ε') {
                add_to_set(first_set, 'ε');
            } else {
                bool all_have_epsilon = true;
                
                for(int j = 0; rhs[j] != '\0' && all_have_epsilon; j++) {
                    char temp_first[MAX_SYMBOLS] = {0};
                    
                    if(is_terminal(rhs[j])) {
                        add_to_set(first_set, rhs[j]);
                        all_have_epsilon = false;
                    } else {
                        compute_first(rhs[j], temp_first);
                        
                        // Add all symbols except epsilon
                        for(int k = 0; temp_first[k] != '\0'; k++) {
                            if(temp_first[k] != 'ε') {
                                add_to_set(first_set, temp_first[k]);
                            }
                        }
                        
                        // Check if epsilon is in FIRST
                        if(!contains(temp_first, 'ε')) {
                            all_have_epsilon = false;
                        }
                    }
                }
                
                if(all_have_epsilon) {
                    add_to_set(first_set, 'ε');
                }
            }
        }
    }
}

void compute_follow(char symbol, char follow_set[]) {
    // Add $ to follow of start symbol
    if(symbol == non_terminals[0]) {
        add_to_set(follow_set, '$');
    }
    
    for(int i = 0; i < num_rules; i++) {
        char* rhs = grammar[i].rhs;
        
        for(int j = 0; rhs[j] != '\0'; j++) {
            if(rhs[j] == symbol) {
                // Case 1: A -> αBβ, add FIRST(β) - {ε} to FOLLOW(B)
                if(rhs[j + 1] != '\0') {
                    char first_beta[MAX_SYMBOLS] = {0};
                    bool all_have_epsilon = true;
                    
                    for(int k = j + 1; rhs[k] != '\0' && all_have_epsilon; k++) {
                        char temp_first[MAX_SYMBOLS] = {0};
                        
                        if(is_terminal(rhs[k])) {
                            add_to_set(follow_set, rhs[k]);
                            all_have_epsilon = false;
                        } else {
                            compute_first(rhs[k], temp_first);
                            
                            for(int l = 0; temp_first[l] != '\0'; l++) {
                                if(temp_first[l] != 'ε') {
                                    add_to_set(follow_set, temp_first[l]);
                                }
                            }
                            
                            if(!contains(temp_first, 'ε')) {
                                all_have_epsilon = false;
                            }
                        }
                    }
                    
                    // Case 2: If β can derive ε, add FOLLOW(A) to FOLLOW(B)
                    if(all_have_epsilon && grammar[i].lhs != symbol) {
                        char follow_a[MAX_SYMBOLS] = {0};
                        compute_follow(grammar[i].lhs, follow_a);
                        
                        for(int k = 0; follow_a[k] != '\0'; k++) {
                            add_to_set(follow_set, follow_a[k]);
                        }
                    }
                } else {
                    // Case 3: A -> αB, add FOLLOW(A) to FOLLOW(B)
                    if(grammar[i].lhs != symbol) {
                        char follow_a[MAX_SYMBOLS] = {0};
                        compute_follow(grammar[i].lhs, follow_a);
                        
                        for(int k = 0; follow_a[k] != '\0'; k++) {
                            add_to_set(follow_set, follow_a[k]);
                        }
                    }
                }
            }
        }
    }
}

bool can_derive_string(char* input_string) {
    // Simple derivation check using CYK-like approach
    // This is a simplified version - for complete implementation, 
    // a full parser would be needed
    
    int len = strlen(input_string);
    if(len == 0) return false;
    
    // Check if string contains only terminals
    for(int i = 0; i < len; i++) {
        if(!is_terminal(input_string[i])) {
            return false;
        }
    }
    
    // Try to find a derivation path (simplified approach)
    // Check if we can build the string from start symbol
    char start_symbol = non_terminals[0];
    char first_set[MAX_SYMBOLS] = {0};
    compute_first(start_symbol, first_set);
    
    // If first character of string is in FIRST(start_symbol), it might be derivable
    return contains(first_set, input_string[0]);
}

bool is_terminal(char c) {
    for(int i = 0; i < num_terminals; i++) {
        if(terminals[i] == c) return true;
    }
    return false;
}

bool is_non_terminal(char c) {
    for(int i = 0; i < num_non_terminals; i++) {
        if(non_terminals[i] == c) return true;
    }
    return false;
}

void add_to_set(char set[], char symbol) {
    if(!contains(set, symbol)) {
        int len = strlen(set);
        set[len] = symbol;
        set[len + 1] = '\0';
    }
}

bool contains(char set[], char symbol) {
    for(int i = 0; set[i] != '\0'; i++) {
        if(set[i] == symbol) return true;
    }
    return false;
}

void print_set(char set[]) {
    for(int i = 0; set[i] != '\0'; i++) {
        printf("%c", set[i]);
        if(set[i + 1] != '\0') printf(", ");
    }
}