/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */

/* Force hidden visibility for external declarations */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int ext_hidden_volatile;

/* External function with nothrow attribute */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Thread-local storage with initial-exec model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

/* Constructor with hidden visibility */
static void __attribute__((constructor, noinline)) hidden_constructor(void) {
    /* Use __builtin_expect with volatile to potentially trigger helper generation */
    volatile int cond = 1;
    if (__builtin_expect(cond, 0)) {
        /* Access the external volatile */
        int val = ext_hidden_volatile;
        (void)val;
    }
}

/* Destructor with hidden visibility */
static void __attribute__((destructor, noinline)) hidden_destructor(void) {
    /* Use TLS variable */
    tls_var = 42;
}

#pragma GCC visibility pop

/* Define the external volatile variable with hidden visibility */
volatile int ext_hidden_volatile __attribute__((visibility("hidden"))) = 1;

/* Artificial function that might be ignored */
static inline __attribute__((unused, always_inline)) 
void artificial_ignored_func(void) {
    /* Use __builtin_unreachable to create artificial control flow */
    if (ext_hidden_volatile > 100) {
        __builtin_unreachable();
    }
}

/* Complex function pointer type that might generate artificial declarations */
typedef void (*complex_func_ptr)(void) __attribute__((nothrow));

#ifdef __cplusplus
extern "C" {
#endif

/* External C function with nothrow - declared but not defined to keep it external */
void external_c_nothrow(void) __attribute__((nothrow, visibility("hidden")));

/* Function that uses try-catch in C++ mode */
#ifdef __cplusplus
void try_catch_function(void) {
    /* This may trigger generation of exception handling infrastructure */
    try {
        external_c_nothrow();
    } catch (...) {
        /* Catch all exceptions */
    }
}
#endif

#ifdef __cplusplus
}
#endif

/* Main function that uses all patterns */
int main(void) {
    /* Access external volatile */
    int val = ext_hidden_volatile;
    
    /* Use TLS variable */
    tls_var = val;
    
    /* Call artificial function */
    artificial_ignored_func();
    
    /* Use atomic operations that might call into libatomic */
    int atomic_var = 0;
    __atomic_store_n(&atomic_var, 42, __ATOMIC_SEQ_CST);
    
    /* Complex function pointer usage */
    complex_func_ptr fp = 0;
    (void)fp;
    
#ifdef __cplusplus
    /* Use try-catch if in C++ mode */
    try_catch_function();
#endif
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(val), "r"(tls_var), "r"(atomic_var));
    
    return 0;
}

/* Force generation of exception handling tables */
#ifdef __cplusplus
namespace hidden_visibility_ns __attribute__((visibility("hidden"))) {
    extern "C" void namespace_hidden_func(void) __attribute__((nothrow));
}
#endif
