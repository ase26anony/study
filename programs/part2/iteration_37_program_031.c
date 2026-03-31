/* test_hw_doloop.c - Target coverage for hw-doloop.cc lines 429-436 */
#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(int N, int M, int *result1, int *result2, int *result3) {
    volatile int limit = N;  /* Prevent constant propagation */
    volatile int inner_limit = M;
    int i, j, k;
    
    /* Loop 1: Simple countable loop - will have its own basic blocks */
    for (i = 0; i < limit; i++) {
        result1[i] = i * 2;
        /* Memory clobber to prevent optimization */
        asm volatile("" ::: "memory");
    }
    
    /* Loop 2: Adjacent but disjoint loop - no block intersection with Loop 1 */
    for (j = 0; j < inner_limit; j++) {
        result2[j] = j * 3;
        asm volatile("" ::: "memory");
    }
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* First, create outer loop structure */
    k = 0;
    do {
        /* Loop 4: Outer loop containing Loop 3 */
        for (i = 0; i < limit/2; i++) {
            /* Loop 3: Inner loop - entirely within Loop 4's blocks */
            for (j = 0; j < inner_limit/2; j++) {
                result3[k++] = i + j;
                asm volatile("" ::: "memory");
            }
        }
    } while (k < limit * inner_limit / 4);
    
    /* Loop 5 and Loop 6: Partially overlapping loops */
    /* Create complex CFG with goto between loops */
    int x = 0, y = 0;
    int flag = 0;
    
    /* Loop 5 */
    for (i = 0; i < limit; i++) {
        result1[i] += i;
        asm volatile("" ::: "memory");
        
        /* Conditional that may jump into Loop 6 */
        if (flag && i > limit/2) {
            /* This creates partial overlap - some blocks belong to both loops */
            goto overlap_point;
        }
        
        /* Normal continuation of Loop 5 */
        result2[i % M] -= i;
        
        /* Label inside Loop 6's potential body */
overlap_continue:
        result3[i % (M/2 + 1)] *= 2;
    }
    
    /* Reset flag for second overlapping loop */
    flag = 1;
    
    /* Loop 6 - partially overlaps with Loop 5 via goto */
    for (j = 0; j < inner_limit; j++) {
        result2[j] += j * j;
        asm volatile("" : "=m"(result2[j]) : : "memory");
        
overlap_point:
        /* This block can be reached from Loop 5 */
        result1[j % N] -= j;
        
        if (j < inner_limit/2) {
            /* Jump back to Loop 5's body */
            goto overlap_continue;
        }
        
        result3[j % (N/2 + 1)] /= (j + 1);
    }
    
    /* Loop 7: Another simple loop to ensure multiple loop candidates */
    for (i = 0; i < limit/3; i++) {
        for (j = 0; j < inner_limit/3; j++) {
            result3[(i * j) % N] = result1[i] + result2[j];
            asm volatile("" ::: "memory");
        }
    }
}

/* Main function to drive execution */
int main() {
    const int N = 100;
    const int M = 50;
    
    /* Allocate arrays with volatile pointers to prevent optimization */
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(M * sizeof(int));
    int *array3 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) array1[i] = i;
    for (int i = 0; i < M; i++) array2[i] = i * 2;
    for (int i = 0; i < N; i++) array3[i] = 1;
    
    /* Call the function with all the loop patterns */
    test_loop_patterns(N, M, array1, array2, array3);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < N; i++) checksum += array1[i];
    for (int i = 0; i < M; i++) checksum += array2[i];
    for (int i = 0; i < N; i++) checksum += array3[i];
    
    printf("Checksum: %d\n", checksum);
    
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
