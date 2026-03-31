/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -c test_targhooks.c -o test_targhooks.o */
/* For C++ mode: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force hidden visibility on external declarations */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int ext_hidden_volatile;

/* External function with nothrow attribute and hidden visibility */
extern void hidden_nothrow_func(void) __attribute__((nothrow));

/* Thread-local storage with initial-exec model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

#pragma GCC visibility pop

/* Define the external volatile variable */
volatile int ext_hidden_volatile = 0;

/* Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) hidden_constructor(void) {
    /* Use __builtin_expect with volatile to potentially trigger internal helpers */
    if (__builtin_expect(ext_hidden_volatile != 0, 0)) {
        /* This should never happen, but forces analysis */
        __builtin_unreachable();
    }
}

/* Destructor with hidden visibility */
static void __attribute__((destructor, visibility("hidden"))) hidden_destructor(void) {
    /* Access TLS variable */
    tls_var = 1;
}

/* Artificial function that might be ignored */
static inline __attribute__((unused, always_inline)) 
void artificial_helper(int x) {
    if (__builtin_constant_p(x)) {
        /* This branch may create artificial symbols */
        asm volatile ("# constant path" : : "r"(x));
    } else {
        asm volatile ("# non-constant path" : : "r"(x));
    }
}

/* Complex function pointer type */
typedef void (*complex_func_ptr)(void) __attribute__((nothrow));

#ifdef __cplusplus
extern "C" {
#endif

/* External C function declaration with nothrow - will remain external */
void external_c_nothrow(void) __attribute__((nothrow, visibility("hidden")));

/* Try to trigger exception handling infrastructure */
void test_exception_context(void) {
#ifdef __cplusplus
    try {
        /* Call external nothrow function */
        hidden_nothrow_func();
    } catch (...) {
        /* Catch everything */
    }
#endif
}

#ifdef __cplusplus
}
#endif

/* Main function orchestrates everything */
int main(void) {
    int result = 0;
    
    /* Access external volatile with hidden visibility */
    result += ext_hidden_volatile;
    
    /* Call the hidden nothrow function if available */
    void (*fn_ptr)(void) = (void (*)(void))hidden_nothrow_func;
    if (fn_ptr) {
        /* This keeps the reference alive */
        asm volatile ("# keep reference %0" : : "r"(fn_ptr));
    }
    
    /* Use TLS variable */
    tls_var = 42;
    result += tls_var;
    
    /* Use artificial helper */
    artificial_helper(result);
    
    /* Complex function pointer manipulation */
    complex_func_ptr cfp = 0;
    asm volatile ("# complex func ptr %0" : : "r"(cfp));
    
#ifdef __cplusplus
    /* Test exception context in C++ */
    test_exception_context();
#endif
    
    /* Prevent dead code elimination */
    asm volatile ("# result = %0" : : "r"(result));
    
    return result;
}

/* Define a weak symbol that might trigger artificial declarations */
extern int weak_hidden_var __attribute__((weak, visibility("hidden")));
int weak_hidden_var = 0;

/* Atomic operations that might call into libatomic */
#include <stdatomic.h>
void test_atomic(void) {
    atomic_int ai = ATOMIC_VAR_INIT(0);
    atomic_fetch_add_explicit(&ai, 1, memory_order_seq_cst);
    int val = atomic_load_explicit(&ai, memory_order_acquire);
    asm volatile ("# atomic val = %0" : : "r"(val));
}
