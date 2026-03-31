/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* For C++ mode: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force generation of hidden visibility external symbols */
#pragma GCC visibility push(hidden)

/* Pattern 1: External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;
volatile int hidden_volatile_var = 42;

/* Pattern 2: Function with constructor attribute and hidden visibility */
static void __attribute__((constructor, noinline)) hidden_constructor() {
    /* Use __builtin_expect with volatile to force internal helpers */
    volatile int cond = 1;
    if (__builtin_expect(cond, 0)) {
        hidden_volatile_var++;
    }
}

/* Pattern 3: Artificial function that might be generated */
static inline __attribute__((always_inline, unused)) 
void artificial_helper(volatile int* p) __attribute__((nothrow));
static inline void artificial_helper(volatile int* p) {
    /* Complex builtin that might require wrapper */
    if (!__builtin_constant_p(*p)) {
        *p = __builtin_abs(*p);
    }
}

#pragma GCC visibility pop

/* Pattern 4: TLS with explicit model (may generate internal helpers) */
__thread int tls_var __attribute__((tls_model("initial-exec")));

/* Pattern 5: External nothrow function declaration */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Pattern 6: Complex builtin usage that might generate internal symbols */
static int use_complex_builtin(void) {
    volatile int x = 10;
    int y = 0;
    
    /* __builtin_add_overflow might generate internal helpers */
    if (__builtin_add_overflow(x, hidden_volatile_var, &y)) {
        return -1;
    }
    
    /* __builtin_mul_overflow */
    int z;
    if (__builtin_mul_overflow(y, 2, &z)) {
        return -2;
    }
    
    return z;
}

/* Pattern 7: Function pointer with volatile qualifier */
extern volatile void (*volatile_ext_fn)(void);
static volatile void (*volatile_local_fn)(void);

/* Pattern 8: Try to trigger exception handling infrastructure */
#ifdef __cplusplus
extern "C" {
#endif

void try_nothrow_external(void) {
    /* Access the external nothrow function (even if not defined) */
    /* The compiler might generate a stub or reference */
    external_nothrow_func();
}

#ifdef __cplusplus
}
#endif

/* Main function that uses all patterns */
int main(void) {
    int result = 0;
    
    /* Use the hidden volatile variable */
    result += hidden_volatile_var;
    
    /* Use TLS variable */
    tls_var = 100;
    result += tls_var;
    
    /* Use complex builtin */
    result += use_complex_builtin();
    
    /* Use volatile function pointer pattern */
    volatile_local_fn = volatile_ext_fn;
    if (volatile_local_fn) {
        /* Prevent optimization */
        __asm__ volatile("" : : "r"(volatile_local_fn));
    }
    
    /* Call the try_nothrow_external function */
    try_nothrow_external();
    
    /* Use artificial helper */
    artificial_helper(&hidden_volatile_var);
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(result));
    
    return result;
}

/* Pattern 9: Unused static inline with unreachable (might be marked artificial) */
static inline __attribute__((unused)) 
void unreachable_helper(void) {
    if (0) {
        __builtin_unreachable();
    }
}

/* Pattern 10: Namespace with hidden visibility (C++ only) */
#ifdef __cplusplus
namespace hidden_ns __attribute__((visibility("hidden"))) {
    extern "C" {
        int hidden_namespace_var = 123;
    }
    
    /* This might trigger special handling */
    struct __attribute__((visibility("hidden"))) HiddenStruct {
        volatile int member;
        HiddenStruct() : member(456) {}
    };
    
    HiddenStruct hidden_instance;
}
#endif

/* Pattern 11: Weak symbol with hidden visibility */
extern int weak_hidden_var __attribute__((weak, visibility("hidden")));
int weak_hidden_var = 789;

/* Pattern 12: Alias to builtin that might be treated specially */
extern int __attribute__((alias("hidden_volatile_var"))) 
alias_to_hidden __attribute__((visibility("hidden")));
