/* test_targhooks.c - Test program to cover target hooks in GCC */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* Or for C++: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force C++ mode for exception handling features */
#ifdef __cplusplus
#include <atomic>
#include <cstdio>
#else
#include <stdio.h>
#endif

/* Pattern 1: Hidden visibility external volatile variable */
#pragma GCC visibility push(hidden)
extern volatile int hidden_volatile_var;
#pragma GCC visibility pop

/* Define it to satisfy linker (but keep it external for the declaration) */
volatile int hidden_volatile_var = 42;

/* Pattern 2: TLS with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* Pattern 3: External function with nothrow attribute */
#ifdef __cplusplus
extern "C" {
#endif
void external_nothrow_func(void) __attribute__((nothrow));
#ifdef __cplusplus
}
#endif

/* Pattern 4: Constructor with hidden visibility and builtin usage */
static void __attribute__((constructor, visibility("hidden"))) init_constructor(void) {
    /* Use __builtin_expect with volatile condition */
    volatile int cond = 1;
    if (__builtin_expect(cond, 0)) {
        /* Access hidden volatile to force side effects */
        int val = hidden_volatile_var;
        (void)val;
    }
    
    /* Use __builtin_constant_p on non-constant expression */
    int non_const = tls_var;
    if (!__builtin_constant_p(non_const)) {
        /* Force compiler to generate internal helpers */
        asm volatile("" : : "r"(non_const));
    }
}

/* Pattern 5: Destructor with similar properties */
static void __attribute__((destructor, visibility("hidden"))) cleanup_destructor(void) {
    /* Complex builtin usage */
    volatile int* ptr = &hidden_volatile_var;
    if (__builtin_constant_p((long)ptr)) {
        /* This branch unlikely but forces analysis */
        asm volatile("" : : "r"(ptr));
    }
}

/* Pattern 6: Artificial/ignored declaration helper */
static inline void __attribute__((unused, always_inline)) 
artificial_helper(void) {
    /* Use __builtin_unreachable to create interesting control flow */
    volatile int x = hidden_volatile_var;
    if (x == 0) {
        __builtin_unreachable();
    }
}

/* Pattern 7: External volatile function pointer */
extern volatile void (* volatile ext_fn_ptr)(void);

/* Pattern 8: Complex function pointer type that might generate internal typedefs */
typedef void (*complex_fn_ptr)(int, ...) __attribute__((nothrow));

/* Pattern 9: Hidden namespace with external C function (C++ only) */
#ifdef __cplusplus
namespace hidden_ns __attribute__((visibility("hidden"))) {
    extern "C" {
        void hidden_extern_func(void);
    }
}
#endif

/* Pattern 10: Atomic operations that may call libatomic helpers */
#ifdef __cplusplus
static void atomic_operations(void) {
    std::atomic<int> atomic_var{0};
    atomic_var.store(1, std::memory_order_seq_cst);
    int val = atomic_var.load(std::memory_order_acquire);
    (void)val;
}
#endif

/* Main function orchestrates all patterns */
int main(void) {
    int checksum = 0;
    
    /* 1. Access hidden volatile variable */
    checksum += hidden_volatile_var;
    
    /* 2. Use TLS variable */
    checksum += tls_var;
    tls_var = checksum;
    
    /* 3. Call artificial helper */
    artificial_helper();
    
    /* 4. Try-catch with external nothrow function (C++ only) */
    #ifdef __cplusplus
    try {
        /* Declare but don't define - keeps it external */
        external_nothrow_func();
    } catch (...) {
        /* Catch any exception */
        checksum += 1000;
    }
    
    /* 5. Perform atomic operations */
    atomic_operations();
    #endif
    
    /* 6. Use external volatile function pointer if available */
    if (ext_fn_ptr != 0) {
        /* This should remain external reference */
        checksum += 2000;
    }
    
    /* 7. Complex function pointer usage */
    complex_fn_ptr fn = 0;
    (void)fn;
    
    /* 8. Force use of values to prevent optimization */
    asm volatile("" : : "r"(checksum), "r"(tls_var), "r"(&hidden_volatile_var));
    
    /* 9. Print checksum to prevent dead code elimination */
    #ifdef __cplusplus
    std::printf("Checksum: %d\n", checksum);
    #else
    printf("Checksum: %d\n", checksum);
    #endif
    
    return checksum != 0 ? 0 : 1;
}

/* Additional definitions to satisfy references */
#ifdef __cplusplus
namespace hidden_ns {
    extern "C" void hidden_extern_func(void) {
        /* Empty implementation */
    }
}
#endif

/* Weak definition for external function pointer */
volatile void (* volatile ext_fn_ptr)(void) = 0;

/* Additional test for LTO scenarios */
#ifdef __cplusplus
/* Template that may generate hidden external symbols */
template<typename T>
struct __attribute__((visibility("hidden"))) HiddenTemplate {
    static T value;
    static void __attribute__((noinline)) set_value(T v) {
        value = v;
    }
};

template<typename T>
T HiddenTemplate<T>::value = T();

/* Instantiate template to force symbol generation */
template struct HiddenTemplate<int>;
#endif
