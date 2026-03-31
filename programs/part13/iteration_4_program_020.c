/* test_source.c - Simple program to generate gcov data */
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
    printf("Main function\n");
    func1();
    func2();
    
    // Create some branching for coverage
    int value = 10;
    if (value > 5) {
        printf("Value is greater than 5\n");
    } else {
        printf("Value is 5 or less\n");
    }
    
    return 0;
}
