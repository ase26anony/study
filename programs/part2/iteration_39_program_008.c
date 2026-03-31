/* test.c - Simple program to generate coverage data */
#include <stdio.h>
#include <stdlib.h>

int func1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
        return x * 2;
    } else {
        printf("Non-positive: %d\n", x);
        return x - 1;
    }
}

int func2(int a, int b) {
    if (a > b) {
        printf("a > b: %d > %d\n", a, b);
        return a - b;
    } else if (a < b) {
        printf("a < b: %d < %d\n", a, b);
        return b - a;
    } else {
        printf("a == b: %d == %d\n", a, b);
        return 0;
    }
}

void hot_function(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i;
        if (i % 2 == 0) {
            sum += 1;
        }
    }
    printf("Sum after %d iterations: %d\n", iterations, sum);
}

void cold_function() {
    printf("This function is rarely called\n");
}

int main(int argc, char *argv[]) {
    int input = 0;
    
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    printf("Running with input: %d\n", input);
    
    // Different execution paths based on input
    switch (input) {
        case 1:
            func1(10);
            func2(5, 3);
            hot_function(100);
            break;
        case 2:
            func1(-5);
            func2(2, 8);
            hot_function(50);
            break;
        case 3:
            func1(0);
            func2(7, 7);
            cold_function();
            break;
        default:
            func1(input);
            func2(input, input * 2);
            hot_function(10);
            break;
    }
    
    return 0;
}
