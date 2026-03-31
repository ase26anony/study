/* Simple test program to generate gcov data */
#include <stdio.h>

void func1() {
    printf("Function 1\n");
    for (int i = 0; i < 10; i++) {
        printf("Loop iteration %d\n", i);
    }
}

void func2() {
    printf("Function 2\n");
    int x = 5;
    if (x > 0) {
        printf("Positive\n");
    } else {
        printf("Non-positive\n");
    }
}

int main() {
    printf("Starting test program\n");
    func1();
    func2();
    
    // Create some branching for coverage
    for (int i = 0; i < 3; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    
    printf("Test program completed\n");
    return 0;
}
