#include <stdio.h>
#include <ctype.h>
#include <string.h>

static const char *input;
static int pos = 0;
static int error = 0;

static void skip_whitespace(void) {
    while (input[pos] == ' ' || input[pos] == '\t' || input[pos] == '\r')
        pos++;
}

static char peek(void) {
    skip_whitespace();
    return input[pos];
}

static void syntax_error(const char *msg) {
    if (!error) {
        fprintf(stderr, "Syntax Error at position %d: %s (got '%c')\n",
                pos, msg, input[pos] ? input[pos] : '#');
    }
    error = 1;
}

/* Forward declarations */
static void E(void);
static void Eprime(void);
static void T(void);
static void Tprime(void);
static void F(void);

static void E(void) {
    if (error) return;
    T();
    Eprime();
}

static void Eprime(void) {
    if (error) return;
    while (peek() == '+') {          // E' -> + T E' | ε
        pos++;                       // match '+'
        T();
        if (error) return;
        // loop continues to chain multiple + terms
    }
    // epsilon
}

static void T(void) {
    if (error) return;
    F();
    Tprime();
}

static void Tprime(void) {
    if (error) return;
    while (peek() == '*') {          // T' -> * F T' | ε
        pos++;                       // match '*'
        F();
        if (error) return;
        // loop continues to chain multiple * factors
    }
    // epsilon
}

static void F(void) {
    if (error) return;
    char c = peek();

    if (c == '(') {                  // F -> ( E )
        pos++;                       // match '('
        E();
        if (error) return;
        if (peek() == ')') {
            pos++;                   // match ')'
        } else {
            syntax_error("Expected ')'");
        }
    } else if (isalpha((unsigned char)c) || c == '_') {
        // F -> id (identifier starting with letter/_; then letters/digits/_)
        pos++; // consumed first char
        while (isalnum((unsigned char)input[pos]) || input[pos] == '_') pos++;
    } else if (c == '\0') {
        syntax_error("Unexpected end of input, expected id or '('");
    } else {
        syntax_error("Expected id or '('");
    }
}

int main(void) {
    char expr[256];

    printf("Enter arithmetic expression: ");
    if (!fgets(expr, sizeof(expr), stdin)) {
        fprintf(stderr, "Failed to read input.\n");
        return 1;
    }
    // strip trailing newline
    expr[strcspn(expr, "\n")] = '\0';

    input = expr;
    pos = 0;
    error = 0;

    E();
    skip_whitespace();

    if (!error && input[pos] == '\0') {
        printf("String Accepted!\n");
    } else if (!error) {
        fprintf(stderr, "Syntax Error: Unexpected character '%c' at position %d\n",
                input[pos], pos);
        error = 1;
    } else {
        printf("String Rejected due to syntax errors.\n");
    }

    // Keep the console open if run by double-clicking on Windows
    printf("Press Enter to exit...");
    getchar();
    return error ? 1 : 0;
}
