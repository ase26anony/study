/* test_hw_doloop.c - Target coverage for hw-doloop.cc lines 429-436 */
#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(volatile int N) {
    volatile int array1[1000], array2[1000], array3[1000];
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    volatile int i, j, k;
    
    /* Initialize arrays with non-zero values */
    for (int idx = 0; idx < 1000; idx++) {
        array1[idx] = idx % 100;
        array2[idx] = (idx * 3) % 100;
        array3[idx] = (idx * 7) % 100;
    }
    
    /* LOOP 1: Simple countable loop - will be analyzed by hw-doloop */
    for (i = 0; i < N; i++) {
        sum1 += array1[i];
        /* Memory clobber to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* LOOP 2: Adjacent but disjoint loop (no block intersection with Loop 1) */
    for (j = 0; j < N/2; j++) {
        sum2 += array2[j];
        asm volatile("" : : : "memory");
    }
    
    /* LOOP 3: Perfectly nested within conditional - creates hierarchical relationship */
    /* This should trigger: other->loops.safe_push(loop) when loop is Loop 3, other is Loop 4 */
    if (N > 10) {
        for (k = 0; k < N/3; k++) {
            sum3 += array3[k];
            asm volatile("" : : : "memory");
            
            /* LOOP 4: Inner loop within Loop 3 - perfect nesting */
            /* Loop 4's blocks are a subset of Loop 3's blocks */
            volatile int m;
            for (m = 0; m < 5; m++) {
                array1[k] += m;
                asm volatile("" : : : "memory");
            }
        }
    }
    
    /* LOOP 5 and LOOP 6: Partially overlapping loops using goto */
    /* This creates the scenario where bitmaps intersect but neither is subset of the other */
    /* Should trigger: else if (!bitmap_intersect_compl_p(...)) other->loops.safe_push(loop) */
    
    volatile int x = 0, y = 0;
    volatile int shared_value = 0;
    
    /* LOOP 5 */
    for (x = 0; x < N; x++) {
        shared_value += x;
        asm volatile("" : : : "memory");
        
        /* Conditional that may jump into LOOP 6 */
        if (x == N/2) {
            goto partial_overlap_label;  /* Jump into middle of LOOP 6 */
        }
        
        /* Normal continuation of LOOP 5 */
        array1[x] = shared_value;
    }
    
    /* LOOP 6 - Partially overlaps with LOOP 5 via goto */
    for (y = 0; y < N; y++) {
        shared_value -= y;
        asm volatile("" : : : "memory");
        
partial_overlap_label:  /* Label that LOOP 5 can jump to */
        /* This block belongs to both LOOP 5 (via goto) and LOOP 6 */
        array2[y] = shared_value;
        
        /* Prevent infinite loops by ensuring we don't jump back */
        if (x >= N) {
            /* Exit both loops */
            break;
        }
    }
    
    /* LOOP 7: Do-while loop for CFG variation */
    volatile int z = 0;
    do {
        array3[z] = sum1 + sum2 + sum3;
        asm volatile("" : : : "memory");
        z++;
    } while (z < 10);
    
    /* Prevent dead code elimination */
    volatile int result = sum1 + sum2 + sum3 + shared_value;
    asm volatile("" : : "r"(result) : "memory");
}

/* Additional function with switch-case containing loops */
/* Creates more complex CFG patterns for bitmap analysis */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void complex_loop_patterns(volatile int selector) {
    volatile int arr[100];
    volatile int total = 0;
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    switch (selector) {
        case 0: {
            /* Loop in case 0 */
            for (int i = 0; i < 50; i++) {
                total += arr[i];
                asm volatile("" : : : "memory");
            }
            break;
        }
        case 1: {
            /* Loop in case 1 - shares some blocks with case 0 loop? */
            for (int i = 50; i < 100; i++) {
                total -= arr[i];
                asm volatile("" : : : "memory");
            }
            break;
        }
        default: {
            /* Loop in default - different structure */
            int i = 0;
            while (i < 100) {
                total ^= arr[i];
                asm volatile("" : : : "memory");
                i += 2;
            }
        }
    }
    
    asm volatile("" : : "r"(total) : "memory");
}

int main() {
    volatile int N = 100;  /* Volatile to prevent constant propagation */
    
    /* Call the test function multiple times with different N values */
    /* to ensure different execution paths are taken */
    test_loop_patterns(N);
    test_loop_patterns(N/2);
    test_loop_patterns(N*2);
    
    complex_loop_patterns(0);
    complex_loop_patterns(1);
    complex_loop_patterns(2);
    
    /* Compute and print a checksum to ensure execution */
    volatile int checksum = N;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
