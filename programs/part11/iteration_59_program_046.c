/* test_targhooks.c - Comprehensive test to trigger target hooks coverage */

/* Force C++ mode for exception handling */
#ifdef __cplusplus
extern "C" {
#endif

/* Pattern 1: Hidden visibility external volatile variable */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 42;

/* Pattern 2: TLS with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* Pattern 3: Constructor with hidden visibility and builtin usage */
static void __attribute__((constructor, visibility("hidden"), noinline)) 
init_constructor(void) {
    /* Use volatile to force side effects */
    volatile int x = hidden_volatile;
    
    /* Complex builtin usage that might require internal helpers */
    if (__builtin_constant_p(x) && __builtin_expect(x > 0, 1)) {
        tls_var = x;
    }
    
    /* Another builtin that may generate internal symbols */
    __builtin_prefetch(&hidden_volatile, 0, 3);
}

/* Pattern 4: Destructor with similar attributes */
static void __attribute__((destructor, visibility("hidden"), noinline))
cleanup_destructor(void) {
    /* Use atomic builtin that may require internal helpers */
    __sync_fetch_and_add(&tls_var, 1);
}

/* Pattern 5: Artificial/ignored declaration candidate */
static inline void __attribute__((always_inline, unused, artificial))
internal_helper(void) {
    /* Unreachable code that might be marked artificial */
    if (__builtin_constant_p(0)) {
        __builtin_unreachable();
    }
}

/* Pattern 6: External nothrow function declaration */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Pattern 7: Complex function pointer type */
typedef void (*complex_func_ptr)(void) __attribute__((nothrow));

/* Pattern 8: Volatile function pointer */
extern volatile void (*volatile_ext_fn)(void);

#ifdef __cplusplus
} /* extern "C" */

/* C++ specific patterns */
#include <atomic>

/* Pattern 9: Atomic operations that may call internal helpers */
void atomic_operations(void) {
    std::atomic<int> atomic_var{0};
    atomic_var.store(1, std::memory_order_seq_cst);
    int val = atomic_var.load(std::memory_order_acquire);
    
    /* Use the value to prevent optimization */
    asm volatile("" : : "r"(val));
}

/* Pattern 10: Exception handling with external nothrow */
void test_exception_context(void) {
    try {
        external_nothrow_func();
    } catch (...) {
        /* Catch all - external_nothrow_func is marked nothrow,
           but compiler may still generate exception infrastructure */
    }
}

/* Pattern 11: Hidden visibility namespace */
namespace __attribute__((visibility("hidden"))) hidden_ns {
    extern "C" void hidden_extern_func(void);
}

/* Forward declaration for the hidden function */
extern "C" void hidden_ns::hidden_extern_func(void) {
    /* Empty implementation - but declared as extern "C" in hidden namespace */
}

#endif /* __cplusplus */

/* Main function orchestrating all patterns */
int main(void) {
    /* Force use of hidden volatile variable */
    int volatile_val = hidden_volatile;
    
    /* Use TLS variable */
    tls_var += volatile_val;
    
    /* Call internal helper */
    internal_helper();
    
#ifdef __cplusplus
    /* C++ specific patterns */
    atomic_operations();
    test_exception_context();
    
    /* Use hidden namespace function */
    hidden_ns::hidden_extern_func();
#endif

    /* Pattern 12: Complex builtin with volatile */
    volatile int condition = 1;
    if (__builtin_expect(condition, 0)) {
        volatile_val++;
    }
    
    /* Pattern 13: Address manipulation that might create internal symbols */
    void* tls_addr = &tls_var;
    void* volatile_addr = (void*)&hidden_volatile;
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long checksum = 
        (unsigned long)tls_addr ^ 
        (unsigned long)volatile_addr ^ 
        (unsigned long)volatile_val ^
        (unsigned long)tls_var;
    
    /* Use inline asm to ensure checksum is used */
    asm volatile("" : : "r"(checksum));
    
    return (int)(checksum & 0x7FFFFFFF);
}

/* Additional patterns outside main */

/* Pattern 14: #pragma GCC visibility */
#pragma GCC visibility push(hidden)
extern int pragma_hidden_var;
#pragma GCC visibility pop

int pragma_hidden_var = 123;

/* Pattern 15: Weak symbol with hidden visibility */
extern int __attribute__((weak, visibility("hidden"))) weak_hidden_symbol;
int weak_hidden_symbol = 456;

/* Pattern 16: Alias to builtin that might trigger internal symbol creation */
void my_memcpy(void* dest, const void* src, size_t n) 
    __attribute__((visibility("hidden"), noinline));
    
void my_memcpy(void* dest, const void* src, size_t n) {
    /* This might be optimized to __builtin_memcpy internally */
    char* d = (char*)dest;
    const char* s = (const char*)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

/* Pattern 17: Used attribute with external linkage */
int __attribute__((used, visibility("hidden"))) used_hidden_global = 789;

/* Pattern 18: Section attribute that might trigger special handling */
int __attribute__((section(".special_section"), visibility("hidden"))) 
    section_hidden_var = 999;
