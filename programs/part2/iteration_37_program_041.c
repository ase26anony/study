/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */

#include <stdio.h>
#include <stdlib.h>

/* Force architecture with hardware loop support */
#ifdef __ARM_ARCH
__attribute__((target("arch=armv8-a")))
#elif __riscv
__attribute__((target("arch=rv64gc_zba")))
#endif
__attribute__((noinline, hot))
void test_loop_patterns(int N, int *results) {
    volatile int limit = N;  /* Prevent constant propagation */
    int i, j, k, m;
    
    /* Loop 1: Simple countable loop (disjoint from others) */
    int sum1 = 0;
    for (i = 0; i < limit; i++) {
        sum1 += i * 2;
        /* Simple side effect to prevent dead code elimination */
        asm volatile("" : "+r"(sum1) : : "memory");
    }
    results[0] = sum1;
    
    /* Loop 2: Perfectly nested inside Loop 3 */
    int sum2 = 0;
    j = 0;
    do {
        /* Loop 3: Outer loop containing Loop 2 */
        for (k = 0; k < limit/2; k++) {
            sum2 += j * k;
            asm volatile("" : "+r"(sum2) : : "memory");
            
            /* Loop 2 body - perfectly nested */
            if (j < limit) {
                results[1] += k;
                j++;
            }
        }
    } while (j < limit);
    
    /* Loop 4: Partially overlapping with Loop 5 via conditional goto */
    int sum4 = 0;
    int sum5 = 0;
    int flag = 0;
    
    /* Label for partial overlap */
    partial_overlap:
    
    /* Loop 4 */
    for (m = 0; m < limit; m++) {
        sum4 += m * 3;
        asm volatile("" : "+r"(sum4) : : "memory");
        
        /* Conditional that creates partial overlap with Loop 5 */
        if (flag && m > limit/2) {
            /* Jump into Loop 5's body */
            goto inside_loop5;
        }
        
        /* Normal continue path */
        continue;
        
    inside_loop5:
        /* This block belongs to BOTH Loop 4 and Loop 5 */
        sum5 += m * 4;
        asm volatile("" : "+r"(sum5) : : "memory");
        
        /* Jump back to Loop 4's increment? No, break the overlap */
        break;
    }
    
    /* Loop 5: Overlaps with Loop 4 */
    if (!flag) {
        flag = 1;
        int n;
        for (n = 0; n < limit; n++) {
            sum5 += n * 5;
            asm volatile("" : "+r"(sum5) : : "memory");
            
            /* Entry point from Loop 4 */
            if (n == limit/2) {
                goto partial_overlap;
            }
        }
    }
    
    results[2] = sum4;
    results[3] = sum5;
    
    /* Loop 6: Adjacent but disjoint from Loop 7 */
    int sum6 = 0;
    for (int p = 0; p < limit/3; p++) {
        sum6 += p * 6;
        asm volatile("" : "+r"(sum6) : : "memory");
    }
    results[4] = sum6;
    
    /* Loop 7: Disjoint from Loop 6 */
    int sum7 = 0;
    int q = limit/3;
    while (q < limit) {
        sum7 += q * 7;
        asm volatile("" : "+r"(sum7) : : "memory");
        q++;
    }
    results[5] = sum7;
}

/* Complex nesting pattern for hierarchical relationships */
#ifdef __ARM_ARCH
__attribute__((target("arch=armv8-a")))
#elif __riscv
__attribute__((target("arch=rv64gc_zba")))
#endif
__attribute__((noinline))
void nested_loop_hierarchy(int N, int *results) {
    volatile int outer_limit = N;
    volatile int mid_limit = N/2;
    volatile int inner_limit = N/4;
    
    /* Triple nested loop - creates clear containment hierarchy */
    int total = 0;
    for (int a = 0; a < outer_limit; a++) {
        for (int b = 0; b < mid_limit; b++) {
            for (int c = 0; c < inner_limit; c++) {
                total += a * b * c;
                asm volatile("" : "+r"(total) : : "memory");
            }
        }
    }
    
    /* Separate but adjacent double nested loop */
    int total2 = 0;
    for (int x = 0; x < outer_limit; x++) {
        for (int y = 0; y < mid_limit; y++) {
            total2 += x + y;
            asm volatile("" : "+r"(total2) : : "memory");
        }
    }
    
    results[6] = total;
    results[7] = total2;
}

int main() {
    /* Use volatile to prevent compile-time computation */
    volatile int N = 1000;
    int results[10] = {0};
    
    /* Call the function with carefully constructed loops */
    test_loop_patterns(N, results);
    
    /* Additional call for nested hierarchy */
    nested_loop_hierarchy(N, results + 6);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += results[i];
        checksum ^= results[i] * 31;
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Use results to prevent dead code elimination */
    if (checksum == 0) {
        printf("Unexpected zero checksum\n");
    }
    
    return 0;
}
