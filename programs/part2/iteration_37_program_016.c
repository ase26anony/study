/* test_hw_loops.c */
/* Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_loops.c -o test.o */
/* For coverage analysis: gcc -O2 -march=armv8-a -fmodulo-sched -fdump-rtl-doloop -c test_hw_loops.c */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int N = 100;
volatile int M = 50;
volatile int K = 75;

/* Arrays to store results and prevent dead code elimination */
int array1[200];
int array2[200];
int array3[200];

/* Mark function as hot to encourage hardware loop optimization */
__attribute__((hot, noinline))
void test_loop_patterns(void) {
    int i, j, k;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Loop 1: Simple countable loop - will be analyzed by hw-doloop */
    for (i = 0; i < N; i++) {
        array1[i] = i * 2;
        sum1 += array1[i];
    }
    
    /* Loop 2: Adjacent but disjoint loop (shares no basic blocks with Loop 1) */
    for (j = 0; j < M; j++) {
        array2[j] = j * 3;
        sum2 += array2[j];
    }
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* This should trigger: loop->loops.safe_push(other) */
    for (k = 0; k < K; k++) {
        /* Loop 4: Outer loop containing Loop 3 */
        for (i = 0; i < 10; i++) {
            array3[k] += k * i;
            /* Simple side effect to prevent optimization */
            asm volatile("" : : : "memory");
        }
    }
    
    /* Loop 5 and Loop 6: Partially overlapping loops */
    /* This should trigger: other->loops.safe_push(loop) */
    int x = 0;
    
    /* Loop 5: First loop with conditional branching */
    for (i = 0; i < N; i++) {
        if (i % 3 == 0) {
            /* Jump to label inside Loop 6's conceptual body */
            /* This creates partial overlap in CFG */
            goto partial_overlap;
        }
        array1[i] += i;
        continue;
        
    partial_overlap:
        /* This label is inside both Loop 5 and Loop 6's CFG */
        array2[i % M] = i;
        
        /* Loop 6: Second loop that overlaps with Loop 5 */
        /* Using do-while for CFG variation */
        j = 0;
        do {
            array3[j] += array1[i] + array2[i % M];
            j++;
            /* Force side effect */
            asm volatile("" : : : "memory");
        } while (j < 5);
        
        /* Continue with Loop 5 */
        if (i % 2 == 0) {
            x++;
        }
    }
    
    /* Loop 7: Another countable loop with complex exit condition */
    /* Creates more CFG edges for analysis */
    int counter = 0;
    while (counter < N) {
        int temp = counter;
        for (int inner = 0; inner < 3; inner++) {
            array1[temp] += inner;
            temp = (temp + 1) % N;
        }
        counter += 2;
        
        /* Conditional break to create additional basic blocks */
        if (counter > N/2) {
            /* Early exit creates separate exit block */
            array2[0] = counter;
            if (counter % 7 == 0) break;
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
}

/* Additional test function with switch-case inside loop */
/* Creates more complex CFG patterns */
__attribute__((noinline))
void test_switch_loop(void) {
    volatile int limit = 30;
    int state = 0;
    
    for (int i = 0; i < limit; i++) {
        switch (i % 4) {
            case 0:
                array1[i] = i;
                /* Fall through to create shared basic blocks */
            case 1:
                array2[i] = i * 2;
                break;
            case 2:
                /* Nested loop inside switch case */
                for (int j = 0; j < 5; j++) {
                    array3[j] += i;
                }
                break;
            case 3:
                array1[i] = array2[i] + array3[i % 10];
                break;
        }
        
        /* Another loop that might share blocks with the outer loop */
        if (state == 0 && i > limit/2) {
            int k = 0;
            while (k < 3) {
                array3[k] = array1[i] + k;
                k++;
            }
            state = 1;
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
    
    /* Call the function with complex loop patterns */
    test_loop_patterns();
    test_switch_loop();
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += array1[i] + array2[i] + array3[i];
        checksum &= 0xFFFF; /* Prevent overflow */
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
