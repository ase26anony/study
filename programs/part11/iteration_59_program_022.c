/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */

/* Force generation of hidden visibility external symbols */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;

/* External function with nothrow attribute */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

#pragma GCC visibility pop

/* Define the volatile variable */
volatile int hidden_volatile_var = 1;

/* Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) init_func(void) {
    /* Use builtin with volatile argument */
    if (__builtin_constant_p(hidden_volatile_var)) {
        /* This branch won't be taken but forces analysis */
        tls_var = 1;
    }
}

/* Artificial function that might be generated */
static inline __attribute__((unused, always_inline)) 
void artificial_helper(void) {
    __builtin_unreachable();
}

/* Complex builtin usage that might trigger helper generation */
int use_complex_builtin(int x) {
    /* __builtin_expect_with_probability might trigger internal helpers */
    return __builtin_expect(x, 1) ? x : 0;
}

/* Function using atomic operations that might need libatomic helpers */
int use_atomic(void) {
    static _Atomic int atomic_counter = 0;
    int old = __atomic_fetch_add(&atomic_counter, 1, __ATOMIC_SEQ_CST);
    return old;
}

/* External nothrow function declaration (keep it external) */
void external_nothrow_func(void) {
    /* Empty implementation - but declaration is what matters */
}

/* Main function orchestrating everything */
int main(void) {
    int result = 0;
    
    /* Access volatile with hidden visibility */
    result += hidden_volatile_var;
    
    /* Use TLS variable */
    tls_var = 42;
    result += tls_var;
    
    /* Call function using atomic operations */
    result += use_atomic();
    
    /* Use complex builtin */
    result += use_complex_builtin(result);
    
    /* Try to call external nothrow function */
    external_nothrow_func();
    
    /* Use inline artificial helper */
    artificial_helper();
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(result));
    
    return result;
}

/* C++ specific section for exception handling */
#ifdef __cplusplus
#include <new>

/* Hidden visibility class with noexcept methods */
class __attribute__((visibility("hidden"))) HiddenClass {
public:
    HiddenClass() noexcept {}
    ~HiddenClass() noexcept {}
    
    void method() noexcept {
        /* Might trigger compiler helpers */
        operator delete(operator new(100));
    }
};

/* Function that might trigger exception handling helpers */
void test_cpp_exceptions() {
    HiddenClass obj;
    obj.method();
    
    try {
        external_nothrow_func();
    } catch (...) {
        /* Catch all */
    }
}
#endif
