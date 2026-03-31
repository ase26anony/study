/* test_targhooks.c - Test program to cover specific target hook lines */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */
/* Or for C++: g++ -O1 -fvisibility-inlines-hidden -fexceptions -c test_targhooks.cc */

#ifdef __cplusplus
#include <atomic>
extern "C" {
#endif

/* Force generation of hidden visibility external symbol */
#pragma GCC visibility push(hidden)
extern volatile int hidden_volatile;
volatile int hidden_volatile = 42;
#pragma GCC visibility pop

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

/* External volatile function pointer */
extern volatile void (* volatile ext_fn_ptr)(void);

/* Declare external nothrow function */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Hidden visibility constructor */
static void __attribute__((constructor, visibility("hidden"))) hidden_init(void) {
    /* Use __builtin with volatile argument */
    int volatile_flag = 1;
    if (__builtin_constant_p(*(volatile int*)&volatile_flag)) {
        /* This branch unlikely but forces analysis */
        asm volatile ("nop" : : "r"(volatile_flag));
    }
    
    /* Use __builtin_expect with volatile */
    volatile int v = 0;
    if (__builtin_expect(v != 0, 0)) {
        asm volatile ("nop");
    }
}

/* Artificial/ignored declaration candidate */
static inline void __attribute__((unused, always_inline)) 
artificial_helper(void) {
    /* This may generate artificial symbols */
    __builtin_unreachable();
}

/* Complex function pointer type */
typedef void (*complex_fp_t)(void) __attribute__((nothrow));

#ifdef __cplusplus
} /* extern "C" */

/* C++ specific code */
namespace hidden_ns __attribute__((visibility("hidden"))) {
    extern "C" void hidden_extern_func(void);
    
    class AtomicUser {
    public:
        void use_atomic() {
            std::atomic<int> atom{0};
            atom.store(1, std::memory_order_seq_cst);
            int val = atom.load(std::memory_order_seq_cst);
            asm volatile ("" : : "r"(val));
        }
    };
}

/* Try block with nothrow external function */
void test_exception_context(void) {
    try {
        external_nothrow_func();
    } catch (...) {
        /* Catch all but external_nothrow_func is nothrow */
        asm volatile ("nop");
    }
}
#endif

/* Main orchestrator */
int main(void) {
    /* 1. Access hidden volatile variable */
    int volatile_val = hidden_volatile + 1;
    asm volatile ("" : : "r"(volatile_val));
    
    /* 2. Use TLS variable */
    tls_var = 100;
    int tls_addr = (int)(long)&tls_var;
    asm volatile ("" : : "r"(tls_addr));
    
    /* 3. Call artificial helper (may be optimized but parsed) */
    artificial_helper();
    
    /* 4. Use complex function pointer type (dead code but parsed) */
    complex_fp_t fp = 0;
    if (0) {  /* Never true */
        fp();
    }
    
#ifdef __cplusplus
    /* 5. Use C++ features */
    hidden_ns::AtomicUser au;
    au.use_atomic();
    
    test_exception_context();
#endif
    
    /* 6. Use __builtin with side effects */
    int builtin_test = __builtin_popcount(volatile_val);
    
    /* 7. External volatile function pointer simulation */
    void (*local_fn)(void) = 0;
    /* Simulate external volatile pointer access */
    asm volatile ("mov %0, %1" : "=r"(local_fn) : "r"(ext_fn_ptr));
    
    /* Prevent dead code elimination */
    int checksum = volatile_val + tls_addr + builtin_test;
    
    return checksum & 0xFF;  /* Return non-zero but bounded */
}

/* Additional definitions to satisfy references */
#ifdef __cplusplus
extern "C" {
#endif

/* Define the hidden extern function */
void hidden_ns_hidden_extern_func(void) {
    asm volatile ("nop");
}

/* Weak definition of external nothrow function */
void __attribute__((weak)) external_nothrow_func(void) {
    asm volatile ("nop");
}

/* Define the volatile function pointer */
volatile void (* volatile ext_fn_ptr)(void) = 0;

#ifdef __cplusplus
}
#endif

/* Additional test for LTO scenarios */
#ifdef __cplusplus
/* Template that may generate hidden helpers */
template<typename T>
struct __attribute__((visibility("hidden"))) HiddenTemplate {
    static T value;
    static void __attribute__((noinline)) set(T v) {
        value = v;
    }
};

template<typename T>
T HiddenTemplate<T>::value = 0;

/* Instantiate to force generation */
template struct HiddenTemplate<int>;
#endif
