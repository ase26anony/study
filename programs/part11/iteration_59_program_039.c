/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* For C++ mode: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force generation of hidden visibility external symbols */
#pragma GCC visibility push(hidden)

/* Pattern 1: External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;
volatile int hidden_volatile_var = 42;

/* Pattern 2: External function with nothrow attribute */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Pattern 3: Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

/* Pattern 4: Function pointer with volatile qualifier */
extern volatile void (*volatile ext_fn_ptr)(void);

/* Pattern 5: Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) hidden_constructor(void) {
    /* Use builtin with volatile argument to potentially trigger helper generation */
    volatile int v = 0;
    if (__builtin_constant_p(v)) {
        /* This branch won't be taken, but may trigger internal symbol creation */
        asm volatile ("nop");
    }
}

/* Pattern 6: Artificial inline function with unreachable */
static inline __attribute__((unused, always_inline)) 
void artificial_unreachable(void) {
    __builtin_unreachable();
}

#pragma GCC visibility pop

/* Pattern 7: Complex function pointer type */
typedef void (*complex_fn_ptr)(void) __attribute__((nothrow));

/* Pattern 8: Namespace with hidden visibility (C++ only) */
#ifdef __cplusplus
namespace hidden_ns __attribute__((visibility("hidden"))) {
    extern "C" void hidden_extern_func(void);
}
#endif

/* Pattern 9: Builtin with complex expression */
static int use_builtin_expect(void) {
    volatile int condition = 1;
    return __builtin_expect(condition, 1) ? 100 : 200;
}

/* Pattern 10: Atomic operations that may call internal helpers */
#ifdef __cplusplus
#include <atomic>
static int use_atomic(void) {
    std::atomic<int> atomic_var{0};
    atomic_var.store(1, std::memory_order_seq_cst);
    return atomic_var.load(std::memory_order_seq_cst);
}
#else
/* For C, use GCC atomic builtins */
static int use_atomic(void) {
    int atomic_var = 0;
    __atomic_store_n(&atomic_var, 1, __ATOMIC_SEQ_CST);
    return __atomic_load_n(&atomic_var, __ATOMIC_SEQ_CST);
}
#endif

/* Main function orchestrates all patterns */
int main(void) {
    int checksum = 0;
    
    /* Access hidden volatile variable */
    checksum += hidden_volatile_var;
    
    /* Use builtin with volatile */
    checksum += use_builtin_expect();
    
    /* Access TLS variable */
    tls_var = 123;
    checksum += tls_var;
    
    /* Use atomic operations */
    checksum += use_atomic();
    
    /* Try-catch block for exception handling (C++ only) */
    #ifdef __cplusplus
    try {
        /* Call to external nothrow function (remains undefined, keeping it external) */
        /* This may trigger creation of exception handling infrastructure */
        external_nothrow_func();
    } catch (...) {
        checksum += 1;
    }
    #endif
    
    /* Use complex function pointer type in dead code */
    complex_fn_ptr dead_ptr = 0;
    if (0) {
        dead_ptr();
    }
    
    /* Force use of artificial function */
    if (checksum > 1000) {
        artificial_unreachable();
    }
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(checksum));
    
    return checksum & 0xFF; /* Return non-zero to ensure execution */
}

/* Additional definitions to satisfy references */

/* Define the external function pointer */
volatile void (*volatile ext_fn_ptr)(void) = 0;

/* Weak definition of external nothrow function to keep it external but resolvable */
void external_nothrow_func(void) __attribute__((weak, nothrow));
void external_nothrow_func(void) {
    /* Empty implementation */
}

#ifdef __cplusplus
namespace hidden_ns {
    extern "C" void hidden_extern_func(void) {
        /* Empty implementation */
    }
}
#endif

/* Additional test case: volatile asm to force side effects */
static void force_volatile_access(void) {
    volatile int tmp = 0;
    asm volatile ("# volatile asm" : "+r"(tmp));
}

/* Pattern: Use __sync builtins which may generate internal helpers */
static int use_sync_builtins(void) {
    int val = 5;
    return __sync_add_and_fetch(&val, 3);
}

/* Call the sync builtin function */
static int extra_checksum = 0;
__attribute__((constructor)) static void init_extra(void) {
    extra_checksum = use_sync_builtins();
    force_volatile_access();
}
