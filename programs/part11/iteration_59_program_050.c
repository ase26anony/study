/* test_targhooks.c - Test program to trigger specific target hooks in GCC */

/* Force C++ compilation for exception handling */
#ifdef __cplusplus
#include <atomic>
#endif

/* Pattern 1: Hidden visibility external volatile variable */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 42;

/* Pattern 2: TLS with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* Pattern 3: Constructor with hidden visibility and builtin usage */
static void __attribute__((constructor, visibility("hidden"), noinline)) 
init_constructor() {
    /* Use __builtin_expect with volatile condition */
    volatile int cond = 1;
    if (__builtin_expect(cond, 0)) {
        /* Access hidden volatile */
        int val = hidden_volatile;
        /* Use __builtin_constant_p on non-constant */
        if (!__builtin_constant_p(val)) {
            tls_var += val;
        }
    }
}

/* Pattern 4: Destructor with similar attributes */
static void __attribute__((destructor, visibility("hidden"))) 
cleanup_destructor() {
    /* Force side effects */
    asm volatile("" : : "r"(tls_var));
}

/* Pattern 5: Artificial/ignored declaration helper */
static inline void __attribute__((unused, always_inline))
artificial_helper() {
    __builtin_unreachable();
}

/* Pattern 6: Complex function pointer type */
typedef void (*complex_fn_ptr)(void) __attribute__((nothrow));

/* C++ specific patterns */
#ifdef __cplusplus
/* Pattern 7: External nothrow function for exception context */
extern "C" void external_nothrow_func() __attribute__((nothrow));

/* Pattern 8: Hidden visibility namespace */
namespace __attribute__((visibility("hidden"))) hidden_ns {
    extern "C" int hidden_extern_var;
}

/* Pattern 9: Atomic operations that may need libatomic helpers */
void atomic_operation() {
    std::atomic<int> atomic_var{0};
    atomic_var.store(1, std::memory_order_seq_cst);
    int val = atomic_var.load(std::memory_order_seq_cst);
    asm volatile("" : : "r"(val));
}
#endif

/* Pattern 10: External volatile function pointer */
extern volatile void (* volatile ext_fn_ptr)(void);

/* Pattern 11: Push/pop visibility pragma */
#pragma GCC visibility push(hidden)
extern int pragma_hidden_var;
#pragma GCC visibility pop

/* Main function orchestrates all patterns */
int main() {
    int checksum = 0;
    
    /* 1. Access hidden volatile variable */
    checksum ^= hidden_volatile;
    
    /* 2. Use TLS variable */
    tls_var += 1;
    checksum ^= tls_var;
    
    /* 3. Call artificial helper (may be optimized out) */
    artificial_helper();
    
    /* 4. Use complex function pointer type */
    complex_fn_ptr fn = 0;
    checksum ^= (long)fn;
    
    /* 5. External volatile function pointer */
    if (ext_fn_ptr) {
        /* Would call if defined */
        checksum ^= 0xDEADBEEF;
    }
    
    /* 6. Access pragma-hidden variable */
    pragma_hidden_var = checksum;
    
#ifdef __cplusplus
    /* 7. Try-catch with external nothrow function */
    try {
        /* external_nothrow_func(); */ /* Would call if defined */
        checksum ^= 0x12345678;
    } catch (...) {
        checksum ^= 0x87654321;
    }
    
    /* 8. Atomic operation */
    atomic_operation();
    
    /* 9. Hidden namespace variable */
    hidden_ns::hidden_extern_var = checksum;
#endif
    
    /* 10. Use __builtin with volatile in main */
    volatile int v = 123;
    if (__builtin_constant_p(v)) {
        checksum ^= 1;
    } else {
        checksum ^= 2;
    }
    
    /* 11. Force use of values to prevent optimization */
    asm volatile("" : : "r"(checksum));
    
    /* Print to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* Additional definitions to satisfy extern declarations */
int pragma_hidden_var = 0;

#ifdef __cplusplus
namespace hidden_ns {
    int hidden_extern_var = 0;
}
#endif

volatile void (* volatile ext_fn_ptr)(void) = 0;
