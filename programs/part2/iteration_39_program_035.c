/* test.c - Simple program to generate coverage data */
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
    if (x > y) {
        printf("x > y\n");
    } else if (x < y) {
        printf("x < y\n");
    } else {
        printf("x == y\n");
    }
}

int main(int argc, char *argv[]) {
    int input = 0;
    
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    func1(input);
    func2(input, input * 2);
    
    // Additional branching for coverage
    switch (input % 3) {
        case 0:
            printf("Case 0\n");
            break;
        case 1:
            printf("Case 1\n");
            break;
        case 2:
            printf("Case 2\n");
            break;
    }
    
    return 0;
}
