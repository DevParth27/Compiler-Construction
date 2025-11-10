#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_EXPR_LEN 512
#define MAX_STACK_SIZE 256
#define MAX_TAC 1024

// Stacks for operators and operands (strings)
static char op_stack[MAX_STACK_SIZE][8];
static int op_top = -1;

static char operand_stack[MAX_STACK_SIZE][64];
static int operand_top = -1;

// 3-Address Code line
typedef struct {
    char op[8];
    char arg1[64];
    char arg2[64];
    char res[64];
} TAC;

static TAC before[MAX_TAC];
static TAC after[MAX_TAC];
static int before_count = 0;
static int after_count = 0;
static int temp_counter = 0;

static void trim(char *s) {
    // Trim leading
    int start = 0; while (isspace((unsigned char)s[start])) start++;
    int len = (int)strlen(s);
    int end = len - 1; while (end >= start && isspace((unsigned char)s[end])) end--;
    int newlen = end >= start ? (end - start + 1) : 0;
    memmove(s, s + start, newlen);
    s[newlen] = '\0';
}

static int precedence(char c) {
    switch (c) {
        case '+': case '-': return 1;
        case '*': case '/': return 2;
        default: return 0;
    }
}

static int is_operator_char(char c) {
    return c=='+' || c=='-' || c=='*' || c=='/';
}

static void push_op(const char *op) {
    if (op_top < MAX_STACK_SIZE-1) {
        ++op_top; strcpy(op_stack[op_top], op);
    }
}

static const char* pop_op() {
    if (op_top >= 0) return op_stack[op_top--];
    return "";
}

static void push_operand(const char *val) {
    if (operand_top < MAX_STACK_SIZE-1) {
        ++operand_top; strcpy(operand_stack[operand_top], val);
    }
}

static const char* pop_operand() {
    if (operand_top >= 0) return operand_stack[operand_top--];
    return "";
}

static void infix_to_postfix(const char *infix, char *postfix) {
    int i = 0, j = 0; op_top = -1;
    while (infix[i] != '\0') {
        if (isspace((unsigned char)infix[i])) { i++; continue; }

        if (isalnum((unsigned char)infix[i]) || infix[i]=='_') {
            // read identifier/number
            char tok[64]; int k=0;
            while (isalnum((unsigned char)infix[i]) || infix[i]=='_') tok[k++] = infix[i++];
            tok[k]='\0';
            if (j>0) postfix[j++]=' ';
            strcpy(postfix+j, tok);
            j += (int)strlen(tok);
        } else if (infix[i]=='(') {
            push_op("("); i++;
        } else if (infix[i]==')') {
            while (op_top>=0 && strcmp(op_stack[op_top],"(")!=0) {
                if (j>0) postfix[j++]=' ';
                const char *op = pop_op();
                strcpy(postfix+j, op);
                j += (int)strlen(op);
            }
            if (op_top>=0) pop_op(); // pop '('
            i++;
        } else if (is_operator_char(infix[i])) {
            char op[2] = { infix[i], '\0' };
            while (op_top>=0 && strcmp(op_stack[op_top],"(")!=0 &&
                   precedence(op_stack[op_top][0]) >= precedence(infix[i])) {
                if (j>0) postfix[j++]=' ';
                const char *popd = pop_op();
                strcpy(postfix+j, popd);
                j += (int)strlen(popd);
            }
            push_op(op); i++;
        } else if (infix[i]=='=') {
            // Treat '=' as separator for assignment; caller handles lhs separately
            i++;
        } else {
            // Skip any other char (e.g., ';')
            i++;
        }
    }
    while (op_top>=0) {
        if (j>0) postfix[j++]=' ';
        const char *op = pop_op();
        strcpy(postfix+j, op);
        j += (int)strlen(op);
    }
    postfix[j] = '\0';
}

static int is_number(const char *s) {
    if (*s=='\0') return 0;
    for (int i=0; s[i]; ++i) if (!isdigit((unsigned char)s[i])) return 0;
    return 1;
}

static void add_tac(TAC *arr, int *count, const char *op, const char *a1, const char *a2, const char *res) {
    strcpy(arr[*count].op, op);
    strcpy(arr[*count].arg1, a1);
    strcpy(arr[*count].arg2, a2);
    strcpy(arr[*count].res, res);
    (*count)++;
}

static void print_tac(const TAC *arr, int count) {
    for (int i=0;i<count;i++) {
        const TAC *t = &arr[i];
        if (strcmp(t->op, "=")==0) {
            printf("%s = %s\n", t->res, t->arg1);
        } else {
            printf("%s = %s %s %s\n", t->res, t->arg1, t->op, t->arg2);
        }
    }
}

static void postfix_to_tac_before(char *postfix, char *final_val_out) {
    operand_top = -1; before_count = 0; temp_counter = 0;
    char *tok = strtok(postfix, " ");
    while (tok) {
        if (strlen(tok)==1 && is_operator_char(tok[0])) {
            const char *b = pop_operand();
            const char *a = pop_operand();
            char tmp[16]; sprintf(tmp, "t%d", temp_counter++);
            add_tac(before, &before_count, tok, a, b, tmp);
            push_operand(tmp);
        } else {
            push_operand(tok);
        }
        tok = strtok(NULL, " ");
    }
    const char *final = pop_operand();
    strcpy(final_val_out, final);
}

