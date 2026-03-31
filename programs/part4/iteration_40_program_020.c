/* test_targhooks.c - Comprehensive built-in function test for targhooks.cc coverage */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int vol_global = 0;
static volatile unsigned int vol_seed = 12345;

/* ========== Helper Functions with Various Attributes ========== */

/* Function with explicit hidden visibility and nothrow attribute */
static int __attribute__((visibility("hidden"), nothrow, used))
hidden_static_builtin(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x);
    }
    return __builtin_clz((unsigned int)(-x));
}

/* Noinline function to prevent optimization */
static int __attribute__((noinline, noclone))
test_builtin_arithmetic(volatile int seed) {
    int result = 0;
    volatile int counter = seed;
    
    /* Loop with volatile counter to prevent dead code elimination */
    for (volatile int i = 0; i < 5; i++) {
        /* Use math built-ins */
        int abs_val = __builtin_abs(counter);
        /* Use sqrt built-in (float version to avoid double promotion issues) */
        float sqrt_val = __builtin_sqrtf((float)abs_val);
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        result += (int)sqrt_val + abs_val;
        counter += 7;
    }
    
    return result;
}

static unsigned int __attribute__((noinline, noclone))
test_builtin_bitops(volatile unsigned int seed) {
    unsigned int result = 0;
    
    /* Use various bit operation built-ins */
    result += __builtin_popcount(seed);
    result += __builtin_clz(seed | 1);  /* Ensure non-zero */
    result += __builtin_ctz(seed | 1);
    result += __builtin_ffs(seed | 1);
    
    /* Use parity built-in */
    result += __builtin_parity(seed);
    
    /* Byte swap built-in */
    result += __builtin_bswap32(seed);
    
    return result;
}

static int __attribute__((noinline, noclone))
test_builtin_overflow(volatile int a, volatile int b) {
    int result = 0;
    int overflow_result;
    bool overflow_flag;
    
    /* Test addition overflow */
    overflow_flag = __builtin_add_overflow(a, b, &overflow_result);
    result += overflow_result;
    if (overflow_flag) {
        result |= 0x1000;
    }
    
    /* Test multiplication overflow */
    overflow_flag = __builtin_mul_overflow(a, b, &overflow_result);
    result += overflow_result;
    if (overflow_flag) {
        result |= 0x2000;
    }
    
    /* Test subtraction overflow */
    overflow_flag = __builtin_sub_overflow(a, b, &overflow_result);
    result += overflow_result;
    if (overflow_flag) {
        result |= 0x4000;
    }
    
    return result;
}

/* Function with used attribute to force emission */
static void __attribute__((used, noinline))
force_builtin_usage(void) {
    volatile int x = 42;
    volatile int y = __builtin_abs(x);
    
    /* Use __builtin_constant_p */
    if (__builtin_constant_p(x)) {
        y += 10;
    }
    
    /* Use __builtin_types_compatible_p */
    if (__builtin_types_compatible_p(typeof(x), int)) {
        y += 20;
    }
    
    vol_global = y;
}

/* ========== External Linkage Simulation ========== */

/* Forward declaration of external function */
extern int external_builtin_helper(int) __attribute__((visibility("default")));

/* The actual definition (simulating external linkage within same file) */
int external_builtin_helper(int x) {
    /* Use __builtin_unreachable under specific condition */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* Use __builtin_assume to provide optimization hint */
    __builtin_assume(x >= 0);
    
    return __builtin_popcount((unsigned int)x);
}

/* Another external-like function */
void hidden_visibility_func(void) {
    volatile int x = vol_seed;
    
    /* Use __builtin_expect with probability hint */
    if (__builtin_expect(x > 1000, 0)) {
        x = __builtin_clz((unsigned int)x);
    }
    
    /* Use __builtin_prefetch */
    __builtin_prefetch(&vol_global, 0, 3);
    
    vol_global += x;
}

/* ========== Main Test Function ========== */

int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int checksum = 0;
    
    /* Initialize volatile seed from argc */
    vol_seed = (unsigned int)(seed * 1103515245 + 12345);
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops(vol_seed);
    
    /* Test 3: Overflow built-ins */
    checksum += test_builtin_overflow(seed, 1000);
    
    /* Test 4: Hidden visibility static function */
    checksum += hidden_static_builtin(seed);
    
    /* Test 5: Force builtin usage */
    force_builtin_usage();
    
    /* Test 6: External linkage simulation */
    checksum += external_builtin_helper(seed);
    
    /* Test 7: Hidden visibility function */
    hidden_visibility_func();
    
    /* Test 8: Direct built-in calls with volatile results */
    volatile int direct_result = 0;
    direct_result += __builtin_abs(seed - 50);
    direct_result += __builtin_clz((unsigned int)(seed | 1));
    direct_result += __builtin_ctz((unsigned int)(seed | 1));
    
    /* Use __builtin_choose_expr */
    int chosen = __builtin_choose_expr(
        seed > 0,
        __builtin_popcount((unsigned int)seed),
        __builtin_ffs(-seed)
    );
    direct_result += chosen;
    
    /* Final checksum computation */
    checksum += direct_result + vol_global;
    
    /* Prevent tail-call optimization */
    asm volatile("" : : "r"(checksum) : "memory");
    
    return checksum & 0xFF;
}

/* ========== Additional Complex Patterns ========== */

/* Function with assembly and built-in mix */
static int __attribute__((noinline))
mixed_asm_builtin(int x) {
    int result;
    
    /* Inline assembly with memory clobber */
    asm volatile (
        "mov %1, %%eax\n\t"
        "add $1, %%eax"
        : "=a"(result)
        : "r"(x)
        : "cc"
    );
    
    /* Built-in after assembly */
    result = __builtin_abs(result);
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Recursive function using built-ins */
static int __attribute__((noinline))
recursive_builtin(int x, int depth) {
    if (depth <= 0) {
        return __builtin_abs(x);
    }
    
    int next = __builtin_popcount((unsigned int)x);
    return recursive_builtin(next, depth - 1) + x;
}

/* Function using vector built-ins (if available) */
static void __attribute__((noinline))
vector_builtins(void) {
    /* Use __builtin_shuffle if available */
    #ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
    volatile int sync_val = 0;
    __sync_val_compare_and_swap(&sync_val, 0, 1);
    #endif
    
    /* Use atomic built-ins */
    volatile int atomic_val = 0;
    __atomic_store_n(&atomic_val, 42, __ATOMIC_SEQ_CST);
    int loaded = __atomic_load_n(&atomic_val, __ATOMIC_ACQUIRE);
    
    vol_global = loaded;
}
