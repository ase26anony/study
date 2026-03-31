/* test_targhooks.c - Test program to trigger specific target hooks in GCC */

/* Force C++ compilation for exception handling */
#ifdef __cplusplus
    #include <atomic>
    #include <cstdio>
#else
    #include <stdio.h>
#endif

/* Pattern 1: Hidden visibility constructor with builtin usage */
static void __attribute__((constructor, visibility("hidden"))) 
hidden_constructor() {
    volatile int v = 0;
    /* Use __builtin_expect with volatile to potentially create internal helpers */
    if (__builtin_expect(v != 0, 0)) {
        __builtin_unreachable();
    }
}

/* Pattern 2: External volatile with hidden visibility */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 42;

/* Pattern 3: Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* Pattern 4: Artificial function with unreachable code */
static inline void __attribute__((unused, always_inline)) 
artificial_helper(int x) {
    if (x > 1000) {
        __builtin_unreachable();
    }
}

/* Pattern 5: Complex function pointer type */
typedef void (*complex_fn_ptr)(void) __attribute__((nothrow));

/* Pattern 6: External nothrow function declaration */
#ifdef __cplusplus
extern "C" {
#endif
    void external_nothrow_func(void) __attribute__((nothrow));
#ifdef __cplusplus
}
#endif

/* Pattern 7: Volatile function pointer */
extern volatile void (* volatile volatile_fn_ptr)(void);

/* Use pragma to push hidden visibility */
#pragma GCC visibility push(hidden)
    extern int hidden_external_var;
    int hidden_external_var = 123;
#pragma GCC visibility pop

/* Pattern 8: Namespace with hidden visibility (C++ only) */
#ifdef __cplusplus
namespace hidden_ns __attribute__((visibility("hidden"))) {
    extern "C" void namespace_hidden_func(void);
}
#endif

/* Main function orchestrates all patterns */
int main() {
    int checksum = 0;
    
    /* Access hidden volatile */
    checksum += hidden_volatile;
    
    /* Use TLS variable */
    checksum += tls_var;
    
    /* Call artificial helper */
    artificial_helper(checksum);
    
    /* Use volatile function pointer if available */
    if (volatile_fn_ptr) {
        /* Just reference it, don't call (would crash) */
        checksum += (long)volatile_fn_ptr & 0xFF;
    }
    
    /* Access hidden external var */
    checksum += hidden_external_var;
    
#ifdef __cplusplus
    /* C++ specific patterns */
    try {
        /* Reference external nothrow function */
        if (external_nothrow_func) {
            checksum += 1;
        }
        
        /* Use atomic operations that may call internal helpers */
        std::atomic<int> atomic_var{0};
        atomic_var.store(1, std::memory_order_seq_cst);
        checksum += atomic_var.load(std::memory_order_seq_cst);
        
        /* Reference namespace hidden function */
        if (hidden_ns::namespace_hidden_func) {
            checksum += 2;
        }
    } catch (...) {
        checksum += 1000;
    }
    
    /* Use complex function pointer type */
    complex_fn_ptr fn = nullptr;
    checksum += (long)fn & 0xFF;
#endif
    
    /* Force side effects with inline assembly */
    asm volatile ("" : : "r"(checksum));
    
    printf("Checksum: %d\n", checksum);
    return checksum & 1;
}

/* Define some referenced symbols to avoid linker errors */
void external_nothrow_func(void) {
    /* Empty implementation */
}

#ifdef __cplusplus
namespace hidden_ns {
    extern "C" void namespace_hidden_func(void) {
        /* Empty implementation */
    }
}
#endif

volatile void (* volatile volatile_fn_ptr)(void) = 0;
