/* test1.c - Primary source file with various pragmas and attributes */
#include <stdio.h>
#include <stdlib.h>

/* Pattern: Use attributes that might influence dump file naming */
__attribute__((optimize("O3"))) 
__attribute__((noinline))
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern: Try to set dumpdir via pragma (non-standard but attempted) */
/* #pragma GCC option "-dumpdir ./test_dumps/" */
/* #pragma GCC option "-dumpbase test1" */

/* Pattern: Create a constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init(void) {
    /* This ensures the driver goes through all compilation phases */
}

/* Pattern: Include a benign error condition wrapped in #if 0 */
#if 0
    This would be a syntax error if enabled
    int x = ;  /* Malformed statement */
#endif

/* Pattern: Trigger warnings that could be promoted to errors with -Werror */
int unused_variable;  /* -Wunused-variable */

/* Pattern: Use _Static_assert for compile-time validation */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* Main function with coverage-friendly code */
int main(int argc, char **argv) {
    int result;
    
    if (argc > 1) {
        int n = atoi(argv[1]);
        if (n < 0 || n > 10) n = 5;
        result = compute_factorial(n);
    } else {
        result = compute_factorial(5);
    }
    
    printf("Result: %d\n", result);
    
    /* Pattern: Return different values for coverage */
    return (result > 100) ? 0 : 1;
}
