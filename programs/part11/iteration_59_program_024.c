/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */
/* For C++: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc */

#ifdef __cplusplus
extern "C" {
#endif

/* Force generation of hidden visibility external symbols */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;
volatile int hidden_volatile_var = 1;

/* External function with nothrow attribute */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

#pragma GCC visibility pop

/* Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) hidden_constructor(void) {
    /* Use volatile to prevent optimization */
    volatile int x = 0;
    
    /* Use __builtin_expect with volatile condition */
    if (__builtin_expect(hidden_volatile_var != 0, 1)) {
        x = 1;
    }
    
    /* Use __builtin_constant_p on non-constant expression */
    if (!__builtin_constant_p(x)) {
        hidden_volatile_var = x;
    }
}

/* Artificial function that might be generated */
static inline void __attribute__((always_inline, unused, visibility("hidden"))) 
artificial_helper(volatile int *p) {
    /* Force side effects */
    *p = *p + 1;
    /* No-throw builtin */
    __builtin_unreachable();
}

#ifdef __cplusplus
} /* extern "C" */

#include <atomic>

/* C++ specific code to trigger exception handling helpers */
namespace hidden_visibility {
    __attribute__((visibility("hidden"))) 
    extern "C" void cpp_nothrow_func() noexcept;
    
    class HiddenClass {
    public:
        HiddenClass() {
            /* Atomic operation that may need helper */
            std::atomic<int> atomic_var{0};
            atomic_var.store(1, std::memory_order_seq_cst);
        }
    };
}

/* Try block with external nothrow function */
void test_exception_context() {
    try {
        /* Call to external nothrow function */
        hidden_visibility::cpp_nothrow_func();
    } catch (...) {
        /* Catch everything */
    }
}

#endif /* __cplusplus */

/* Complex function pointer type */
typedef void (*complex_func_ptr)(volatile int*, ...) __attribute__((nothrow));

/* Main function that uses all patterns */
int main() {
    /* Use hidden volatile variable */
    int val = hidden_volatile_var;
    
    /* Initialize TLS */
    tls_var = val;
    
    /* Use artificial helper */
    artificial_helper(&val);
    
    /* Complex function pointer (dead code but parsed) */
    complex_func_ptr unused_ptr = 0;
    
#ifdef __cplusplus
    /* C++ specific tests */
    hidden_visibility::HiddenClass obj;
    test_exception_context();
    
    /* Use atomic operations */
    std::atomic<int> atomic_val{0};
    atomic_val.fetch_add(1, std::memory_order_seq_cst);
#endif
    
    /* Force use of all variables to prevent optimization */
    asm volatile("" : : "r"(val), "r"(tls_var));
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = val + tls_var;
    
#ifdef __cplusplus
    checksum += atomic_val.load();
#endif
    
    return checksum == 0 ? 0 : 1;
}

/* External function declarations (never defined to keep them external) */
void external_nothrow_func(void) {
    /* Empty but defined to avoid linker error in this test */
}

#ifdef __cplusplus
void hidden_visibility::cpp_nothrow_func() noexcept {
    /* Empty */
}
#endif
