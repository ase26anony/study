/* test_hw_doloop.c
 * Designed to trigger uncovered lines in hw-doloop.cc (lines 429-436)
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_doloop.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force architecture target for ARM hardware loops */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(int N, int *results) {
    volatile int limit = N;  /* Prevent constant propagation */
    int i, j, k;
    
    /* Array for loop computations */
    int arr[1000];
    for (i = 0; i < 1000; i++) arr[i] = i;
    
    /* ========== Loop Set 1: Simple adjacent loops ========== */
    /* Loop A - Simple countable loop */
    int sum_a = 0;
    for (i = 0; i < limit; i++) {
        sum_a += arr[i % 1000];
        /* Memory clobber to prevent optimization */
        asm volatile("" : : : "memory");
    }
    results[0] = sum_a;
    
    /* Loop B - Adjacent but disjoint loop */
    int sum_b = 0;
    for (j = 0; j < limit/2; j++) {
        sum_b += arr[(j * 2) % 1000];
        asm volatile("" : : : "memory");
    }
    results[1] = sum_b;
    
    /* ========== Loop Set 2: Perfectly nested loops ========== */
    /* Outer Loop C */
    int sum_c = 0;
    for (i = 0; i < limit/3; i++) {
        /* Inner Loop D - completely inside Loop C */
        int sum_d = 0;
        for (j = 0; j < 5; j++) {
            sum_d += arr[(i + j) % 1000];
            asm volatile("" : : : "memory");
        }
        sum_c += sum_d;
        asm volatile("" : : : "memory");
    }
    results[2] = sum_c;
    
    /* ========== Loop Set 3: Partially overlapping loops ========== */
    /* This creates the partial overlap scenario for bitmap_intersect_compl_p */
    
    /* Loop E - with conditional that can jump into Loop F */
    int sum_e = 0;
    int counter = 0;
    
    /* Label for partial overlap */
    partial_overlap_start:
    
    /* Loop E body */
    for (i = 0; i < limit/2; i++) {
        sum_e += arr[i % 1000];
        counter++;
        
        /* Conditional that creates partial overlap */
        if (counter > limit/4) {
            /* Jump into Loop F's domain */
            goto inside_loop_f;
        }
        asm volatile("" : : : "memory");
    }
    
    /* Jump over Loop F to avoid direct nesting */
    goto after_loop_f;
    
inside_loop_f:
    /* Loop F - partially overlaps with Loop E via the goto */
    int sum_f = 0;
    for (k = 0; k < limit/3; k++) {
        sum_f += arr[(k + 10) % 1000];
        asm volatile("" : : : "memory");
        
        /* Jump back to Loop E's start to create mutual partial overlap */
        if (k == limit/6) {
            goto partial_overlap_start;
        }
    }
    
after_loop_f:
    results[3] = sum_e;
    results[4] = sum_f;
    
    /* ========== Loop Set 4: Do-while loops for CFG variation ========== */
    /* Loop G - do-while loop */
    int sum_g = 0;
    i = 0;
    do {
        sum_g += arr[i % 1000];
        asm volatile("" : : : "memory");
        i++;
    } while (i < limit/4);
    results[5] = sum_g;
    
    /* Loop H - adjacent do-while */
    int sum_h = 0;
    j = limit/4;
    do {
        sum_h += arr[j % 1000];
        asm volatile("" : : : "memory");
        j++;
    } while (j < limit/2);
    results[6] = sum_h;
    
    /* ========== Loop Set 5: Complex partial overlap pattern ========== */
    /* Loop I and Loop J share some blocks but neither is subset of the other */
    
    int sum_i = 0, sum_j = 0;
    int toggle = 0;
    
loop_i_start:
    /* Loop I */
    for (i = 0; i < limit/5; i++) {
        sum_i += arr[i % 1000];
        
        /* Shared block - executed by both loops */
        shared_block:
        toggle = 1 - toggle;
        asm volatile("" : : : "memory");
        
        if (toggle) {
            /* Jump to Loop J */
            goto loop_j_body;
        }
    }
    goto after_loop_j;
    
loop_j_body:
    /* Loop J */
    for (j = 0; j < limit/5; j++) {
        sum_j += arr[(j + 5) % 1000];
        
        /* Enter the shared block */
        goto shared_block;
    }
    
after_loop_j:
    results[7] = sum_i;
    results[8] = sum_j;
}

/* Main function to drive execution */
int main() {
    volatile int N = 100;  /* Runtime value to prevent optimization */
    int results[10] = {0};
    int checksum = 0;
    
    /* Call the function with all loop patterns */
    test_loop_patterns(N, results);
    
    /* Compute checksum to ensure all loops executed */
    for (int i = 0; i < 10; i++) {
        checksum += results[i];
        printf("Result[%d] = %d\n", i, results[i]);
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional test with different N to explore more paths */
    N = 50;
    test_loop_patterns(N, results);
    
    return 0;
}
