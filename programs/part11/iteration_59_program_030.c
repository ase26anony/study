/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */

/* Force hidden visibility for external declarations */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;

/* External function with nothrow attribute */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

#pragma GCC visibility pop

/* Define the volatile variable */
volatile int hidden_volatile_var = 1;

/* Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) init_func(void) {
    /* Use builtin with volatile argument */
    if (__builtin_constant_p(hidden_volatile_var)) {
        /* This branch won't be taken, but forces analysis */
        asm volatile ("nop");
    }
    
    /* Use __builtin_expect with volatile */
    if (__builtin_expect(hidden_volatile_var != 0, 1)) {
        tls_var = 1;
    }
}

/* Artificial function that might be ignored */
static inline void __attribute__((unused, always_inline)) 
artificial_helper(void) {
    /* Complex builtin usage */
    if (__builtin_unreachable(), 0) {
        /* Never reached */
    }
}

/* Function with complex exception handling */
#ifdef __cplusplus
extern "C" {
#endif

void try_except_func(void) {
    /* Try to trigger exception handling infrastructure */
    volatile int *ptr = 0;
    
    /* This might generate internal exception handling symbols */
    if (hidden_volatile_var) {
        /* Access through volatile pointer */
        asm volatile ("" : : "r"(ptr));
    }
    
    /* Call external nothrow function */
    external_nothrow_func();
}

#ifdef __cplusplus
}
#endif

/* Main function orchestrates everything */
int main(void) {
    /* Force use of all patterns */
    
    /* 1. Use volatile external variable */
    int val = hidden_volatile_var + 1;
    
    /* 2. Use TLS variable */
    tls_var = val;
    
    /* 3. Call function with exception handling context */
    try_except_func();
    
    /* 4. Use artificial helper */
    artificial_helper();
    
    /* 5. Complex builtin usage that might generate helpers */
    int result = __builtin_ffs(val) + __builtin_popcount(tls_var);
    
    /* 6. Atomic operation that might use libatomic helpers */
    __atomic_store_n(&tls_var, result, __ATOMIC_SEQ_CST);
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(result));
    
    return result;
}

/* Define external function to avoid undefined reference */
void external_nothrow_func(void) {
    /* Empty but defined */
}
