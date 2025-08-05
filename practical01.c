#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

bool isArithmeticOperator(char ch) {
    return (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || 
            ch == '=' || ch == '<' || ch == '>' || ch == '!' || ch == '&' || 
            ch == '|' || ch == '^' || ch == '~');
}

bool isMultiCharOperator(char *str, int pos) {
    if (pos < strlen(str) - 1) {
        char current = str[pos];
        char next = str[pos + 1];
        
        if ((current == '+' && next == '+') || 
            (current == '-' && next == '-') ||  
            (current == '=' && next == '=') ||  
            (current == '!' && next == '=') ||  
            (current == '<' && next == '=') ||  
            (current == '>' && next == '=') ||  
            (current == '&' && next == '&') ||  
            (current == '|' && next == '|') ||  
            (current == '<' && next == '<') ||  
            (current == '>' && next == '>'))   
            return true;
    }
    return false;
}

int countOperators(char *input) {
    int count = 0;
    int len = strlen(input);
    
    printf("\n TASK 4\n");
    printf("Input string: %s\n", input);
    printf("Operators found: ");
    
    for (int i = 0; i < len; i++) {
        if (isMultiCharOperator(input, i)) {
            printf("%.2s ", &input[i]);
            count++;
            i++; 
        }
        else if (isArithmeticOperator(input[i])) {
            printf("%c ", input[i]);
            count++;
        }
    }
    
    printf("\nTotal operators count: %d\n", count);
    return count;
}

bool validateInputString(char *input) {
    int len = strlen(input);
    bool isValid = true;
    
    printf("\n TASK 5\n");
    printf("Input string: \"%s\"\n", input);
    printf("Checking each character:\n");
    
    for (int i = 0; i < len; i++) {
        char ch = input[i];
        
        if (isalnum(ch)) {
            printf("'%c' - Valid (alphanumeric)\n", ch);
        }
        else if (isspace(ch)) {
            if (ch == ' ') {
                printf("' ' - Valid (whitespace)\n");
            } else if (ch == '\t') {
                printf("'\\t' - Valid (tab)\n");
            } else if (ch == '\n') {
                printf("'\\n' - Valid (newline)\n");
            } else {
                printf("'%c' - Valid (whitespace)\n", ch);
            }
        }
        else if (isArithmeticOperator(ch)) {
            printf("'%c' - Valid (arithmetic operator)\n", ch);
        }
        else {
            printf("'%c' - INVALID (not alphanumeric, whitespace, or arithmetic operator)\n", ch);
            isValid = false;
        }
    }
    
    printf("\nValidation result: %s\n", isValid ? "VALID" : "INVALID");
    return isValid;
}

int main() {
    char codeSnippet[256];
    char validateStr[256];
    printf("Enter code snippet to count operators: ");
    fgets(codeSnippet, sizeof(codeSnippet), stdin);

    printf("Enter string to validate: ");
    fgets(validateStr, sizeof(validateStr), stdin);

    countOperators(codeSnippet);
    validateInputString(validateStr);
    return 0;
}