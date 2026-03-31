/* test_targhooks.c - Test program to cover lines 981-990 in targhooks.cc */

/* Force C++ compilation for exception handling */
#ifdef __cplusplus
#include <atomic>
#include <cstdio>
#else
#include <stdio.h>
#endif

/* Pattern 1: Hidden visibility constructor with builtin usage */
static void __attribute__((constructor, visibility("hidden"), noinline)) 
hidden_constructor() {
    volatile int v = 42;
    /* Use __builtin_constant_p with volatile to force internal handling */
    if (__builtin_constant_p(v)) {
        /* This branch won't be taken, but creates analysis context */
        asm volatile ("nop");
    }
    /* Use __builtin_expect with volatile condition */
    if (__builtin_expect(v != 0, 1)) {
        asm volatile ("nop" : : : "memory");
    }
}

/* Pattern 2: External volatile with hidden visibility */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 123;

/* Pattern 3: Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 456;

/* Pattern 4: Artificial/ignored declaration with unreachable code */
static inline void __attribute__((unused, always_inline)) 
artificial_helper() {
    /* This creates an artificial control flow path */
    if (__builtin_unreachable(), 0) {
        /* Never reached */
    }
}

/* Pattern 5: External function with nothrow attribute */
#ifdef __cplusplus
extern "C" {
#endif
    void external_nothrow_func() __attribute__((nothrow));
#ifdef __cplusplus
}
#endif

/* Forward declaration for the external function */
void external_nothrow_func() {
    /* Empty implementation - keeps it external for declaration */
}

/* Pattern 6: Complex function pointer type that may generate artificial types */
typedef void (*complex_func_ptr)(void) __attribute__((nothrow, visibility("hidden")));
complex_func_ptr hidden_func_ptr = 0;

/* Pattern 7: Volatile function pointer */
extern volatile void (*volatile ext_volatile_fn)(void);
volatile void (*volatile ext_volatile_fn)(void) = 0;

/* Pattern 8: Use #pragma for hidden visibility on external declarations */
#pragma GCC visibility push(hidden)
extern int hidden_external_var;
#pragma GCC visibility pop
int hidden_external_var = 789;

/* Pattern 9: Atomic operations that may call into libatomic */
#ifdef __cplusplus
std::atomic<int> atomic_var{0};
#endif

/* Pattern 10: Static function with hidden visibility and used attribute */
static void __attribute__((used, visibility("hidden"))) 
static_but_used() {
    asm volatile ("nop" : : : "memory");
}

/* Main function orchestrates all patterns */
int main() {
    int checksum = 0;
    
    /* Access hidden volatile variable */
    checksum ^= hidden_volatile;
    
    /* Access TLS variable */
    checksum ^= tls_var;
    
    /* Call artificial helper */
    artificial_helper();
    
    /* Use volatile function pointer */
    if (ext_volatile_fn) {
        ext_volatile_fn();
    }
    
    /* Access hidden external variable */
    checksum ^= hidden_external_var;
    
    /* Use hidden function pointer */
    if (hidden_func_ptr) {
        hidden_func_ptr();
    }
    
    /* Call static but used function */
    static_but_used();
    
#ifdef __cplusplus
    /* C++ specific patterns */
    
    /* Atomic operations */
    atomic_var.store(1, std::memory_order_seq_cst);
    checksum ^= atomic_var.load(std::memory_order_seq_cst);
    
    /* Exception handling with nothrow external function */
    try {
        external_nothrow_func();
    } catch (...) {
        /* Should not catch since function is nothrow */
    }
    
    /* Use std::atomic_thread_fence which may generate internal calls */
    std::atomic_thread_fence(std::memory_order_acq_rel);
#endif
    
    /* Force use of builtins with side effects */
    checksum += __builtin_popcount(checksum);
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(checksum));
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}

/* Additional patterns in separate compilation unit context */
/* This simulates what might happen during LTO */

/* Pattern 11: Weak symbol with hidden visibility */
extern int __attribute__((weak, visibility("hidden"))) weak_hidden_symbol;
int __attribute__((weak, visibility("hidden"))) weak_hidden_symbol = 999;

/* Pattern 12: Alias to builtin with hidden visibility */
extern void __attribute__((alias("printf"), visibility("hidden"))) 
hidden_printf_alias(const char*, ...);

/* Pattern 13: Section attribute with hidden visibility */
int __attribute__((section(".hidden_section"), visibility("hidden"), used))
section_hidden_var = 1111;

/* Pattern 14: Noinline function that uses many builtins */
static int __attribute__((noinline, visibility("hidden")))
complex_builtin_user(int x) {
    volatile int v = x;
    int result = 0;
    
    result += __builtin_ffs(v);
    result += __builtin_clz(v);
    result += __builtin_ctz(v);
    result += __builtin_parity(v);
    
    /* Force conditional with builtin */
    if (__builtin_expect(result > 0, 1)) {
        result = __builtin_bswap32(result);
    }
    
    /* Use __builtin_constant_p in a way that can't be determined at compile time */
    if (!__builtin_constant_p(x)) {
        result ^= __builtin_sqrt(result);
    }
    
    return result;
}

/* Call the complex function to ensure it's processed */
static void __attribute__((constructor))
call_complex_func() {
    int r = complex_builtin_user(42);
    asm volatile ("" : : "r"(r));
}
