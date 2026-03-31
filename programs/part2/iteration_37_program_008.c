/* test_hw_doloop.c
 * Designed to trigger uncovered lines in GCC's hw-doloop.cc
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_doloop.c
 */

#include <stdio.h>
#include <stdint.h>

/* Force function to be compiled for ARM with hardware loop support */
#ifdef __arm__
__attribute__((target("arch=armv8-a")))
#endif
__attribute__((noinline, hot))
void test_loop_patterns(int N, int M, int *result1, int *result2, int *result3) {
    volatile int limit1 = N;
    volatile int limit2 = M;
    volatile int limit3 = N + M;
    
    int i, j, k;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Loop 1: Simple countable loop - will have its own basic blocks */
    for (i = 0; i < limit1; i++) {
        sum1 += i * 2;
        result1[i] = sum1;
    }
    
    /* Loop 2: Adjacent but disjoint loop (no block intersection with Loop 1) */
    for (j = 0; j < limit2; j++) {
        sum2 += j * 3;
        result2[j] = sum2;
        /* Simple side effect to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Loop 3: Perfectly nested inside conditional - creates hierarchical relationship */
    /* This should trigger: loop->loops.safe_push(other) */
    if (limit1 > 0) {
        for (k = 0; k < limit3; k++) {
            sum3 += k * 5;
            result3[k] = sum3;
            /* Nested loop inside the same basic block region */
            int m;
            for (m = 0; m < 2; m++) {
                result3[k] += m;
            }
        }
    }
    
    /* Loop 4: do-while loop for CFG variation */
    int l = 0;
    do {
        sum1 += l * 7;
        result1[l % N] = sum1;
        l++;
    } while (l < limit1);
    
    /* Complex pattern for partial overlap - critical for bitmap_intersect_compl_p logic */
    /* This creates two loops that share some basic blocks but aren't perfectly nested */
    
    /* Loop A */
    int a = 0;
    int shared_var = 0;
    
    /* Label for goto - creates CFG edge between loops */
    loop_a_start:
    for (; a < limit1; a++) {
        shared_var += a * 11;
        result1[a] = shared_var;
        
        /* Conditional that may jump to Loop B */
        if (shared_var % 7 == 0 && a < limit1 - 1) {
            /* This goto creates partial overlap between Loop A and Loop B */
            goto loop_b_entry;
        }
        
        /* Continue normal execution */
        result2[a % M] = shared_var;
        
        loop_b_entry:
        /* Loop B starts here - shares this basic block with Loop A */
        if (a < limit2) {
            int b;
            for (b = 0; b < 3; b++) {
                shared_var += b * 13;
                result3[b] = shared_var;
                
                /* Jump back to Loop A */
                if (b == 1 && shared_var % 3 == 0) {
                    a++;
                    goto loop_a_start;
                }
            }
        }
    }
    
    /* Final side effect */
    asm volatile("" : : : "memory");
}

/* Another function with more complex nesting patterns */
#ifdef __arm__
__attribute__((target("arch=armv8-a")))
#endif
__attribute__((noinline))
void nested_loop_patterns(int N, int *arr1, int *arr2) {
    volatile int outer_limit = N;
    volatile int inner_limit = N / 2 + 1;
    
    /* Multiple nested loops to create complex bitmap relationships */
    
    /* Outer loop 1 */
    for (int i = 0; i < outer_limit; i++) {
        arr1[i] = i;
        
        /* Inner loop 1 - perfectly nested */
        for (int j = 0; j < inner_limit; j++) {
            arr2[j] += i * j;
            
            /* Innermost loop - triple nesting */
            for (int k = 0; k < 2; k++) {
                arr1[i] += k;
                asm volatile("" : : : "memory");
            }
        }
        
        /* Another loop at same level but not nested in first inner loop */
        /* This creates sibling relationship in loop hierarchy */
        if (i % 2 == 0) {
            int m = 0;
            while (m < 3) {
                arr2[m] += i * 3;
                m++;
            }
        }
    }
    
    /* Separate but adjacent loop structure */
    int x = 0;
    int y = 0;
    
    /* Two loops with potential partial overlap through shared condition */
    for (x = 0; x < outer_limit; x++) {
        if (arr1[x] % 2 == 0) {
            shared_condition:
            arr2[x % inner_limit] = x * x;
            
            /* This block is shared when y-loop jumps here */
            for (y = 0; y < 2; y++) {
                arr1[x] += y;
            }
        }
    }
    
    /* Second loop that can jump into first loop's body */
    for (y = 0; y < inner_limit; y++) {
        if (arr2[y] % 3 == 0) {
            x = y;
            goto shared_condition;
        }
    }
}

int main() {
    const int N = 100;
    const int M = 50;
    
    int array1[N];
    int array2[M];
    int array3[N + M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) array1[i] = 0;
    for (int i = 0; i < M; i++) array2[i] = 0;
    for (int i = 0; i < N + M; i++) array3[i] = 0;
    
    /* Call the function with all the loop patterns */
    test_loop_patterns(N, M, array1, array2, array3);
    
    /* Call second function for more patterns */
    nested_loop_patterns(N, array1, array2);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < N; i++) checksum += array1[i];
    for (int i = 0; i < M; i++) checksum += array2[i];
    for (int i = 0; i < N + M; i++) checksum += array3[i];
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
