/* test_hw_doloop.c - Target coverage for hw-doloop.cc lines 429-436 */
#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __ARM_ARCH
#warning "Compiling for ARM with hardware loop support"
#endif

/* Prevent inlining to preserve CFG structure */
__attribute__((noinline, optimize("O2")))
#ifdef __arm__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(volatile int N, int* results) {
    volatile int M = N / 2;
    volatile int K = N / 3;
    int i, j, k;
    
    /* Loop 1: Simple countable loop (disjoint from others initially) */
    int sum1 = 0;
    for (i = 0; i < N; i++) {
        sum1 += i * 2;
        /* Simple side effect to prevent dead code elimination */
        asm volatile("" : "+r"(sum1) : : "memory");
    }
    results[0] = sum1;
    
    /* Loop 2: Perfectly nested inside Loop 3 */
    int sum2 = 0;
    for (j = 0; j < M; j++) {
        /* Loop 3: Outer loop containing Loop 2 */
        for (k = 0; k < K; k++) {
            sum2 += j * k;
            asm volatile("" : "+r"(sum2) : : "memory");
        }
    }
    results[1] = sum2;
    
    /* Loop 4: Do-while loop (different structure) */
    int sum3 = 0;
    i = 0;
    do {
        sum3 += i * 3;
        asm volatile("" : "+r"(sum3) : : "memory");
        i++;
    } while (i < N);
    results[2] = sum3;
    
    /* Create partial overlap between Loop 5 and Loop 6 */
    int sum4 = 0;
    int sum5 = 0;
    
    /* Loop 5: Loop with conditional that can jump to Loop 6 */
    for (i = 0; i < N; i++) {
        sum4 += i * 4;
        asm volatile("" : "+r"(sum4) : : "memory");
        
        /* Conditional that creates CFG edge to Loop 6 */
        if (i % 3 == 0) {
            /* This label will be inside Loop 6's basic blocks */
            goto loop6_entry;
        }
        
        /* Normal continuation of Loop 5 */
        sum4 += 1;
        
        /* Jump back to avoid falling into Loop 6 */
        if (i < N - 1) {
            continue;
        }
        
loop6_entry:
        /* Loop 6: Partially overlapping loop */
        /* This block belongs to BOTH Loop 5 and Loop 6 */
        for (j = 0; j < M; j++) {
            sum5 += i * j;
            asm volatile("" : "+r"(sum5) : : "memory");
            
            /* Break early to keep loop simple */
            if (j > 5) break;
        }
        
        /* After Loop 6, we're still in Loop 5 */
        sum4 += 2;
    }
    results[3] = sum4;
    results[4] = sum5;
    
    /* Loop 7: Another disjoint loop */
    int sum6 = 0;
    for (i = N - 1; i >= 0; i--) {
        sum6 += i;
        asm volatile("" : "+r"(sum6) : : "memory");
    }
    results[5] = sum6;
    
    /* Loop 8: Contains a small inner loop (perfect nesting) */
    int sum7 = 0;
    for (i = 0; i < K; i++) {
        /* Loop 9: Inner loop */
        for (j = 0; j < 3; j++) {
            sum7 += i + j;
            asm volatile("" : "+r"(sum7) : : "memory");
        }
    }
    results[6] = sum7;
}

/* Additional function with complex CFG to create more opportunities */
__attribute__((noinline))
void create_complex_cfg(volatile int N, int* arr) {
    int i, j;
    
    /* Loop A and Loop B with shared basic block */
    for (i = 0; i < N; i++) {
        arr[i] = i;
        
shared_block:
        /* This block will be shared between loops */
        arr[i] += 1;
        
        if (i % 2 == 0) {
            /* Loop B: Starts in the middle of Loop A */
            for (j = 0; j < 2; j++) {
                arr[i] += j;
                /* Jump back to shared block */
                if (j == 0) goto shared_block;
            }
        }
    }
}

int main() {
    volatile int N = 100;  /* Volatile to prevent constant propagation */
    int results[10] = {0};
    int arr[100] = {0};
    
    /* Call the function with carefully constructed loops */
    test_loop_patterns(N, results);
    
    /* Create additional CFG complexity */
    create_complex_cfg(N > 50 ? 50 : N, arr);
    
    /* Compute checksum to ensure all loops execute */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += results[i];
    }
    for (int i = 0; i < 100; i++) {
        checksum += arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
