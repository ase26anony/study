/* test.c - Simple program to generate GCOV coverage data */
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

int function2(int x, int y) {
    int result = 0;
    for (int i = 0; i < x; i++) {
        if (i % 2 == 0) {
            result += y;
        } else {
            result -= y;
        }
    }
    return result;
}

void function3() {
    printf("Function3 called\n");
    // Some branching logic
    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            printf("First iteration\n");
        } else if (i == 1) {
            printf("Second iteration\n");
        } else {
            printf("Third iteration\n");
        }
    }
}

int main(int argc, char *argv[]) {
    int input = 0;
    
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    printf("Running with input: %d\n", input);
    
    int r1 = function1(input);
    printf("function1 result: %d\n", r1);
    
    int r2 = function2(input, 3);
    printf("function2 result: %d\n", r2);
    
    function3();
    
    // Additional branching based on input
    if (input > 10) {
        printf("Large input detected\n");
    } else if (input < -5) {
        printf("Very small input\n");
    } else {
        printf("Normal range input\n");
    }
    
    return 0;
}