static void postfix_to_tac_after(char *postfix, char *final_val_out) {
    operand_top = -1; after_count = 0; temp_counter = 0;
    char *tok = strtok(postfix, " ");
    while (tok) {
        if (strlen(tok)==1 && is_operator_char(tok[0])) {
            const char *b = pop_operand();
            const char *a = pop_operand();

            // Constant folding and algebraic simplifications
            if (is_number(a) && is_number(b)) {
                long va = strtol(a, NULL, 10);
                long vb = strtol(b, NULL, 10);
                long vr = 0;
                switch(tok[0]) {
                    case '+': vr = va + vb; break;
                    case '-': vr = va - vb; break;
                    case '*': vr = va * vb; break;
                    case '/': vr = vb!=0 ? (va / vb) : 0; break; // simple
                }
                char num[32]; sprintf(num, "%ld", vr);
                push_operand(num);
            } else if (tok[0]=='+') {
                if (is_number(a) && strcmp(a,"0")==0) {
                    push_operand(b);
                } else if (is_number(b) && strcmp(b,"0")==0) {
                    push_operand(a);
                } else {
                    char tmp[16]; sprintf(tmp, "t%d", temp_counter++);
                    add_tac(after, &after_count, tok, a, b, tmp);
                    push_operand(tmp);
                }
            } else if (tok[0]=='-') {
                if (is_number(b) && strcmp(b,"0")==0) {
                    push_operand(a);
                } else {
                    char tmp[16]; sprintf(tmp, "t%d", temp_counter++);
                    add_tac(after, &after_count, tok, a, b, tmp);
                    push_operand(tmp);
                }
            } else if (tok[0]=='*') {
                if ((is_number(a) && strcmp(a,"0")==0) || (is_number(b) && strcmp(b,"0")==0)) {
                    push_operand("0");
                } else if (is_number(a) && strcmp(a,"1")==0) {
                    push_operand(b);
                } else if (is_number(b) && strcmp(b,"1")==0) {
                    push_operand(a);
                } else {
                    char tmp[16]; sprintf(tmp, "t%d", temp_counter++);
                    add_tac(after, &after_count, tok, a, b, tmp);
                    push_operand(tmp);
                }
            } else if (tok[0]=='/') {
                if (is_number(b) && strcmp(b,"1")==0) {
                    push_operand(a);
                } else {
                    char tmp[16]; sprintf(tmp, "t%d", temp_counter++);
                    add_tac(after, &after_count, tok, a, b, tmp);
                    push_operand(tmp);
                }
            }
        } else {
            push_operand(tok);
        }
        tok = strtok(NULL, " ");
    }
    const char *final = pop_operand();
    strcpy(final_val_out, final);
}

int main() {
    printf("PRACTICAL 08: Code Optimization (Constant Folding & Simplifications)\n");
    printf("Enter an assignment or expression (e.g., x = 3*(2+5) + y*1 - 0):\n");

    char input[MAX_EXPR_LEN];
    if (!fgets(input, sizeof(input), stdin)) return 0;
    trim(input);

    // Split assignment if present
    char lhs[64] = ""; char rhs[MAX_EXPR_LEN];
    char *eq = strchr(input, '=');
    if (eq) {
        size_t L = (size_t)(eq - input);
        strncpy(lhs, input, L);
        lhs[L] = '\0'; trim(lhs);
        strcpy(rhs, eq+1); trim(rhs);
    } else {
        strcpy(rhs, input);
    }

    // Convert RHS to postfix
    char postfix[MAX_EXPR_LEN];
    infix_to_postfix(rhs, postfix);

    // Copy for two passes
    char pf1[MAX_EXPR_LEN]; strcpy(pf1, postfix);
    char pf2[MAX_EXPR_LEN]; strcpy(pf2, postfix);

    // Before optimization
    char final_before[64];
    postfix_to_tac_before(pf1, final_before);

    // After optimization
    char final_after[64];
    postfix_to_tac_after(pf2, final_after);

    // If assignment, add final move
    if (lhs[0] != '\0') {
        add_tac(before, &before_count, "=", final_before, "", lhs);
        add_tac(after, &after_count, "=", final_after, "", lhs);
    }

    printf("\nInput: %s\n", input);
    printf("Postfix: %s\n", postfix);

    printf("\n--- Before Optimization (3-Address Code) ---\n");
    print_tac(before, before_count);
    int ops_before = 0; for (int i=0;i<before_count;i++) if (strcmp(before[i].op,"=")!=0) ops_before++;

    printf("\n--- After Optimization (3-Address Code) ---\n");
    print_tac(after, after_count);
    int ops_after = 0; for (int i=0;i<after_count;i++) if (strcmp(after[i].op,"=")!=0) ops_after++;

    printf("\nResource Utilization Comparison:\n");
    printf("Arithmetic operations: %d -> %d (reduction: %d)\n", ops_before, ops_after, ops_before - ops_after);
    printf("Temporaries used may differ due to folding.\n");

    return 0;
}