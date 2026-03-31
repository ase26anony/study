/* test_hw_loops.c
 * Designed to trigger specific uncovered lines in GCC's hw-doloop.cc
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_loops.c -o test.o
 */

#include <stdio.h>
#include <stdint.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int N = 100;
volatile int M = 50;
volatile int K = 75;

/* Arrays for side effects */
int array1[200];
int array2[200];
int array3[200];

/* Mark function as hot to encourage hardware loop optimization */
__attribute__((hot, noinline))
void test_loop_patterns(void) {
    volatile int i, j, k;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Loop 1: Simple countable loop - will be analyzed by hw-doloop */
    for (i = 0; i < N; i++) {
        array1[i] = i * 2;
        sum1 += array1[i];
    }
    
    /* Loop 2: Adjacent but disjoint loop (no basic block overlap with Loop 1) */
    for (j = 0; j < M; j++) {
        array2[j] = j * 3;
        sum2 += array2[j];
    }
    
    /* Loop 3: Perfectly nested within Loop 4 */
    /* This should trigger: loop->loops.safe_push(other) */
    for (k = 0; k < K; k++) {
        array3[k] = k * 4;
        sum3 += array3[k];
        
        /* Loop 4: Outer loop containing Loop 3 */
        /* This creates perfect nesting where Loop 3's blocks are a subset of Loop 4's */
        int m;
        for (m = 0; m < 10; m++) {
            /* Simple operation to create basic blocks */
            array1[k] += m;
            /* Memory clobber to prevent optimization */
            asm volatile("" : : : "memory");
        }
    }
    
    /* Loop 5: do-while loop for CFG variation */
    int counter = 0;
    do {
        array1[counter % 100] += counter;
        counter++;
        asm volatile("" : : : "memory");
    } while (counter < 25);
    
    /* Loop 6 and Loop 7: Partially overlapping loops */
    /* This should trigger: other->loops.safe_push(loop) */
    int x, y;
    
    /* Loop 6 */
    for (x = 0; x < 40; x++) {
        array2[x] = x * x;
        
        /* Conditional that creates partial overlap */
        if (x > 20) {
            /* Jump to label inside Loop 7 */
            goto partial_overlap;
        }
        
        array1[x] += x;
        
        /* Label that Loop 7 will jump back to */
        loop7_continue:
        asm volatile("" : : : "memory");
    }
    
    /* Loop 7 - partially overlaps with Loop 6 */
    for (y = 10; y < 30; y++) {
        array3[y] = y * y * y;
        
        /* Jump into Loop 6's body */
        if (y == 15) {
            goto loop7_continue;
        }
        
        partial_overlap:
        array2[y] += y;
        asm volatile("" : : : "memory");
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
}

/* Additional test function with more complex nesting */
__attribute__((noinline))
void nested_loops_complex(void) {
    volatile int a, b, c;
    
    /* Multiple levels of nesting */
    for (a = 0; a < 20; a++) {
        for (b = 0; b < 15; b++) {
            /* Innermost loop - should be analyzed */
            for (c = 0; c < 10; c++) {
                array1[(a + b + c) % 200] += 1;
                asm volatile("" : : : "memory");
            }
        }
    }
    
    /* Adjacent loop with shared condition */
    int d = 0;
    while (d < 30) {
        array2[d % 200] = d;
        d++;
        
        /* Conditional that could create partial overlap */
        if (d == 15) {
            /* This creates a basic block that might be shared */
            for (int e = 0; e < 5; e++) {
                array3[e] += array2[d];
            }
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
    
    /* Call functions with loop patterns */
    test_loop_patterns();
    nested_loops_complex();
    
    /* Compute checksum to ensure all loops execute */
    int checksum = 0;
    for (int i = 0; i < 200; i++) {
        checksum += array1[i] + array2[i] + array3[i];
        checksum &= 0xFFFF; /* Prevent overflow */
    }
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
