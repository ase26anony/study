/* test.c - Simple program to generate coverage data */
#include <stdio.h>
#include <stdlib.h>

int function1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
        return x * 2;
    } else {
        printf("Non-positive: %d\n", x);
        return x - 1;
    }
}

int function2(int y) {
    for (int i = 0; i < y; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    return y * 3;
}

void function3(const char* str) {
    if (str && str[0] != '\0') {
        printf("String: %s\n", str);
    } else {
        printf("Empty string\n");
    }
}

int main(int argc, char* argv[]) {
    int input = 1;
    
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    printf("Running with input: %d\n", input);
    
    int result1 = function1(input);
    int result2 = function2(input);
    
    if (input % 2 == 0) {
        function3("Even input");
    } else {
        function3("Odd input");
    }
    
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}
