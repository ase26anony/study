/* test_targhooks.c - Comprehensive test to trigger target-specific hooks */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */
/* Or for C++: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc */

#include <stdatomic.h>
#include <stdint.h>

/* Pattern 1: External volatile with hidden visibility */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 42;

/* Pattern 2: External function pointer with volatile qualifiers */
extern volatile void (* volatile ext_fn_ptr)(void);
volatile void (* volatile ext_fn_ptr)(void) = 0;

/* Pattern 3: TLS with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* Pattern 4: Constructor with hidden visibility and builtin usage */
static void __attribute__((constructor, visibility("hidden"), noinline)) 
init_constructor(void) {
    /* Use __builtin_constant_p with non-constant argument */
    volatile int v = 5;
    if (__builtin_constant_p(v)) {
        /* This branch won't be taken, but creates interesting IR */
        tls_var = 99;
    }
    
    /* Use __builtin_expect with volatile condition */
    if (__builtin_expect(v > 0, 1)) {
        hidden_volatile = v;
    }
}

/* Pattern 5: Destructor with similar attributes */
static void __attribute__((destructor, visibility("hidden"))) 
cleanup_destructor(void) {
    /* Force side effects */
    asm volatile("" : : "r"(tls_var));
}

/* Pattern 6: Artificial/ignored declaration candidate */
static inline void __attribute__((unused, always_inline)) 
artificial_helper(int x) {
    if (x < 0) {
        __builtin_unreachable();
    }
    /* Complex expression that might generate internal symbols */
    atomic_signal_fence(memory_order_seq_cst);
}

/* Pattern 7: Nothrow external function declaration */
#ifdef __cplusplus
extern "C" {
#endif
    void external_nothrow_func(void) __attribute__((nothrow));
#ifdef __cplusplus
}
#endif

/* Pattern 8: Complex function pointer type */
typedef void (*complex_fp_t)(int, ...) __attribute__((nothrow));

/* Pattern 9: Hidden visibility namespace (C++ only) */
#ifdef __cplusplus
namespace __hidden_visibility_ns __attribute__((visibility("hidden"))) {
    extern "C" int hidden_extern_var;
}
int __hidden_visibility_ns::hidden_extern_var = 123;
#endif

/* Pattern 10: Volatile atomic operations */
_Atomic int atomic_counter = ATOMIC_VAR_INIT(0);

/* Main function orchestrates all patterns */
int main(void) {
    int checksum = 0;
    
    /* Access external volatile with hidden visibility */
    checksum ^= hidden_volatile;
    
    /* Use external volatile function pointer */
    if (ext_fn_ptr) {
        ext_fn_ptr();
    }
    
    /* Access TLS variable */
    checksum ^= tls_var;
    tls_var++;
    
    /* Call artificial helper */
    artificial_helper(checksum);
    
    /* Use atomic operations */
    atomic_fetch_add_explicit(&atomic_counter, 1, memory_order_seq_cst);
    checksum ^= atomic_load_explicit(&atomic_counter, memory_order_relaxed);
    
    /* Complex function pointer usage */
    complex_fp_t fp = 0;
    checksum ^= (uintptr_t)fp;
    
    /* Force use of all values to prevent optimization */
    asm volatile("" : : "r"(checksum));
    
    /* C++ specific patterns */
    #ifdef __cplusplus
    try {
        /* Call external nothrow function */
        external_nothrow_func();
        
        /* Access hidden namespace variable */
        checksum ^= __hidden_visibility_ns::hidden_extern_var;
    } catch (...) {
        /* Empty handler */
    }
    #endif
    
    /* Additional volatile asm to prevent dead code elimination */
    asm volatile("" : : "r"(checksum), "m"(hidden_volatile), "m"(tls_var));
    
    return checksum & 0xFF; /* Return non-zero to ensure execution */
}

/* Pattern 11: Additional external declarations with pragma visibility */
#pragma GCC visibility push(hidden)
extern int pragma_hidden_var;
#pragma GCC visibility pop

/* Define it to avoid linker errors */
int pragma_hidden_var = 456;

/* Pattern 12: Builtin with complex expression in static initializer */
static int builtin_init = __builtin_constant_p(0) ? 1 : 0;

/* Pattern 13: Weak symbol with hidden visibility */
extern int weak_hidden_symbol __attribute__((weak, visibility("hidden")));
int weak_hidden_symbol = 789;

/* Pattern 14: Noinline function with volatile asm */
static void __attribute__((noinline, used)) 
force_volatile_side_effects(void) {
    volatile int sink = 0;
    asm volatile("" : "+r"(sink) : : "memory");
    sink = hidden_volatile;
    asm volatile("" : : "r"(sink));
}

/* Call it from a constructor to ensure early execution */
static void __attribute__((constructor)) 
call_force_side_effects(void) {
    force_volatile_side_effects();
}

/* Pattern 15: Use __sync builtins which may generate internal helpers */
void use_sync_builtins(void) {
    int val = 0;
    __sync_fetch_and_add(&val, 1);
    __sync_synchronize();
    asm volatile("" : : "r"(val));
}
