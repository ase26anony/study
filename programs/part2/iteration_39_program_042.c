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

void function3() {
    printf("Function3 called\n");
    // Some branching logic
    int a = 1, b = 2;
    if (a < b) {
        printf("a < b\n");
    } else {
        printf("a >= b\n");
    }
}

int main(int argc, char *argv[]) {
    int input = 0;
    
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    printf("Input value: %d\n", input);
    
    int result1 = function1(input);
    printf("function1 result: %d\n", result1);
    
    int result2 = function2(input > 3 ? 3 : input);
    printf("function2 result: %d\n", result2);
    
    function3();
    
    return 0;
}
