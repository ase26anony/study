/* Target: MIPS architecture with delay slots */
/* Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -S -o reorg_test.s reorg_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_kernel(int *a, int *b, int n) {
    int i, temp1, temp2, temp3, temp4;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4;
    int s1 = 5, s2 = 6, s3 = 7, s4 = 8;
    
    for (i = 0; i < n; i++) {
        /* Create multiple basic blocks with jumps to labels */
        if (__builtin_expect((a[i] & 1) != 0, 0)) {
            /* Jump pattern 1: simple jump to label with eligible follower */
            goto label1;
        } else {
            /* Alternate path with memory barrier to constrain scheduling */
            __asm__ volatile("" ::: "memory");
            goto label2;
        }
        
    label1:
        /* Eligible instruction: simple integer arithmetic, non-trapping */
        /* Uses register set r1-r4, distinct from s1-s4 used elsewhere */
        temp1 = r1 + r2;  /* Simple add, cannot trap */
        acc1 += temp1;
        
        /* Immediately another conditional jump to create scheduling pressure */
        if (__builtin_expect(b[i] > 0, 1)) {
            goto label3;
        }
        /* Continue with different operations */
        r1 = r3 ^ r4;  /* Bitwise op, safe */
        continue;
        
    label2:
        /* Another eligible instruction after label */
        /* Uses different register set s1-s4 */
        temp2 = s1 - s2;  /* Simple subtract, cannot trap */
        acc2 += temp2;
        
        /* Nested conditional to increase control flow complexity */
        if (i % 3 == 0) {
            goto label4;
        }
        s1 = s2 * s3;  /* Multiplication, safe for integers */
        continue;
        
    label3:
        /* Third eligible instruction pattern */
        temp3 = r2 | r3;  /* Bitwise OR, non-trapping */
        acc3 += temp3;
        
        /* Loop with varying trip count for scheduler stress */
        for (int j = 0; j < (i % 5) + 1; j++) {
            r4 = r4 + j;
        }
        continue;
        
    label4:
        /* Fourth eligible instruction pattern */
        temp4 = s3 & s4;  /* Bitwise AND, safe */
        acc4 += temp4;
        
        /* Mix with floating point in different path to diversify resources */
        if (i % 7 == 0) {
            float ftemp = (float)s1 * 0.5f;
            __asm__ volatile("" : "+f"(ftemp) : : );
        }
    }
    
    /* Use results to prevent dead code elimination */
    a[0] = acc1 + acc2 + acc3 + acc4;
}

/* Secondary function with different jump patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void branch_dense_loop(int *arr, int size) {
    int i = 0;
    int sum = 0;
    
    /* Very tight loop with high branch density */
    while (i < size) {
        /* Pattern: jump to label followed by simple arithmetic */
        if (__builtin_expect(arr[i] < 0, 0)) {
            goto negative_label;
        }
        
        /* Default path */
        sum += arr[i] * 2;
        i++;
        continue;
        
    negative_label:
        /* Candidate for delay slot filling */
        /* Simple shift operation, cannot trap */
        int shifted = arr[i] << 2;
        sum += shifted;
        i++;
        
        /* Immediate another conditional */
        if (i % 4 == 0) {
            goto skip_label;
        }
        continue;
        
    skip_label:
        /* Another simple operation after label */
        sum += 1;
        i++;
    }
    
    arr[0] = sum;
}

/* Main computational kernel */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
int main() {
    const int N = 1000;
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pattern that creates branch variance */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 17) % 73;
        array2[i] = (i * 23) % 59 - 30;  /* Some negative values */
    }
    
    /* Execute kernels multiple times to increase optimization opportunities */
    for (int iter = 0; iter < 10; iter++) {
        delay_slot_kernel(array1, array2, N);
        branch_dense_loop(array2, N);
        
        /* Small computation between iterations to break patterns */
        for (int i = 0; i < N; i++) {
            array1[i] ^= array2[i];  /* Non-trapping operation */
        }
    }
    
    /* Final reduction to observable output */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i];
    }
    
    printf("Result: %d\n", final_sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
