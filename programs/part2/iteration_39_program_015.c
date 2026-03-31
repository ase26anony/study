/* test.c - Simple program with multiple functions and branches for coverage testing */
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
    for (int i = 1; i <= 10; i++) {
        if (i > threshold) {
            sum += i;
            printf("Adding %d (sum=%d)\n", i, sum);
        }
    }
    printf("Final sum: %d\n", sum);
}

int main(int argc, char *argv[]) {
    int input = 1;
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    printf("Running with input: %d\n", input);
    
    int result1 = function1(input);
    printf("function1 result: %d\n", result1);
    
    int result2 = function2(input, 10);
    printf("function2 result: %d\n", result2);
    
    function3(input);
    
    return 0;
}
