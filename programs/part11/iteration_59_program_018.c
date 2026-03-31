/* test_targhooks.c - Test program to trigger specific GCC target hooks */

/* Force C++ compilation for exception handling features */
#ifdef __cplusplus
    #include <atomic>
    #include <cstdio>
#else
    #include <stdio.h>
#endif

/* Pattern 1: Hidden visibility constructor with builtin usage */
static void __attribute__((constructor, visibility("hidden"))) 
hidden_constructor() {
    volatile int v = 0;
    /* Use __builtin_expect with volatile to potentially trigger internal helpers */
    if (__builtin_expect(v != 0, 0)) {
        __builtin_unreachable();
    }
}

/* Pattern 2: External volatile with hidden visibility */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 42;

/* Pattern 3: Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* Pattern 4: Artificial/ignored declaration */
static inline void __attribute__((unused, always_inline)) 
artificial_helper(int x) {
    if (x > 100) {
        __builtin_unreachable();
    }
}

/* Pattern 5: Complex function pointer type */
typedef void (*complex_fn_ptr)(void) __attribute__((nothrow));

/* C++ specific patterns */
#ifdef __cplusplus
/* Pattern 6: External nothrow function for exception context */
extern "C" void external_nothrow_func() __attribute__((nothrow));

/* Pattern 7: Atomic operations that may use internal helpers */
std::atomic<int> atomic_counter(0);

/* Pattern 8: Hidden visibility namespace with external linkage */
namespace __attribute__((visibility("hidden"))) hidden_ns {
    extern "C" int hidden_extern_var;
}

int hidden_ns::hidden_extern_var = 200;

/* Pattern 9: Volatile function pointer */
extern volatile void (* volatile ext_fn_ptr)(void);

/* Helper to force usage */
template<typename T>
void use(T&& arg) {
    asm volatile("" : : "r"(arg) : "memory");
}

#endif

/* Main function orchestrating all patterns */
int main() {
    int checksum = 0;
    
    /* Access hidden volatile */
    checksum ^= hidden_volatile;
    
    /* Use TLS variable */
    checksum ^= tls_var;
    tls_var = checksum;
    
    /* Call artificial helper */
    artificial_helper(checksum);
    
    /* Take address of complex function pointer type */
    complex_fn_ptr ptr = 0;
    checksum ^= (long)ptr;
    
    #ifdef __cplusplus
    /* C++ specific patterns */
    
    /* Use atomic operations */
    atomic_counter.fetch_add(1, std::memory_order_seq_cst);
    checksum ^= atomic_counter.load();
    
    /* Access hidden namespace variable */
    checksum ^= hidden_ns::hidden_extern_var;
    
    /* Try-catch with external nothrow function */
    try {
        /* The compiler may generate internal stubs for this */
        if (external_nothrow_func) {
            /* Force reference to prevent optimization */
            use(&external_nothrow_func);
        }
    } catch (...) {
        /* Empty handler */
    }
    
    /* Use volatile function pointer pattern */
    if (ext_fn_ptr) {
        /* Force reference */
        use(&ext_fn_ptr);
    }
    
    /* Force generation of exception handling tables */
    volatile bool flag = false;
    if (__builtin_expect(flag, 0)) {
        throw 42;
    }
    #endif
    
    /* Additional patterns that work in both C and C++ */
    
    /* Pattern: Use __builtin_constant_p with non-constant expression */
    volatile int non_const = checksum;
    if (__builtin_constant_p(non_const)) {
        checksum |= 0x80000000;
    }
    
    /* Pattern: Hidden visibility pragma section */
    #pragma GCC visibility push(hidden)
    extern int pragma_hidden_var;
    int pragma_hidden_var = 300;
    checksum ^= pragma_hidden_var;
    #pragma GCC visibility pop
    
    /* Pattern: Force generation of stack protection canary */
    char buffer[100];
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i ^ checksum;
    }
    
    /* Prevent dead code elimination */
    #ifdef __cplusplus
    std::printf("Checksum: %d\n", checksum);
    #else
    printf("Checksum: %d\n", checksum);
    #endif
    
    return checksum & 0xFF;
}

/* Additional definitions to satisfy references */
#ifdef __cplusplus
/* Declare but don't define to keep external */
extern "C" void external_nothrow_func() {
    /* Should not be reached if we keep it external */
}

volatile void (* volatile ext_fn_ptr)(void) = nullptr;
#endif

/* Pattern: Multiple compilation units simulation */
/* This could be split into separate files for better coverage */
static int __attribute__((used, visibility("hidden"))) 
internal_used_hidden = 999;

/* Force generation of static constructors/destructors */
void __attribute__((constructor)) global_ctor() {
    internal_used_hidden++;
}

void __attribute__((destructor)) global_dtor() {
    internal_used_hidden--;
}
