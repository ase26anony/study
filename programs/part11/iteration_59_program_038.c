/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */
/* For C++: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc */

#ifdef __cplusplus
extern "C" {
#endif

/* Force generation of hidden external volatile variable */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 1;

/* Hidden constructor with volatile access */
static void __attribute__((constructor, visibility("hidden"))) init_constructor() {
    /* Use __builtin with volatile operand */
    int volatile_val = hidden_volatile;
    if (__builtin_constant_p(volatile_val)) {
        /* This branch won't be taken but forces analysis */
    }
    __builtin_expect(volatile_val > 0, 1);
}

/* Hidden destructor */
static void __attribute__((destructor, visibility("hidden"))) cleanup_destructor() {
    /* Access volatile to prevent optimization */
    asm volatile("" : : "r"(hidden_volatile));
}

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"), visibility("hidden")));

/* External nothrow function declaration with hidden visibility */
extern void hidden_nothrow_func(void) __attribute__((nothrow, visibility("hidden")));

/* Artificial function that might be ignored */
static inline void __attribute__((always_inline, unused, visibility("hidden"))) 
artificial_helper(int x) {
    if (x < 0) {
        __builtin_unreachable();
    }
    asm volatile("" : : "r"(x));
}

#ifdef __cplusplus
} /* extern "C" */

#include <atomic>

/* C++ specific code that might trigger exception handling helpers */
namespace hidden_visibility {
    __attribute__((visibility("hidden"))) 
    extern "C" void cpp_hidden_func() noexcept {
        std::atomic<int> atomic_var{0};
        atomic_var.store(1, std::memory_order_seq_cst);
        
        /* Use TLS */
        extern __thread int tls_var;
        tls_var = atomic_var.load(std::memory_order_acquire);
    }
    
    /* External volatile function pointer */
    extern volatile void (* volatile hidden_fn_ptr)(void) 
        __attribute__((visibility("hidden")));
}

/* Try block with external nothrow function */
void test_exception_context() {
    try {
        hidden_nothrow_func();
    } catch (...) {
        /* Catch all - but hidden_nothrow_func is nothrow */
    }
}

#endif /* __cplusplus */

/* Main function orchestrates everything */
int main() {
    /* Force use of hidden volatile */
    int val = hidden_volatile;
    
    /* Use TLS */
    tls_var = val + 1;
    
    /* Call artificial helper */
    artificial_helper(val);
    
#ifdef __cplusplus
    /* C++ specific calls */
    hidden_visibility::cpp_hidden_func();
    test_exception_context();
    
    /* Use volatile function pointer if available */
    if (hidden_visibility::hidden_fn_ptr) {
        hidden_visibility::hidden_fn_ptr();
    }
#endif
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(val), "r"(tls_var));
    
    return 0;
}

/* Define the external nothrow function to avoid undefined reference */
void hidden_nothrow_func(void) {
    /* Do nothing but access volatile */
    asm volatile("" : : "r"(hidden_volatile));
}

#ifdef __cplusplus
/* Define the volatile function pointer */
namespace hidden_visibility {
    volatile void (* volatile hidden_fn_ptr)(void) = nullptr;
}
#endif
