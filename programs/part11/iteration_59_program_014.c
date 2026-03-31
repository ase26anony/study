#ifdef __cplusplus
#include <atomic>
#include <cstdio>
#else
#include <stdio.h>
#endif

/* Pattern 1: Hidden visibility constructor with builtin usage */
static void __attribute__((constructor, visibility("hidden"))) hidden_init() {
    volatile int v = 0;
    /* Use __builtin_constant_p with non-constant to potentially create helper */
    if (__builtin_constant_p(v)) {
        /* This branch won't be taken, but may trigger internal symbol creation */
        asm volatile ("nop" : : "r"(v));
    }
}

/* Pattern 2: External volatile with hidden visibility */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 42;

/* Pattern 3: Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* Pattern 4: Artificial/ignored declaration candidate */
static inline void __attribute__((unused, always_inline)) 
artificial_helper() {
    /* __builtin_unreachable may create artificial control flow */
    if (hidden_volatile > 1000) {
        __builtin_unreachable();
    }
}

#ifdef __cplusplus
/* Pattern 5: Nothrow external function for exception context */
extern "C" void external_nothrow_func() __attribute__((nothrow));

/* Hidden visibility namespace */
namespace __attribute__((visibility("hidden"))) hidden_ns {
    extern "C" int hidden_extern_var;
}

int hidden_ns::hidden_extern_var = 200;

/* Pattern 6: Complex function pointer type */
typedef void (*complex_fp_t)(int, ...) __attribute__((nothrow));
complex_fp_t dead_fp __attribute__((unused));

/* Pattern 7: Atomic operations that may call libatomic helpers */
std::atomic<int> atomic_counter;

void test_atomic_helpers() {
    atomic_counter.store(1, std::memory_order_seq_cst);
    int val = atomic_counter.load(std::memory_order_acquire);
    atomic_counter.fetch_add(val, std::memory_order_relaxed);
}
#endif

/* Pattern 8: Volatile function pointer */
extern volatile void (*volatile ext_fn_ptr)(void);
volatile void (*volatile ext_fn_ptr)(void) = 0;

/* Pattern 9: Push/pop visibility pragma */
#pragma GCC visibility push(hidden)
extern int pragma_hidden_var;
#pragma GCC visibility pop
int pragma_hidden_var = 300;

/* Pattern 10: Builtin expect with volatile condition */
int use_builtin_expect() {
    volatile int cond = hidden_volatile > 0;
    return __builtin_expect(cond, 1) ? 1 : 0;
}

int main() {
    int checksum = 0;
    
    /* Access hidden volatile - forces external linkage with side effects */
    checksum += hidden_volatile;
    
    /* Use TLS variable */
    checksum += tls_var;
    tls_var = checksum;
    
    /* Call artificial helper */
    artificial_helper();
    
    /* Use builtin expect pattern */
    checksum += use_builtin_expect();
    
#ifdef __cplusplus
    /* Try-catch with external nothrow function */
    try {
        /* external_nothrow_func is declared but not defined - remains external */
        if (checksum > 0) {
            /* This keeps the reference but won't actually call it */
            asm volatile ("# reference to external_nothrow_func" : : "i"(external_nothrow_func));
        }
    } catch (...) {
        checksum += 1000;
    }
    
    /* Test atomic helpers */
    test_atomic_helpers();
    checksum += atomic_counter.load(std::memory_order_relaxed);
    
    /* Access hidden namespace variable */
    checksum += hidden_ns::hidden_extern_var;
#endif
    
    /* Volatile function pointer usage */
    if (ext_fn_ptr) {
        /* Prevent actual call but keep reference */
        asm volatile ("# volatile fn ptr" : : "r"(ext_fn_ptr));
    }
    
    /* Access pragma-hidden variable */
    checksum += pragma_hidden_var;
    
    /* Complex computation to prevent optimization */
    checksum = (checksum * 1103515245 + 12345) & 0x7fffffff;
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional pattern: Inline assembly that references hidden symbols */
    asm volatile (
        "# References to force symbol generation\n"
        "# %0\n"
        "# %1\n"
        : 
        : "i"(hidden_volatile), "i"(tls_var@tlsgd)
    );
    
    return checksum == 0 ? 1 : 0;
}

/* Force generation of exception handling tables */
#ifdef __cplusplus
void dummy_exception_test() {
    volatile int* ptr = nullptr;
    try {
        if (hidden_volatile > 50) {
            *ptr = 42;  // Potential null dereference
        }
    } catch (...) {
        // Catch all
    }
}
#endif
