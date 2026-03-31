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
    }
}

int main(int argc, char *argv[]) {
    int input = 0;
    
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    printf("Input value: %d\n", input);
    
    func1(input);
    func2(input, input * 10);
    
    if (input % 3 == 0) {
        printf("Divisible by 3\n");
    } else if (input % 3 == 1) {
        printf("Remainder 1\n");
    } else {
        printf("Remainder 2\n");
    }
    
    return 0;
}
