/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */
#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __ARM_ARCH
#warning "Compiling for ARM with hardware loop support"
#endif

/* Prevent inlining to preserve CFG structure */
__attribute__((noinline, optimize("O0")))
void test_loop_patterns(int N, int M, int K) {
    volatile int prevent_opt = 0;
    int array1[1000], array2[1000], array3[1000];
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Initialize arrays with volatile to prevent constant propagation */
    for (int i = 0; i < 1000; i++) {
        array1[i] = i + prevent_opt;
        array2[i] = i * 2 + prevent_opt;
        array3[i] = i * 3 + prevent_opt;
    }
    
    /* LOOP 1: Simple countable loop - will be analyzed for hardware loop */
    for (int i = 0; i < N; i++) {
        sum1 += array1[i % 1000];
        /* Memory clobber to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* LOOP 2: Adjacent loop with different counter - disjoint from Loop 1 */
    for (int j = 0; j < M; j++) {
        sum2 += array2[j % 1000];
        asm volatile("" : : : "memory");
    }
    
    /* LOOP 3: Perfectly nested loop inside conditional */
    if (K > 0) {
        for (int k = 0; k < K; k++) {
            sum3 += array3[k % 1000];
            asm volatile("" : : : "memory");
            
            /* LOOP 4: Inner nested loop - perfectly contained within Loop 3 */
            for (int l = 0; l < 5; l++) {
                array1[(k * 5 + l) % 1000] += l;
                asm volatile("" : : : "memory");
            }
        }
    }
    
    /* LOOP 5: do-while loop for CFG variation */
    int cnt = 0;
    do {
        array2[cnt % 1000] -= sum1;
        asm volatile("" : : : "memory");
        cnt++;
    } while (cnt < 10);
    
    /* CRITICAL SECTION: Create partial overlap between loops */
    /* LOOP 6 and LOOP 7 will have intersecting but not subset block bitmaps */
    
    /* Label for goto to create CFG edge between loops */
    loop6_start:
    
    /* LOOP 6: Loop with conditional goto to another loop */
    for (int i = 0; i < N/2; i++) {
        array1[i % 1000] *= 2;
        asm volatile("" : : : "memory");
        
        /* Conditional that creates edge to LOOP 7 */
        if (i == N/4 && prevent_opt == 0) {
            /* This goto creates partial overlap in CFG */
            goto loop7_middle;
        }
    }
    
    /* Normal exit from LOOP 6 */
    goto after_loops;
    
    /* LOOP 7: Partially overlapping loop */
    for (int j = 0; j < M/2; j++) {
        loop7_middle:  /* Label in the middle of LOOP 7 body */
        array2[j % 1000] /= 2;
        asm volatile("" : : : "memory");
        
        /* Conditional goto back to LOOP 6 */
        if (j == M/4 && prevent_opt == 0) {
            goto loop6_start;
        }
    }
    
    after_loops:
    
    /* Final computation to use all results */
    int final_sum = sum1 + sum2 + sum3 + array1[0] + array2[0] + array3[0];
    
    /* Force result to be used */
    asm volatile("" : : "r"(final_sum) : "memory");
    
    /* Print to prevent dead code elimination */
    if (prevent_opt) {
        printf("%d\n", final_sum);
    }
}

/* Main function with volatile parameters */
int main() {
    /* Use volatile to prevent compile-time evaluation */
    volatile int N = 100;
    volatile int M = 50;
    volatile int K = 25;
    
    /* Call the function with all loop patterns */
    test_loop_patterns(N, M, K);
    
    /* Additional test with different parameters */
    N = 200;
    M = 100;
    K = 50;
    test_loop_patterns(N, M, K);
    
    return 0;
}
