/* coverage_source.c - Simple program to generate GCOV data */
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int i, sum = 0;
    
    for (i = 0; i < 10; i++) {
        sum += i;
    }
    
    printf("Sum: %d\n", sum);
    printf("Factorial of 5: %d\n", factorial(5));
    
    return 0;
}
