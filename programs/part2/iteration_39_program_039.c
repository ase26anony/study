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

int function2(int x, int y) {
    for (int i = 0; i < x; i++) {
        if (i % 2 == 0) {
            y += i;
        } else {
            y -= i;
        }
    }
    return y;
}

void function3(int threshold) {
    int sum = 0;
    for (int i = 1; i <= threshold; i++) {
        if (i % 3 == 0) {
            sum += i;
        } else if (i % 5 == 0) {
            sum -= i;
        }
    }
    printf("Function3 result: %d\n", sum);
}

int main(int argc, char *argv[]) {
    int input = 1;
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    printf("Running with input: %d\n", input);
    
    int result1 = function1(input);
    int result2 = function2(input, input * 10);
    
    if (input > 5) {
        function3(input);
    }
    
    printf("Results: %d, %d\n", result1, result2);
    return 0;
}
