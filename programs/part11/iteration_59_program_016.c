/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* Also try: g++ -O1 -fvisibility-inlines-hidden -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force generation of hidden visibility external symbols */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;

/* External function with nothrow attribute */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* External volatile function pointer */
extern volatile void (*volatile ext_fn_ptr)(void);

#pragma GCC visibility pop

/* Define the volatile variable (still with hidden visibility) */
volatile int hidden_volatile_var __attribute__((visibility("hidden"))) = 42;

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec"))) = 100;

/* Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) hidden_constructor(void) {
    /* Use __builtin_expect with volatile condition */
    volatile int cond = 1;
    if (__builtin_expect(cond, 0)) {
        hidden_volatile_var++;
    }
    
    /* Use __builtin_constant_p on non-constant expression */
    int x = hidden_volatile_var;
    if (!__builtin_constant_p(x)) {
        tls_var = x;
    }
}

/* Destructor with hidden visibility */
static void __attribute__((destructor, visibility("hidden"))) hidden_destructor(void) {
    /* Force use of builtins with side effects */
    asm volatile("" : : "r"(tls_var));
}

/* Artificial function that may be ignored */
static inline void __attribute__((unused, artificial)) artificial_helper(void) {
    /* This might trigger compiler-internal symbol generation */
    __builtin_unreachable();
}

/* Complex function pointer type */
typedef void (*complex_fn_ptr)(void) __attribute__((nothrow));

#ifdef __cplusplus
extern "C" {
#endif

/* Function with C linkage and hidden visibility */
void __attribute__((visibility("hidden"))) hidden_extern_func(void) {
    /* Access volatile external */
    int local = hidden_volatile_var + 1;
    
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(local));
    
    /* Call external nothrow function if declared */
    #ifdef __cplusplus
    try {
        external_nothrow_func();
    } catch (...) {
        /* Should not happen with nothrow */
    }
    #endif
    
    /* Use the volatile function pointer */
    if (ext_fn_ptr) {
        ext_fn_ptr();
    }
}

#ifdef __cplusplus
}
#endif

/* Inline function with unreachable that may generate artificial symbols */
static inline __attribute__((always_inline)) int inline_check(int x) {
    if (x < 0) {
        __builtin_unreachable();
    }
    return x + 1;
}

/* Main function that uses all patterns */
int main(void) {
    /* Initialize TLS */
    tls_var = 200;
    
    /* Use hidden volatile variable */
    int val = hidden_volatile_var;
    
    /* Call hidden extern function */
    hidden_extern_func();
    
    /* Use inline function with builtin */
    val = inline_check(val);
    
    /* Complex expression with builtins */
    volatile int test = 0;
    if (__builtin_constant_p(test)) {
        /* This branch won't be taken, but forces analysis */
        artificial_helper();
    }
    
    /* Atomic operations (C++ style if available) */
    #ifdef __cplusplus
    {
        std::atomic<int> atomic_var(0);
        atomic_var.store(1, std::memory_order_seq_cst);
        val += atomic_var.load(std::memory_order_seq_cst);
    }
    #endif
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long checksum = (unsigned long)&tls_var;
    checksum ^= (unsigned long)val;
    checksum ^= (unsigned long)&hidden_volatile_var;
    
    /* Print checksum to force side effects */
    printf("Checksum: %lu\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}

/* Dummy definitions to satisfy references */
void external_nothrow_func(void) {
    /* Empty but defined to avoid linker errors */
}

volatile void (*volatile ext_fn_ptr)(void) = 0;

/* Additional patterns in separate compilation unit style */
#ifndef __cplusplus
/* C-specific patterns */

/* Use __builtin_choose_expr with volatile */
static int choose_expr_test(void) {
    volatile int v = 0;
    return __builtin_choose_expr(v == 0, 1, 0);
}

/* Use offsetof with volatile struct */
struct volatile_struct {
    volatile int a;
    int b;
};

static size_t get_offset(void) {
    return __builtin_offsetof(struct volatile_struct, b);
}

#endif
