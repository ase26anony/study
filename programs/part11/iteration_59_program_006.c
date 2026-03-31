/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* For C++ mode: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force generation of hidden visibility external symbols */
#pragma GCC visibility push(hidden)

/* Pattern 1: External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;
volatile int hidden_volatile_var = 42;

/* Pattern 2: Function with constructor attribute and hidden visibility */
static void __attribute__((constructor, noinline)) hidden_constructor() {
    /* Use __builtin_expect with volatile to force internal handling */
    volatile int cond = 1;
    if (__builtin_expect(cond, 0)) {
        /* Access volatile to prevent optimization */
        asm volatile ("" : : "r"(hidden_volatile_var));
    }
}

/* Pattern 3: Artificial/ignored declaration through unused attribute */
static inline void __attribute__((unused, always_inline)) 
artificial_helper(int x) {
    if (__builtin_constant_p(x)) {
        __builtin_unreachable();
    }
}

#pragma GCC visibility pop

/* Pattern 4: TLS with explicit model - may generate internal helpers */
__thread int tls_var __attribute__((tls_model("initial-exec")));

/* Pattern 5: External function with nothrow attribute */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* C++ specific patterns */
#ifdef __cplusplus
extern "C" {
#endif

/* Pattern 6: Function pointer with volatile qualifiers */
extern volatile void (*volatile ext_fn_ptr)(void);

/* Pattern 7: Complex builtin usage that might generate internal wrappers */
int use_complex_builtin(void) {
    volatile long long v = 0x123456789ABCDEF0LL;
    /* __builtin_ffsll on volatile may need special handling */
    return __builtin_ffsll(v);
}

#ifdef __cplusplus
} /* extern "C" */

#include <atomic>

/* Pattern 8: Atomic operations that may call into libatomic */
void atomic_operation(std::atomic<int>& atom) {
    atom.fetch_add(1, std::memory_order_seq_cst);
}

/* Pattern 9: Try block with external nothrow function */
void try_external_nothrow() {
    try {
        external_nothrow_func();
    } catch (...) {
        /* Catch all but external_nothrow_func is marked nothrow */
    }
}

#endif /* __cplusplus */

/* Main function orchestrates all patterns */
int main(void) {
    int result = 0;
    
    /* Access hidden volatile variable */
    result += hidden_volatile_var;
    
    /* Use TLS variable */
    tls_var = 100;
    result += tls_var;
    
    /* Use complex builtin */
    result += use_complex_builtin();
    
    /* Call artificial helper (may be optimized out but parsed) */
    artificial_helper(result);
    
#ifdef __cplusplus
    /* C++ specific patterns */
    std::atomic<int> atom(0);
    atomic_operation(atom);
    result += atom.load();
    
    try_external_nothrow();
    
    /* Use volatile function pointer if defined */
    if (ext_fn_ptr) {
        /* This should remain as external reference */
        asm volatile ("# function pointer reference" : : "r"(ext_fn_ptr));
    }
#endif
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(result));
    
    return result != 0 ? 0 : 1;
}

/* Additional definitions to satisfy references */

/* Define the external nothrow function (but keep it external in separate TU) */
/* Comment out for testing - we want DECL_EXTERNAL to be true */
/*
void external_nothrow_func(void) {
    // Empty implementation
}
*/

/* Define volatile function pointer */
volatile void (*volatile ext_fn_ptr)(void) = 0;

/* Additional test case that might trigger the specific lines */
/* A hidden, external, artificial symbol that's used but volatile */
__attribute__((visibility("hidden"), used, noinline))
extern volatile int artificial_hidden_volatile;

volatile int artificial_hidden_volatile = 0xDEADBEEF;

/* Function that uses the artificial symbol in a way that might trigger flags */
static void __attribute__((noinline)) use_artificial_symbol(void) {
    /* Complex expression with volatile access */
    int val = artificial_hidden_volatile;
    if (__builtin_expect(val != 0, 1)) {
        asm volatile ("# using artificial symbol" : : "r"(val));
    }
}

/* Call it from a constructor to ensure it's processed early */
static void __attribute__((constructor)) init_use(void) {
    use_artificial_symbol();
}
