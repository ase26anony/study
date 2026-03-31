/* test_hw_doloop.c
 * Designed to trigger specific uncovered lines in GCC's hw-doloop.cc
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_doloop.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force runtime values to prevent constant propagation */
volatile int N = 100;
volatile int M = 50;
volatile int K = 75;

/* Arrays for side effects */
int array1[200];
int array2[200];
int array3[200];

/* Mark function as performance-critical and target ARM */
__attribute__((target("arch=armv8-a"), noinline, hot))
void test_loop_patterns(void) {
    volatile int limit1 = N;
    volatile int limit2 = M;
    volatile int limit3 = K;
    
    int i, j, k;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Loop 1: Simple countable loop - will be analyzed by hw-doloop */
    for (i = 0; i < limit1; i++) {
        array1[i] = i * 2;
        sum1 += array1[i];
        
        /* Create a conditional that can jump to Loop 2 for partial overlap */
        if (i == limit1/2) {
            /* This goto creates CFG edges between loops 1 and 2 */
            goto partial_overlap_entry;
        }
    }
    
    /* Loop 2: Another countable loop, adjacent to Loop 1 */
    j = 0;
    do {
        array2[j] = j * 3;
        sum2 += array2[j];
        j++;
    } while (j < limit2);
    
    /* Label for partial overlap */
partial_overlap_entry:
    
    /* Loop 3: Perfectly nested inside a conditional structure */
    /* This creates a loop that's entirely contained within another's blocks */
    if (sum1 > 0) {
        for (k = 0; k < limit3; k++) {
            array3[k] = k * 5;
            sum3 += array3[k];
            
            /* Nested loop inside Loop 3 - creates perfect nesting */
            /* Loop 4: Perfectly nested within Loop 3 */
            {
                int m;
                for (m = 0; m < 10; m++) {
                    /* Force side effect to prevent optimization */
                    asm volatile("" : "+r"(m) : : "memory");
                    array1[k] += m;
                }
            }
        }
    }
    
    /* Loop 5: Another loop that shares some blocks with Loop 1 via goto */
    /* This creates partial overlap scenario */
    {
        int n = 0;
        while (n < limit1/2) {
            array2[n] += sum1;
            n++;
            
            /* Jump back to Loop 1's body to create overlapping CFG */
            if (n == limit1/4) {
                goto loop1_body;
            }
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    
    return;
    
/* Label inside Loop 1's body for partial overlap */
loop1_body:
    /* This creates a shared basic block between Loop 1 and Loop 5 */
    array1[i-1] += 1000;
    goto partial_overlap_entry;
}

/* Additional function to create more complex CFG relationships */
__attribute__((target("arch=armv8-a"), noinline))
void complex_loop_relationships(void) {
    volatile int a = N;
    volatile int b = M;
    int x, y;
    
    /* Loop A and Loop B are adjacent but disjoint */
    for (x = 0; x < a; x++) {
        array1[x] = x * x;
    }
    
    for (y = 0; y < b; y++) {
        array2[y] = y + array1[y % a];
    }
    
    /* Loop C contains an if with a nested loop (Loop D) */
    /* This creates: Loop D is perfectly nested in part of Loop C */
    for (x = 0; x < a; x++) {
        if (x % 3 == 0) {
            /* Loop D - perfectly nested within Loop C's conditional path */
            for (y = 0; y < 5; y++) {
                array3[x] += y;
            }
        } else {
            array3[x] = x;
        }
    }
    
    /* Loop E and Loop F partially overlap via switch with fall-through */
    {
        int z = 0;
        while (z < a) {
            switch (z % 4) {
                case 0:
                    /* Shared block with Loop F */
                    array1[z] = -1;
                    /* FALLTHROUGH */
                case 1:
                    /* Loop F starts here but includes case 0 block */
                    for (y = 0; y < 3; y++) {
                        array2[z] += y;
                    }
                    break;
                default:
                    array3[z] = z * 2;
            }
            z++;
        }
    }
}

int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < 200; i++) {
        array1[i] = 0;
        array2[i] = 0;
        array3[i] = 0;
    }
    
    /* Call functions with complex loop patterns */
    test_loop_patterns();
    complex_loop_relationships();
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < 200; i++) {
        checksum += array1[i] + array2[i] + array3[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
