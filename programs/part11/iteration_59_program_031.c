/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */

/* Force generation of hidden external volatile variable */
extern volatile int hidden_volatile_var __attribute__((visibility("hidden")));
volatile int hidden_volatile_var = 0;

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 42;

/* Hidden constructor with volatile builtin usage */
static void __attribute__((constructor, visibility("hidden"))) init_func(void) {
    /* Use __builtin_expect with volatile to force internal helpers */
    volatile int cond = hidden_volatile_var;
    if (__builtin_expect(cond, 0)) {
        /* This should never happen but forces analysis */
        __builtin_unreachable();
    }
}

/* Hidden destructor */
static void __attribute__((destructor, visibility("hidden"))) cleanup_func(void) {
    /* Access TLS to ensure it's used */
    tls_var++;
}

/* External nothrow function declaration - will remain external */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Complex builtin that may require internal helpers */
static inline int __attribute__((always_inline, unused)) 
use_complex_builtin(int x) {
    /* __builtin_constant_p on non-constant expression */
    if (__builtin_constant_p(x + hidden_volatile_var)) {
        return 1;
    }
    /* __builtin_expect with side effects */
    return __builtin_expect(x > 0, 1) ? x : -x;
}

/* Artificial function pointer type */
typedef void (*func_ptr_t)(void) __attribute__((nothrow));

/* Function that uses atomic operations (may call libatomic helpers) */
#ifdef __cplusplus
extern "C" {
#endif

void use_atomic_ops(void) {
    /* Use GCC atomic builtins */
    int atomic_var = 0;
    __atomic_store_n(&atomic_var, 1, __ATOMIC_SEQ_CST);
    int val = __atomic_load_n(&atomic_var, __ATOMIC_ACQUIRE);
    
    /* Force use of the value */
    asm volatile("" : : "r"(val) : "memory");
}

#ifdef __cplusplus
}
#endif

/* C++ specific code to trigger exception handling helpers */
#ifdef __cplusplus
#include <new>

/* External C++ nothrow function */
extern "C" void cpp_external_nothrow() __attribute__((nothrow));

/* Class with nothrow new */
struct NothrowTest {
    void* operator new(size_t size) noexcept {
        return ::operator new(size, std::nothrow);
    }
    
    void operator delete(void* ptr) noexcept {
        ::operator delete(ptr, std::nothrow);
    }
};

void test_exception_helpers() {
    /* Try block with external nothrow function */
    try {
        cpp_external_nothrow();
    } catch (...) {
        /* Should not be reached if function is truly nothrow */
    }
    
    /* Use nothrow new */
    NothrowTest* obj = new(std::nothrow) NothrowTest;
    delete obj;
}
#endif

/* Main function orchestrates everything */
int main(void) {
    /* Force use of init/cleanup functions */
    hidden_volatile_var = 1;
    
    /* Use complex builtin */
    int result = use_complex_builtin(hidden_volatile_var);
    
    /* Use atomic operations */
    use_atomic_ops();
    
    /* Access TLS */
    tls_var = result + tls_var;
    
    /* Use function pointer type (even if dead) */
    func_ptr_t unused_ptr = 0;
    (void)unused_ptr;
    
#ifdef __cplusplus
    /* Test C++ exception helpers if in C++ mode */
    test_exception_helpers();
#endif
    
    /* Compute checksum to prevent optimization */
    int checksum = hidden_volatile_var + tls_var + result;
    
    /* Force output to prevent dead code elimination */
    asm volatile("" : : "r"(checksum) : "memory");
    
    return checksum != 0 ? 0 : 1;
}

/* Additional external declarations to keep symbols external */

/* External volatile function pointer */
extern volatile void (*volatile ext_fn_ptr)(void);
volatile void (*volatile ext_fn_ptr)(void) = 0;

/* Hidden visibility external variable */
#pragma GCC visibility push(hidden)
extern int hidden_external_var;
#pragma GCC visibility pop
int hidden_external_var = 100;

/* Force generation of COMDAT group with hidden visibility */
static inline __attribute__((always_inline, visibility("hidden"))) 
int inline_helper(int x) {
    return x * 2;
}

/* Use the inline helper */
int use_inline_helper(void) {
    return inline_helper(42);
}
