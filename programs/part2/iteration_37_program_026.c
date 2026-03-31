/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */

#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __ARM_ARCH
#else
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(volatile int N) {
    volatile int array1[1024];
    volatile int array2[1024];
    volatile int array3[1024];
    volatile int sum = 0;
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < 1024; i++) {
        array1[i] = i;
        array2[i] = i * 2;
        array3[i] = i * 3;
    }
    
    /* Loop 1: Simple countable loop - will be analyzed by hw-doloop */
    for (int i = 0; i < N; i++) {
        sum += array1[i];
        /* Memory clobber to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Loop 2: Adjacent but disjoint loop (shares no basic blocks with Loop 1) */
    int j = 0;
    do {
        array2[j] = array2[j] * 3 + 1;
        asm volatile("" : : : "memory");
        j++;
    } while (j < N);
    
    /* Loop 3: Perfectly nested within Loop 4 */
    /* First, create an outer loop */
    for (int outer = 0; outer < N/2; outer++) {
        /* Loop 3: Inner loop - entirely contained within Loop 4's blocks */
        for (int inner = 0; inner < 10; inner++) {
            array3[outer] += inner;
            asm volatile("" : : : "memory");
        }
        sum += array3[outer];
    }
    
    /* Loop 4 and Loop 5: Partially overlapping loops */
    /* Create a complex CFG with goto to force partial overlap */
    int k = 0;
    int m = 0;
    
    /* Loop 4 */
    for (k = 0; k < N; k++) {
        if (k % 3 == 0) {
            /* This conditional branch creates separate basic blocks */
            array1[k] = array1[k] * 2;
            /* Goto that jumps into Loop 5's region */
            if (k == N/2) {
                goto overlap_region;
            }
        } else {
            array1[k] = array1[k] / 2;
        }
        asm volatile("" : : : "memory");
    }
    
    /* Reset m before Loop 5 */
    m = 0;
    
    /* Loop 5 - partially overlaps with Loop 4 via the goto */
    overlap_start:
    while (m < N) {
        overlap_region:
        array2[m] = array2[m] + array1[m];
        asm volatile("" : : : "memory");
        
        if (m % 4 == 0) {
            /* Jump back to Loop 4's region */
            if (m < N/2) {
                m++;
                continue;  /* Stay in Loop 5 */
            }
        }
        m++;
    }
    
    /* Loop 6: Another simple loop to ensure multiple loop analysis */
    for (int p = N-1; p >= 0; p--) {
        sum -= array3[p];
        asm volatile("" : : : "memory");
    }
    
    /* Prevent dead code elimination */
    volatile int result = sum;
    (void)result;
}

/* Mark function as hot to encourage hardware loop optimization */
__attribute__((hot))
#ifdef __ARM_ARCH
#else
__attribute__((target("arch=armv8-a")))
#endif
void hot_loop_function(volatile int M) {
    volatile int buffer[512];
    volatile int acc = 0;
    
    /* Initialize */
    for (int i = 0; i < 512; i++) {
        buffer[i] = i % 64;
    }
    
    /* Multiple adjacent loops */
    for (int i = 0; i < M; i++) {
        acc += buffer[i % 512];
    }
    
    for (int i = M; i > 0; i--) {
        acc -= buffer[i % 512];
    }
    
    /* Nested loop structure */
    for (int outer = 0; outer < M/4; outer++) {
        for (int inner = 0; inner < 8; inner++) {
            buffer[outer] += inner;
        }
    }
    
    (void)acc;
}

int main() {
    volatile int N = 100;  /* Volatile to prevent constant propagation */
    
    /* Call the test function multiple times with different values
       to ensure various loop bounds are considered */
    test_loop_patterns(N);
    test_loop_patterns(N + 50);
    
    hot_loop_function(N);
    hot_loop_function(N / 2);
    
    /* Compute and print a checksum to ensure all code executes */
    volatile int checksum = N * 12345;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
