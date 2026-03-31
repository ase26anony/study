/* test_main.c - Main test file with various pragmas and attributes */
#include <stdio.h>
#include <stdlib.h>

/* Force dump directory allocation via pragma-like behavior */
/* Note: #pragma GCC option is not standard, so we'll rely on compilation flags */
/* But we can use other pragmas that affect driver behavior */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC optimize("O0")
#pragma GCC target("arch=x86-64")

/* Function with optimization attribute to influence driver decisions */
__attribute__((optimize("O3"), noinline, cold))
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Another function with different attributes */
__attribute__((optimize("Os"), hot, flatten))
int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

/* Force coverage instrumentation */
__attribute__((noinline))
void coverage_marker(void) {
    volatile int x = 0;
    (void)x;
}

/* Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init(void) {
    /* This ensures the driver goes through all compilation phases */
    static int initialized = 0;
    if (!initialized) {
        initialized = 1;
    }
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup(void) {
    /* Empty - just to engage driver */
}

int main(int argc, char **argv) {
    /* Simple computation to give compiler work */
    int result = 0;
    
    if (argc > 1) {
        int n = atoi(argv[1]);
        if (n > 0) {
            result = compute_factorial(n % 10);
            result += fibonacci(n % 10);
        }
    }
    
    coverage_marker();
    
    /* Compile-time assertion to ensure certain conditions */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
