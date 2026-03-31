/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */

/* Force generation of hidden visibility external symbols */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int ext_hidden_volatile;

/* External function with nothrow attribute */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

#pragma GCC visibility pop

/* Define the external volatile variable */
volatile int ext_hidden_volatile = 1;

/* Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) init_constructor(void) {
    /* Use __builtin_expect with volatile to potentially trigger helper generation */
    if (__builtin_expect(ext_hidden_volatile != 0, 1)) {
        tls_var = ext_hidden_volatile;
    }
}

/* Destructor with hidden visibility */
static void __attribute__((destructor, visibility("hidden"))) cleanup_destructor(void) {
    /* Use atomic operation that may need helper */
    __atomic_store_n(&tls_var, 0, __ATOMIC_SEQ_CST);
}

/* Artificial function that might be ignored */
static inline __attribute__((unused, always_inline)) 
void artificial_helper(void) {
    /* Use builtin that can't be constant folded */
    if (!__builtin_constant_p(ext_hidden_volatile)) {
        __builtin_unreachable();
    }
}

/* External nothrow function declaration (no definition to keep it external) */
void external_nothrow_func(void) __attribute__((nothrow));

/* Main function that uses various patterns */
int main(void) {
    int result = 0;
    
    /* Access volatile external with hidden visibility */
    result += ext_hidden_volatile;
    
    /* Use TLS variable */
    tls_var = result;
    result += tls_var;
    
    /* Call artificial helper */
    artificial_helper();
    
    /* Take address of external function (keeps it referenced) */
    void (*fn_ptr)(void) = external_nothrow_func;
    
    /* Use inline asm to prevent optimization */
    asm volatile("" : "+r"(result) : : "memory");
    
    /* Complex expression with builtins */
    result += __builtin_popcount(result);
    
    /* Try to trigger exception handling helpers in C++ mode */
    #ifdef __cplusplus
    try {
        if (fn_ptr) fn_ptr();
    } catch (...) {
        result++;
    }
    #endif
    
    return result;
}

/* C++ specific code to trigger more helpers */
#ifdef __cplusplus
#include <atomic>

namespace hidden_visibility_ns __attribute__((visibility("hidden"))) {
    extern "C" void hidden_extern_func(void) __attribute__((nothrow));
    
    /* This may trigger generation of compiler helpers */
    std::atomic<int> atomic_var(0);
    
    void hidden_extern_func(void) {
        /* Use memory_order_seq_cst which may need runtime helpers */
        atomic_var.store(1, std::memory_order_seq_cst);
    }
}

/* Use the hidden namespace */
void use_hidden_namespace(void) {
    hidden_visibility_ns::hidden_extern_func();
}
#endif
