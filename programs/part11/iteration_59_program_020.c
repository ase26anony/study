/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */

/* Force generation of hidden external symbols */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;

/* External function with nothrow attribute */
extern void hidden_nothrow_func(void) __attribute__((nothrow));

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

#pragma GCC visibility pop

/* Define the volatile variable */
volatile int hidden_volatile_var = 1;

/* Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) init_constructor(void) {
    /* Use builtin with volatile argument */
    if (__builtin_constant_p(hidden_volatile_var)) {
        /* This branch won't be taken, but forces analysis */
        asm volatile ("");
    }
}

/* Destructor with hidden visibility */
static void __attribute__((destructor, visibility("hidden"))) cleanup_destructor(void) {
    /* Use __builtin_expect with volatile condition */
    if (__builtin_expect(hidden_volatile_var != 0, 0)) {
        asm volatile ("");
    }
}

/* Artificial function that might be ignored */
static inline __attribute__((unused, always_inline)) 
void artificial_helper(void) {
    /* Complex builtin usage */
    if (__builtin_unreachable(), 0) {
        /* Never reached */
    }
}

/* Function with complex exception handling interaction */
#ifdef __cplusplus
extern "C" {
#endif

/* External function declaration - will remain external */
void external_hidden_func(void) __attribute__((visibility("hidden")));

/* Function that uses try-catch with external nothrow function */
void test_exception_context(void) {
    /* Access TLS variable */
    tls_var = 42;
    
    /* Use volatile variable */
    int val = hidden_volatile_var;
    
    /* Call artificial helper */
    artificial_helper();
    
#ifdef __cplusplus
    /* In C++ mode, try block with nothrow external */
    try {
        hidden_nothrow_func();
    } catch (...) {
        /* Catch all */
    }
#endif
}

#ifdef __cplusplus
}
#endif

/* Main function orchestrates everything */
int main(void) {
    /* Initialize TLS */
    tls_var = 100;
    
    /* Force use of volatile variable */
    int volatile_read = hidden_volatile_var;
    
    /* Call function with exception context */
    test_exception_context();
    
    /* Use atomic operations (may generate internal helpers) */
    {
        int atomic_var = 0;
        /* Memory barrier */
        __sync_synchronize();
        __sync_add_and_fetch(&atomic_var, volatile_read);
    }
    
    /* Take address of external function to force reference */
    void (*fn_ptr)(void) = &test_exception_context;
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(tls_var), "r"(volatile_read), "r"(fn_ptr));
    
    return 0;
}

/* Additional C++ specific code */
#ifdef __cplusplus
#include <atomic>

namespace hidden_visibility_ns __attribute__((visibility("hidden"))) {
    extern "C" void namespace_hidden_func(void) {
        std::atomic<int> atomic_counter{0};
        atomic_counter.fetch_add(1, std::memory_order_seq_cst);
    }
}

/* Unused template that might generate artificial symbols */
template<typename T>
struct __attribute__((visibility("hidden"))) HiddenTemplate {
    static T value __attribute__((used));
    static void __attribute__((nothrow)) init() {
        value = T();
    }
};

template<typename T>
T HiddenTemplate<T>::value;

/* Instantiate template */
template struct HiddenTemplate<int>;
#endif
