/* test_hw_loops.c
 * Designed to trigger specific uncovered lines in GCC's hw-doloop.cc
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_loops.c -o test_hw_loops.o
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining and constant propagation */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_INIT volatile

/* Target-specific attribute for ARM hardware loops */
#ifdef __arm__
#define TARGET_HW_LOOPS __attribute__((target("arch=armv8-a")))
#else
#define TARGET_HW_LOOPS
#endif

/* Global arrays to prevent dead code elimination */
static int array1[1024];
static int array2[1024];
static int array3[1024];

/* Function containing all loop patterns */
TARGET_HW_LOOPS NOINLINE
void test_loop_patterns(int n) {
    VOLATILE_INIT int N = n;
    int i, j, k;
    
    /* Loop 1: Simple countable loop (disjoint from others initially) */
    for (i = 0; i < N; i++) {
        array1[i] = i * 2;
    }
    
    /* Loop 2: Perfectly nested inside Loop 3 */
    /* This should trigger: loop->loops.safe_push(other) */
    for (j = 0; j < N/2; j++) {
        array2[j] = j * 3;
        
        /* Loop 3: Outer loop containing Loop 2 */
        for (k = 0; k < N/4; k++) {
            array3[k] = array2[j] + k;
        }
    }
    
    /* Loop 4: Do-while loop (different structure) */
    i = 0;
    do {
        array1[i] += array2[i % (N/2)];
        i++;
    } while (i < N);
    
    /* Loop 5 and 6: Partially overlapping loops using goto */
    /* This should trigger: other->loops.safe_push(loop) */
    int counter1 = 0;
    int counter2 = 0;
    
    /* Loop 5 */
    for (i = 0; i < N; i++) {
        array1[i] = array1[i] * 2;
        
        /* Conditional that creates partial overlap */
        if (i > N/2) {
            /* Jump into Loop 6's body */
            goto overlap_label;
        }
        
        /* Normal continuation of Loop 5 */
        array2[i % (N/2)] += i;
        continue;
        
    overlap_label:
        /* This block belongs to both Loop 5 and Loop 6 */
        array3[i % (N/4)] = array1[i] + array2[i % (N/2)];
        
        /* Loop 6: Starts here, overlaps with Loop 5 */
        for (j = i; j < N && counter2 < N/2; j++) {
            array3[j % (N/4)] += j;
            counter2++;
            
            /* Jump back to Loop 5 */
            if (counter2 > N/4) {
                goto end_overlap;
            }
        }
        
    end_overlap:
        /* Continue with Loop 5 */
        array1[i] -= 1;
    }
    
    /* Loop 7: Another simple loop to create more CFG complexity */
    for (i = N/4; i < 3*N/4; i++) {
        /* Use inline asm to prevent optimization */
        asm volatile("" : "+r"(array1[i]) : : "memory");
        array2[i % (N/2)] = array1[i] ^ 0x55;
    }
}

/* Additional function with adjacent loops */
TARGET_HW_LOOPS NOINLINE
void more_adjacent_loops(int m) {
    VOLATILE_INIT int M = m;
    int a, b;
    
    /* Loop A and B: Adjacent but disjoint */
    for (a = 0; a < M; a++) {
        array1[a] = a * a;
    }
    
    for (b = 0; b < M/2; b++) {
        array2[b] = b * b * b;
    }
    
    /* Loop C: Contains conditional inner block */
    for (a = 0; a < M; a++) {
        if (a % 3 == 0) {
            /* This creates a basic block inside the loop */
            array3[a % (M/4)] = array1[a] + array2[a % (M/2)];
        } else {
            array3[a % (M/4)] = array1[a] - array2[a % (M/2)];
        }
    }
}

/* Main function to drive execution */
int main() {
    VOLATILE_INIT int iterations = 100;
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        array1[i] = i;
        array2[i] = 1024 - i;
        array3[i] = 0;
    }
    
    /* Call functions with carefully constructed loops */
    test_loop_patterns(iterations);
    more_adjacent_loops(iterations * 2);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += array1[i] + array2[i] + array3[i];
        checksum &= 0xFFFF; /* Prevent overflow */
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
