/* test_targhooks.c - Comprehensive built-in function test for GCC coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int global_volatile_counter = 0;
static volatile int optimization_barrier = 0;

/* Function with multiple attributes that may interact with built-in processing */
static int __attribute__((used, noinline, noclone))
test_attributed_function(int x) __attribute__((nothrow));

/* Built-in function prototype declaration (simulating user declaration) */
int __builtin_popcount(unsigned int) __attribute__((visibility("hidden")));
int __builtin_clz(unsigned int) __attribute__((used));
int __builtin_ctz(unsigned int);

/* Helper to create optimization barrier */
static void __attribute__((always_inline))
barrier(void) {
    asm volatile("" : : : "memory");
    optimization_barrier++;
}

/* Test 1: Arithmetic built-ins with volatile context */
static int __attribute__((noinline, noclone))
test_builtin_arithmetic(volatile int seed) {
    volatile float result = 0.0f;
    volatile int i;
    
    for (i = 0; i < 10; i++) {
        int val = seed + i * 37;
        /* Use abs built-in with volatile intermediate */
        int abs_val = __builtin_abs(val);
        
        /* Use sqrtf built-in (float version) */
        float sqrt_val = __builtin_sqrtf((float)abs_val + 1.0f);
        
        /* Use fabs built-in */
        float fabs_val = __builtin_fabsf(sqrt_val - 2.0f);
        
        result += fabs_val;
        barrier();
    }
    
    /* Use expect built-in to influence branching */
    if (__builtin_expect(result > 50.0f, 0)) {
        return (int)result;
    }
    return (int)result + seed;
}

/* Test 2: Bit manipulation built-ins */
static int __attribute__((noinline, noclone))
test_builtin_bitops(unsigned int seed) {
    volatile unsigned int v = seed;
    volatile int total = 0;
    
    /* Chain multiple bit built-ins */
    total += __builtin_popcount(v);
    barrier();
    
    total += __builtin_clz(v | 1);  /* Ensure non-zero */
    barrier();
    
    total += __builtin_ctz(v | 1);  /* Ensure non-zero */
    barrier();
    
    /* Use parity built-in */
    total += __builtin_parity(v);
    barrier();
    
    /* Use ffs built-in (find first set) */
    total += __builtin_ffs(v);
    
    return total;
}

/* Test 3: Overflow checking built-ins */
static int __attribute__((noinline, noclone))
test_builtin_overflow(int a, int b) {
    volatile int result;
    volatile int overflow;
    
    /* Test addition overflow */
    if (__builtin_add_overflow(a, b, &result)) {
        barrier();
        result = __builtin_abs(a);
    }
    
    /* Test multiplication overflow */
    if (__builtin_mul_overflow(result, 2, &result)) {
        barrier();
        result = b;
    }
    
    /* Test subtraction overflow */
    if (__builtin_sub_overflow(result, a, &result)) {
        barrier();
        result = 1;
    }
    
    /* Use saturating arithmetic built-in (if available) */
    int saturated;
    if (__builtin_add_overflow_p(result, 1000, 0)) {
        saturated = 2147483647;
    } else {
        saturated = result + 1000;
    }
    
    return saturated;
}

/* Test 4: Function with visibility and other attributes */
static int __attribute__((visibility("hidden"), nothrow, used, noinline))
test_attributed_function(int x) {
    volatile int y = x;
    
    /* Use expect within attributed function */
    if (__builtin_expect(y > 0, 1)) {
        /* Use clz built-in */
        y += __builtin_clz((unsigned int)y);
    }
    
    /* Use popcount built-in */
    y += __builtin_popcount((unsigned int)__builtin_abs(y));
    
    barrier();
    return y;
}

/* Test 5: External linkage simulation */
int __attribute__((noinline))
external_builtin_user(int x) {
    volatile int y = x;
    
    /* Use unreachable built-in under specific condition */
    if (y < 0) {
        __builtin_unreachable();
    }
    
    /* Use prefetch built-in */
    __builtin_prefetch(&global_volatile_counter, 0, 3);
    
    /* Use bswap built-in for endianness */
    if (y > 1000) {
        y = __builtin_bswap32((unsigned int)y);
    }
    
    barrier();
    return y;
}

/* Function with hidden visibility attribute */
void __attribute__((visibility("hidden"), used))
hidden_visibility_func(void) {
    volatile int x = 123;
    
    /* Use various built-ins in hidden visibility function */
    x += __builtin_ffs(x);
    x += __builtin_parity((unsigned int)x);
    
    /* Use constant_p built-in */
    if (__builtin_constant_p(x)) {
        x += 10;
    }
    
    barrier();
    global_volatile_counter += x;
}

/* Test 6: Complex expression with multiple built-ins */
static int __attribute__((noinline))
test_complex_expressions(volatile int seed) {
    int a = seed;
    int b = seed * 3;
    volatile int result = 0;
    
    /* Nested built-in calls in complex expressions */
    result = __builtin_abs(__builtin_abs(a) - __builtin_abs(b));
    
    /* Built-in in ternary expression */
    result += __builtin_popcount((unsigned int)a) > 16 ? 
              __builtin_clz((unsigned int)b) : 
              __builtin_ctz((unsigned int)b);
    
    /* Built-in with inline assembly barrier */
    asm volatile("" : "+r"(result) : : "memory");
    
    /* Use types_compatible_p built-in */
    if (__builtin_types_compatible_p(typeof(a), int)) {
        result += 100;
    }
    
    barrier();
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    volatile unsigned int seed;
    int checksum = 0;
    
    /* Initialize seed from various sources */
    seed = (unsigned int)time(NULL);
    seed ^= (unsigned int)argc;
    seed ^= (unsigned int)((size_t)argv & 0xFFFFFFFF);
    
    /* Ensure seed is non-zero and has varied bits */
    seed = (seed << 1) | 1;
    
    printf("Starting built-in tests with seed: %u\n", seed);
    
    /* Run all test functions */
    checksum += test_builtin_arithmetic((int)seed);
    barrier();
    
    checksum += test_builtin_bitops(seed);
    barrier();
    
    checksum += test_builtin_overflow((int)seed, (int)(seed >> 8));
    barrier();
    
    checksum += test_attributed_function((int)seed);
    barrier();
    
    checksum += external_builtin_user((int)seed);
    barrier();
    
    hidden_visibility_func();
    barrier();
    
    checksum += test_complex_expressions((int)seed);
    barrier();
    
    /* Use printf built-in optimization */
    if (__builtin_constant_p(checksum)) {
        printf("Checksum is constant: %d\n", checksum);
    } else {
        printf("Final checksum: %d\n", checksum);
    }
    
    /* Use return built-in */
    return __builtin_expect(checksum != 0, 1) ? 0 : 1;
}

/* Additional static function with built-in usage */
static void __attribute__((constructor))
init_function(void) {
    /* Use built-in in constructor */
    volatile int x = __builtin_abs(-42);
    global_volatile_counter = x;
    
    /* Use choose_expr built-in */
    int y = __builtin_choose_expr(sizeof(int) == 4, 100, 200);
    global_volatile_counter += y;
}
