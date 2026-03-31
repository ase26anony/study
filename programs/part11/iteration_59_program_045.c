/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c */

/* Force generation of hidden visibility external symbols */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;

/* External function with nothrow attribute */
extern void hidden_nothrow_func(void) __attribute__((nothrow));

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

#pragma GCC visibility pop

/* Define the volatile variable */
volatile int hidden_volatile_var = 1;

/* Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) hidden_init(void) {
    /* Use volatile to prevent optimization */
    asm volatile("" : : "r"(hidden_volatile_var));
}

/* Destructor with hidden visibility */
static void __attribute__((destructor, visibility("hidden"))) hidden_fini(void) {
    /* Use builtin with volatile argument */
    if (__builtin_constant_p((volatile int)hidden_volatile_var)) {
        /* This branch unlikely but forces analysis */
    }
}

/* Artificial function that might be generated */
static inline __attribute__((always_inline, unused)) 
void artificial_helper(volatile int *p) __attribute__((nothrow)) {
    /* Complex builtin usage */
    if (__builtin_expect(*p != 0, 0)) {
        __builtin_unreachable();
    }
}

/* C++ specific code */
#ifdef __cplusplus
extern "C" {
#endif

/* External function pointer with volatile qualifier */
extern volatile void (* volatile ext_fn_ptr)(void);

/* Function that uses try-catch with external nothrow function */
void test_exceptions(void) {
    /* Access TLS variable */
    tls_var = hidden_volatile_var;
    
    /* Use the artificial helper */
    artificial_helper(&hidden_volatile_var);
    
    /* Try to call external nothrow function */
    if (hidden_nothrow_func) {
        hidden_nothrow_func();
    }
    
    /* Call through volatile function pointer */
    if (ext_fn_ptr) {
        ext_fn_ptr();
    }
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* Complex builtin usage in main */
int main(void) {
    int result = 0;
    
    /* Force use of volatile variable */
    result += hidden_volatile_var;
    
    /* Use TLS variable */
    tls_var = result;
    result += tls_var;
    
    /* Call exception test function */
    test_exceptions();
    
    /* Use atomic builtins (may generate helpers) */
    __sync_fetch_and_add(&hidden_volatile_var, 1);
    
    /* Complex builtin with volatile */
    if (__builtin_constant_p((volatile int)hidden_volatile_var)) {
        result |= 0x1;
    }
    
    /* Use __builtin_expect with volatile */
    if (__builtin_expect(hidden_volatile_var > 100, 0)) {
        result |= 0x2;
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(result));
    
    return result;
}

/* Define a weak external symbol with hidden visibility */
extern int weak_hidden_var __attribute__((weak, visibility("hidden")));
int *get_weak_hidden_addr(void) {
    return &weak_hidden_var;
}
