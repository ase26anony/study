/* test.c - Simple program to generate coverage data */
#include <stdio.h>
#include <stdlib.h>

void func1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else {
        printf("Non-positive: %d\n", x);
    }
}

void func2(int x, int y) {
    for (int i = 0; i < x; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    
    if (y > 100) {
        printf("Large y: %d\n", y);
    } else if (y > 50) {
        printf("Medium y: %d\n", y);
    } else {
        printf("Small y: %d\n", y);
    }
}

int main(int argc, char *argv[]) {
    int input1 = 0;
    int input2 = 0;
    
    if (argc > 1) {
        input1 = atoi(argv[1]);
    }
    if (argc > 2) {
        input2 = atoi(argv[2]);
    }
    
    printf("Running with inputs: %d, %d\n", input1, input2);
    
    func1(input1);
    func2(input1, input2);
    
    return 0;
}
