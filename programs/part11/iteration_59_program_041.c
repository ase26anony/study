/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* For C++: g++ -O2 -fvisibility=hidden -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force hidden visibility on external declarations */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;

/* External function with nothrow attribute */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

/* Function with constructor attribute and hidden visibility */
static void __attribute__((constructor, used)) hidden_constructor(void) {
    /* Use __builtin with volatile argument */
    if (__builtin_constant_p((volatile int)hidden_volatile_var)) {
        /* This branch won't be taken, but forces analysis */
        tls_var = 1;
    }
}

#pragma GCC visibility pop

/* Define the volatile variable (still with hidden visibility) */
volatile int hidden_volatile_var __attribute__((visibility("hidden"))) = 42;

/* Define TLS variable */
__thread int tls_var = 0;

/* Try to trigger exception handling helpers */
#ifdef __cplusplus
extern "C" {
#endif

/* Declare an external function that might need wrapper */
void __attribute__((nothrow)) potential_helper(void);

#ifdef __cplusplus
}
#endif

/* Complex builtin usage that might require helper generation */
static inline int __attribute__((always_inline)) use_complex_builtin(int x) {
    /* __builtin_expect with volatile to prevent optimization */
    volatile int v = x;
    return __builtin_expect(v, 0) ? 1 : 0;
}

/* Artificial function that might be ignored */
static void __attribute__((unused, artificial)) artificial_helper(void) {
    __builtin_unreachable();
}

/* Main function that uses all patterns */
int main(void) {
    int result = 0;
    
    /* Access volatile with hidden visibility */
    result += hidden_volatile_var;
    
    /* Use TLS variable */
    result += tls_var;
    
    /* Use complex builtin */
    result += use_complex_builtin(result);
    
    /* Try to call external nothrow function */
    /* This remains an external reference, might trigger helper */
    external_nothrow_func();
    
#ifdef __cplusplus
    /* In C++ mode, try exception handling with external function */
    try {
        potential_helper();
    } catch (...) {
        result++;
    }
#endif
    
    /* Use atomic operations that might need libatomic helpers */
    {
        int atomic_var = 0;
        /* This might call into libatomic */
        __atomic_store_n(&atomic_var, result, __ATOMIC_SEQ_CST);
        result = __atomic_load_n(&atomic_var, __ATOMIC_SEQ_CST);
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile ("" : : "r"(result));
    
    return result;
}

/* Define a dummy external function to satisfy references */
void external_nothrow_func(void) {
    /* Do nothing */
}

#ifdef __cplusplus
void potential_helper(void) {
    /* Do nothing */
}
#endif
