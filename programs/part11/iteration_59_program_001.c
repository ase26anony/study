/* test_targhooks.c - Designed to trigger target hooks for uncovered lines 981-990 */

/* Force C++ compilation for exception handling */
#ifdef __cplusplus
#include <atomic>
#endif

/* 1. Hidden visibility constructor with builtin usage */
static void __attribute__((constructor, visibility("hidden"), noinline)) 
hidden_constructor() {
    volatile int v = 0;
    /* Force compiler to generate internal helpers for __builtin_expect */
    if (__builtin_expect(v != 0, 0)) {
        __builtin_unreachable();
    }
}

/* 2. External volatile with hidden visibility */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 42;

/* 3. External nothrow function declaration */
#ifdef __cplusplus
extern "C" {
#endif
    void external_nothrow_func(void) __attribute__((nothrow));
#ifdef __cplusplus
}
#endif

/* 4. TLS with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* 5. Artificial/ignored declaration pattern */
static inline void __attribute__((unused, always_inline)) 
artificial_helper(int x) {
    if (x > 100) {
        __builtin_unreachable();
    }
}

/* 6. Complex function pointer type that may generate internal stubs */
typedef void (*complex_fp_t)(void) __attribute__((nothrow));
static complex_fp_t volatile fp_storage;

/* Main function orchestrating all patterns */
int main(void) {
    int checksum = 0;
    
    /* Access hidden volatile */
    checksum ^= hidden_volatile;
    
    /* Use TLS variable */
    checksum ^= tls_var;
    tls_var = checksum;
    
    /* Call artificial helper (may be optimized but parsed) */
    artificial_helper(checksum);
    
    /* Use complex function pointer */
    fp_storage = 0;
    
#ifdef __cplusplus
    /* C++ specific: exception handling with nothrow external */
    try {
        /* external_nothrow_func is declared but not defined - remains external */
        if (checksum == 0) {
            /* This may trigger compiler to generate exception handling stubs */
            asm volatile("" : : : "memory");
        }
    } catch (...) {
        checksum |= 1;
    }
    
    /* Atomic operation that may call into libatomic */
    std::atomic<int> atomic_var{0};
    atomic_var.store(42, std::memory_order_seq_cst);
    checksum ^= atomic_var.load(std::memory_order_relaxed);
#endif
    
    /* Force use of values to prevent optimization */
    asm volatile("" : "+r"(checksum) : : "memory");
    
    /* Use builtin with volatile argument */
    volatile int vol_arg = checksum;
    if (__builtin_constant_p(vol_arg)) {
        checksum |= 0x80000000;
    }
    
    return checksum & 0xFF;
}

/* Additional external declarations with hidden visibility */
#pragma GCC visibility push(hidden)
extern int hidden_external_var;
int hidden_external_var = 123;
#pragma GCC visibility pop

/* Namespace with hidden visibility (C++ only) */
#ifdef __cplusplus
namespace __hidden_internal {
    __attribute__((visibility("hidden")))
    extern "C" void internal_helper(void) {
        asm volatile("" : : : "memory");
    }
}
#endif
