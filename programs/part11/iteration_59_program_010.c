/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* For C++ mode: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force hidden visibility on external declarations */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;

/* External function with nothrow attribute */
extern void external_nothrow_func(void) __attribute__((nothrow));

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

/* Function with constructor attribute and hidden visibility */
static void __attribute__((constructor, noinline)) hidden_constructor(void) {
    /* Use __builtin with volatile argument */
    volatile int v = 1;
    if (__builtin_constant_p(v)) {
        /* This branch won't be taken, but forces analysis */
        asm volatile ("nop");
    }
    
    /* Use __builtin_expect with volatile */
    if (__builtin_expect(v != 0, 1)) {
        hidden_volatile_var = 1;
    }
}

/* Artificial function that might be ignored */
static inline __attribute__((unused, always_inline)) 
void artificial_helper(void) {
    /* Use __builtin_unreachable */
    if (hidden_volatile_var == 0)
        __builtin_unreachable();
}

/* Complex function pointer type */
typedef void (*complex_func_ptr)(void) __attribute__((nothrow));

#pragma GCC visibility pop

/* Define the external volatile variable */
volatile int hidden_volatile_var = 0;

/* Define the TLS variable */
__thread int tls_var = 42;

#ifdef __cplusplus
extern "C" {
#endif

/* Try to trigger exception handling infrastructure */
void test_exception_context(void) {
    /* Access volatile external */
    int val = hidden_volatile_var;
    
    /* Use TLS variable */
    tls_var = val + 1;
    
    /* Call artificial helper */
    artificial_helper();
    
#ifdef __cplusplus
    /* In C++ mode, try-catch with external nothrow function */
    try {
        /* external_nothrow_func(); */ /* Don't call if not defined */
    } catch (...) {
        /* Empty handler */
    }
#endif
    
    /* Force use of values to prevent optimization */
    asm volatile ("" : : "r"(val), "r"(tls_var));
}

/* Main function orchestrates everything */
int main(void) {
    /* Constructor will run before main */
    
    /* Access hidden volatile */
    hidden_volatile_var++;
    
    /* Use TLS */
    tls_var = hidden_volatile_var * 2;
    
    /* Test exception context */
    test_exception_context();
    
    /* Complex atomic operation that might need libatomic helpers */
    int atomic_var = 0;
    __atomic_store_n(&atomic_var, tls_var, __ATOMIC_SEQ_CST);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = hidden_volatile_var + tls_var + atomic_var;
    
    /* Use checksum in a way that can't be optimized away */
    asm volatile ("# checksum = %0" : : "r"(checksum));
    
    return checksum != 0 ? 0 : 1;
}

#ifdef __cplusplus
}
#endif

/* Another attempt: declare external function with hidden visibility */
extern void __attribute__((visibility("hidden"))) hidden_external_func(void);

/* Use it in a dead code path that might still be analyzed */
void unused_dead_code(void) {
    if (0) {
        hidden_external_func();
    }
}

/* Force generation of stack protector helpers if enabled */
char large_buffer[256];

void use_buffer(void) {
    for (int i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = i;
    }
}
