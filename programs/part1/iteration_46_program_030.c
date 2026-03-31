#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int main() {
    printf("Testing coverage generation\n");
    
    // Generate some coverage data
    int fact = factorial(5);
    printf("Factorial of 5: %d\n", fact);
    
    int fib = fibonacci(6);
    printf("Fibonacci of 6: %d\n", fib);
    
    // Conditional coverage
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            printf("%d is even\n", i);
        } else {
            printf("%d is odd\n", i);
        }
    }
    
    return 0;
}
