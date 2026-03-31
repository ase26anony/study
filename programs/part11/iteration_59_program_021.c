/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* For C++ mode: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force generation of artificial, external, hidden visibility symbols */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdatomic.h>
#include <stdio.h>

/* Pattern 1: External volatile variable with hidden visibility */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 42;

/* Pattern 2: External function with nothrow attribute */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Pattern 3: Function pointer with volatile qualifier */
extern volatile void (*volatile ext_fn_ptr)(void);

/* Pattern 4: TLS variable with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

/* Pattern 5: Built-in usage with volatile arguments */
static int use_builtin_with_volatile(volatile int *ptr) {
    /* __builtin_constant_p with non-constant volatile argument */
    if (__builtin_constant_p(*ptr)) {
        return 1;
    }
    /* __builtin_expect with volatile condition */
    return __builtin_expect(*ptr > 0, 1);
}

/* Pattern 6: Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) 
hidden_constructor(void) {
    volatile int local_volatile = 123;
    use_builtin_with_volatile(&local_volatile);
}

/* Pattern 7: Artificial/ignored declaration candidate */
static inline void __attribute__((unused, always_inline)) 
artificial_helper(void) {
    __builtin_unreachable();
}

/* Pattern 8: Complex function pointer type */
typedef void (*complex_fp_t)(void) __attribute__((nothrow));

/* Pattern 9: External declaration with hidden visibility in C++ namespace */
#ifdef __cplusplus
namespace hidden_ns __attribute__((visibility("hidden"))) {
    extern "C" void namespace_hidden_func(void);
}
#endif

/* Pattern 10: Use #pragma for visibility control */
#pragma GCC visibility push(hidden)
extern int pragma_hidden_var;
#pragma GCC visibility pop

/* Main function that uses all patterns */
int main(void) {
    int checksum = 0;
    
    /* Use external volatile with hidden visibility */
    checksum += hidden_volatile;
    
    /* Force side effects with asm */
    asm volatile("" : "+r" (checksum));
    
    /* Use TLS variable */
    tls_var = checksum;
    checksum += tls_var;
    
    /* Use volatile function pointer pattern */
    if (ext_fn_ptr) {
        /* Indirect call through volatile pointer */
        void (*local_fp)(void) = (void (*)(void))ext_fn_ptr;
        /* local_fp(); */ /* Don't actually call undefined function */
    }
    
    /* Use complex function pointer type */
    complex_fp_t fp = 0;
    checksum += (long)fp;
    
#ifdef __cplusplus
    /* C++ specific patterns */
    try {
        /* Call external nothrow function */
        external_nothrow_func();
    } catch (...) {
        checksum += 1;
    }
    
    /* Use atomic operations that may need libatomic helpers */
    _Atomic int atomic_var = 0;
    atomic_fetch_add_explicit(&atomic_var, 1, memory_order_seq_cst);
    checksum += atomic_var;
#endif
    
    /* Use artificial helper */
    artificial_helper();
    
    /* Use pragma-controlled variable */
    pragma_hidden_var = checksum;
    checksum += pragma_hidden_var;
    
    /* Print to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}

/* Define some of the external symbols to avoid linker errors */
void external_nothrow_func(void) {
    /* Empty implementation */
}

int pragma_hidden_var = 0;

volatile void (*volatile ext_fn_ptr)(void) = 0;

#ifdef __cplusplus
namespace hidden_ns {
    extern "C" void namespace_hidden_func(void) {
        /* Empty implementation */
    }
}
}

/* Additional C++ test case */
#ifdef __cplusplus
#include <exception>

/* Class with noexcept methods that may need exception helpers */
class NoThrowClass {
public:
    NoThrowClass() noexcept {
        volatile int x = 0;
        __atomic_store_n(&x, 1, __ATOMIC_SEQ_CST);
    }
    
    ~NoThrowClass() noexcept {
        __builtin_unreachable();
    }
    
    void method() noexcept {
        throw std::exception();
    }
};

/* Global object with hidden visibility */
NoThrowClass __attribute__((visibility("hidden"))) global_obj;

/* Template that may generate artificial symbols */
template<typename T>
struct HiddenTemplate {
    static T value __attribute__((visibility("hidden")));
    
    static void set(T v) noexcept {
        value = v;
    }
};

template<typename T>
T HiddenTemplate<T>::value = T();

/* Instantiate template */
template struct HiddenTemplate<int>;
#endif
