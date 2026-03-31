/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* For C++ mode: g++ -O2 -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force generation of hidden visibility external symbols */
#pragma GCC visibility push(hidden)

/* Pattern 1: External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;
volatile int hidden_volatile_var = 42;

/* Pattern 2: Function with constructor attribute and hidden visibility */
static void __attribute__((constructor, noinline)) hidden_constructor() {
    /* Use __builtin with volatile argument to potentially generate helpers */
    volatile int v = 0;
    if (__builtin_constant_p(v)) {
        /* This branch unlikely but forces analysis */
        __builtin_unreachable();
    }
    __builtin_expect(!!v, 0);
}

/* Pattern 3: TLS with explicit model - may generate internal helpers */
__thread int tls_var __attribute__((tls_model("initial-exec")));

/* Pattern 4: Artificial function that might be ignored */
static inline __attribute__((unused, always_inline)) 
void artificial_ignored() {
    if (__builtin_expect(0, 0)) {
        __builtin_unreachable();
    }
}

#pragma GCC visibility pop

/* Pattern 5: External function declared nothrow for C++ exception handling */
#ifdef __cplusplus
extern "C" {
#endif
    void external_nothrow_func() __attribute__((nothrow));
#ifdef __cplusplus
}
#endif

/* Pattern 6: Complex function pointer type that might generate metadata */
typedef void (*complex_func_ptr)(void) __attribute__((nothrow));

/* Pattern 7: Volatile function pointer */
extern volatile void (*volatile_ext_fn_ptr)(void);

/* Main function orchestrates all patterns */
int main() {
    int checksum = 0;
    
    /* Access hidden volatile variable */
    checksum ^= hidden_volatile_var;
    
    /* Force use of TLS variable */
    tls_var = checksum;
    checksum += tls_var;
    
    /* Call artificial ignored function */
    artificial_ignored();
    
    /* Use volatile function pointer if available */
    if (volatile_ext_fn_ptr) {
        /* Take address to force reference */
        checksum ^= (int)(long)volatile_ext_fn_ptr;
    }
    
    /* Use complex function pointer type */
    complex_func_ptr unused_ptr = 0;
    checksum ^= (int)(long)unused_ptr;
    
    /* Force side effects with inline assembly */
    asm volatile ("" : : "r"(checksum));
    
    /* C++ specific patterns */
#ifdef __cplusplus
    try {
        /* Call external nothrow function - remains external reference */
        external_nothrow_func();
    } catch (...) {
        checksum += 1;
    }
    
    /* Use atomic operations that may call internal helpers */
    std::atomic<int> atomic_var(0);
    atomic_var.store(1, std::memory_order_seq_cst);
    checksum += atomic_var.load(std::memory_order_seq_cst);
#endif
    
    /* Additional pattern: __builtin with complex expression */
    volatile int cond = 0;
    if (__builtin_expect(cond, 0)) {
        checksum += 100;
    }
    
    /* Use __builtin_constant_p on non-constant */
    int non_const = checksum;
    if (__builtin_constant_p(non_const)) {
        checksum += 1000;
    }
    
    /* Return checksum to prevent optimization */
    return checksum & 0xFF;
}

/* Pattern 8: Namespace with hidden visibility (C++ only) */
#ifdef __cplusplus
namespace hidden_ns __attribute__((visibility("hidden"))) {
    extern "C" void hidden_extern_func() {
        /* Empty but externally visible with hidden visibility */
    }
}
#endif

/* Pattern 9: Weak symbol with hidden visibility */
extern int weak_hidden_var __attribute__((weak, visibility("hidden")));

/* Pattern 10: Force generation of exception handling personality function */
#ifdef __cplusplus
void __attribute__((noinline)) throw_if_true(bool cond) {
    if (cond) {
        throw 42;
    }
}

int test_exception() {
    try {
        throw_if_true(false);
        return 0;
    } catch (int x) {
        return x;
    }
}
#endif

/* Additional compilation suggestions:
   1. For LTO: gcc -O2 -flto -fuse-linker-plugin -c test_targhooks.c
   2. For maximum coverage: gcc -O0 -fno-inline -fvisibility=hidden -fno-builtin -fexceptions -c test_targhooks.c
   3. With debug symbols: gcc -g3 -O1 -fvisibility=hidden -fno-builtin -c test_targhooks.c
*/
