/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */
#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __ARM_ARCH
#  define TARGET_ATTR __attribute__((target("arch=armv8-a")))
#else
#  define TARGET_ATTR __attribute__((target("arch=armv8-a")))
#endif

/* Prevent inlining and optimization of loops */
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))

/* Use volatile to prevent constant propagation */
static volatile int g_limit = 100;
static volatile int g_seed = 42;

/* Arrays for side effects */
static int array1[1000];
static int array2[1000];
static int array3[1000];

/* Function containing all loop patterns */
TARGET_ATTR HOT NOINLINE
static void test_loop_patterns(int n) {
    int i, j, k;
    volatile int limit1 = n;
    volatile int limit2 = n / 2;
    volatile int limit3 = n * 2;
    
    /* Pattern 1: Simple countable loop (disjoint from others) */
    for (i = 0; i < limit1; i++) {
        array1[i] = i * 2;
        /* Simple side effect */
        asm volatile("" : : : "memory");
    }
    
    /* Pattern 2: Perfectly nested loop (loop2 entirely inside loop3) */
    for (j = 0; j < limit2; j++) {
        /* Outer loop body */
        array2[j] = j * 3;
        asm volatile("" : : : "memory");
        
        /* Inner loop - perfectly nested */
        for (k = 0; k < 10; k++) {
            array3[j * 10 + k] = j + k;
            asm volatile("" : : : "memory");
        }
    }
    
    /* Pattern 3: Do-while loop (variation in CFG structure) */
    i = 0;
    do {
        array1[i] += array2[i];
        asm volatile("" : : : "memory");
        i++;
    } while (i < limit1);
    
    /* Pattern 4: Loop with conditional that creates partial overlap */
    /* This creates CFG where loops 4 and 5 intersect but neither is subset */
    int loop4_counter = 0;
    int loop5_counter = 0;
    
    /* Loop 4 */
    for (i = 0; i < limit2; i++) {
        array1[i] = i * i;
        asm volatile("" : : : "memory");
        
        /* Conditional that can jump into loop 5's region */
        if (i % 3 == 0) {
            /* Label for goto target from loop 5 */
loop5_entry:
            array2[i] = i * 4;
            asm volatile("" : : : "memory");
        }
        
        loop4_counter++;
    }
    
    /* Loop 5 - partially overlaps with loop 4 via goto */
    for (j = 5; j < limit2 + 5; j++) {
        array3[j] = j * 5;
        asm volatile("" : : : "memory");
        
        /* This goto creates CFG edge from loop 5 to loop 4's body */
        if (j % 4 == 0 && j < limit2) {
            goto loop5_entry;  /* Jump into loop 4's conditional block */
        }
        
        loop5_counter++;
    }
    
    /* Pattern 6: Another disjoint loop */
    int sum = 0;
    for (i = limit1 - 1; i >= 0; i--) {
        sum += array1[i];
        asm volatile("" : : : "memory");
    }
    
    /* Use results to prevent dead code elimination */
    array1[0] = sum + loop4_counter + loop5_counter;
}

/* Additional function with more complex nesting */
TARGET_ATTR HOT NOINLINE
static void nested_loop_patterns(int n) {
    volatile int outer_limit = n / 3;
    volatile int middle_limit = n / 4;
    volatile int inner_limit = n / 5;
    
    /* Triple nested loops for hierarchical analysis */
    for (int i = 0; i < outer_limit; i++) {
        array1[i] = i;
        asm volatile("" : : : "memory");
        
        for (int j = 0; j < middle_limit; j++) {
            array2[j] = i + j;
            asm volatile("" : : : "memory");
            
            for (int k = 0; k < inner_limit; k++) {
                array3[k] = i + j + k;
                asm volatile("" : : : "memory");
            }
        }
    }
    
    /* Adjacent loop with shared prefix block */
    int shared_start = 0;
    while (shared_start < outer_limit) {
        array1[shared_start] *= 2;
        asm volatile("" : : : "memory");
        shared_start++;
    }
    
    /* Loop that shares the same initial block */
    int shared_var = 0;
    while (shared_var < middle_limit) {
        /* This block is shared with previous loop's initial iteration */
        if (shared_var == 0) {
            array2[0] = 999;
            asm volatile("" : : : "memory");
        }
        array2[shared_var] += 1;
        asm volatile("" : : : "memory");
        shared_var++;
    }
}

int main(void) {
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int N = g_limit;
    volatile int seed = g_seed;
    
    /* Initialize arrays */
    for (int i = 0; i < 1000; i++) {
        array1[i] = (i + seed) % 100;
        array2[i] = (i * seed) % 100;
        array3[i] = (seed - i) % 100;
    }
    
    /* Call functions with loop patterns */
    test_loop_patterns(N);
    nested_loop_patterns(N);
    
    /* Compute checksum to ensure all loops execute */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += array1[i] + array2[i] + array3[i];
        checksum &= 0xFFFF;  /* Prevent overflow */
    }
    
    printf("Checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}
