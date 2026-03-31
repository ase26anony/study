/* test_targhooks.c - Comprehensive test to trigger target-specific hooks */

/* Force C++ mode for exception handling */
#ifdef __cplusplus
extern "C" {
#endif

/* Pattern 1: Hidden visibility external volatile variable */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 42;

/* Pattern 2: TLS with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* Pattern 3: External nothrow function declaration */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Pattern 4: Hidden constructor with builtin usage */
static void __attribute__((constructor, visibility("hidden"), noinline)) 
hidden_constructor(void) {
    /* Use __builtin_expect with volatile condition */
    volatile int cond = 1;
    if (__builtin_expect(cond, 0)) {
        /* Use __builtin_constant_p on non-constant */
        int x = hidden_volatile;
        if (!__builtin_constant_p(x)) {
            tls_var = x + 1;
        }
    }
}

/* Pattern 5: Hidden destructor */
static void __attribute__((destructor, visibility("hidden"))) 
hidden_destructor(void) {
    /* Access volatile to prevent optimization */
    asm volatile("" : : "r"(hidden_volatile));
}

/* Pattern 6: Artificial unused function with builtin */
static inline void __attribute__((unused, always_inline))
artificial_helper(void) {
    /* Complex builtin usage */
    volatile int v = 0;
    if (__builtin_constant_p(v)) {
        __builtin_unreachable();
    }
}

/* Pattern 7: External function pointer with volatile qualifiers */
extern volatile void (* volatile ext_fn_ptr)(void);

/* Pattern 8: Complex function pointer type */
typedef void (*complex_fn_ptr)(void) __attribute__((nothrow));

/* C++ specific code */
#ifdef __cplusplus
}
#include <atomic>

/* Pattern 9: Atomic operations that may trigger internal helpers */
static void atomic_operations(void) {
    std::atomic<int> atomic_var{0};
    atomic_var.store(1, std::memory_order_seq_cst);
    
    /* Loop with atomic operations */
    for (int i = 0; i < 10; ++i) {
        atomic_var.fetch_add(1, std::memory_order_acq_rel);
    }
}

/* Pattern 10: Try block with external nothrow function */
static void try_external_nothrow(void) {
    try {
        /* Call to external nothrow function */
        external_nothrow_func();
    } catch (...) {
        /* Catch but do nothing */
    }
}

/* Hidden visibility namespace */
namespace __attribute__((visibility("hidden"))) hidden_ns {
    extern "C" void hidden_extern_func(void);
}

/* Pattern 11: Unused template that generates artificial symbols */
template<typename T>
struct __attribute__((visibility("hidden"))) HiddenTemplate {
    static T value __attribute__((used));
};

template<typename T>
T HiddenTemplate<T>::value = T();

#endif /* __cplusplus */

/* Main function orchestrating all patterns */
int main(void) {
    int checksum = 0;
    
    /* Pattern 1: Use hidden volatile variable */
    checksum ^= hidden_volatile;
    
    /* Pattern 2: Use TLS variable */
    tls_var += 1;
    checksum ^= tls_var;
    
    /* Pattern 3: Take address of external function */
    checksum ^= (int)(long)external_nothrow_func;
    
    /* Pattern 4: Call artificial helper */
    artificial_helper();
    
    /* Pattern 5: Indirect call through volatile function pointer */
    if (ext_fn_ptr) {
        checksum ^= 0xDEADBEEF;
    }
    
    /* Pattern 6: Use complex function pointer type */
    complex_fn_ptr fn = 0;
    checksum ^= (int)(long)fn;
    
#ifdef __cplusplus
    /* C++ specific patterns */
    atomic_operations();
    try_external_nothrow();
    
    /* Use hidden template */
    HiddenTemplate<int>::value = checksum;
    checksum ^= HiddenTemplate<int>::value;
    
    /* Force use of hidden namespace symbol */
    checksum ^= (int)(long)hidden_ns::hidden_extern_func;
#endif
    
    /* Pattern 7: Force side effects with inline assembly */
    asm volatile (
        "/* Force use of variables */"
        : 
        : "r"(checksum), "r"(tls_var), "r"(hidden_volatile)
        : "memory"
    );
    
    /* Prevent dead code elimination */
    volatile int result = checksum;
    return result;
}

/* Additional definitions to satisfy references */

/* Define the external nothrow function (but keep it external for other TUs) */
void external_nothrow_func(void) __attribute__((nothrow));
void external_nothrow_func(void) {
    /* Empty implementation */
}

/* Define the volatile function pointer */
volatile void (* volatile ext_fn_ptr)(void) = 0;

#ifdef __cplusplus
/* Define the hidden namespace function */
namespace hidden_ns {
    extern "C" void hidden_extern_func(void) {
        /* Empty implementation */
    }
}

/* Instantiate template to generate symbols */
template struct HiddenTemplate<int>;
template struct HiddenTemplate<long>;
#endif
