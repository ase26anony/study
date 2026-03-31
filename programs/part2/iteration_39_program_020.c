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
    int a = rand() % 100;
    if (a < 50) {
        printf("Less than 50\n");
    } else if (a < 75) {
        printf("50-74\n");
    } else {
        printf("75-99\n");
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
    function3();
    
    printf("Results: %d, %d\n", r1, r2);
    
    return 0;
}
