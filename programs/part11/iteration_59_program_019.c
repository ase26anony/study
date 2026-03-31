/* test_targhooks.c - Test program to trigger specific target hooks in GCC */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* Also try: g++ -O1 -fvisibility-inlines-hidden -fexceptions -c test_targhooks.c -o test_targhooks.o */

#include <stdio.h>
#include <stdatomic.h>

/* Pattern 1: Hidden visibility external volatile variable */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 42;

/* Pattern 2: TLS with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* Pattern 3: External function with nothrow attribute */
extern "C" void external_nothrow_func() __attribute__((nothrow));

/* Pattern 4: Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"), noinline)) 
hidden_constructor() {
    /* Use __builtin with volatile argument */
    volatile int v = 10;
    if (__builtin_constant_p(v)) {
        /* This branch unlikely but forces analysis */
        printf("Constant: %d\n", v);
    }
    
    /* Use __builtin_expect with volatile */
    if (__builtin_expect(v > 5, 1)) {
        /* Force side effects */
        asm volatile("" : : "r"(v));
    }
}

/* Pattern 5: Destructor with similar attributes */
static void __attribute__((destructor, visibility("hidden"))) 
hidden_destructor() {
    /* Access volatile with hidden visibility */
    int val = hidden_volatile;
    asm volatile("" : : "r"(val));
}

/* Pattern 6: Artificial/ignored function */
static inline void __attribute__((unused, always_inline)) 
artificial_helper() {
    /* Use __builtin_unreachable to create interesting control flow */
    if (hidden_volatile == 0) {
        __builtin_unreachable();
    }
}

/* Pattern 7: Complex function pointer type */
typedef void (*complex_fn_ptr)(void) __attribute__((nothrow));

/* Pattern 8: External volatile function pointer */
extern volatile void (* volatile ext_fn_ptr)(void);

/* Pattern 9: Hidden namespace for C++ (if compiled as C++) */
#ifdef __cplusplus
namespace hidden_ns __attribute__((visibility("hidden"))) {
    extern "C" void hidden_extern_func();
}
#endif

/* Pattern 10: Use #pragma for visibility */
#pragma GCC visibility push(hidden)
extern int pragma_hidden_var;
#pragma GCC visibility pop

/* Helper function that uses atomic operations (may trigger libatomic calls) */
static int atomic_helper() {
    atomic_int atomic_var = ATOMIC_VAR_INIT(0);
    atomic_store_explicit(&atomic_var, 1, memory_order_seq_cst);
    return atomic_load_explicit(&atomic_var, memory_order_seq_cst);
}

/* Main function orchestrates all patterns */
int main() {
    int checksum = 0;
    
    /* 1. Access hidden volatile variable */
    checksum ^= hidden_volatile;
    
    /* 2. Access TLS variable */
    checksum ^= tls_var;
    
    /* 3. Call atomic helper */
    checksum ^= atomic_helper();
    
    /* 4. Use external volatile function pointer (if available) */
    if (ext_fn_ptr) {
        /* Take address to force reference */
        checksum ^= (int)(long)ext_fn_ptr;
    }
    
    /* 5. Use artificial helper */
    artificial_helper();
    
    /* 6. Complex function pointer type usage */
    complex_fn_ptr fn = 0;
    checksum ^= (int)(long)fn;
    
    /* 7. Try-catch block for C++ (triggers exception handling infrastructure) */
    #ifdef __cplusplus
    try {
        /* Attempt to call external nothrow function */
        external_nothrow_func();
    } catch (...) {
        /* Catch any exception */
        checksum ^= 0xCAFEBABE;
    }
    #endif
    
    /* 8. Use __builtin with side effects */
    volatile int test_val = checksum;
    if (__builtin_expect(test_val != 0, 0)) {
        asm volatile("" : "+r"(test_val));
    }
    
    /* 9. Reference pragma-hidden variable */
    checksum ^= (int)(long)&pragma_hidden_var;
    
    /* 10. Force use of all values to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    /* Additional: Create a volatile asm to force code generation */
    asm volatile (
        "/* Force code generation %0 %1 */"
        : 
        : "r"(checksum), "r"(tls_var)
    );
    
    return checksum & 0xFF;
}

/* External function declarations (no definitions to keep them external) */
extern "C" void external_nothrow_func() {
    /* Empty implementation if linked, but declaration should keep it external */
}

#ifdef __cplusplus
namespace hidden_ns {
    extern "C" void hidden_extern_func() {
        /* Empty */
    }
}
#endif

/* Define the pragma-hidden variable */
int pragma_hidden_var = 123;

/* Define external volatile function pointer */
volatile void (* volatile ext_fn_ptr)(void) = 0;
