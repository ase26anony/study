/* test_targhooks.c - Test program to cover lines 981-990 in targhooks.cc */

/* Force generation of internal symbols with specific attributes */
#ifdef __cplusplus
extern "C" {
#endif

/* External volatile variable with hidden visibility */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 42;

/* External function pointer with volatile qualifier */
extern volatile void (* volatile ext_fn_ptr)(void);
volatile void (* volatile ext_fn_ptr)(void) = 0;

/* Nothrow external function declaration */
extern void nothrow_helper(void) __attribute__((nothrow));

/* Hidden constructor function */
static void __attribute__((constructor, visibility("hidden"))) 
hidden_constructor(void) {
    /* Use __builtin_expect with volatile condition */
    volatile int cond = 1;
    if (__builtin_expect(cond, 0)) {
        hidden_volatile++;
    }
    
    /* Use __builtin_constant_p on non-constant expression */
    int x = hidden_volatile;
    if (__builtin_constant_p(x)) {
        /* unreachable but forces analysis */
    }
}

/* Hidden destructor function */
static void __attribute__((destructor, visibility("hidden"))) 
hidden_destructor(void) {
    /* Access volatile external */
    asm volatile("" : : "r"(hidden_volatile));
}

/* Artificial unused function with __builtin_unreachable */
static inline void __attribute__((unused, always_inline))
artificial_unused(void) {
    if (hidden_volatile > 100) {
        __builtin_unreachable();
    }
}

#ifdef __cplusplus
} /* extern "C" */

/* C++ specific code for exception handling context */
namespace hidden_ns __attribute__((visibility("hidden"))) {
    extern "C" void cpp_hidden_func(void) noexcept;
}

/* TLS variable with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 123;

/* Complex function pointer type */
typedef void (*complex_fp)(void) __attribute__((nothrow));

/* Try block calling nothrow external */
void test_exception_context(void) {
    complex_fp fp = nullptr;
    
    /* Dead assignment that may generate artificial symbols */
    fp = reinterpret_cast<complex_fp>(&hidden_volatile);
    
    try {
        /* Call to external nothrow function */
        nothrow_helper();
    } catch (...) {
        /* Catch all - forces exception handling setup */
        tls_var++;
    }
}

#endif /* __cplusplus */

/* Main function orchestrating all patterns */
int main(void) {
    int checksum = 0;
    
    /* Constructor already ran, modify volatile */
    hidden_volatile = 100;
    
    /* Use external volatile function pointer */
    if (ext_fn_ptr) {
        ext_fn_ptr();
    }
    
    /* Force use of volatile variable */
    checksum += hidden_volatile;
    
#ifdef __cplusplus
    /* C++ specific patterns */
    test_exception_context();
    
    /* Use TLS variable */
    checksum += tls_var;
    
    /* Call artificial function */
    artificial_unused();
#endif
    
    /* Use __builtin with side effects */
    checksum += __builtin_popcount(hidden_volatile);
    
    /* Prevent optimization */
    asm volatile("" : "+r"(checksum));
    
    return checksum % 256;
}

/* Additional definitions to satisfy external references */
#ifdef __cplusplus
extern "C" {
#endif

void nothrow_helper(void) {
    /* Empty but satisfies external reference */
}

#ifdef __cplusplus
}
#endif
