/* test_hw_doloop.c
 * Designed to trigger uncovered lines in hw-doloop.cc (lines 429-436)
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_doloop.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Force architecture support for hardware loops */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(int N, int M, int *result1, int *result2, int *result3) {
    volatile int limit1 = N;  /* Prevent constant propagation */
    volatile int limit2 = M;
    volatile int limit3 = N/2;
    
    int i, j, k;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Loop 1: Simple countable loop - will be analyzed by hw-doloop */
    for (i = 0; i < limit1; i++) {
        /* Simple body with side effect */
        result1[i] = i * 2;
        sum1 += result1[i];
        
        /* This creates a basic block inside loop1 */
        if (i % 3 == 0) {
            /* Additional basic block within loop1 */
            result1[i] += 1;
        }
    }
    
    /* Loop 2: Adjacent but disjoint loop (no block intersection with loop1) */
    for (j = 0; j < limit2; j++) {
        result2[j] = j * 3;
        sum2 += result2[j];
        
        /* Create conditional for CFG complexity */
        if (j % 4 == 0) {
            result2[j] -= 1;
        }
    }
    
    /* Loop 3: Perfectly nested within loop 4 */
    /* First, create outer loop (loop4) */
    k = 0;
    do {
        /* Loop 3: inner loop - entirely contained within loop4's blocks */
        for (int inner = 0; inner < limit3; inner++) {
            result3[k + inner] = inner * k;
            sum3 += result3[k + inner];
            
            /* Memory barrier to prevent optimization */
            asm volatile("" ::: "memory");
        }
        k++;
    } while (k < limit1 / 2);  /* loop4 continues here */
    
    /* Loop 5 and Loop 6: Partially overlapping loops */
    /* Create complex CFG with goto to achieve partial overlap */
    int x = 0, y = 0;
    
    /* Loop 5 */
    for (x = 0; x < limit1; x++) {
        result1[x] += x;
        
        /* Conditional that will sometimes jump into loop6's region */
        if (x == limit1/2) {
            /* Label for goto target */
            overlap_target:
            result2[0] += x;  /* Shared computation */
            /* Continue in loop5 */
        }
    }
    
    /* Loop 6: Partially overlaps with loop5 via goto */
    for (y = 0; y < limit2; y++) {
        result2[y] += y;
        
        /* This creates partial overlap - some blocks shared, some not */
        if (y == limit2/3) {
            goto overlap_target;  /* Jump into loop5's body */
        }
        
        /* Unique block for loop6 */
        if (y % 5 == 0) {
            result3[y % 10] = y;
        }
    }
    
    /* Loop 7: Another loop with switch for CFG complexity */
    int z;
    for (z = 0; z < limit3; z++) {
        switch (z % 3) {
            case 0:
                result1[z] = z;
                break;
            case 1:
                result2[z] = z * 2;
                /* Fall through to create different block */
            case 2:
                result3[z] = z * 3;
                break;
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(sum1), "+r"(sum2), "+r"(sum3) : : "memory");
}

/* Main function to drive execution */
int main() {
    const int N = 1000;
    const int M = 800;
    
    /* Allocate arrays for loop results */
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(M * sizeof(int));
    int *array3 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) array1[i] = 0;
    for (int i = 0; i < M; i++) array2[i] = 0;
    for (int i = 0; i < N; i++) array3[i] = 0;
    
    /* Call the function with all the loop patterns */
    test_loop_patterns(N, M, array1, array2, array3);
    
    /* Compute checksum to ensure all loops executed */
    long long checksum = 0;
    for (int i = 0; i < N; i++) checksum += array1[i];
    for (int i = 0; i < M; i++) checksum += array2[i];
    for (int i = 0; i < N; i++) checksum += array3[i];
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
