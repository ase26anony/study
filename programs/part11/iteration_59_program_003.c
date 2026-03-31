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

/* Define the external volatile variable */
volatile int hidden_volatile_var = 1;

/* Constructor with hidden visibility - may trigger internal helpers */
static void __attribute__((constructor, visibility("hidden"))) init_func(void) {
    /* Use __builtin_expect with volatile to potentially trigger helper generation */
    if (__builtin_expect(hidden_volatile_var != 0, 1)) {
        tls_var = 42;
    }
}

/* Destructor with similar attributes */
static void __attribute__((destructor, visibility("hidden"))) cleanup_func(void) {
    /* Use another builtin that might require runtime support */
    int x = __builtin_constant_p(tls_var) ? 0 : 1;
    hidden_volatile_var = x;
}

/* Artificial function that might be ignored */
static inline __attribute__((unused, always_inline)) 
void artificial_helper(int *p) {
    /* Use __builtin_unreachable to create interesting control flow */
    if (!p) __builtin_unreachable();
    *p += 1;
}

/* Complex function pointer type */
typedef void (*complex_func_ptr)(void) __attribute__((nothrow));

/* Main function with exception handling in C++ mode */
#ifdef __cplusplus
extern "C" {
#endif

int main(void) {
    /* Access volatile external with hidden visibility */
    int val = hidden_volatile_var;
    
    /* Use TLS variable */
    tls_var = val * 2;
    
    /* Call the artificial helper */
    artificial_helper(&tls_var);
    
    /* Complex expression with builtins */
    int optimized = __builtin_expect(tls_var > 0, 1) ? 
                   __builtin_ffs(tls_var) : 0;
    
    /* Try to call external nothrow function */
    /* This remains an external reference, may trigger stub creation */
    hidden_nothrow_func();
    
    /* Use atomic operations that may call into libatomic */
    {
        int atomic_var = 0;
        /* Memory barrier */
        __atomic_thread_fence(__ATOMIC_SEQ_CST);
        __atomic_store_n(&atomic_var, optimized, __ATOMIC_RELAXED);
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile ("" : : "r"(tls_var), "r"(val));
    
    return tls_var > 100 ? 1 : 0;
}

#ifdef __cplusplus
} /* extern "C" */

/* C++ specific code to trigger exception handling paths */
namespace hidden_visibility_ns __attribute__((visibility("hidden"))) {
    extern "C" void cpp_nothrow_func() noexcept;
    
    class ExceptionTest {
    public:
        ExceptionTest() {
            /* This may trigger generation of exception handling helpers */
            try {
                cpp_nothrow_func();
            } catch (...) {
                /* Catch all - but function is noexcept */
            }
        }
    };
    
    /* Global object to force initialization */
    ExceptionTest global_obj __attribute__((used));
}

#endif /* __cplusplus */

/* Another approach: function with volatile asm and noinline */
static void __attribute__((noinline, used)) 
volatile_asm_func(void) {
    /* Volatile asm may create special handling */
    __asm__ volatile (
        "nop\n\t"
        "nop\n\t"
        : : : "memory"
    );
}

/* Force reference to volatile_asm_func */
static void (*volatile func_ptr)(void) = volatile_asm_func;

/* Weak external symbol with hidden visibility */
extern int __attribute__((weak, visibility("hidden"))) weak_hidden_symbol;

/* Array of complex pointers that might trigger metadata generation */
complex_func_ptr func_array[] __attribute__((visibility("hidden"))) = {
    (complex_func_ptr)0,
    (complex_func_ptr)main
};
