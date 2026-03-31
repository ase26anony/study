/* Target: MIPS architecture with delay slots */
/* Compile with: -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if supported */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void compute_kernel(int *result, const int *data, int size) {
    int temp1 = 0, temp2 = 0, temp3 = 0;
    int a = 1, b = 2, c = 3, d = 4;
    volatile int barrier = 0; /* Prevent optimizations */
    
    /* Multiple jump-to-label patterns to create scheduling opportunities */
    for (int i = 0; i < size; i++) {
        /* Vary control flow to create different scheduling contexts */
        if (__builtin_expect((data[i] & 1) != 0, 1)) {
            /* Pattern 1: Jump to label with simple arithmetic follower */
            if (temp1 < 100) {
                goto label_arithmetic_1;
            } else {
                temp1 = 0;
            }
            
            /* This should not be reached immediately */
            barrier = temp1;
            continue;
            
        label_arithmetic_1:
            /* Candidate for delay slot: Simple, non-trapping integer operation */
            /* Uses distinct registers from the jump's context */
            c = a + b;  /* Simple add, no trapping, not in &set or &needed */
            temp1 += c;
            continue;
        }
        
        /* Another branch pattern */
        if (__builtin_expect((data[i] & 2) != 0, 0)) {
            int x = 5, y = 6, z = 7;
            
            if (temp2 < 50) {
                goto label_arithmetic_2;
            }
            
            /* Memory barrier to constrain scheduling */
            __asm__ volatile("" ::: "memory");
            temp2 = 0;
            continue;
            
        label_arithmetic_2:
            /* Another candidate: bitwise operation, no trapping */
            z = x & y;  /* Safe operation, distinct resources */
            temp2 += z;
            continue;
        }
        
        /* Third pattern with different register usage */
        {
            int r1 = 10, r2 = 20, r3 = 30;
            
            if (temp3 < 75) {
                goto label_arithmetic_3;
            }
            
            /* Mix with floating point to diversify resource usage */
            float ftemp = 1.5f;
            barrier = (int)ftemp;
            temp3 = 0;
            continue;
            
        label_arithmetic_3:
            /* Simple subtraction, safe, splittable */
            r3 = r2 - r1;  /* Non-trapping, eligible for delay slot */
            temp3 += r3;
            
            /* Insert a nop-like asm to prevent sequence formation */
            __asm__ volatile("" ::: "memory");
        }
        
        /* Complex loop with varying trip counts to stress scheduler */
        for (int j = 0; j < (i & 3); j++) {
            /* Create more basic blocks */
            if (j & 1) {
                d = c ^ a;  /* More arithmetic */
            } else {
                d = b | c;
            }
            barrier = d;
        }
    }
    
    /* Combine results */
    *result = temp1 + temp2 + temp3;
}

/* Secondary function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void nested_branch_pattern(int *out, int n) {
    int acc = 0;
    int v1 = 1, v2 = 2, v3 = 3;
    
    /* Nested loops create complex CFG */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 4; j++) {
            /* Multiple conditional jumps to labels */
            switch ((i + j) & 3) {
                case 0:
                    if (acc < 1000) {
                        goto target_label_0;
                    }
                    v1 *= 2;
                    break;
                    
                case 1:
                    if ((i ^ j) & 1) {
                        goto target_label_1;
                    }
                    v2 += 3;
                    break;
                    
                default:
                    /* Force a jump */
                    goto target_label_2;
            }
            
            /* These should not be reached directly */
            __asm__ volatile("" ::: "memory");
            continue;
            
        target_label_0:
            /* Eligible instruction: simple assignment */
            v3 = v1 + v2;  /* Safe, non-trapping */
            acc += v3;
            continue;
            
        target_label_1:
            /* Another eligible instruction */
            v1 = v2 - v3;  /* Simple subtraction */
            acc += v1;
            continue;
            
        target_label_2:
            /* Bitwise operation candidate */
            v2 = v3 ^ v1;  /* No trapping possible */
            acc += v2;
            continue;
        }
        
        /* Memory operation in different path */
        if (i & 1) {
            *out = acc;  /* Store to prevent elimination */
        }
    }
    
    *out = acc;
}

/* Main driver that creates the right context */
int main() {
    const int SIZE = 1000;
    int *data = (int*)malloc(SIZE * sizeof(int));
    int result1 = 0, result2 = 0;
    
    /* Initialize with pattern that creates branch variance */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i * 1103515245 + 12345;  /* Simple PRNG pattern */
    }
    
    /* Execute kernels to generate RTL patterns */
    compute_kernel(&result1, data, SIZE);
    nested_branch_pattern(&result2, SIZE / 2);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Verify with a simple check */
    if (result1 != 0 || result2 != 0) {
        printf("Computation performed successfully.\n");
    }
    
    free(data);
    return 0;
}
