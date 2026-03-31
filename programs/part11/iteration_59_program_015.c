/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -c test_targhooks.c -o test_targhooks.o */
/* Also try: g++ -O1 -fvisibility-inlines-hidden -fexceptions -c test_targhooks.cc -o test_targhooks.o */

#ifdef __cplusplus
extern "C" {
#endif

/* Force generation of volatile external symbol with hidden visibility */
extern volatile int hidden_volatile __attribute__((visibility("hidden")));
volatile int hidden_volatile = 0;

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 42;

/* Function with constructor attribute and hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) init_func() {
    /* Use volatile to prevent optimization */
    volatile int x = 0;
    /* Complex builtin that might need helper */
    if (__builtin_constant_p(x)) {
        hidden_volatile = 1;
    }
}

/* External nothrow function declaration */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Function pointer with volatile qualifier */
extern volatile void (* volatile volatile_func_ptr)(void);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
/* C++ specific code for exception handling */
#include <atomic>

/* Atomic operation that might need libatomic helpers */
std::atomic<int> atomic_var{0};

/* Try block with external nothrow function */
void test_exception_context() {
    try {
        external_nothrow_func();
    } catch (...) {
        /* Ignore */
    }
}

/* Inline function with unreachable that might be marked artificial */
static inline __attribute__((unused, always_inline)) 
void unreachable_helper() {
    if (atomic_var.load(std::memory_order_seq_cst) > 1000) {
        __builtin_unreachable();
    }
}
#endif

/* Main function orchestrating everything */
int main() {
    /* Access volatile external with hidden visibility */
    int val = hidden_volatile + 1;
    
    /* Use TLS variable */
    tls_var += val;
    
    /* Call constructor-initiated code via volatile access */
    hidden_volatile = tls_var;
    
#ifdef __cplusplus
    /* Use atomic operations */
    atomic_var.fetch_add(1, std::memory_order_seq_cst);
    
    /* Test exception context */
    test_exception_context();
    
    /* Use the unreachable helper */
    unreachable_helper();
#endif
    
    /* Use builtin with volatile argument */
    volatile int cond = 0;
    if (__builtin_expect(cond, 0)) {
        hidden_volatile = 100;
    }
    
    /* Take address of volatile function pointer */
    if (volatile_func_ptr) {
        /* Don't actually call it (would crash), just reference it */
        void (*local_ptr)(void) = (void (*)(void))volatile_func_ptr;
        (void)local_ptr;
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile ("" : : "r"(val), "r"(tls_var));
    
    return val + tls_var;
}

/* Additional definitions to satisfy references */
#ifdef __cplusplus
extern "C" {
#endif

/* Define the volatile function pointer */
volatile void (* volatile volatile_func_ptr)(void) = 0;

#ifdef __cplusplus
}
#endif
