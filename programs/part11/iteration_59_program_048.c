/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* For C++ mode: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force generation of hidden visibility external symbols */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int hidden_volatile_ext;

/* External function with nothrow attribute */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* External function pointer with volatile qualifier */
extern volatile void (*volatile ext_fn_ptr)(void);

#pragma GCC visibility pop

/* Define the external volatile variable (still with hidden visibility) */
volatile int hidden_volatile_ext __attribute__((visibility("hidden"))) = 42;

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) hidden_constructor(void) {
    /* Use __builtin with volatile argument */
    int volatile_flag = 1;
    if (__builtin_constant_p(*(volatile int*)&volatile_flag)) {
        /* This branch is unlikely but forces analysis */
        tls_var++;
    }
    
    /* Use __builtin_expect with volatile condition */
    volatile int v = 0;
    if (__builtin_expect(v != 0, 0)) {
        hidden_volatile_ext++;
    }
}

/* Destructor with hidden visibility */
static void __attribute__((destructor, visibility("hidden"))) hidden_destructor(void) {
    /* Access volatile external */
    int local = hidden_volatile_ext + 1;
    
    /* Prevent optimization */
    __asm__ volatile ("" : : "r"(local));
}

/* Artificial function that may be ignored */
static inline void __attribute__((unused, always_inline)) 
artificial_ignored_func(void) {
    /* Use __builtin_unreachable to create artificial control flow */
    if (hidden_volatile_ext > 1000) {
        __builtin_unreachable();
    }
}

/* Complex function pointer type */
typedef void (*complex_func_ptr)(void) __attribute__((nothrow));

/* Function with C++ exception handling (compile as C++) */
#ifdef __cplusplus
extern "C" {
#endif

void test_exception_context(void) {
    /* Dead assignment with complex type that may generate artificial symbols */
    complex_func_ptr unused_ptr = 0;
    
    /* Force use of atomic operations (may call libatomic helpers) */
    int atomic_val = 0;
    /* Simulate atomic operation */
    __atomic_store_n(&atomic_val, 1, __ATOMIC_SEQ_CST);
    
    /* Call the artificial function */
    artificial_ignored_func();
}

#ifdef __cplusplus
} /* extern "C" */

/* C++ specific code */
#include <new>

/* Class with noexcept methods */
class NoThrowClass {
public:
    NoThrowClass() noexcept {
        tls_var++;
    }
    
    ~NoThrowClass() noexcept {
        hidden_volatile_ext--;
    }
    
    void method() noexcept {
        /* Use __builtin_expect */
        if (__builtin_expect(hidden_volatile_ext == 0, 0)) {
            tls_var = 0;
        }
    }
};

/* Namespace with hidden visibility */
namespace __attribute__((visibility("hidden"))) HiddenNamespace {
    extern "C" void hidden_extern_func(void) {
        /* Empty but external linkage with hidden visibility */
    }
}

#endif /* __cplusplus */

/* Main function orchestrates all patterns */
int main(void) {
    /* Access volatile external */
    int val = hidden_volatile_ext;
    
    /* Use TLS variable */
    tls_var = val + tls_var;
    
    /* Call function that uses atomic operations */
    test_exception_context();
    
#ifdef __cplusplus
    /* C++ specific tests */
    try {
        /* Create object with noexcept constructor */
        NoThrowClass obj;
        obj.method();
        
        /* Call hidden namespace function */
        HiddenNamespace::hidden_extern_func();
        
        /* Attempt to call external nothrow function */
        /* Note: not defined, so remains external */
        /* external_nothrow_func(); */ /* Would cause link error */
    } catch (...) {
        /* Should not be reached since all is noexcept */
        tls_var = -1;
    }
    
    /* Use placement new which may generate internal helpers */
    char buffer[sizeof(NoThrowClass)];
    NoThrowClass* p = new (buffer) NoThrowClass();
    p->~NoThrowClass();
#endif
    
    /* Attempt to use external function pointer */
    /* Note: not defined, so remains external volatile */
    /* if (ext_fn_ptr) ext_fn_ptr(); */ /* Would cause link error */
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = val + tls_var;
    
    /* Use inline asm to force use of checksum */
    __asm__ volatile (
        "/* %0 */"
        : 
        : "r" (checksum)
    );
    
    return checksum != 0 ? 0 : 1;
}

/* Additional external declarations to ensure they exist */
/* These remain undefined, keeping them external */

/* External nothrow function declaration */
void external_nothrow_func(void) __attribute__((nothrow));

/* External volatile function pointer */
volatile void (*volatile ext_fn_ptr)(void);

/* Hidden visibility variable in different section */
volatile int __attribute__((section(".hidden_data"), visibility("hidden"))) 
another_hidden_var = 1;
