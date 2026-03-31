/* test_builtin_hooks.c - Comprehensive test for GCC built-in function hooks */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int global_counter = 0;
static volatile int result_accumulator = 0;

/* Barrier to prevent optimization */
#define OPT_BARRIER() asm volatile("" : : : "memory")

/* Helper with multiple attributes that may trigger hook processing */
static int __attribute__((used, noinline, noclone)) 
test_builtin_arithmetic(volatile int seed) 
{
    int local_sum = 0;
    
    /* Use volatile counter to prevent loop unrolling */
    volatile int i;
    for (i = 0; i < 5; i++) {
        /* Mix different built-ins with arithmetic */
        int val = seed + i * 17;
        
        /* Built-in with explicit attribute-like behavior */
        int abs_val = __builtin_abs(val);
        OPT_BARRIER();
        
        /* Another built-in call */
        int clz_val = val > 0 ? __builtin_clz(val) : 32;
        OPT_BARRIER();
        
        local_sum += abs_val ^ clz_val;
    }
    
    /* Use result to prevent dead code elimination */
    global_counter += local_sum;
    return local_sum;
}

/* Function with explicit visibility attribute */
static void __attribute__((visibility("hidden"), nothrow, used, noinline))
test_attributed_function(volatile int seed) 
{
    /* Use built-in with branch prediction */
    if (__builtin_expect(seed > 100, 0)) {
        /* Use another built-in */
        int popcnt = __builtin_popcount(seed);
        result_accumulator += popcnt;
    } else {
        /* Different built-in in else branch */
        int ctz = seed > 0 ? __builtin_ctz(seed) : 0;
        result_accumulator -= ctz;
    }
    
    /* Memory barrier */
    OPT_BARRIER();
}

/* Function using overflow built-ins */
static int __attribute__((noinline, noclone))
test_builtin_overflow(volatile int a, volatile int b) 
{
    int result = 0;
    int overflow;
    
    /* Test add overflow */
    if (__builtin_add_overflow(a, b, &result)) {
        overflow = 1;
    } else {
        overflow = 0;
    }
    
    /* Test mul overflow with the result */
    int mul_result;
    if (__builtin_mul_overflow(result, 3, &mul_result)) {
        overflow |= 2;
    }
    
    /* Use built-in for unreachable code */
    if (overflow == 3) {
        __builtin_unreachable();  /* Should not happen */
    }
    
    OPT_BARRIER();
    return mul_result + overflow;
}

/* Function with external linkage declared static */
static int __attribute__((used))
test_external_linkage_sim(volatile int x) 
{
    /* Declare a built-in-like prototype */
    int __builtin_user_special(int) __attribute__((visibility("hidden")));
    
    /* Call external function that uses built-ins */
    return external_builtin_user(x);
}

/* Function using frame address built-in */
static void __attribute__((noinline))
test_frame_address(void) 
{
    void* frame_addr = __builtin_frame_address(0);
    OPT_BARRIER();
    
    /* Use the address to prevent optimization */
    if ((long)frame_addr & 1) {
        result_accumulator += 1;
    }
}

/* Function using synchronization built-ins */
static int __attribute__((noinline))
test_sync_builtins(volatile int* ptr) 
{
    int old = __sync_fetch_and_add(ptr, 1);
    OPT_BARRIER();
    
    /* Use other sync built-in */
    __sync_synchronize();
    
    return old;
}

/* External function definition (simulating another TU) */
int __attribute__((visibility("hidden"), nothrow))
external_builtin_user(int x) 
{
    /* Use multiple built-ins */
    int abs_x = __builtin_abs(x);
    int popcnt = __builtin_popcount(abs_x);
    
    /* Use expect built-in */
    if (__builtin_expect(popcnt > 16, 0)) {
        return -1;
    }
    
    /* Complex expression with built-in */
    return __builtin_ffs(x) + popcnt;
}

/* Another externally visible function */
void __attribute__((used))
hidden_visibility_func(void) 
{
    /* Use volatile to prevent constant folding */
    volatile int x = 42;
    
    /* Call built-in with side-effect prevention */
    int result = __builtin_abs(x) + __builtin_clz(x);
    
    OPT_BARRIER();
    global_counter += result;
}

/* Main test driver */
int main(int argc, char** argv) 
{
    volatile int seed;
    
    /* Get non-constant seed */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL) & 0xFF;
    }
    
    /* Initialize volatile state */
    global_counter = seed;
    result_accumulator = 0;
    
    printf("Starting built-in hook test with seed: %d\n", seed);
    
    /* Test 1: Arithmetic built-ins */
    int r1 = test_builtin_arithmetic(seed);
    printf("Test 1 result: %d\n", r1);
    
    /* Test 2: Attributed function */
    test_attributed_function(seed);
    printf("Test 2 accumulator: %d\n", result_accumulator);
    
    /* Test 3: Overflow built-ins */
    int r3 = test_builtin_overflow(seed, seed * 2);
    printf("Test 3 result: %d\n", r3);
    
    /* Test 4: External linkage simulation */
    int r4 = test_external_linkage_sim(seed);
    printf("Test 4 result: %d\n", r4);
    
    /* Test 5: Frame address */
    test_frame_address();
    
    /* Test 6: Sync built-ins */
    volatile int sync_var = 0;
    int r6 = test_sync_builtins(&sync_var);
    printf("Test 6 result: %d (sync_var=%d)\n", r6, sync_var);
    
    /* Test 7: Hidden visibility function */
    hidden_visibility_func();
    
    /* Final checksum to ensure all results are used */
    int final_checksum = r1 + r3 + r4 + r6 + global_counter + result_accumulator;
    
    printf("Final checksum: %d\n", final_checksum);
    printf("Global counter: %d\n", global_counter);
    
    return final_checksum & 0xFF;
}

/* Additional function with artificial attributes */
static void __attribute__((constructor, used, visibility("hidden")))
init_function(void) 
{
    /* Use built-in in constructor */
    int x = __builtin_abs(-42);
    OPT_BARRIER();
    global_counter += x;
}
