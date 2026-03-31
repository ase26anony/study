/* test_targhooks.c - Comprehensive built-in function test for targhooks.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int global_counter = 0;
static volatile int optimization_barrier = 0;

/* Helper with multiple attributes that may interact with hook */
static int __attribute__((used, noinline, noclone)) 
test_builtin_arithmetic(volatile int seed) {
    int result = 0;
    volatile int temp;
    
    /* Use math built-ins in a loop */
    for (volatile int i = 0; i < 5; i++) {
        int val = seed + i * 100;
        
        /* __builtin_abs with volatile argument */
        temp = __builtin_abs(val);
        result += temp;
        
        /* __builtin_expect to influence branching */
        if (__builtin_expect((val & 1) != 0, 0)) {
            result -= 1;
        }
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Function with explicit hidden visibility attribute */
static void __attribute__((visibility("hidden"), nothrow, used))
test_attributed_function(volatile int *output) {
    /* Use __builtin_expect with attributes */
    int x = *output;
    if (__builtin_expect(x > 100, 0)) {
        *output = __builtin_abs(x);
    } else {
        /* Use __builtin_clz */
        unsigned int ux = (unsigned int)x;
        if (ux != 0) {
            *output += __builtin_clz(ux);
        }
    }
    
    /* Mark as artificial/used through builtin */
    __builtin___clear_cache((void *)output, (void *)(output + 1));
}

/* Function using overflow built-ins */
static int __attribute__((noinline))
test_builtin_overflow(volatile int a, volatile int b) {
    int result = 0;
    int overflow;
    
    /* __builtin_add_overflow */
    if (__builtin_add_overflow(a, b, &result)) {
        result = __builtin_abs(a);
    }
    
    /* __builtin_mul_overflow */
    int product;
    if (__builtin_mul_overflow(a, b, &product)) {
        result += __builtin_clz((unsigned int)product);
    } else {
        result += product;
    }
    
    /* __builtin_sub_overflow */
    int diff;
    __builtin_sub_overflow(a, b, &diff);
    result += diff;
    
    return result;
}

/* Function with bit operation built-ins */
static unsigned int __attribute__((noinline))
test_builtin_bitops(volatile unsigned int seed) {
    unsigned int result = 0;
    
    /* Various bit operation built-ins */
    result += __builtin_popcount(seed);
    result += __builtin_ctz(seed | 1);  /* Avoid ctz(0) */
    result += __builtin_ffs(seed);
    
    /* Use in conditional */
    if (__builtin_parity(seed)) {
        result = __builtin_bswap32(result);
    }
    
    return result;
}

/* External function definition (simulating another TU) */
int external_builtin_user(int x) {
    /* Use __builtin_unreachable under specific condition */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* Use __builtin_constant_p */
    if (__builtin_constant_p(x)) {
        return __builtin_abs(x);
    }
    
    /* Complex expression with builtin */
    return __builtin_sqrtf((float)(x * x + 1));
}

/* Hidden visibility function definition */
void __attribute__((visibility("hidden"), nothrow))
hidden_visibility_func(void) {
    /* Use sync builtins */
    int val = __sync_fetch_and_add(&global_counter, 1);
    
    /* Use __builtin_assume_aligned */
    int *ptr = &val;
    ptr = (int*)__builtin_assume_aligned(ptr, 4);
    
    /* Prevent dead code elimination */
    optimization_barrier = val;
}

/* Main test driver */
int main(int argc, char **argv) {
    volatile int seed = argc;
    int checksum = 0;
    
    /* Initialize with time for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL) & 0xFF;
    }
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops((unsigned int)seed);
    
    /* Test 3: Overflow built-ins */
    checksum += test_builtin_overflow(seed, seed * 2);
    
    /* Test 4: Attributed function with built-ins */
    volatile int attr_output = checksum;
    test_attributed_function(&attr_output);
    checksum += attr_output;
    
    /* Test 5: External function simulation */
    checksum += external_builtin_user(seed);
    
    /* Test 6: Hidden visibility function */
    hidden_visibility_func();
    checksum += optimization_barrier;
    
    /* Test 7: Direct built-in usage in main */
    checksum = __builtin_bswap32(__builtin_bswap32(checksum));
    
    /* Use __builtin_expect for final condition */
    if (__builtin_expect(checksum != 0, 1)) {
        /* Use __builtin_trap in unreachable path */
        if (checksum < 0) {
            __builtin_trap();
        }
        printf("Result: %d\n", checksum);
    }
    
    /* Final memory barrier */
    asm volatile("" : : : "memory");
    
    return checksum & 0xFF;
}

/* Additional function to increase declaration count */
static int __attribute__((unused, visibility("hidden")))
unused_builtin_wrapper(int x) {
    /* Use multiple builtins */
    int a = __builtin_abs(x);
    int b = __builtin_clz((unsigned int)a);
    return __builtin_sadd_overflow(a, b, &a) ? 0 : a;
}
