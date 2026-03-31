/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */
/* For C++ mode: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc */

#ifdef __cplusplus
extern "C" {
#endif

/* Force hidden visibility on external declarations */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;

/* External function with nothrow attribute */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

#pragma GCC visibility pop

/* Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) hidden_constructor(void) {
    /* Use __builtin_expect with volatile condition */
    volatile int cond = 1;
    if (__builtin_expect(cond, 0)) {
        /* Access hidden volatile to force side effects */
        int val = hidden_volatile_var;
        (void)val;
    }
}

/* Destructor with similar pattern */
static void __attribute__((destructor, visibility("hidden"))) hidden_destructor(void) {
    /* Use another builtin that might generate helpers */
    int x = 0;
    if (!__builtin_constant_p(x)) {
        tls_var = 1;
    }
}

/* Artificial inline function that might be ignored */
static inline __attribute__((unused, always_inline)) 
void artificial_helper(void) {
    /* Use __builtin_unreachable to create interesting control flow */
    if (hidden_volatile_var > 100) {
        __builtin_unreachable();
    }
}

#ifdef __cplusplus
}
#endif

/* C++ specific code to trigger exception handling paths */
#ifdef __cplusplus
#include <atomic>

namespace hidden_visibility {
    __attribute__((visibility("hidden"))) 
    extern "C" void cpp_external_func(void) noexcept;
    
    class HiddenClass {
    public:
        static std::atomic<int> atomic_counter;
        
        __attribute__((noinline))
        static void update_counter() {
            /* Atomic operation that might call into libatomic */
            atomic_counter.fetch_add(1, std::memory_order_seq_cst);
        }
    };
    
    std::atomic<int> HiddenClass::atomic_counter{0};
}

/* Try block calling nothrow external function */
void test_exception_context() {
    try {
        external_nothrow_func();
    } catch (...) {
        /* Catch all but the function is nothrow */
    }
}
#endif

/* Main function orchestrates everything */
int main(void) {
    /* Define the previously declared volatile variable */
    volatile int hidden_volatile_var = 42;
    
    /* Use the artificial helper */
    artificial_helper();
    
    /* Access TLS variable */
    tls_var = hidden_volatile_var;
    
    /* Complex builtin usage that might generate internal helpers */
    int result = __builtin_constant_p(tls_var) ? 1 : 0;
    
    /* Force use of external function pointer pattern */
    {
        /* Declare volatile function pointer */
        extern void (* volatile volatile_func_ptr)(void);
        
        /* Take address of constructor to prevent optimization */
        void (*ctor_ptr)(void) = &hidden_constructor;
        (void)ctor_ptr;
    }
    
#ifdef __cplusplus
    /* Use C++ features if in C++ mode */
    hidden_visibility::HiddenClass::update_counter();
    test_exception_context();
    
    /* Use the atomic variable */
    int counter_val = hidden_visibility::HiddenClass::atomic_counter.load();
    result += counter_val;
#endif
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(result), "r"(tls_var), "r"(hidden_volatile_var));
    
    return result;
}

/* Define the external nothrow function (but keep it external by not defining here) */
/* The compiler will see the declaration but not the definition, keeping it DECL_EXTERNAL */

/* Alternative: Define a weak symbol with hidden visibility */
__attribute__((weak, visibility("hidden")))
void weak_hidden_func(void) {
    /* This might trigger the flags: weak symbols can be DECL_EXTERNAL */
}

/* Complex type that might generate artificial typedefs */
typedef void (*complex_func_ptr)(int, ...) __attribute__((nothrow));

/* Use the complex type in a dead assignment */
static void use_complex_type(void) {
    complex_func_ptr unused_ptr = 0;
    (void)unused_ptr;
}

/* Force the use of the complex type function */
__attribute__((constructor))
static void init_complex_type(void) {
    use_complex_type();
}
