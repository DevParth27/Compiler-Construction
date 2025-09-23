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

// main
int main() {
    int choice;
    char symbol;
    char first_set[MAX_SYMBOLS];
    char follow_set[MAX_SYMBOLS];
    char input_string[MAX_STRING_LEN];

    printf("CFG Construction and FIRST/FOLLOW Computation (fixed-point, BFS derivation)\n");
    printf("=======================================================================\n\n");

    while (1) {
        menu();
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) { // handle bad input
            while (getchar() != '\n');
            printf("Invalid input. Try again.\n\n");
            continue;
        }
        while (getchar() != '\n'); // clear rest of line

        switch (choice) {
            case 1:
                input_grammar();
                // compute FIRST/FOLLOW after input
                compute_all_first();
                compute_all_follow();
                break;
            case 2:
                display_grammar();
                break;
            case 3:
                switch (detect_ambiguity()) {
                    case 0: printf("Grammar is NOT ambiguous (heuristic check).\n"); break;
                    case 1: printf("Grammar is AMBIGUOUS (heuristic check).\n"); break;
                    default: printf("Error in ambiguity detection.\n");
                }
                break;
            case 4:
                printf("Enter non-terminal to compute FIRST: ");
                if (scanf(" %c", &symbol) != 1) { printf("Bad input\n"); break; }
                while (getchar() != '\n');
                memset(first_set, 0, sizeof(first_set));
                compute_first_of_symbol(symbol, first_set);
                printf("FIRST(%c) = { ", symbol);
                print_set_str(first_set);
                printf(" }\n");
                break;
            case 5:
                printf("Enter non-terminal to compute FOLLOW: ");
                if (scanf(" %c", &symbol) != 1) { printf("Bad input\n"); break; }
                while (getchar() != '\n');
                memset(follow_set, 0, sizeof(follow_set));
                compute_follow_of_symbol(symbol, follow_set);
                printf("FOLLOW(%c) = { ", symbol);
                print_set_str(follow_set);
                printf(" }\n");
                break;
            case 6:
                printf("Enter string to check derivation (only terminals): ");
                if (scanf("%s", input_string) != 1) { printf("Bad input\n"); break; }
                while (getchar() != '\n');
                if (can_derive_string(input_string)) {
                    printf("String '%s' CAN be derived from the grammar.\n", input_string);
                } else {
                    printf("String '%s' CANNOT be derived from the grammar.\n", input_string);
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

void input_grammar() {
    char line[256];
    printf("Enter number of production rules: ");
    if (scanf("%d", &num_rules) != 1) {
        printf("Invalid number. Aborting input.\n");
        num_rules = 0;
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    if (num_rules <= 0 || num_rules > MAX_RULES) {
        printf("Number of rules must be between 1 and %d\n", MAX_RULES);
        num_rules = 0;
        return;
    }

    // reset sets
    num_non_terminals = 0;
    num_terminals = 0;
    for (int i=0;i<MAX_NT;i++) FIRST[i][0] = FOLLOW[i][0] = '\0';

    printf("Enter production rules (format: A->abc  or  A->ε for epsilon). Use single uppercase for non-terminals.\n");
    for (int i = 0; i < num_rules; ++i) {
        printf("Rule %d: ", i+1);
        if (!fgets(line, sizeof(line), stdin)) {
            printf("Input error\n"); num_rules = i; break;
        }
        trim_newline(line);
        if (strlen(line) < 3) { printf("Bad format. Try again.\n"); --i; continue; }

        // find "->"
        char *arrow = strstr(line, "->");
        if (!arrow || arrow == line) { printf("Bad format. Use A->rhs\n"); --i; continue; }
        char lhs = line[0];
        char *rhs = arrow + 2;
        if (*rhs == '\0') { printf("RHS empty. Try again.\n"); --i; continue; }

        grammar[i].lhs = lhs;
        strncpy(grammar[i].rhs, rhs, MAX_STRING_LEN-1);
        grammar[i].rhs[MAX_STRING_LEN-1] = '\0';

        // record non-terminal (lhs)
        if (!is_non_terminal_char(lhs)) {
            non_terminals[num_non_terminals++] = lhs;
        }

        // scan rhs for non-terminals and terminals
        for (int j = 0; grammar[i].rhs[j] != '\0'; ++j) {
            char ch = grammar[i].rhs[j];
            if (ch == ' ' || ch == '\t') continue;
            if (ch == 'ε') continue; // epsilon is not a terminal here
            if (isupper((unsigned char)ch)) {
                if (!is_non_terminal_char(ch)) non_terminals[num_non_terminals++] = ch;
            } else {
                // treat everything else (non uppercase, non-epsilon) as terminal
                bool found = false;
                for (int t=0;t<num_terminals;t++) if (terminals[t] == ch) { found = true; break; }
                if (!found) terminals[num_terminals++] = ch;
            }
        }
    }

    // compute FIRST and FOLLOW after reading grammar
    compute_all_first();
    compute_all_follow();
    printf("Grammar input completed!\n");
}

void display_grammar() {
    if (num_rules == 0) {
        printf("No grammar rules entered yet!\n");
        return;
    }
    printf("Grammar Rules:\n");
    for (int i=0;i<num_rules;i++) {
        printf("%c -> %s\n", grammar[i].lhs, grammar[i].rhs);
    }
    printf("\nNon-terminals: { ");
    for (int i=0;i<num_non_terminals;i++) printf("%c ", non_terminals[i]);
    printf("}\n");
    printf("Terminals: { ");
    for (int i=0;i<num_terminals;i++) printf("%c ", terminals[i]);
    printf("}\n");

    // show FIRST and FOLLOW for convenience
    compute_all_first();
    compute_all_follow();
    printf("\nComputed FIRST sets:\n");
    for (int i=0;i<num_non_terminals;i++) {
        printf("FIRST(%c) = { ", non_terminals[i]); print_set_str(FIRST[i]); printf(" }\n");
    }
    printf("\nComputed FOLLOW sets:\n");
    for (int i=0;i<num_non_terminals;i++) {
        printf("FOLLOW(%c) = { ", non_terminals[i]); print_set_str(FOLLOW[i]); printf(" }\n");
    }
}

int nt_index(char c) {
    for (int i=0;i<num_non_terminals;i++) if (non_terminals[i]==c) return i;
    return -1;
}

bool is_non_terminal_char(char c) {
    for (int i=0;i<num_non_terminals;i++) if (non_terminals[i]==c) return true;
    return false;
}

bool is_terminal_char(char c) {
    for (int i=0;i<num_terminals;i++) if (terminals[i]==c) return true;
    // treat 'ε' and '$' specially: not terminal, but potentially in sets
    return false;
}

void add_to_set_str(char set[], char symbol) {
    if (!contains_char(set, symbol)) {
        int len = strlen(set);
        set[len] = symbol;
        set[len+1] = '\0';
    }
}

bool contains_char(const char set[], char symbol) {
    if (!set) return false;
    for (int i=0; set[i] != '\0'; i++) if (set[i] == symbol) return true;
    return false;
}

void print_set_str(const char set[]) {
    for (int i=0; set[i] != '\0'; ++i) {
        printf("%c", set[i]);
        if (set[i+1] != '\0') printf(", ");
    }
}

// Iterative fixed-point FIRST computation for all non-terminals
void compute_all_first() {
    // clear FIRST
    for (int i=0;i<num_non_terminals;i++) FIRST[i][0] = '\0';

    bool changed = true;
    while (changed) {
        changed = false;
        for (int r=0;r<num_rules;r++) {
            char A = grammar[r].lhs;
            int Ai = nt_index(A);
            char *rhs = grammar[r].rhs;

            // handle epsilon-only production
            if (rhs[0] == 'ε' && rhs[1] == '\0') {
                if (!contains_char(FIRST[Ai], 'ε')) { add_to_set_str(FIRST[Ai], 'ε'); changed = true; }
                continue;
            }

            bool all_eps = true;
            for (int k=0; rhs[k] != '\0'; ++k) {
                char sym = rhs[k];
                if (sym == ' ') continue;
                if (isupper((unsigned char)sym)) {
                    int idx = nt_index(sym);
                    if (idx == -1) continue; // unknown non-terminal (shouldn't happen)
                    // add FIRST(sym) except epsilon
                    for (int p=0; FIRST[idx][p] != '\0'; ++p) {
                        char x = FIRST[idx][p];
                        if (x != 'ε' && !contains_char(FIRST[Ai], x)) { add_to_set_str(FIRST[Ai], x); changed = true; }
                    }
                    if (contains_char(FIRST[idx], 'ε')) {
                        // continue to next symbol
                        all_eps = all_eps && true;
                    } else {
                        all_eps = false;
                        break;
                    }
                } else {
                    // terminal
                    if (!contains_char(FIRST[Ai], sym)) { add_to_set_str(FIRST[Ai], sym); changed = true; }
                    all_eps = false;
                    break;
                }
            }
            if (all_eps) { // all symbols could derive epsilon
                if (!contains_char(FIRST[Ai], 'ε')) { add_to_set_str(FIRST[Ai], 'ε'); changed = true; }
            }
        }
    }
}

// Iterative fixed-point FOLLOW computation for all non-terminals
void compute_all_follow() {
    // clear follow
    for (int i=0;i<num_non_terminals;i++) FOLLOW[i][0] = '\0';
    if (num_non_terminals > 0) add_to_set_str(FOLLOW[0], '$'); // put $ in follow of start symbol (non_terminals[0])

    bool changed = true;
    while (changed) {
        changed = false;
        for (int r=0;r<num_rules;r++) {
            char A = grammar[r].lhs;
            int Ai = nt_index(A);
            char *rhs = grammar[r].rhs;
            int L = strlen(rhs);
            for (int i=0;i<L;i++) {
                char B = rhs[i];
                if (!isupper((unsigned char)B)) continue;
                int Bi = nt_index(B);
                // compute FIRST of beta (symbols after B)
                bool beta_all_eps = true;
                for (int k=i+1; k<L; ++k) {
                    char sym = rhs[k];
                    if (sym == ' ') continue;
                    if (isupper((unsigned char)sym)) {
                        int idx = nt_index(sym);
                        // add FIRST(sym) - epsilon to FOLLOW(B)
                        for (int p=0; FIRST[idx][p] != '\0'; ++p) {
                            char x = FIRST[idx][p];
                            if (x != 'ε' && !contains_char(FOLLOW[Bi], x)) {
                                add_to_set_str(FOLLOW[Bi], x); changed = true;
                            }
                        }
                        if (contains_char(FIRST[idx], 'ε')) {
                            // continue scanning
                            beta_all_eps = beta_all_eps && true;
                        } else {
                            beta_all_eps = false;
                            break;
                        }
                    } else {
                        // terminal: add it to FOLLOW(B)
                        if (!contains_char(FOLLOW[Bi], sym)) { add_to_set_str(FOLLOW[Bi], sym); changed = true; }
                        beta_all_eps = false;
                        break;
                    }
                }
                // If beta can derive epsilon (or B at end), add FOLLOW(A) to FOLLOW(B)
                if (beta_all_eps) {
                    for (int p=0; FOLLOW[Ai][p] != '\0'; ++p) {
                        char x = FOLLOW[Ai][p];
                        if (!contains_char(FOLLOW[Bi], x)) { add_to_set_str(FOLLOW[Bi], x); changed = true; }
                    }
                }
            }
        }
    }
}

void compute_first_of_symbol(char symbol, char out[]) {
    int idx = nt_index(symbol);
    if (idx == -1) { out[0] = '\0'; return; }
    strcpy(out, FIRST[idx]);
}

void compute_follow_of_symbol(char symbol, char out[]) {
    int idx = nt_index(symbol);
    if (idx == -1) { out[0] = '\0'; return; }
    strcpy(out, FOLLOW[idx]);
}

// Simple heuristic ambiguity detection:
// For each pair of productions with same LHS, if their FIRST sets intersect (including same starting terminal), mark ambiguous.
// This is only a heuristic — full ambiguity detection is undecidable in general.
int detect_ambiguity() {
    for (int i=0;i<num_rules;i++) {
        for (int j=i+1;j<num_rules;j++) {
            if (grammar[i].lhs == grammar[j].lhs) {
                // get first terminal sets for their RHS (compute FIRST of first symbol or use FIRST of non-terminal)
                char f1[MAX_SYMBOLS] = {0}, f2[MAX_SYMBOLS] = {0};
                // compute FIRST for first symbol (or symbol sequence)
                // We'll use small helper: if RHS begins with non-terminal use FIRST[nt], else terminal or epsilon
                char *r1 = grammar[i].rhs;
                char *r2 = grammar[j].rhs;
                // handle r1
                if (r1[0] == 'ε') add_to_set_str(f1, 'ε');
                else {
                    int p=0;
                    bool cont = true;
                    while (r1[p] != '\0' && cont) {
                        char s = r1[p];
                        if (s == ' ') { p++; continue; }
                        if (isupper((unsigned char)s)) {
                            int idx = nt_index(s);
                            if (idx>=0) {
                                for (int q=0; FIRST[idx][q] != '\0'; ++q) add_to_set_str(f1, FIRST[idx][q]);
                                cont = contains_char(FIRST[idx], 'ε');
                            } else { cont = false; }
                        } else {
                            add_to_set_str(f1, s);
                            cont = false;
                        }
                        p++;
                    }
                }
                // handle r2
                if (r2[0] == 'ε') add_to_set_str(f2, 'ε');
                else {
                    int p=0;
                    bool cont = true;
                    while (r2[p] != '\0' && cont) {
                        char s = r2[p];
                        if (s == ' ') { p++; continue; }
                        if (isupper((unsigned char)s)) {
                            int idx = nt_index(s);
                            if (idx>=0) {
                                for (int q=0; FIRST[idx][q] != '\0'; ++q) add_to_set_str(f2, FIRST[idx][q]);
                                cont = contains_char(FIRST[idx], 'ε');
                            } else { cont = false; }
                        } else {
                            add_to_set_str(f2, s);
                            cont = false;
                        }
                        p++;
                    }
                }
                // check intersection
                for (int a=0; f1[a] != '\0'; ++a) {
                    if (contains_char(f2, f1[a])) return 1;
                }
            }
        }
    }
    return 0;
}

// BFS-based derivation checking: we generate sentential forms from start by applying productions
// Stop conditions: queue size limit, max expansions, and terminal-length checks to avoid blowup.
bool can_derive_string(const char* input_string) {
    if (num_non_terminals == 0 || num_rules == 0) return false;
    int target_len = strlen(input_string);
    // Validate input contains only terminals known in grammar
    for (int i=0;i<target_len;i++) {
        char c = input_string[i];
        // ascii epsilon or uppercase are not allowed in input; also ensure it's among terminals (or allow unknown terminals)
        if (isupper((unsigned char)c) || c=='ε') return false;
    }

    // BFS queue of sentential forms
    char queue[MAX_QUEUE][MAX_STRING_LEN];
    int qhead = 0, qtail = 0;
    int expansions = 0;
    int max_expansions = 20000; // safety limit
    // start symbol is non_terminals[0]
    char start[3] = { non_terminals[0], '\0', '\0' };
    strcpy(queue[qtail++], start);

    while (qhead < qtail && expansions < max_expansions) {
        char cur[MAX_STRING_LEN];
        strcpy(cur, queue[qhead++]);
        expansions++;

        // If current is all terminals (no uppercase), compare
        bool all_term = true;
        int cur_term_count = 0;
        for (int i=0; cur[i] != '\0'; ++i) {
            if (isupper((unsigned char)cur[i])) { all_term = false; break; }
            if (cur[i] != ' ' && cur[i] != '\0') cur_term_count++;
        }
        if (all_term) {
            // compare directly
            if (strcmp(cur, input_string) == 0) return true;
            // skip if length differs
            if (cur_term_count > target_len) continue;
        }

        // prune if number of terminals seen so far exceeds target
        int terminals_count = 0;
        int nonterm_count = 0;
        for (int i=0; cur[i] != '\0'; ++i) {
            if (isupper((unsigned char)cur[i])) nonterm_count++;
            else if (cur[i] != ' ') terminals_count++;
        }
        if (terminals_count > target_len) continue;
        if (terminals_count + nonterm_count > 2*target_len + 5) continue; // heuristic prune

        // expand first non-terminal occurrence (or expand all occurrences - choose first to reduce branching)
        bool expanded_any = false;
        for (int pos = 0; cur[pos] != '\0'; ++pos) {
            char sym = cur[pos];
            if (!isupper((unsigned char)sym)) continue;
            // for each production with lhs == sym, construct new sentential form
            for (int r=0;r<num_rules;r++) {
                if (grammar[r].lhs != sym) continue;
                char nxt[MAX_STRING_LEN] = {0};
                // copy prefix up to pos
                int p=0;
                for (int k=0;k<pos;k++) nxt[p++] = cur[k];
                // append rhs (treat 'ε' as empty)
                if (!(grammar[r].rhs[0]=='ε' && grammar[r].rhs[1]=='\0')) {
                    for (int k=0; grammar[r].rhs[k] != '\0'; ++k) nxt[p++] = grammar[r].rhs[k];
                }
                // append rest of cur after pos
                for (int k=pos+1; cur[k] != '\0'; ++k) nxt[p++] = cur[k];
                nxt[p] = '\0';

                // queue new sentential form if within limits
                if (qtail < MAX_QUEUE) {
                    // further prune: do not push if too long (safeguard)
                    int termcount = 0;
                    for (int z=0; nxt[z] != '\0'; ++z) if (!isupper((unsigned char)nxt[z])) termcount++;
                    if (termcount <= target_len + 5) {
                        strcpy(queue[qtail++], nxt);
                    }
                }
            }
            expanded_any = true;
            break; // expand only first non-terminal in the current sentential form to limit branching (BFS will still explore)
        }
        // if no non-terminals and not equal to target, continue
    }
    return false;
}
