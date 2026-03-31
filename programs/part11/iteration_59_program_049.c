/* test_targhooks.c - Test program to cover lines 981-990 in targhooks.cc */

/* Force C++ mode for exception handling features */
#ifdef __cplusplus
#include <atomic>
#endif

/* External volatile variable with hidden visibility */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 42;

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* Hidden constructor function */
static void __attribute__((constructor, visibility("hidden"))) hidden_init() {
    /* Use volatile to force side effects */
    int local = hidden_volatile;
    
    /* Complex builtin that might require internal helpers */
    if (__builtin_constant_p(local) && __builtin_expect(local > 0, 1)) {
        tls_var = local;
    }
}

/* External nothrow function declaration */
#ifdef __cplusplus
extern "C" {
#endif
void external_nothrow_func(void) __attribute__((nothrow));
#ifdef __cplusplus
}
#endif

/* Artificial function that might be ignored */
static inline void __attribute__((unused, artificial)) 
artificial_helper(int x) {
    if (x < 0) __builtin_unreachable();
}

/* Hidden visibility namespace */
#ifdef __cplusplus
namespace hidden_space __attribute__((visibility("hidden"))) {
    extern "C" void hidden_extern_func();
}
#endif

/* Function with volatile function pointer */
void test_volatile_funcptr(void) {
    /* External volatile function pointer */
    extern volatile void (* volatile ext_fn_ptr)(void);
    
    /* Force use of the pointer */
    if (ext_fn_ptr) {
        /* This should generate internal handling */
        ext_fn_ptr();
    }
}

/* Main function orchestrates all patterns */
int main(void) {
    /* Access hidden volatile */
    int val = hidden_volatile + 1;
    
    /* Use TLS variable */
    tls_var += val;
    
    /* Use artificial helper */
    artificial_helper(tls_var);
    
#ifdef __cplusplus
    /* Try-catch with external nothrow function */
    try {
        external_nothrow_func();
    } catch (...) {
        /* Catch any exceptions */
        tls_var = -1;
    }
    
    /* Atomic operation that might use internal helpers */
    std::atomic<int> atomic_var{0};
    atomic_var.store(1, std::memory_order_seq_cst);
    val += atomic_var.load(std::memory_order_seq_cst);
#endif
    
    /* Test volatile function pointer */
    test_volatile_funcptr();
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(val), "r"(tls_var));
    
    /* Compute checksum */
    int checksum = val + tls_var;
    
    return checksum > 0 ? 0 : 1;
}

/* Force generation of hidden external symbols */
#pragma GCC visibility push(hidden)
extern int hidden_external_var;
#pragma GCC visibility pop

/* Define the hidden external variable */
int hidden_external_var = 123;

/* Additional patterns to increase coverage */

/* Complex builtin usage that might generate internal helpers */
static int __attribute__((used)) 
use_complex_builtins(void) {
    volatile int v = 0;
    
    /* These builtins might require internal handling */
    int r1 = __builtin_ffs(v);
    int r2 = __builtin_popcount(v);
    int r3 = __builtin_bswap32(v);
    
    /* Use __sync builtins which might need helpers */
    int sync_val = 0;
    __sync_fetch_and_add(&sync_val, 1);
    
    return r1 + r2 + r3 + sync_val;
}

/* Call the function to ensure it's used */
static void __attribute__((destructor)) cleanup(void) {
    use_complex_builtins();
}

/* Force the generation of exception handling personality function */
#ifdef __cplusplus
void __attribute__((noinline)) throw_exception(void) {
    throw 42;
}

int catch_exception(void) {
    try {
        throw_exception();
    } catch (int e) {
        return e;
    }
    return 0;
}
#endif
