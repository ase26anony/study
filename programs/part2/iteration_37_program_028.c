/* test_hw_doloop.c
 * Designed to trigger uncovered bitmap intersection logic in hw-doloop.cc
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_doloop.c
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and constant propagation */
static volatile int N = 100;
static volatile int M = 50;
static volatile int K = 75;

/* Mark function as performance-critical to encourage hardware loop optimization */
__attribute__((noinline, hot))
void test_loop_patterns(int *arr1, int *arr2, int *arr3) {
    volatile int i, j, k;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Loop 1: Simple countable loop - will have its own basic blocks */
    for (i = 0; i < N; i++) {
        arr1[i] = i * 2;
        sum1 += arr1[i];
    }
    
    /* Loop 2: Adjacent but disjoint loop - no block intersection with Loop 1 */
    for (j = 0; j < M; j++) {
        arr2[j] = j * 3;
        sum2 += arr2[j];
    }
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* First, create outer loop structure */
    k = 0;
    do {
        /* Loop 4: Outer loop containing Loop 3 */
        for (i = 0; i < K; i++) {
            /* Loop 3: Inner loop - entirely within Loop 4's blocks */
            for (j = 0; j < 10; j++) {
                arr3[k] = i + j;
                sum3 += arr3[k];
                k++;
            }
        }
    } while (k < N);
    
    /* Loop 5 and Loop 6: Partially overlapping loops */
    /* Create complex CFG with goto to force partial overlap */
    int x = 0, y = 0;
    
    /* Loop 5: with conditional that can jump into Loop 6 */
    for (i = 0; i < N; i++) {
        if (i % 3 == 0) {
            /* This goto creates partial overlap with Loop 6 */
            if (i > N/2) {
                goto overlap_point;
            }
            arr1[i] += 5;
        } else {
            arr1[i] -= 2;
        }
        sum1 += arr1[i];
    }
    
    /* Some code between loops to create separate basic blocks */
    sum2 = sum1 * 2;
    
overlap_point:
    /* Loop 6: Shares some blocks with Loop 5 via the goto */
    for (j = 0; j < M; j++) {
        arr2[j] = arr2[j] * 2 + j;
        sum2 += arr2[j];
        
        /* This if creates another potential overlap point */
        if (j == M/2) {
            /* Could jump back to Loop 5's domain, but we won't to keep it simple */
            arr2[j] = 0;
        }
    }
    
    /* Loop 7: Another loop with complex exit that might overlap with others */
    int counter = 0;
    while (counter < K) {
        arr3[counter] = sum1 + sum2 + counter;
        
        /* Conditional break that creates additional basic blocks */
        if (arr3[counter] > 1000) {
            break;
        }
        
        counter++;
        
        /* Nested if to create more blocks for bitmap analysis */
        if (counter % 7 == 0) {
            arr3[counter] = 0;
        }
    }
    
    /* Force side effects to prevent dead code elimination */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
}

/* Additional function to create more loop relationships */
__attribute__((noinline))
void nested_loop_complexity(int *arr, int size) {
    volatile int i, j;
    
    /* Loop A: Outer loop */
    for (i = 0; i < size; i++) {
        /* Loop B: First inner loop - perfectly nested */
        for (j = 0; j < 5; j++) {
            arr[i] += j;
        }
        
        /* Loop C: Second inner loop at same nesting level as B */
        /* This creates adjacency within the same parent */
        int k = 0;
        while (k < 3) {
            arr[i] -= k;
            k++;
        }
    }
    
    /* Loop D: Separate but adjacent to the nested structure */
    for (i = size - 1; i >= 0; i--) {
        arr[i] = arr[i] * 2;
    }
}

int main() {
    /* Use volatile to prevent compile-time optimization */
    volatile int array_size = 1000;
    
    /* Allocate arrays on stack */
    int array1[1000];
    int array2[1000];
    int array3[1000];
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < 1000; i++) {
        array1[i] = i % 100;
        array2[i] = (i * 2) % 100;
        array3[i] = (i * 3) % 100;
    }
    
    /* Call the function with complex loop patterns */
    test_loop_patterns(array1, array2, array3);
    
    /* Call another function to add more loop relationships */
    nested_loop_complexity(array1, 500);
    
    /* Compute checksum to ensure all loops execute */
    int checksum = 0;
    for (int i = 0; i < 1000; i++) {
        checksum += array1[i] + array2[i] + array3[i];
        checksum &= 0xFFFF; /* Prevent overflow */
    }
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0;
}
