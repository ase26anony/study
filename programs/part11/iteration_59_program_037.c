/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */
/* Also try: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc */

/* Force generation of hidden external symbols */
#pragma GCC visibility push(hidden)

/* External volatile with hidden visibility */
extern volatile int ext_hidden_volatile;
volatile int ext_hidden_volatile = 1;

/* Hidden constructor/destructor */
static void __attribute__((constructor, used)) hidden_init(void) {
    /* Use __builtin with volatile operand */
    if (__builtin_constant_p(ext_hidden_volatile)) {
        /* This branch won't be taken, but forces analysis */
        asm volatile ("");
    }
}

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 42;

/* Function with nothrow attribute */
extern "C" void external_nothrow_func(void) __attribute__((nothrow));

#ifdef __cplusplus
extern "C" {
#endif

/* Complex builtin usage that might generate helpers */
static inline int __attribute__((always_inline, unused)) 
use_complex_builtin(int x) {
    /* __builtin_expect with volatile condition */
    volatile int v = x;
    if (__builtin_expect(v > 0, 1)) {
        return __builtin_popcount(x);
    }
    return 0;
}

/* Artificial function that might be ignored */
static int __attribute__((unused, artificial)) 
artificial_helper(int x) {
    if (x == 0) {
        __builtin_unreachable();
    }
    return x * 2;
}

#ifdef __cplusplus
} /* extern "C" */

/* C++ specific code for exception handling */
namespace hidden_visibility {
    __attribute__((visibility("hidden"))) 
    extern "C" void cpp_nothrow_func() noexcept;
}

/* Try block with external nothrow function */
void test_exception_context() {
    try {
        /* Reference external function (remains undefined) */
        external_nothrow_func();
    } catch (...) {
        /* Catch everything */
    }
}

/* Use atomic operations that might call into libatomic */
#include <atomic>
std::atomic<int> atomic_var{0};

void test_atomic() {
    atomic_var.fetch_add(1, std::memory_order_seq_cst);
}

#endif /* __cplusplus */

#pragma GCC visibility pop

/* Main function that uses all patterns */
int main(int argc, char **argv) {
    /* Force use of external volatile */
    int val = ext_hidden_volatile + argc;
    
    /* Use TLS variable */
    tls_var += val;
    
    /* Use complex builtin */
    val = use_complex_builtin(val);
    
#ifdef __cplusplus
    /* C++ specific tests */
    test_exception_context();
    test_atomic();
    
    /* Use atomic variable */
    val += atomic_var.load(std::memory_order_relaxed);
#endif
    
    /* Use artificial helper (might be optimized out) */
    val += artificial_helper(val);
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(val), "r"(tls_var));
    
    return val & 0xFF;
}

/* Undefined external functions (kept external) */
extern "C" void external_nothrow_func(void) {
    /* Never defined here - remains external */
}

#ifdef __cplusplus
namespace hidden_visibility {
    extern "C" void cpp_nothrow_func() noexcept {
        /* Empty implementation */
    }
}
#endif
