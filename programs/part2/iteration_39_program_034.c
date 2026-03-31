/* test.c - Simple test program for gcov-tool testing */
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

int function2(int y, int z) {
    int result = 0;
    for (int i = 0; i < y; i++) {
        if (i % 2 == 0) {
            result += z;
        } else {
            result -= z;
        }
    }
    return result;
}

void function3(int a) {
    switch (a) {
        case 1:
            printf("Case 1\n");
            break;
        case 2:
            printf("Case 2\n");
            break;
        default:
            printf("Default case: %d\n", a);
            break;
    }
}

int main(int argc, char *argv[]) {
    int input = 1;
    
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    printf("Running with input: %d\n", input);
    
    int r1 = function1(input);
    int r2 = function2(input, 3);
    function3(input);
    
    printf("Results: %d, %d\n", r1, r2);
    
    return 0;
}
