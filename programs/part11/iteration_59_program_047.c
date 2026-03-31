/* test_targhooks.c - Comprehensive test to trigger target hook coverage */

/* Force C++ mode for exception handling */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdatomic.h>
#include <stdio.h>

/* Pattern 1: Hidden visibility external volatile variable */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 42;

/* Pattern 2: TLS with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* Pattern 3: External nothrow function declaration */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Pattern 4: Volatile function pointer */
extern volatile void (* volatile ext_fn_ptr)(void);

/* Pattern 5: Constructor with hidden visibility and builtin usage */
static void __attribute__((constructor, visibility("hidden"), noinline)) 
init_constructor(void) {
    /* Use __builtin_expect with volatile condition */
    volatile int cond = 1;
    if (__builtin_expect(cond, 0)) {
        /* Use __builtin_constant_p on non-constant expression */
        int x = hidden_volatile;
        if (!__builtin_constant_p(x)) {
            /* Force side effects */
            asm volatile("" : : "r"(x));
        }
    }
}

/* Pattern 6: Destructor with similar attributes */
static void __attribute__((destructor, visibility("hidden"))) 
cleanup_destructor(void) {
    /* Access TLS variable */
    tls_var++;
}

/* Pattern 7: Artificial unused function with builtin */
static inline void __attribute__((unused, always_inline))
artificial_helper(void) {
    /* Complex builtin usage that might generate internal symbols */
    int y = __atomic_load_n(&hidden_volatile, __ATOMIC_SEQ_CST);
    if (y > 100) {
        __builtin_unreachable();
    }
}

/* Pattern 8: Complex function pointer type */
typedef void (*complex_fn_ptr)(void) __attribute__((nothrow));

/* Pattern 9: Hidden namespace for C++ (if compiled as C++) */
#ifdef __cplusplus
namespace hidden_space __attribute__((visibility("hidden"))) {
    extern "C" void hidden_extern_func(void);
}
#endif

/* Pattern 10: Atomic operations that may call libatomic helpers */
void atomic_operations(void) {
    atomic_int atomic_var = ATOMIC_VAR_INIT(0);
    
    /* These may generate internal nothrow atomic helpers */
    __atomic_fetch_add(&atomic_var, 1, __ATOMIC_SEQ_CST);
    __atomic_compare_exchange_n(&atomic_var, &(int){0}, 1, 
                                0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
}

/* Main function orchestrates all patterns */
int main(void) {
    int checksum = 0;
    
    /* 1. Access hidden volatile variable */
    checksum += hidden_volatile;
    
    /* 2. Access and modify TLS variable */
    tls_var += checksum;
    checksum += tls_var;
    
    /* 3. Call atomic operations */
    atomic_operations();
    
    /* 4. Use volatile function pointer if available */
    if (ext_fn_ptr) {
        /* This keeps the reference alive */
        checksum += (long)ext_fn_ptr;
    }
    
    /* 5. Use artificial helper */
    artificial_helper();
    
    /* 6. Complex function pointer usage (dead code that gets eliminated) */
    complex_fn_ptr dead_ptr = 0;
    checksum += (long)dead_ptr;  /* Will be eliminated but parsed */
    
    /* 7. Try-catch block for C++ */
    #ifdef __cplusplus
    try {
        /* Call external nothrow function */
        external_nothrow_func();
    } catch (...) {
        checksum += 1;
    }
    #endif
    
    /* 8. More builtin usage with volatile */
    volatile int v = checksum;
    if (__builtin_constant_p(v)) {
        checksum += 1000;
    }
    
    /* 9. Force use of values to prevent optimization */
    asm volatile("" : : "r"(checksum));
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}

/* Additional definitions to satisfy references */
#ifdef __cplusplus
namespace hidden_space {
    extern "C" void hidden_extern_func(void) {
        /* Empty but defined */
    }
}
#endif

/* Define the external function (weak to avoid multiple definition issues) */
void external_nothrow_func(void) __attribute__((weak, nothrow));
void external_nothrow_func(void) {
    /* Empty implementation */
}

/* Define volatile function pointer */
volatile void (* volatile ext_fn_ptr)(void) = 0;

#ifdef __cplusplus
} /* extern "C" */
#endif
