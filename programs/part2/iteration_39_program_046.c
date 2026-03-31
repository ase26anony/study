/* test.c - Simple test program for gcov-tool testing */
#include <stdio.h>
#include <stdlib.h>

void func1(int x) {
    printf("func1: %d\n", x);
    if (x > 0) {
        printf("Positive\n");
    } else {
        printf("Non-positive\n");
    }
}

void func2(int x, int y) {
    printf("func2: %d, %d\n", x, y);
    for (int i = 0; i < x; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
}

int main(int argc, char *argv[]) {
    int input = 0;
    
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    printf("Running with input: %d\n", input);
    
    func1(input);
    
    if (input > 5) {
        func2(input, input * 2);
    } else {
        func2(3, 6);
    }
    
    return 0;
}
