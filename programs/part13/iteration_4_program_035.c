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
    for (int i = 0; i < 3; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    
    return 0;
}
