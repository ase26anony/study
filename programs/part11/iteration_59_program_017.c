/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* For C++ mode: g++ -O1 -fvisibility-inlines-hidden -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force generation of artificial, external, hidden symbols with specific attributes */

/* Pattern 1: Hidden visibility external volatile variable */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 42;

/* Pattern 2: TLS with explicit model - may generate internal helpers */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* Pattern 3: Constructor with hidden visibility and volatile builtin usage */
static void __attribute__((constructor, visibility("hidden"), noinline)) 
init_constructor(void) {
    /* Use __builtin_constant_p with volatile argument */
    volatile int v = 5;
    if (__builtin_constant_p(v)) {
        /* This branch won't be taken, but forces analysis */
        hidden_volatile = 1;
    }
    
    /* Use __builtin_expect with volatile condition */
    if (__builtin_expect(v > 0, 1)) {
        tls_var = hidden_volatile;
    }
}

/* Pattern 4: Artificial function marked as unused but with complex attributes */
static inline void __attribute__((unused, artificial, noinline))
artificial_helper(void) {
    /* Use __builtin_unreachable to create interesting control flow */
    if (hidden_volatile == 0) {
        __builtin_unreachable();
    }
    asm volatile ("" : : "r"(tls_var));
}

/* Pattern 5: External function declaration with nothrow attribute */
#ifdef __cplusplus
extern "C" {
#endif
    extern void external_nothrow_func(void) __attribute__((nothrow));
#ifdef __cplusplus
}
#endif

/* Pattern 6: Complex function pointer type that may generate artificial decls */
typedef void (*complex_fp_t)(void) __attribute__((nothrow));
static complex_fp_t volatile fp_var;

/* Pattern 7: Hidden visibility pragma section */
#pragma GCC visibility push(hidden)
extern int hidden_extern_var;
#pragma GCC visibility pop
int hidden_extern_var = 99;

/* Pattern 8: Function with volatile asm and builtin usage */
static int __attribute__((noinline, used))
use_volatile_builtins(void) {
    volatile int result = 0;
    
    /* Force use of volatile external variable */
    result = hidden_volatile;
    
    /* Use atomic builtin which may generate internal helpers */
    __atomic_store_n(&tls_var, result, __ATOMIC_SEQ_CST);
    
    /* Use builtin with side effects */
    result = __builtin_ffs(result);
    
    /* Call artificial helper */
    artificial_helper();
    
    return result;
}

/* Main function orchestrates all patterns */
int main(void) {
    int checksum = 0;
    
    /* Constructor will run before main */
    
    /* Pattern 1: Use hidden volatile variable */
    checksum += hidden_volatile;
    
    /* Pattern 2: Use TLS variable */
    checksum += tls_var;
    tls_var = checksum;
    
    /* Pattern 3: Use function with volatile builtins */
    checksum += use_volatile_builtins();
    
    /* Pattern 4: Set and use complex function pointer */
    fp_var = 0;
    if (fp_var) {
        fp_var();
    }
    
    /* Pattern 5: Try to call external nothrow function */
    #ifdef __cplusplus
    try {
        external_nothrow_func();
    } catch (...) {
        checksum += 1;
    }
    #else
    /* In C, just declare we might call it */
    if (hidden_volatile > 0) {
        /* external_nothrow_func(); */ /* Don't actually call undefined function */
    }
    #endif
    
    /* Pattern 6: Use hidden extern var */
    checksum += hidden_extern_var;
    
    /* Pattern 7: Use atomic operations that may generate internal helpers */
    int atomic_val = 0;
    __atomic_exchange(&tls_var, &checksum, &atomic_val, __ATOMIC_ACQ_REL);
    checksum += atomic_val;
    
    /* Pattern 8: Use __builtin_constant_p in a way that forces analysis */
    volatile int dynamic_val = checksum;
    if (!__builtin_constant_p(dynamic_val)) {
        checksum += 1000;
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(checksum));
    
    return checksum & 0xFF; /* Return non-zero to prevent optimization */
}

/* Additional C++ specific code */
#ifdef __cplusplus
/* Namespace with hidden visibility containing extern "C" function */
namespace hidden_visibility_ns {
    __attribute__((visibility("hidden")))
    extern "C" void hidden_extern_c_func(void) __attribute__((nothrow));
}

/* Static object with constructor that uses volatile */
class VolatileUser {
    volatile int* ptr;
public:
    VolatileUser() : ptr(&hidden_volatile) {
        *ptr = 123;
    }
    ~VolatileUser() {
        asm volatile ("" : : "r"(ptr));
    }
};

static VolatileUser volatile_user __attribute__((used));

/* Exception handling context with external nothrow */
void test_exception_context() {
    volatile int should_throw = 0;
    try {
        if (should_throw) {
            throw 42;
        }
        /* Reference external nothrow function */
        external_nothrow_func();
    } catch (int e) {
        hidden_volatile = e;
    }
}

/* Force instantiation */
__attribute__((constructor))
static void init_cpp_stuff() {
    test_exception_context();
}
#endif
