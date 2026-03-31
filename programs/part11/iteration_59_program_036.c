/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* For C++: g++ -O1 -fvisibility-inlines-hidden -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force generation of artificial, external, hidden symbols with specific attributes */

/* Pattern 1: Hidden visibility external volatile variable */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 42;

/* Pattern 2: TLS with explicit model - may generate internal helpers */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* Pattern 3: Constructor with hidden visibility and builtin usage */
static void __attribute__((constructor, visibility("hidden"), noinline)) 
init_constructor(void) {
    /* Use volatile to prevent optimization */
    volatile int x = hidden_volatile;
    
    /* Complex builtin usage that may generate internal helpers */
    if (__builtin_constant_p(x)) {
        /* This branch unlikely but forces analysis */
        __builtin_unreachable();
    }
    
    /* Use __builtin_expect with volatile condition */
    if (__builtin_expect(x > 0, 1)) {
        tls_var = x;
    }
}

/* Pattern 4: Destructor with similar attributes */
static void __attribute__((destructor, visibility("hidden"), noinline))
cleanup_destructor(void) {
    /* Access TLS and volatile */
    volatile int y = tls_var + hidden_volatile;
    (void)y; /* Suppress unused warning */
}

/* Pattern 5: Artificial function marked as used but ignored */
static inline void __attribute__((always_inline, unused, artificial))
internal_helper(void) {
    /* Complex expression that may generate internal symbols */
    volatile int* ptr = &hidden_volatile;
    __atomic_fetch_add(ptr, 1, __ATOMIC_SEQ_CST);
}

/* Pattern 6: External nothrow function declaration */
#ifdef __cplusplus
extern "C" {
#endif
    void external_nothrow_func(void) __attribute__((nothrow));
#ifdef __cplusplus
}
#endif

/* Pattern 7: Complex function pointer type */
typedef void (*complex_func_ptr)(void) __attribute__((nothrow));

/* Pattern 8: External volatile function pointer */
extern volatile void (*volatile ext_fn_ptr)(void);

/* Main function orchestrating all patterns */
int main(void) {
    /* Force use of hidden volatile */
    int local = hidden_volatile + 1;
    
    /* Use TLS variable */
    tls_var += local;
    
    /* Call internal helper */
    internal_helper();
    
    /* Use external volatile function pointer pattern */
    static void (*local_fn)(void) = 0;
    if (ext_fn_ptr) {
        local_fn = (void (*)(void))ext_fn_ptr;
    }
    
    /* Complex builtin usage */
    int result = __builtin_popcount(tls_var);
    
    /* Force side effects with asm */
    asm volatile ("" : : "r"(result), "r"(local) : "memory");
    
    /* C++ specific patterns */
#ifdef __cplusplus
    try {
        /* Attempt to call external nothrow function */
        external_nothrow_func();
    } catch (...) {
        /* Catch any exception */
        result = -1;
    }
    
    /* Use atomic operations that may call internal helpers */
    std::atomic<int> atomic_var(0);
    atomic_var.fetch_add(result, std::memory_order_seq_cst);
#endif
    
    /* Use complex function pointer type */
    complex_func_ptr fn = 0;
    (void)fn;
    
    /* Prevent dead code elimination */
    volatile int checksum = result + tls_var + local;
    
    return checksum & 0xFF;
}

/* Additional definitions to satisfy external references */
void (*volatile ext_fn_ptr)(void) = 0;

#ifdef __cplusplus
/* C++ specific implementation */
#include <atomic>

void external_nothrow_func(void) {
    /* Empty implementation - keeps it external for analysis */
}

/* Namespace with hidden visibility */
namespace hidden_ns __attribute__((visibility("hidden"))) {
    extern "C" void hidden_extern_func(void) {
        /* May trigger special symbol generation */
    }
}

/* Use #pragma for visibility control */
#pragma GCC visibility push(hidden)
extern int pragma_hidden_var;
#pragma GCC visibility pop

int pragma_hidden_var = 123;

#else
/* C implementation of external_nothrow_func */
void external_nothrow_func(void) {
    /* Empty implementation */
}
#endif

/* Pattern 9: Unused static inline with builtin_unreachable */
static inline void __attribute__((unused, noinline))
dead_code_helper(void) {
    if (hidden_volatile == 0) {
        __builtin_unreachable();
    }
}

/* Pattern 10: Force generation of exception handling infrastructure */
#ifdef __cplusplus
void throw_if_zero(int x) {
    if (x == 0) {
        throw x;
    }
}

int test_exception(void) {
    try {
        throw_if_zero(hidden_volatile);
        return 1;
    } catch (int e) {
        return e;
    }
}
#endif
