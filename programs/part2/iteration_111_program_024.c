/* test_target.c - Main source file */
/* Use pragmas to influence driver behavior */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC optimize("O0")

/* Force dump directory and base name allocation through attributes */
__attribute__((optimize("O3"))) void optimized_function(void);
__attribute__((cold, noinline)) void cold_function(void);

/* Create conditions for dump file generation */
static volatile int force_optimization_barrier = 0;

void optimized_function(void) {
    /* Complex enough to trigger optimization decisions */
    double x = 3.14159;
    for (int i = 0; i < 100; i++) {
        x = x * 1.0001;
        force_optimization_barrier = (int)x;
    }
}

void cold_function(void) {
    /* This function should be placed in cold section */
    char buffer[64];
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i;
    }
}

/* Trigger coverage instrumentation */
__attribute__((constructor)) static void init_coverage(void) {
    /* This runs before main, engaging driver's constructor handling */
    volatile int dummy = 0;
    (void)dummy;
}

/* Main function with compile-time assertions */
int main(void) {
    /* Force some compiler analysis */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    /* Call both functions to ensure they're not optimized away */
    optimized_function();
    cold_function();
    
    /* Simple computation for coverage */
    int result = 0;
    for (int i = 1; i <= 10; i++) {
        result += i;
    }
    
    /* Return success */
    return result == 55 ? 0 : 1;
}

#pragma GCC diagnostic pop
