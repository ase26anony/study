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
    while (x > 0) {
        printf("Countdown: %d\n", x);
        x--;
    }
}

int main() {
    printf("Starting test program\n");
    
    func1();
    func2();
    
    // Create some branching for coverage
    int value = 1;
    if (value > 0) {
        printf("Positive value\n");
    } else {
        printf("Non-positive value\n");
    }
    
    printf("Test program completed\n");
    return 0;
}
