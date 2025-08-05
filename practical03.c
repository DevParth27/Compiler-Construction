#include <stdio.h>
#include <ctype.h>
#include <string.h>

char *input;
int pos = 0;
int error = 0;

void E();
void Eprime();
void T();
void Tprime();
void F();

void match(char expected) {
    if (input[pos] == expected) {
        pos++;
    } else {
        printf("Syntax Error: Expected '%c' at position %d\n", expected, pos);
        error = 1;
    }
}

void E() {
    T();
    Eprime();
}

void Eprime() {
    if (input[pos] == '+') {
        match('+');
        T();
        Eprime();
    }
}

void T() {
    F();
    Tprime();
}
void Tprime() {
    if (input[pos] == '*') {
        match('*');
        F();
        Tprime();
    }
}
void F() {
    if (isalpha(input[pos])) {
        match(input[pos]);
    } else {
        printf("Syntax Error: Expected id at position %d\n", pos);
        error = 1;
    }
}

int main() {
    char expr[100];
    printf("Enter arithmetic expression: ");
    fgets(expr, sizeof(expr), stdin);
    expr[strcspn(expr, "\n")] = 0; 
    input = expr;
    pos = 0;
    error = 0;
    E();
    if (!error && input[pos] == '\0') {
        printf("String Accepted!\n");
    } else if (!error) {
        printf("Syntax Error: Unexpected character '%c' at position %d\n", input[pos], pos);
    } else {
        printf("String Rejected due to syntax errors.\n");
    }
    return 0;
}
