/* test_hw_loops.c - Coverage test for hw-doloop.cc bitmap intersection logic */

#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __ARM_ARCH
#define TARGET_ATTR __attribute__((target("arch=armv8-a")))
#else
#define TARGET_ATTR __attribute__((target("arch=armv8-a")))
#endif

/* Prevent inlining to preserve CFG structure */
TARGET_ATTR __attribute__((noinline,noipa))
void test_loop_patterns(int n, int *result) {
    volatile int N1 = n + 10;  /* Prevent constant propagation */
    volatile int N2 = n + 20;
    volatile int N3 = n + 30;
    volatile int N4 = n + 40;
    
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int temp1[100], temp2[100], temp3[100];
    
    /* Initialize arrays to prevent optimization */
    for (int i = 0; i < 100; i++) {
        temp1[i] = i;
        temp2[i] = i * 2;
        temp3[i] = i * 3;
    }
    
    /* ============================================
       LOOP 1: Simple countable loop (disjoint from others)
       This creates a basic loop that should be recognized
       ============================================ */
    for (int i = 0; i < N1; i++) {
        /* Simple arithmetic with side effect */
        sum1 += temp1[i % 100];
        asm volatile ("" : : : "memory");  /* Memory clobber prevents removal */
    }
    
    /* ============================================
       LOOP 2: Perfectly nested loop structure
       This creates a parent-child relationship
       ============================================ */
    for (int i = 0; i < N2; i++) {
        sum2 += temp2[i % 100];
        
        /* Nested loop - should be contained within Loop 2's blocks */
        for (int j = 0; j < 5; j++) {
            sum2 += j;
            asm volatile ("" : : : "memory");
        }
    }
    
    /* ============================================
       LOOP 3: Do-while loop variation
       Creates different CFG structure
       ============================================ */
    int k = 0;
    do {
        sum3 += temp3[k % 100];
        k++;
        asm volatile ("" : : : "memory");
    } while (k < N3);
    
    /* ============================================
       LOOP 4 and 5: Partially overlapping loops
       This is the key to trigger bitmap_intersect_compl_p logic
       ============================================ */
    
    /* Shared label for partial overlap */
    shared_computation:
        sum4 += 1;
        asm volatile ("" : : : "memory");
    
    /* LOOP 4: Loop with conditional goto to shared block */
    for (int i = 0; i < N4; i++) {
        sum4 += temp1[i % 100];
        
        /* Conditional that sometimes jumps to shared block */
        if (i % 3 == 0) {
            goto shared_computation;
        }
        
        /* Normal loop continuation */
        sum4 += i;
    }
    
    /* LOOP 5: Another loop that also uses the shared block */
    for (int i = 0; i < n; i++) {
        sum4 += temp2[i % 100];
        
        /* Different condition to jump to same shared block */
        if (i % 4 == 0) {
            goto shared_computation;
        }
        
        /* This creates partial overlap:
           - Both loops contain the shared_computation block
           - But each has unique blocks too
           - Neither is subset of the other
        */
        sum4 -= i;
    }
    
    /* ============================================
       LOOP 6: Adjacent loop with complex condition
       Creates another disjoint loop for comparison
       ============================================ */
    int m = 0;
    while (m < n) {
        int val = temp3[m % 100];
        sum1 += (val > 50) ? val : -val;
        m++;
        asm volatile ("" : : : "memory");
    }
    
    /* Store results to prevent dead code elimination */
    result[0] = sum1;
    result[1] = sum2;
    result[2] = sum3;
    result[3] = sum4;
}

/* ============================================
   Additional test function with more complex nesting
   ============================================ */
TARGET_ATTR __attribute__((noinline,noipa))
void test_nested_loops(int n, int *result) {
    volatile int limit = n;
    int arr1[50], arr2[50];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        arr1[i] = i;
        arr2[i] = 50 - i;
    }
    
    /* Complex nesting pattern */
    for (int i = 0; i < limit; i++) {
        /* Outer loop body */
        sum += arr1[i % 50];
        
        /* Middle loop - partially overlapping with inner */
        for (int j = 0; j < (i % 10); j++) {
            sum += arr2[j];
            
            /* Conditional that creates partial overlap */
            if (j == 5) {
                shared_label:
                    sum += 1000;
                    asm volatile ("" : : : "memory");
            }
        }
        
        /* Inner loop that also uses shared_label */
        for (int k = 0; k < 3; k++) {
            sum -= k;
            if (k == 1) {
                goto shared_label;
            }
        }
    }
    
    result[4] = sum;
}

int main() {
    volatile int base_n = 50;  /* Volatile to prevent constant propagation */
    int results[10] = {0};
    
    /* Call test functions multiple times with different values
       to ensure various execution paths are taken */
    for (int run = 0; run < 3; run++) {
        int n = base_n + run * 10;
        
        test_loop_patterns(n, results);
        test_nested_loops(n, results + 4);
        
        /* Compute checksum to ensure all loops executed */
        int checksum = 0;
        for (int i = 0; i < 10; i++) {
            checksum += results[i];
        }
        
        /* Print to prevent optimization and verify execution */
        printf("Run %d: Checksum = %d\n", run, checksum);
    }
    
    return 0;
}
