/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* For C++ mode: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force generation of hidden visibility external symbols */
#pragma GCC visibility push(hidden)

/* Pattern 1: External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;
volatile int hidden_volatile_var = 42;

/* Pattern 2: External function with nothrow attribute and hidden visibility */
extern void hidden_nothrow_func(void) __attribute__((nothrow));
void hidden_nothrow_func(void) {
    /* Empty but external linkage with hidden visibility */
}

/* Pattern 3: Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

#pragma GCC visibility pop

/* Pattern 4: Constructor with hidden visibility and volatile builtin usage */
static void __attribute__((constructor, visibility("hidden"))) init_constructor(void) {
    /* Use __builtin_constant_p with volatile argument */
    volatile int v = 0;
    if (__builtin_constant_p(v)) {
        /* This branch won't be taken, but forces analysis */
        hidden_volatile_var = 1;
    }
    
    /* Use __builtin_expect with volatile condition */
    if (__builtin_expect(v != 0, 0)) {
        hidden_volatile_var = 2;
    }
}

/* Pattern 5: Destructor with similar properties */
static void __attribute__((destructor, visibility("hidden"))) cleanup_destructor(void) {
    /* Access TLS variable */
    tls_var++;
}

/* Pattern 6: Artificial/ignored declaration helper */
static inline void __attribute__((unused, always_inline)) 
artificial_helper(void) {
    /* Use __builtin_unreachable to create artificial control flow */
    if (hidden_volatile_var == 0) {
        __builtin_unreachable();
    }
}

/* Pattern 7: Complex function pointer type that may generate internal symbols */
typedef void (*complex_func_ptr)(void) __attribute__((nothrow));
static complex_func_ptr unused_ptr __attribute__((unused));

/* C++ specific patterns */
#ifdef __cplusplus
extern "C" {
#endif

/* Pattern 8: External volatile function pointer */
extern volatile void (*volatile ext_volatile_fn_ptr)(void);
volatile void (*volatile ext_volatile_fn_ptr)(void) = 0;

/* Pattern 9: Namespace with hidden visibility containing external declaration */
#ifdef __cplusplus
namespace hidden_visibility_ns __attribute__((visibility("hidden"))) {
    extern "C" void namespace_hidden_func(void) __attribute__((nothrow));
}
#endif

#ifdef __cplusplus
}
#endif

/* Main function that uses all patterns */
int main(void) {
    int checksum = 0;
    
    /* Force use of hidden volatile variable */
    checksum += hidden_volatile_var;
    
    /* Call hidden nothrow function */
    hidden_nothrow_func();
    
    /* Use TLS variable */
    checksum += tls_var;
    tls_var = checksum;
    
    /* Call artificial helper */
    artificial_helper();
    
    /* Use volatile function pointer */
    if (ext_volatile_fn_ptr) {
        ext_volatile_fn_ptr();
    }
    
    /* C++ specific: try block with nothrow external call */
    #ifdef __cplusplus
    try {
        hidden_nothrow_func();
    } catch (...) {
        checksum += 1000;
    }
    
    /* Use atomic operations that may call internal helpers */
    {
        volatile std::atomic<int> atomic_var(0);
        atomic_var.store(42, std::memory_order_seq_cst);
        checksum += atomic_var.load(std::memory_order_seq_cst);
    }
    #endif
    
    /* Force side effects to prevent optimization */
    __asm__ volatile ("" : : "r"(checksum));
    
    /* Use builtins with complex expressions */
    checksum += __builtin_constant_p(checksum) ? 1 : 0;
    checksum += __builtin_expect(checksum > 0, 1) ? 2 : 0;
    
    /* Access address of TLS variable (may generate internal helpers) */
    void* tls_addr = &tls_var;
    __asm__ volatile ("" : : "r"(tls_addr));
    
    return checksum & 0xFF; /* Return non-zero to prevent optimization */
}

/* Additional patterns in separate compilation unit style */

/* Pattern 10: Weak external symbol with hidden visibility */
extern int weak_hidden_var __attribute__((weak, visibility("hidden")));
int weak_hidden_var = 0;

/* Pattern 11: Alias to builtin that may trigger internal symbol generation */
extern void __internal_alias(void) __attribute__((alias("hidden_nothrow_func"), visibility("hidden")));

/* Pattern 12: Section attribute that may trigger special handling */
static int __attribute__((section(".unusual_section"), used, visibility("hidden"))) 
section_var = 999;

/* Pattern 13: Used attribute on external declaration */
extern int used_external __attribute__((used, visibility("hidden")));
int used_external = 123;

/* Pattern 14: Noinline artificial function */
static void __attribute__((noinline, artificial, visibility("hidden"))) 
noinline_artificial(void) {
    /* Complex expression that might need internal helpers */
    volatile double x = 3.14159;
    volatile double y = __builtin_pow(x, 2.0);
    __asm__ volatile ("" : : "r"(y));
}

/* Call the noinline artificial function from constructor */
static void __attribute__((constructor)) call_artificial(void) {
    noinline_artificial();
}
