/* test_targhooks.c - Test program to trigger target-specific hooks */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* For C++ mode: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force external linkage with hidden visibility */
#pragma GCC visibility push(hidden)

/* Pattern 1: External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;
volatile int hidden_volatile_var = 42;

/* Pattern 2: External function with nothrow attribute and hidden visibility */
extern void hidden_nothrow_func(void) __attribute__((nothrow, visibility("hidden")));

/* Pattern 3: Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

/* Pattern 4: Function pointer with volatile qualifier */
extern volatile void (* volatile hidden_fn_ptr)(void);

#pragma GCC visibility pop

/* Pattern 5: Constructor with hidden visibility and builtin usage */
static void __attribute__((constructor, visibility("hidden"), noinline)) 
hidden_constructor(void) {
    /* Use __builtin_expect with volatile condition */
    volatile int cond = hidden_volatile_var;
    if (__builtin_expect(cond != 0, 1)) {
        /* Use __builtin_constant_p on non-constant expression */
        int result = __builtin_constant_p(cond) ? 1 : 0;
        /* Force side effect */
        asm volatile("" : "+r"(result));
    }
}

/* Pattern 6: Artificial/ignored declaration */
static inline void __attribute__((unused, always_inline)) 
artificial_helper(void) {
    /* This may generate artificial symbols */
    __builtin_unreachable();
}

/* Pattern 7: Complex builtin that may require internal wrapper */
static int use_complex_builtin(int x) {
    /* __builtin_add_overflow may generate internal helpers */
    int result;
    if (__builtin_add_overflow(x, hidden_volatile_var, &result)) {
        return 0;
    }
    return result;
}

#ifdef __cplusplus
extern "C" {
#endif

/* Pattern 8: C++ exception handling with external nothrow function */
#ifdef __cplusplus
#include <new>
#include <atomic>

/* External C function marked nothrow */
extern "C" void external_nothrow_func(void) __attribute__((nothrow));

/* Atomic operation that may call libatomic helpers */
void atomic_operation(void) {
    std::atomic<int> atomic_var{0};
    atomic_var.store(42, std::memory_order_seq_cst);
    int val = atomic_var.load(std::memory_order_acquire);
    
    /* Try-catch with external nothrow function */
    try {
        external_nothrow_func();
    } catch (...) {
        /* Should not be reached if function is truly nothrow */
        atomic_var.store(val + 1, std::memory_order_relaxed);
    }
}
#endif

/* Pattern 9: External volatile function pointer usage */
void use_volatile_fn_ptr(void) {
    if (hidden_fn_ptr) {
        /* Indirect call through volatile pointer */
        hidden_fn_ptr();
    }
}

/* Pattern 10: Complex typedef that may generate artificial symbols */
typedef void (*complex_fn_ptr)(int, ...) __attribute__((nothrow));

/* Main function orchestrates all patterns */
int main(void) {
    int checksum = 0;
    
    /* Access hidden volatile variable */
    checksum += hidden_volatile_var;
    
    /* Use complex builtin */
    checksum += use_complex_builtin(checksum);
    
    /* Access TLS variable */
    tls_var = checksum;
    checksum += tls_var;
    
    /* Use volatile function pointer pattern */
    use_volatile_fn_ptr();
    
    /* Call artificial helper (though it's unreachable) */
    artificial_helper();
    
#ifdef __cplusplus
    /* C++ specific patterns */
    atomic_operation();
#endif
    
    /* Take address of various symbols to ensure they're used */
    checksum += (int)(long)&hidden_volatile_var;
    checksum += (int)(long)&tls_var;
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(checksum));
    
    return checksum & 0xFF; /* Return non-zero to prevent optimization */
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* Additional definitions to satisfy external references */

/* Define the hidden nothrow function */
void hidden_nothrow_func(void) {
    /* Empty but marked with attributes */
    asm volatile("" : : : "memory");
}

/* Define the volatile function pointer */
volatile void (* volatile hidden_fn_ptr)(void) = 0;

#ifdef __cplusplus
/* Define the external C nothrow function */
extern "C" void external_nothrow_func(void) {
    /* Empty implementation */
}
#endif
