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
    int result = 0;
    for (int i = 0; i < a; i++) {
        if (i % 2 == 0) {
            result += b;
        } else {
            result -= b;
        }
    }
    return result;
}

void func3(int mode) {
    switch (mode) {
        case 1:
            printf("Mode 1\n");
            break;
        case 2:
            printf("Mode 2\n");
            break;
        case 3:
            printf("Mode 3\n");
            break;
        default:
            printf("Unknown mode\n");
    }
}

int main(int argc, char *argv[]) {
    int input = 1;
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    printf("Running with input: %d\n", input);
    
    int r1 = func1(input);
    int r2 = func2(input, 3);
    func3(input % 4);
    
    printf("Results: %d, %d\n", r1, r2);
    return 0;
}
