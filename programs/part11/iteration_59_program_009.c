/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */

/* Force generation of hidden external symbols */
#pragma GCC visibility push(hidden)

/* External volatile with hidden visibility - triggers DECL_EXTERNAL + VISIBILITY_HIDDEN */
extern volatile int hidden_volatile_ext;

/* External nothrow function with hidden visibility */
extern void hidden_nothrow_func(void) __attribute__((nothrow));

/* External function pointer - volatile to force usage */
extern volatile void (*volatile hidden_fn_ptr)(void);

#pragma GCC visibility pop

/* Define the external volatile to satisfy linker */
volatile int hidden_volatile_ext = 1;

/* Thread-local storage with explicit model - may generate internal helpers */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 42;

/* Constructor with hidden visibility - static but runs before main */
static void __attribute__((constructor, visibility("hidden"))) hidden_constructor(void) {
    /* Use __builtin with volatile argument - may generate internal wrappers */
    volatile int cond = 0;
    if (__builtin_expect(cond, 0)) {
        __builtin_unreachable();
    }
    
    /* Access external volatile */
    int tmp = hidden_volatile_ext + 1;
    
    /* Use asm to prevent optimization */
    __asm__ volatile ("" : : "r"(tmp));
}

/* Artificial function marked unused - may be ignored */
static inline void __attribute__((unused, artificial)) artificial_helper(void) {
    __builtin_unreachable();
}

/* C++ specific code to trigger exception handling paths */
#ifdef __cplusplus
extern "C" {
#endif

/* Function that uses the nothrow external in a try context */
void use_nothrow_in_try(void) {
    /* Declare external C nothrow function */
    extern void ext_c_nothrow(void) __attribute__((nothrow));
    
    /* In C++ mode, this would be in a try block */
    #ifdef __cplusplus
    try {
        ext_c_nothrow();
    } catch (...) {
        /* Catch all */
    }
    #else
    /* In C, just call it */
    ext_c_nothrow();
    #endif
}

#ifdef __cplusplus
}
#endif

/* Main function orchestrates all patterns */
int main(void) {
    /* Force use of hidden_constructor */
    /* Constructor runs before main, but reference it to ensure inclusion */
    void (*ctor_ref)(void) = &hidden_constructor;
    __asm__ volatile ("" : : "r"(ctor_ref));
    
    /* Access TLS variable - may generate TLS initialization helpers */
    int tls_val = tls_var;
    
    /* Use external volatile */
    int volatile_val = hidden_volatile_ext;
    
    /* Call the nothrow function user */
    use_nothrow_in_try();
    
    /* Use external function pointer if available */
    if (hidden_fn_ptr) {
        hidden_fn_ptr();
    }
    
    /* Use atomic operations (C++ style or via builtins) */
    int atomic_val = 0;
    __atomic_store_n(&atomic_val, tls_val + volatile_val, __ATOMIC_SEQ_CST);
    
    /* Call artificial helper */
    artificial_helper();
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = tls_val + volatile_val + atomic_val;
    
    /* Use checksum in a way that can't be optimized away */
    __asm__ volatile ("" : : "r"(checksum));
    
    return checksum != 0 ? 0 : 1;
}

/* Additional definitions to satisfy references */

/* Define the hidden nothrow function */
void __attribute__((visibility("hidden"))) hidden_nothrow_func(void) {
    /* Empty but marked nothrow */
}

/* Define external C nothrow function */
void ext_c_nothrow(void) {
    /* Empty implementation */
}

/* Define the hidden function pointer */
void (*volatile hidden_fn_ptr)(void) = 0;
