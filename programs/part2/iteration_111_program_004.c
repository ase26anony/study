/* test_target.c - Main test file to trigger dumpdir/dumpbase allocation */
/* Use pragmas to influence driver behavior */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

/* Attempt to set dump options via pragma (may not work in all GCC versions) */
#pragma GCC option "-dumpdir ./test_dumps/"
#pragma GCC option "-dumpbase test_target"

#pragma GCC diagnostic pop

/* Force optimization level changes to influence driver decisions */
__attribute__((optimize("O3"))) 
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Create a warning that can be controlled */
__attribute__((unused))
static int unused_variable;

/* Use constructor to ensure driver processes the whole pipeline */
static void __attribute__((constructor)) init_coverage(void) {
    /* This function runs before main, ensuring compilation proceeds */
}

/* Main function with trivial work */
int main(void) {
    int result = compute_factorial(5);
    
    /* Compile-time assertion to ensure compilation succeeds */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return (result == 120) ? 0 : 1;
}
