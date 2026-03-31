#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    printf("Factorial of 5: %d\n", factorial(5));
    printf("Factorial of 3: %d\n", factorial(3));
    return 0;
}
