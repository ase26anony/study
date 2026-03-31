/* Simple test program to generate gcov data */
#include <stdio.h>

void func1(int n) {
    for (int i = 0; i < n; i++) {
        printf("func1: %d\n", i);
    }
}

void func2(int n) {
    for (int i = 0; i < n; i++) {
        printf("func2: %d\n", i * 2);
    }
}

int main() {
    printf("Starting test program...\n");
    
    func1(5);
    func2(3);
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += i;
    }
    printf("Sum: %d\n", sum);
    
    return 0;
}
