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
        if (y % 2 == 0) {
            printf("Even y: %d\n", y);
        } else {
            printf("Odd y: %d\n", y);
        }
    }
}

int main(int argc, char *argv[]) {
    int input = 0;
    
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    func1(input);
    func2(input, input * 2);
    
    return 0;
}
