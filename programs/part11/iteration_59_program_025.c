/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* For C++: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force generation of hidden external artificial symbols */
#ifdef __cplusplus
extern "C" {
#endif

/* Pattern 1: External volatile with hidden visibility */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 0;

/* Pattern 2: TLS with explicit model that may need runtime helpers */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 42;

/* Pattern 3: Function with nothrow attribute but external linkage */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Pattern 4: Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) hidden_init(void) {
    /* Use builtin with volatile to force internal helpers */
    if (__builtin_constant_p(hidden_volatile)) {
        /* This branch won't be taken but forces analysis */
        asm volatile ("nop");
    }
}

/* Pattern 5: Artificial function that might be generated */
static inline __attribute__((unused, always_inline)) 
void artificial_helper(int x) {
    /* Use volatile asm to prevent optimization */
    asm volatile ("" : "+r" (x));
    if (x) __builtin_unreachable();
}

#ifdef __cplusplus
} /* extern "C" */

/* C++ specific patterns */
#include <atomic>

/* Pattern 6: Atomic operations that may call libatomic helpers */
void atomic_operation(std::atomic<int>& atom) {
    atom.store(42, std::memory_order_seq_cst);
}

/* Pattern 7: Exception handling with external nothrow function */
void try_external_nothrow() {
    try {
        external_nothrow_func();
    } catch (...) {
        /* Catch everything - external_nothrow_func is marked nothrow,
           but compiler might generate exception handling infrastructure */
    }
}

#endif

/* Pattern 8: Complex builtin usage */
static int use_complex_builtins(void) {
    volatile int v = hidden_volatile;
    int result = 0;
    
    /* __builtin_expect with volatile condition */
    if (__builtin_expect(v != 0, 0)) {
        result = 1;
    }
    
    /* __builtin_constant_p on non-constant expression */
    if (!__builtin_constant_p(v + tls_var)) {
        result |= 2;
    }
    
    /* Use artificial helper */
    artificial_helper(result);
    
    return result;
}

/* Pattern 9: Function pointer with volatile and external linkage */
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*func_ptr)(void);
extern volatile func_ptr volatile_ext_fn __attribute__((visibility("hidden")));

#ifdef __cplusplus
}
#endif

/* Pattern 10: Forced usage of all patterns */
int main(void) {
    int checksum = 0;
    
    /* Access hidden volatile */
    checksum ^= hidden_volatile;
    
    /* Access TLS */
    checksum ^= tls_var;
    
    /* Use complex builtins */
    checksum ^= use_complex_builtins();
    
#ifdef __cplusplus
    /* C++ specific patterns */
    std::atomic<int> atom;
    atomic_operation(atom);
    checksum ^= atom.load(std::memory_order_relaxed);
    
    try_external_nothrow();
#endif
    
    /* Use volatile external function pointer if available */
    if (volatile_ext_fn) {
        /* This should remain external even if null */
        asm volatile ("# function pointer exists %0" : : "r"((void*)volatile_ext_fn));
    }
    
    /* Prevent dead code elimination */
    asm volatile ("# checksum = %0" : : "r"(checksum));
    
    return checksum & 0xFF;
}

/* Additional definitions to satisfy references */
volatile func_ptr volatile_ext_fn = 0;

#ifdef __cplusplus
extern "C" {
#endif

/* Define the external nothrow function */
void external_nothrow_func(void) {
    /* Empty but marked nothrow */
}

#ifdef __cplusplus
}
#endif
