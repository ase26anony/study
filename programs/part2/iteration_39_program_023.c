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

void function3(int threshold) {
    if (threshold > 100) {
        printf("High threshold\n");
    } else if (threshold > 50) {
        printf("Medium threshold\n");
    } else {
        printf("Low threshold\n");
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
    function3(input * 20);
    
    printf("Results: %d, %d\n", r1, r2);
    return 0;
}
