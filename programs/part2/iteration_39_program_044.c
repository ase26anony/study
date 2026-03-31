/* test.c - Simple test program for gcov-tool testing */
#include <stdio.h>
#include <stdlib.h>

void func1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else {
        printf("Non-positive: %d\n", x);
    }
}

void func2(int a, int b) {
    for (int i = 0; i < a; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    
    if (b > 100) {
        printf("Large b: %d\n", b);
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
    
    // Different execution paths based on input
    if (input == 1) {
        printf("Path A\n");
    } else if (input == 2) {
        printf("Path B\n");
    } else {
        printf("Path C\n");
    }
    
    return 0;
}
