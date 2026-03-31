/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */

/* Force hidden visibility for external declarations */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;

/* External function with nothrow attribute and hidden visibility */
extern void hidden_nothrow_func(void) __attribute__((nothrow));

/* Thread-local storage with initial-exec model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

#pragma GCC visibility pop

/* Define the volatile variable */
volatile int hidden_volatile_var = 0;

/* Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) init_constructor(void) {
    /* Use builtin with volatile argument */
    if (__builtin_constant_p(hidden_volatile_var)) {
        /* This branch won't be taken, but forces analysis */
        tls_var = 1;
    }
    
    /* Use expect with volatile condition */
    if (__builtin_expect(hidden_volatile_var != 0, 0)) {
        tls_var = 2;
    }
}

/* Destructor with hidden visibility */
static void __attribute__((destructor, visibility("hidden"))) cleanup_destructor(void) {
    /* Access TLS variable */
    hidden_volatile_var += tls_var;
}

/* Artificial function that might be generated */
static inline void __attribute__((always_inline, unused)) 
artificial_helper(void) {
    /* Force volatile semantics */
    asm volatile ("" : : "r"(hidden_volatile_var));
    
    /* Unreachable code */
    if (hidden_volatile_var < 0) {
        __builtin_unreachable();
    }
}

/* Complex builtin usage that might require helper generation */
static int use_complex_builtin(void) {
    /* __sync builtins often generate internal helpers */
    return __sync_fetch_and_add(&hidden_volatile_var, 1);
}

/* Function with exception handling in C++ mode */
#ifdef __cplusplus
extern "C" {
#endif

void try_nothrow_external(void) {
    /* Try to call external nothrow function */
    if (hidden_volatile_var) {
        hidden_nothrow_func();
    }
}

#ifdef __cplusplus
}
#endif

/* Main function orchestrates everything */
int main(void) {
    int result = 0;
    
    /* Force use of all patterns */
    
    /* 1. Use constructor/destructor pattern */
    /* Already called via attribute */
    
    /* 2. Access volatile with hidden visibility */
    result += hidden_volatile_var;
    
    /* 3. Use TLS variable */
    tls_var = 42;
    result += tls_var;
    
    /* 4. Use complex builtin */
    result += use_complex_builtin();
    
    /* 5. Call function that tries external nothrow */
    try_nothrow_external();
    
    /* 6. Use artificial helper */
    artificial_helper();
    
    /* 7. Atomic operations that might need helpers */
    __atomic_store_n(&hidden_volatile_var, result, __ATOMIC_SEQ_CST);
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(result));
    
    return result != 0 ? 0 : 1;
}

/* Also compile as C++ to trigger exception handling paths */
#ifdef __cplusplus
#include <atomic>

namespace hidden_visibility {
    /* Hidden visibility namespace */
    __attribute__((visibility("hidden"))) 
    extern "C" void external_in_hidden_ns(void) noexcept;
    
    class HiddenClass {
    public:
        HiddenClass() {
            /* Use atomic which might need helpers */
            std::atomic<int> atomic_var{0};
            atomic_var.store(1, std::memory_order_seq_cst);
        }
        
        ~HiddenClass() noexcept {
            hidden_volatile_var++;
        }
    };
    
    /* Global object with hidden visibility */
    __attribute__((visibility("hidden")))
    HiddenClass hidden_obj;
}

/* Try-catch with external nothrow function */
void test_exception_context(void) {
    try {
        hidden_nothrow_func();
    } catch (...) {
        /* External C function shouldn't throw, but we try */
        hidden_volatile_var = 1;
    }
}
#endif
