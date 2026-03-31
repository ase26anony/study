/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */

/* Force generation of hidden external symbols */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;
volatile int hidden_volatile_var = 0;

/* Hidden constructor that uses builtins with volatile */
static void __attribute__((constructor, noinline)) hidden_init(void) {
    /* Use __builtin_expect with volatile to potentially trigger helper generation */
    if (__builtin_expect(hidden_volatile_var != 0, 0)) {
        /* This should never happen but forces analysis */
        __builtin_unreachable();
    }
}

/* Hidden destructor */
static void __attribute__((destructor, noinline)) hidden_fini(void) {
    /* Access volatile to prevent optimization */
    asm volatile("" : : "r"(hidden_volatile_var));
}

/* External function declaration with nothrow and hidden visibility */
extern void hidden_nothrow_func(void) __attribute__((nothrow, visibility("hidden")));

/* TLS variable with explicit model - may generate internal helpers */
__thread int tls_var __attribute__((tls_model("initial-exec")));

/* Artificial inline function that might be ignored */
static inline __attribute__((unused, always_inline)) 
void artificial_helper(int x) {
    if (__builtin_constant_p(x)) {
        /* This branch might create artificial symbols */
        asm volatile("" : : "i"(x));
    }
}

#pragma GCC visibility pop

/* Now in default visibility */

/* Try to trigger exception handling infrastructure */
#ifdef __cplusplus
extern "C" {
#endif

/* Declare external C function that might need exception handling wrapper */
extern void external_c_func(void) __attribute__((nothrow));

#ifdef __cplusplus
}
#endif

/* Complex builtin usage that might require internal wrappers */
static int use_complex_builtins(void) {
    volatile int v = hidden_volatile_var;
    
    /* Multiple builtins that might trigger internal symbol creation */
    int r = __builtin_ffs(v);
    r += __builtin_popcount(v);
    r += __builtin_ctz(v | 1);  /* Avoid undefined behavior */
    
    /* Use artificial helper */
    artificial_helper(r);
    
    return r;
}

/* Function that uses TLS and volatile */
static int use_tls_and_volatile(void) {
    tls_var = hidden_volatile_var + 1;
    return tls_var;
}

/* Main function orchestrates everything */
int main(void) {
    int result = 0;
    
    /* Force use of hidden volatile */
    result += hidden_volatile_var;
    
    /* Use complex builtins */
    result += use_complex_builtins();
    
    /* Use TLS */
    result += use_tls_and_volatile();
    
    /* Try to reference external nothrow function (even if not defined) */
    /* This keeps it as an external reference */
    if (0) {
        hidden_nothrow_func();
        #ifdef __cplusplus
        try {
            external_c_func();
        } catch (...) {
            /* Empty handler */
        }
        #endif
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(result));
    
    return result;
}

/* Define the external function to avoid linker errors, but make it weak */
void __attribute__((weak, visibility("hidden"))) hidden_nothrow_func(void) {
    /* Empty implementation */
}

#ifdef __cplusplus
extern "C" void __attribute__((weak)) external_c_func(void) {
    /* Empty implementation */
}
#endif
