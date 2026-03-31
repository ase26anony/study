/* test_targhooks.c - Comprehensive built-in function test for targhooks.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_builtin_caller(void);

/* Volatile variables to prevent optimization */
static volatile int global_counter = 0;
static volatile int result_accumulator = 0;

/* ============================================
   Pattern 1: Direct built-in usage with volatile
   ============================================ */
void test_builtin_arithmetic(volatile int seed) {
    /* Use math built-ins with volatile operands */
    int val1 = seed * 2 - 100;
    int val2 = seed / 3 + 50;
    
    /* Built-ins that should trigger declaration processing */
    int abs_result = __builtin_abs(val1);
    int popcount_result = __builtin_popcount((unsigned int)val2);
    
    /* Prevent dead code elimination */
    global_counter += abs_result;
    result_accumulator ^= popcount_result;
    
    /* Use sqrt built-in with float */
    float fval = (float)seed * 0.5f;
    float sqrt_result = __builtin_sqrtf(fval);
    result_accumulator += (int)sqrt_result;
}

/* ============================================
   Pattern 2: Built-ins in loops with barriers
   ============================================ */
__attribute__((noinline, noclone))
int test_builtin_bitops(volatile int seed) {
    int total = 0;
    volatile int i;
    
    for (i = 0; i < 10; i++) {
        int value = seed + i * 7;
        
        /* Multiple bit operation built-ins */
        int clz_result = __builtin_clz((unsigned int)value);
        int ctz_result = __builtin_ctz((unsigned int)value | 1); /* Avoid 0 */
        int parity_result = __builtin_parity((unsigned int)value);
        
        /* Optimization barrier */
        asm volatile("" : : : "memory");
        
        total += clz_result + ctz_result + parity_result;
    }
    
    return total;
}

/* ============================================
   Pattern 3: Overflow built-ins with branching
   ============================================ */
__attribute__((noinline))
void test_builtin_overflow(int seed) {
    int a = seed * 3;
    int b = seed + 100;
    int result;
    int overflow_flag;
    
    /* Use overflow checking built-ins */
    if (__builtin_add_overflow(a, b, &result)) {
        result_accumulator += 1;
    } else {
        result_accumulator += result;
    }
    
    if (__builtin_mul_overflow(a, b, &result)) {
        result_accumulator *= 2;
    }
    
    /* Use expect built-in */
    int likely_value = __builtin_expect((seed > 0), 1);
    result_accumulator += likely_value;
}

/* ============================================
   Pattern 4: Function with explicit attributes
   ============================================ */
/* Hidden visibility, nothrow, used - matching target flags */
static int __attribute__((visibility("hidden"), nothrow, used))
attributed_builtin_helper(int x) {
    /* Use expect built-in inside attributed function */
    if (__builtin_expect(x > 100, 0)) {
        return __builtin_abs(x) + __builtin_clz((unsigned int)x);
    }
    return __builtin_popcount((unsigned int)x);
}

void test_attributed_function(int seed) {
    /* Call the attributed function */
    int res = attributed_builtin_helper(seed);
    result_accumulator += res;
    
    /* Call it again with different value */
    res = attributed_builtin_helper(seed * 2);
    global_counter += res;
}

/* ============================================
   Pattern 5: External linkage simulation
   ============================================ */
/* External function that uses built-ins - defined later */
extern int external_builtin_user(int x);

/* Function with external declaration but internal definition */
int external_builtin_user(int x) {
    /* Use unreachable built-in under condition */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* Use various built-ins */
    int result = __builtin_abs(x);
    result += __builtin_ffs(x | 1); /* Ensure non-zero */
    
    /* Use prefetch built-in */
    int* ptr = &global_counter;
    __builtin_prefetch(ptr, 0, 3);
    
    return result;
}

/* ============================================
   Pattern 6: Static function with built-ins
   ============================================ */
static void __attribute__((noinline))
static_builtin_caller(volatile int seed) {
    /* Use sync built-ins */
    int sync_val = __sync_add_and_fetch(&global_counter, 1);
    
    /* Use bswap built-in */
    unsigned int bswap_val = __builtin_bswap32((unsigned int)seed);
    result_accumulator ^= bswap_val;
    
    /* Use choose expr built-in */
    int choice = __builtin_choose_expr(seed > 0, 
                                       __builtin_abs(seed),
                                       __builtin_popcount((unsigned int)seed));
    global_counter += choice;
}

/* ============================================
   Pattern 7: Complex expression with built-ins
   ============================================ */
__attribute__((noinline, noclone))
int complex_builtin_expr(int seed) {
    /* Nested built-in calls in complex expression */
    int result = __builtin_abs(seed) * 
                 (__builtin_popcount((unsigned int)seed) + 1) / 
                 (__builtin_clz((unsigned int)seed | 1) + 1);
    
    /* Use built-in with side effects */
    int old_val = __sync_fetch_and_or(&global_counter, result);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    return result + old_val;
}

/* ============================================
   Main test driver
   ============================================ */
int main(int argc, char *argv[]) {
    /* Initialize volatile seed from runtime */
    volatile int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    
    printf("Starting built-in test with seed: %d\n", seed);
    
    /* Execute all test patterns */
    test_builtin_arithmetic(seed);
    
    int bitops_result = test_builtin_bitops(seed);
    result_accumulator += bitops_result;
    
    test_builtin_overflow(seed);
    
    test_attributed_function(seed);
    
    int external_result = external_builtin_user(seed);
    result_accumulator += external_result;
    
    static_builtin_caller(seed);
    
    int complex_result = complex_builtin_expr(seed);
    result_accumulator += complex_result;
    
    /* Additional built-in usage in main */
    if (__builtin_constant_p(seed)) {
        /* This branch unlikely but uses another built-in */
        result_accumulator += 1000;
    }
    
    /* Use object size built-in */
    char buffer[100];
    size_t obj_size = __builtin_object_size(buffer, 0);
    result_accumulator += (int)obj_size;
    
    /* Final result checksum */
    int final_result = result_accumulator + global_counter;
    
    printf("Test completed. Result: %d (accumulator: %d, counter: %d)\n",
           final_result, result_accumulator, global_counter);
    
    return final_result % 256; /* Return non-zero result */
}

/* ============================================
   Additional function to increase declaration count
   ============================================ */
/* Force emission with used attribute */
static void __attribute__((used, visibility("hidden")))
extra_builtin_user(void) {
    volatile int x = 42;
    int r1 = __builtin_abs(x);
    int r2 = __builtin_popcount((unsigned int)x);
    int r3 = __builtin_clz((unsigned int)x);
    
    /* Use the results to prevent elimination */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3) : "memory");
}
