/* test_targhooks.c - Test program to trigger target-specific hooks */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */
/* Or for C++: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc */

#ifdef __cplusplus
extern "C" {
#endif

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
extern volatile void (*volatile volatile_func_ptr)(void);

/* Pattern 5: Built-in function that may require internal wrapper */
static inline int use_builtin_constant_p(volatile int x) {
    /* This may trigger internal symbol generation */
    return __builtin_constant_p(x) ? 1 : 0;
}

/* Pattern 6: Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) hidden_constructor(void) {
    /* Use volatile to prevent optimization */
    volatile int local = hidden_volatile_var;
    /* Use builtin with volatile argument */
    int result = use_builtin_constant_p(local);
    /* Force side effect */
    asm volatile("" : : "r"(result));
}

/* Pattern 7: Artificial/ignored declaration candidate */
static inline void __attribute__((unused, always_inline)) 
artificial_helper(void) {
    /* This may be marked artificial and ignored */
    __builtin_unreachable();
}

#pragma GCC visibility pop

#ifdef __cplusplus
} /* extern "C" */

/* C++ specific patterns */
namespace hidden_visibility_ns __attribute__((visibility("hidden"))) {
    extern "C" void cpp_external_func(void) noexcept;
    
    class ExceptionTest {
    public:
        ExceptionTest() {
            /* Access volatile external */
            volatile int val = hidden_volatile_var;
            asm volatile("" : : "r"(val));
        }
        
        void test_nothrow() noexcept {
            /* Call external nothrow function */
            external_nothrow_func();
        }
    };
}

/* Pattern 8: Try block with external nothrow function */
void test_exception_context(void) {
    try {
        hidden_visibility_ns::cpp_external_func();
    } catch (...) {
        /* Handle any exception */
    }
}

/* Pattern 9: Atomic operations that may use libatomic helpers */
#include <atomic>
void test_atomic_operations(void) {
    std::atomic<int> atomic_var{0};
    /* seq_cst operations may call into internal helpers */
    atomic_var.store(1, std::memory_order_seq_cst);
    int val = atomic_var.load(std::memory_order_seq_cst);
    asm volatile("" : : "r"(val));
}

#endif /* __cplusplus */

/* Main function orchestrating all patterns */
int main(void) {
    /* Pattern 1: Use external volatile with hidden visibility */
    int volatile_val = hidden_volatile_var;
    
    /* Pattern 2: Call external nothrow function (if available) */
    /* We'll take address to force reference even if not defined */
    void (*fn_ptr)(void) = (void (*)(void))&external_nothrow_func;
    
    /* Pattern 3: Use TLS variable */
    tls_var = volatile_val + 1;
    int tls_addr = (int)(long)&tls_var;
    
    /* Pattern 4: Use volatile function pointer if available */
    if (volatile_func_ptr) {
        /* Take address only, don't call undefined function */
        void* ptr = (void*)volatile_func_ptr;
        asm volatile("" : : "r"(ptr));
    }
    
    /* Pattern 5: Use builtin with complex expression */
    volatile int arg = 100;
    int builtin_result = use_builtin_constant_p(arg);
    
    /* Pattern 6: Constructor already called before main */
    
    /* Pattern 7: Use artificial helper */
    artificial_helper();
    
#ifdef __cplusplus
    /* C++ specific patterns */
    hidden_visibility_ns::ExceptionTest obj;
    obj.test_nothrow();
    test_exception_context();
    test_atomic_operations();
#endif
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = volatile_val ^ tls_addr ^ builtin_result ^ (int)(long)fn_ptr;
    
    /* Use checksum in a way that can't be optimized away */
    asm volatile("" : : "r"(checksum));
    
    return checksum & 0xFF; /* Return non-zero to indicate execution */
}

/* Additional definitions to satisfy references */

/* Define the external nothrow function */
void external_nothrow_func(void) {
    /* Simple implementation */
    volatile int x = 1;
    asm volatile("" : : "r"(x));
}

/* Define volatile function pointer */
volatile void (*volatile volatile_func_ptr)(void) = 0;

#ifdef __cplusplus
/* Define C++ external function */
extern "C" void hidden_visibility_ns::cpp_external_func(void) noexcept {
    volatile int y = 2;
    asm volatile("" : : "r"(y));
}
#endif
