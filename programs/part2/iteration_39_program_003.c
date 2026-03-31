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
            printf("Even y: %d\n", i);
        } else {
            printf("Odd y: %d\n", i);
        }
    }
}

int main(int argc, char *argv[]) {
    int x = 1;
    int y = 2;
    
    if (argc > 1) {
        x = atoi(argv[1]);
        if (argc > 2) {
            y = atoi(argv[2]);
        }
    }
    
    func1(x);
    func2(x, y);
    
    return 0;
}
