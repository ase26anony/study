/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */
#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(volatile int N) {
    volatile int arr1[1024], arr2[1024], arr3[1024];
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i, j, k;
    
    /* Initialize arrays with volatile to prevent optimization */
    for (i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
    }
    
    /* Loop 1: Simple countable loop - will be a candidate for hardware loop */
    for (i = 0; i < N; i++) {
        sum1 += arr1[i];
        /* Prevent dead code elimination */
        asm volatile("" : "+r"(sum1) : : "memory");
    }
    
    /* Loop 2: Adjacent but disjoint loop (no basic block intersection with Loop 1) */
    for (j = 0; j < N/2; j++) {
        sum2 += arr2[j];
        asm volatile("" : "+r"(sum2) : : "memory");
    }
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* First, create outer loop */
    for (k = 0; k < N/3; k++) {
        /* Loop 4: Inner loop - entirely contained within Loop 3's blocks */
        for (i = 0; i < 10; i++) {
            sum3 += arr3[k] + i;
            asm volatile("" : "+r"(sum3) : : "memory");
        }
    }
    
    /* Loop 5 and Loop 6: Partially overlapping loops */
    /* Create complex CFG with conditional branching between loops */
    int x = 0;
    
    /* Loop 5: do-while loop for CFG variation */
    do {
        if (x < N/2) {
            sum1 += x;
            asm volatile("" : "+r"(sum1) : : "memory");
            
            /* Conditional goto that creates partial overlap with Loop 6 */
            if (x % 3 == 0) {
                /* Jump to label inside Loop 6's body */
                goto overlap_point;
            }
        }
        x++;
    } while (x < N);
    
    /* Loop 6: Another loop that will partially overlap with Loop 5 */
    int y = 0;
    for (y = 0; y < N; y++) {
        overlap_point:
        sum2 += y * 2;
        asm volatile("" : "+r"(sum2) : : "memory");
        
        /* Additional conditional to create more complex CFG */
        if (y % 4 == 0) {
            /* This creates a basic block that might be shared */
            sum3 += y;
            asm volatile("" : "+r"(sum3) : : "memory");
            
            /* Potentially jump back to Loop 5's domain */
            if (y > N/2) {
                /* This creates partial overlap */
                continue;
            }
        }
    }
    
    /* Loop 7: Another adjacent loop with different structure */
    int z;
    for (z = N-1; z >= 0; z--) {
        arr1[z] = sum1 + z;
        asm volatile("" : : "r"(arr1[z]) : "memory");
    }
    
    /* Force use of results to prevent elimination */
    volatile int result = sum1 + sum2 + sum3;
    (void)result;
}

/* Main function to drive execution */
int main() {
    volatile int N = 100;  /* Volatile to prevent constant propagation */
    
    /* Call the function with all loop patterns */
    test_loop_patterns(N);
    
    /* Additional test with different N to potentially trigger different paths */
    test_loop_patterns(N/2);
    
    printf("Hardware loop pattern test completed\n");
    return 0;
}
