/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */

/* Force generation of external volatile symbol with hidden visibility */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 0;

/* Thread-local storage with explicit model - may generate helpers */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 42;

/* Hidden constructor that uses builtins with volatile */
static void __attribute__((constructor, visibility("hidden"))) init_constructor() {
    /* Use volatile to prevent optimization */
    volatile int x = hidden_volatile;
    
    /* Use builtin with non-constant expression */
    if (__builtin_constant_p(x + tls_var)) {
        /* This branch won't be taken, but forces analysis */
        asm volatile ("");
    }
    
    /* Use expect with volatile condition */
    if (__builtin_expect(x > 100, 0)) {
        hidden_volatile = 1;
    }
}

/* Artificial function marked as unused but with complex attributes */
static inline void __attribute__((unused, always_inline)) 
artificial_helper(volatile int* p) {
    /* Use volatile asm to prevent optimization */
    asm volatile ("" : : "r"(*p));
    
    /* Builtin that might generate internal symbols */
    if (!__builtin_constant_p(*p)) {
        __builtin_unreachable();
    }
}

/* External nothrow function declaration - will remain external */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Function with hidden visibility that might trigger special handling */
static void __attribute__((noinline, visibility("hidden"))) 
hidden_func(void) {
    /* Access volatile and TLS */
    volatile int local = hidden_volatile + tls_var;
    
    /* Use the artificial helper */
    artificial_helper(&local);
    
    /* Call external nothrow function */
    external_nothrow_func();
}

/* Main function orchestrates everything */
int main(void) {
    /* Force use of hidden volatile */
    int val = hidden_volatile;
    
    /* Force use of TLS */
    tls_var = val + 1;
    
    /* Call hidden function */
    hidden_func();
    
    /* Complex expression that might generate internal helpers */
    volatile long complex_expr = (long)(&tls_var) + (long)(&hidden_volatile);
    
    /* Use atomic builtins which might generate internal calls */
    __atomic_store_n(&hidden_volatile, complex_expr & 0xFF, __ATOMIC_SEQ_CST);
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(val), "r"(tls_var), "r"(complex_expr));
    
    return (int)complex_expr;
}

/* Define a dummy external function to satisfy reference */
void external_nothrow_func(void) {
    /* Empty but marked nothrow */
}

/* Additional patterns in C++ mode */
#ifdef __cplusplus
extern "C" {
#endif

/* External volatile function pointer */
extern volatile void (*volatile ext_fn_ptr)(void);

/* Hidden namespace with external declaration */
__attribute__((visibility("hidden")))
extern int hidden_external_var;

#ifdef __cplusplus
} /* extern "C" */

/* C++ specific: try block with nothrow external function */
void test_exception_context(void) {
    try {
        external_nothrow_func();
    } catch (...) {
        /* Catch all */
    }
}

/* Static object with constructor that uses volatile */
class HiddenObject {
public:
    HiddenObject() __attribute__((noinline)) {
        volatile int x = hidden_volatile;
        asm volatile ("" : : "r"(x));
    }
    
    __attribute__((visibility("hidden")))
    static HiddenObject instance;
};

HiddenObject HiddenObject::instance;

#endif /* __cplusplus */
