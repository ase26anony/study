/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* For C++: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force generation of hidden external artificial symbols */
#pragma GCC visibility push(hidden)

/* Pattern 1: External volatile with hidden visibility */
extern volatile int hidden_volatile_var;
volatile int hidden_volatile_var = 1;

/* Pattern 2: Function with constructor attribute and hidden visibility */
static void __attribute__((constructor, noinline)) hidden_init(void) {
    /* Use __builtin_expect with volatile to potentially generate helpers */
    if (__builtin_expect(hidden_volatile_var != 0, 1)) {
        hidden_volatile_var = 2;
    }
}

/* Pattern 3: External function declared nothrow */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Pattern 4: Complex builtin usage that might require internal helpers */
static int __attribute__((unused)) use_complex_builtin(int x) {
    /* __builtin_constant_p on non-constant expression */
    if (__builtin_constant_p(x + hidden_volatile_var)) {
        return 1;
    }
    /* __builtin_unreachable creates artificial control flow */
    if (x < 0) __builtin_unreachable();
    return x;
}

#pragma GCC visibility pop

/* Pattern 5: TLS with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 3;

/* Pattern 6: Function pointer with volatile qualifier */
extern void (* volatile volatile_func_ptr)(void);

/* Pattern 7: Inline assembly that references external symbols */
static void __attribute__((used)) force_symbol_reference(void) {
    /* Reference the external volatile to force its use */
    asm volatile("" : : "r"(hidden_volatile_var));
    
    /* Reference TLS variable */
    asm volatile("" : : "r"(tls_var));
}

#ifdef __cplusplus
/* C++ specific patterns */
#include <atomic>

/* Pattern 8: Atomic operations that may call into libatomic */
static void atomic_operation(void) {
    std::atomic<int> atom{0};
    atom.store(1, std::memory_order_seq_cst);
    int val = atom.load(std::memory_order_acquire);
    asm volatile("" : : "r"(val));
}

/* Pattern 9: Exception handling with external nothrow function */
static void try_external_nothrow(void) {
    try {
        /* external_nothrow_func(); */ /* Would need definition */
    } catch (...) {
        /* Catch all */
    }
}

/* Pattern 10: Hidden visibility namespace */
namespace __attribute__((visibility("hidden"))) hidden_ns {
    extern "C" void hidden_extern_func(void);
}

#endif /* __cplusplus */

/* Main function orchestrates everything */
int main(void) {
    /* Force reference to hidden volatile */
    int val = hidden_volatile_var;
    
    /* Use complex builtin */
    val += use_complex_builtin(val);
    
    /* Modify TLS */
    tls_var += val;
    
    /* Force symbol reference */
    force_symbol_reference();
    
#ifdef __cplusplus
    /* C++ specific operations */
    atomic_operation();
    try_external_nothrow();
#endif
    
    /* Use volatile function pointer if available */
    if (volatile_func_ptr) {
        /* volatile_func_ptr(); */ /* Would need definition */
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(val), "r"(tls_var));
    
    return val + tls_var;
}

/* Additional definitions to satisfy references */

/* Define the external nothrow function (but keep it external in a different TU) */
/* In real scenario, this would be in a separate compilation unit */
void external_nothrow_func(void) __attribute__((nothrow));
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

/* Define volatile function pointer */
void (* volatile volatile_func_ptr)(void) = 0;
