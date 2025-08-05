#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_IDENTIFIERS 100
#define MAX_LENGTH 50
#define MAX_KEYWORDS 32

typedef struct {
    char name[MAX_LENGTH];
    char datatype[MAX_LENGTH];
    char scope[MAX_LENGTH];
    int memory_usage;
    int line_number;
} Symbol;

typedef struct {
    Symbol table[MAX_IDENTIFIERS];
    int count;
} SymbolTable;

static const char KEYWORDS[MAX_KEYWORDS][MAX_LENGTH] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
};

static SymbolTable symbol_table = { .count = 0 };
bool is_keyword(const char *word);
bool is_valid_identifier(const char *word);
int find_symbol(const char *name, const char *scope);
int get_memory_size(const char *datatype);
void skip_array_tokens(char **token);

void add_symbol(const char *name, const char *datatype, const char *scope, int line_number);
void parse_declaration(const char *line, int line_number);
void process_input();
void display_symbol_table();

bool is_keyword(const char *word) {
    if (!word) return false;
    
    for (int i = 0; i < MAX_KEYWORDS; i++) {
        if (strcmp(word, KEYWORDS[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool is_valid_identifier(const char *word) {
    if (!word || strlen(word) == 0) return false;
    
    if (!isalpha(word[0]) && word[0] != '_') {
        return false;
    }
    
    for (size_t i = 1; i < strlen(word); i++) {
        if (!isalnum(word[i]) && word[i] != '_') {
            return false;
        }
    }
    
    return true;
}

int find_symbol(const char *name, const char *scope) {
    for (int i = 0; i < symbol_table.count; i++) {
        if (strcmp(symbol_table.table[i].name, name) == 0 && 
            strcmp(symbol_table.table[i].scope, scope) == 0) {
            return i;
        }
    }
    return -1;
}

int get_memory_size(const char *datatype) {
    if (!datatype) return 4;
    
    if (strcmp(datatype, "char") == 0) return 1;
    if (strcmp(datatype, "short") == 0) return 2;
    if (strcmp(datatype, "int") == 0) return 4;
    if (strcmp(datatype, "long") == 0) return 8;
    if (strcmp(datatype, "float") == 0) return 4;
    if (strcmp(datatype, "double") == 0) return 8;
    if (strstr(datatype, "array") != NULL) return 4;
    
    return 4;
}

void skip_array_tokens(char **token) {
    while (*token && strcmp(*token, "]") != 0) {
        *token = strtok(NULL, " \t\n;,()");
    }
}

void add_symbol(const char *name, const char *datatype, const char *scope, int line_number) {
    if (symbol_table.count >= MAX_IDENTIFIERS) {
        fprintf(stderr, "Error: Symbol table is full!\n");
        return;
    }
    
    int existing = find_symbol(name, scope);
    if (existing != -1) {
        printf("Error: Multiple declaration of '%s' in scope '%s'\n", name, scope);
        printf("       First declared at line %d, redeclared at line %d\n", 
               symbol_table.table[existing].line_number, line_number);
        return;
    }
    
    if (is_keyword(name)) {
        printf("Error: Cannot use keyword '%s' as identifier at line %d\n", name, line_number);
        return;
    }
    
    Symbol *sym = &symbol_table.table[symbol_table.count++];
    strncpy(sym->name, name, MAX_LENGTH);
    strncpy(sym->datatype, datatype, MAX_LENGTH);
    strncpy(sym->scope, scope, MAX_LENGTH);
    sym->memory_usage = get_memory_size(datatype);
    sym->line_number = line_number;
    
    printf("Added identifier '%s' to symbol table\n", name);
}

void parse_declaration(const char *line, int line_number) {
    char buffer[256];
    strncpy(buffer, line, sizeof(buffer));
    buffer[sizeof(buffer)-1] = '\0';
    
    char *token = strtok(buffer, " \t\n;,()");
    if (!token) return;
    
    char datatype[MAX_LENGTH] = "";
    strncpy(datatype, token, MAX_LENGTH);
    const char *scope = "global";
    
    while ((token = strtok(NULL, " \t\n;,()")) != NULL) {
        if (!is_valid_identifier(token)) continue;
        
        char final_datatype[MAX_LENGTH];
        strncpy(final_datatype, datatype, MAX_LENGTH);
        
        char *next_token = strtok(NULL, " \t\n;,()");
        if (next_token && (strcmp(next_token, "[") == 0 || strchr(line, '[') != NULL)) {
            strncat(final_datatype, "_array", MAX_LENGTH - strlen(final_datatype) - 1);
            skip_array_tokens(&next_token);
        }
        
        add_symbol(token, final_datatype, scope, line_number);
    }
}

void display_symbol_table() {
    printf("\n");
    printf("====================================================================================\n");
    printf("                              SYMBOL TABLE                                         \n");
    printf("====================================================================================\n");
    printf("| %-15s | %-12s | %-8s | %-10s | %-5s |\n", 
           "Identifier", "Data Type", "Scope", "Memory(B)", "Line");
    printf("|-----------------+-------------+----------+------------+-------|\n");
    
    for (int i = 0; i < symbol_table.count; i++) {
        printf("| %-15s | %-12s | %-8s | %-10d | %-5d |\n",
               symbol_table.table[i].name,
               symbol_table.table[i].datatype,
               symbol_table.table[i].scope,
               symbol_table.table[i].memory_usage,
               symbol_table.table[i].line_number);
    }
    printf("====================================================================================\n");
    printf("Total identifiers: %d\n", symbol_table.count);
}

void process_input() {
    char line[256];
    int line_number = 1;
    
    printf("Symbol Table Constructor and Error Detector\n");
    printf("============================================\n\n");
    printf("Enter C declarations (one per line). Enter 'END' to finish:\n");
    
    while (true) {
        printf("Line %d: ", line_number);
        if (!fgets(line, sizeof(line), stdin)) break;
        
        line[strcspn(line, "\n")] = '\0';
        
        if (strcmp(line, "END") == 0) break;
        if (strlen(line) == 0) continue;
        
        parse_declaration(line, line_number);
        line_number++;
    }
}

int main() {
    process_input();
    display_symbol_table();
    printf("\nProgram completed successfully!\n");
    return 0;
}