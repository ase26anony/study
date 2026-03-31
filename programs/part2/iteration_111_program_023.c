/* test_main.c - Primary test file with various pragmas and attributes */
#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Use pragma to influence driver decisions */
#pragma GCC optimize("O0")
#pragma GCC option "-dumpdir ./test_dumps/"
#pragma GCC option "-dumpbase test_prog"

/* Pattern 2: Function with optimization attribute */
__attribute__((optimize("O3"))) 
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern 3: Constructor to ensure full pipeline processing */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures driver processes constructor code */
    printf("Starting coverage test...\n");
}

/* Pattern 4: Trigger warnings that can be controlled */
#ifdef __clang__
    /* Different compiler, skip */
#else
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-variable"
    /* Trigger but suppress unused variable warning */
    int unused_var = 42;
    #pragma GCC diagnostic pop
#endif

/* Pattern 5: Static assert to engage error handling */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* Main function with trivial work */
int main(void) {
    int result = compute_factorial(5);
    printf("Factorial of 5 is: %d\n", result);
    
    /* Return success */
    return 0;
}
