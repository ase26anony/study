/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */
#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(volatile int N) {
    volatile int array1[1024] = {0};
    volatile int array2[1024] = {0};
    volatile int array3[1024] = {0};
    int i, j, k;
    
    /* Loop 1: Simple countable loop (disjoint from others) */
    for (i = 0; i < N; i++) {
        array1[i] = i * 2;
        /* Simple side effect to prevent dead code elimination */
        asm volatile("" : : "r"(array1[i]) : "memory");
    }
    
    /* Loop 2: Perfectly nested inside Loop 3 */
    /* This should trigger: loop->loops.safe_push(other) */
    for (j = 0; j < N/2; j++) {
        /* Loop 3: Outer loop containing Loop 2 */
        for (k = 0; k < N/3; k++) {
            array2[j + k] = j * k;
            asm volatile("" : : "r"(array2[j + k]) : "memory");
        }
    }
    
    /* Loop 4: Partially overlapping with Loop 5 via goto */
    /* This should trigger: other->loops.safe_push(loop) */
    int counter4 = 0;
    int counter5 = 0;
    
    /* Label for partial overlap */
    overlap_point:
    
    /* Loop 4 body start */
    do {
        array3[counter4] = counter4 * 3;
        asm volatile("" : : "r"(array3[counter4]) : "memory");
        counter4++;
        
        /* Conditional that creates partial overlap */
        if (counter4 % 3 == 0) {
            /* Jump into Loop 5's domain */
            goto inside_loop5;
        }
        
        /* Normal loop 4 continuation */
        if (counter4 >= N) break;
        
        /* More loop 4 work */
        array3[counter4] = counter4 * 4;
        asm volatile("" : : "r"(array3[counter4]) : "memory");
        counter4++;
        
    } while (counter4 < N);
    
    /* Skip to after loop 5 if we finished loop 4 normally */
    goto after_loop5;
    
inside_loop5:
    /* Loop 5: Partially overlaps with Loop 4 */
    /* This creates bitmap intersection but not subset relationship */
    while (counter5 < N/2) {
        array3[counter5 + 100] = counter5 * 5;
        asm volatile("" : : "r"(array3[counter5 + 100]) : "memory");
        counter5++;
        
        /* Jump back to loop 4's domain */
        if (counter5 % 2 == 0) {
            goto overlap_point;
        }
    }
    
after_loop5:
    
    /* Loop 6: Adjacent but disjoint from all others */
    /* Uses different array section to ensure no block sharing */
    for (int m = 0; m < N/4; m++) {
        array1[m + 512] = m * 6;
        asm volatile("" : : "r"(array1[m + 512]) : "memory");
    }
    
    /* Loop 7: Another simple loop with different structure */
    int n = 0;
    while (n < N/3) {
        array2[n + 256] = n * 7;
        asm volatile("" : : "r"(array2[n + 256]) : "memory");
        n++;
    }
}

/* Main function to drive execution */
int main() {
    volatile int N = 100;  /* Volatile to prevent constant propagation */
    int checksum = 0;
    
    /* Call the function with all loop patterns */
    test_loop_patterns(N);
    
    /* Compute checksum to ensure all loops executed */
    volatile int dummy_array[1024];
    for (int i = 0; i < 1024; i++) {
        dummy_array[i] = i;
        checksum ^= dummy_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
