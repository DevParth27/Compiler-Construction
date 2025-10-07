%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_RULES 100
#define MAX_SYMBOLS 64
#define MAX_STRING_LEN 200
#define MAX_NT 26  // assume single uppercase letters as non-terminals
#define MAX_QUEUE 10000
#define MAX_PROD_RHS 100

typedef struct {
    char lhs;
    char rhs[MAX_STRING_LEN];
} Production;

Production grammar[MAX_RULES];
int num_rules = 0;

char non_terminals[MAX_NT];
int num_non_terminals = 0;

char terminals[MAX_SYMBOLS];
int num_terminals = 0;

// FIRST and FOLLOW stored as strings (each char a symbol; 'ε' for epsilon, '$' for end)
char FIRST[MAX_NT][MAX_SYMBOLS];
char FOLLOW[MAX_NT][MAX_SYMBOLS];

// Function prototypes
void menu();
void input_grammar();
void display_grammar();
int detect_ambiguity();
void compute_all_first();
void compute_all_follow();
void compute_first_of_symbol(char symbol, char out[]);
void compute_follow_of_symbol(char symbol, char out[]);
int nt_index(char c);
bool is_non_terminal_char(char c);
bool is_terminal_char(char c);
void add_to_set_str(char set[], char symbol);
bool contains_char(const char set[], char symbol);
void print_set_str(const char set[]);
bool can_derive_string(const char* input_string);
void trim_newline(char* s);
%}

%%
// Lexical rules section
[A-Z]        { /* Non-terminal symbol */ }
[a-z]        { /* Terminal symbol */ }
"->"         { /* Production arrow */ }
"ε"          { /* Epsilon */ }
[ \t\n]      { /* Ignore whitespace */ }
.            { /* Any other character */ }

%%

int main(int argc, char **argv) {
    printf("CFG Construction and FIRST/FOLLOW Computation (Lex version)\n");
    printf("==========================================================\n\n");
    menu();
    return 0;
}

// Copy all helper functions from practical04.c below
void menu() {
    printf("Menu:\n");
    printf("1. Input Grammar\n");
    printf("2. Display Grammar\n");
    printf("3. Detect Ambiguity (simple heuristic)\n");
    printf("4. Compute FIRST of Non-terminal\n");
    printf("5. Compute FOLLOW of Non-terminal\n");
    printf("6. Check String Derivation (BFS up to limits)\n");
    printf("7. Exit\n");
}

void trim_newline(char* s) {
    size_t L = strlen(s);
    if (L == 0) return;
    if (s[L-1] == '\n') s[L-1] = '\0';
    if (L > 1 && s[L-2] == '\r') s[L-2] = '\0';
}

// ... (rest of your functions from practical04.c go here)