/* reorg_coverage.c
 * 
 * This program is designed to trigger the uncovered delay slot filling logic
 * in GCC's reorg.cc (lines 2135-2149). It creates patterns where:
 * 1. Unconditional jumps (goto) target labels
 * 2. The instruction immediately after the label is eligible for moving into delay slots
 * 3. Resource dependencies allow the move
 * 4. The candidate instruction is non-trapping and non-throwing
 * 
 * Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -S reorg_coverage.c
 * Or for other delay slot architectures: -march=sparc, -march=microblaze, etc.
 */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively on MIPS */
#ifdef __GNUC__
#define TARGET_MIPS __attribute__((target("arch=mips")))
#else
#define TARGET_MIPS
#endif

/* Volatile to prevent optimization of critical patterns */
static volatile int guard = 0;

TARGET_MIPS
void delay_slot_patterns(int *arr1, int *arr2, int n) {
    int i;
    int a = 0, b = 0, c = 0, d = 0;
    int t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4;
    int s1 = 5, s2 = 6, s3 = 7, s4 = 8;
    
    /* Create multiple jump-to-label patterns in a loop */
    for (i = 0; i < n; i++) {
        /* Vary the condition to create different scheduling contexts */
        if (__builtin_expect((arr1[i] & 1) != 0, 0)) {
            /* Pattern 1: Jump to label with simple arithmetic follower */
            goto label1;
            
            /* This should never be reached directly */
            a = b + c; /* This might get moved into delay slot */
            
        label1:
            /* Candidate for delay slot filling - simple integer operation
               Uses registers NOT used in the delay slot set */
            t1 = r1 + r2; /* Non-trapping, non-memory, splittable */
            
            /* Continue with other operations */
            arr2[i] = t1 + arr1[i];
        } 
        else if (__builtin_expect(arr1[i] < 0, 0)) {
            /* Pattern 2: Another jump pattern with different registers */
            goto label2;
            
            /* Unreachable code that might affect scheduling */
            d = a ^ b;
            
        label2:
            /* Another eligible candidate - bitwise operation */
            t2 = s1 & s2; /* Safe, non-trapping operation */
            
            arr2[i] = t2 - arr1[i];
        }
        else {
            /* Pattern 3: Nested conditional with jump */
            if (guard != 0) {
                goto label3;
                
                /* Potential delay slot candidate location */
                t3 = r3 * 2;
                
            label3:
                /* Multiplication by constant is safe (no overflow trap in C) */
                t4 = s3 << 1; /* Shift instead of multiply for safety */
                
                arr2[i] = t4 | arr1[i];
            } else {
                /* Alternative path with memory barrier to constrain scheduling */
                asm volatile("" ::: "memory");
                arr2[i] = arr1[i] * 3;
            }
        }
        
        /* Mix in floating point to diversify resource usage */
        if (i % 8 == 0) {
            float ftemp = (float)arr1[i] * 0.5f;
            arr2[i] += (int)ftemp;
        }
        
        /* Create register pressure to force interesting scheduling */
        r1 = r2 ^ i;
        r2 = r3 + i;
        r3 = r4 - i;
        r4 = r1 * 2;
        
        s1 = s2 | i;
        s2 = s3 & i;
        s3 = s4 << 1;
        s4 = s1 >> 1;
    }
    
    /* Additional complex control flow to engage reorg pass */
    for (i = 0; i < n; i += 2) {
        /* Switch-like pattern with goto labels */
        switch (arr1[i] % 4) {
            case 0:
                goto case0_label;
            case0_label:
                t1 = r1 + 1;
                break;
            case 1:
                goto case1_label;
            case1_label:
                t1 = r2 - 1;
                break;
            case 2:
                goto case2_label;
            case2_label:
                t1 = r3 * 2;
                break;
            default:
                goto default_label;
            default_label:
                t1 = r4 / 2; /* Division by constant - safe */
                break;
        }
        arr2[i] += t1;
    }
}

TARGET_MIPS
void nested_loop_pattern(int *arr, int n) {
    int i, j;
    int x = 0, y = 0, z = 0;
    
    /* Create nested loops with varying trip counts */
    for (i = 0; i < n; i++) {
        /* Inner loop with multiple basic blocks */
        for (j = 0; j < (arr[i] & 0x3F); j++) {
            /* Conditional jump to label */
            if ((j & 1) == 0) {
                goto inner_label1;
            inner_label1:
                x = y + z; /* Eligible candidate */
            } else {
                goto inner_label2;
            inner_label2:
                y = x - z; /* Another candidate */
            }
            
            /* Simple operation that doesn't trap */
            z = (x ^ y) + j;
            
            /* Memory operation to create scheduling boundaries */
            arr[i] += z;
        }
        
        /* Loop-carried dependency */
        x = y;
        y = z;
        z = arr[i];
    }
}

TARGET_MIPS
int main() {
    const int N = 1000;
    int *array1, *array2;
    int i, result = 0;
    
    /* Allocate and initialize arrays */
    array1 = (int*)malloc(N * sizeof(int));
    array2 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern to create varied control flow */
    for (i = 0; i < N; i++) {
        array1[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        array2[i] = 0;
    }
    
    /* Set guard to non-zero sometimes */
    guard = array1[0] & 1;
    
    /* Execute the delay slot patterns */
    delay_slot_patterns(array1, array2, N);
    
    /* Execute nested loop pattern */
    nested_loop_pattern(array2, N / 10);
    
    /* Compute result to prevent dead code elimination */
    for (i = 0; i < N; i++) {
        result ^= array2[i];
    }
    
    /* Print result to ensure side effects */
    printf("Result: %d\n", result);
    
    /* Clean up */
    free(array1);
    free(array2);
    
    return 0;
}
