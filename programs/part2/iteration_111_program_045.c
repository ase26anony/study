/* test_coverage.c - Main test file */
/* Pattern: Use #pragma to influence driver behavior */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC optimize("O0")

/* Force dump directory/base allocation through attributes */
__attribute__((noinline)) 
__attribute__((optimize("O3"))) 
static void optimized_function(void) {
    /* Empty but forces different optimization levels */
}

/* Constructor to ensure driver processes entire pipeline */
__attribute__((constructor)) 
static void init(void) {
    /* Marker for driver activity */
    static volatile int marker = 0;
    marker++;
}

/* Main computation to give compiler work */
static int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

/* Function with different attribute to influence dump naming */
__attribute__((cold))
__attribute__((section(".special")))
static void cold_function(void) {
    volatile int x = 42;
    (void)x;
}

/* Trigger warnings that might affect driver behavior */
__attribute__((unused))
static void unused_function(void) {
    /* This will trigger -Wunused-function unless compiled with -w */
}

int main(void) {
    /* Use computed value to prevent optimization */
    volatile int result = factorial(5);
    
    /* Call functions to ensure they're not eliminated */
    optimized_function();
    cold_function();
    
    /* Return success */
    return result != 120; /* factorial(5) = 120 */
}

#pragma GCC diagnostic pop
