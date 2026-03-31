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
    volatile int dummy = 0; /* Prevent optimizations */
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    for (i = 0; i < n; i++) {
        /* Create multiple basic blocks with jumps to labels */
        if (__builtin_expect((a[i] & 1) != 0, 0)) {
            /* Jump pattern 1: simple goto to label with eligible follower */
            goto label1;
        } else if (__builtin_expect(a[i] < 0, 0)) {
            /* Jump pattern 2 */
            goto label2;
        } else {
            /* Jump pattern 3 */
            goto label3;
        }
        
        /* These labels must be immediately followed by simple, non-trapping operations */
        /* that don't conflict with resources used before the jump */
        
    label1:
        /* Candidate for delay slot: simple integer operation using distinct registers */
        /* This should not reference resources in &set or &needed from delay slot */
        r1 = r2 + r3;  /* Simple add, no trap possible */
        /* Continue with other operations */
        temp1 = a[i] * b[i];
        b[i] = temp1 + r1;
        continue;
        
    label2:
        /* Another candidate: bitwise operation */
        s1 = s2 ^ s3;  /* No trapping, uses different register set */
        /* Memory barrier to constrain scheduling in other paths */
        asm volatile("" ::: "memory");
        temp2 = a[i] - b[i];
        a[i] = temp2 ^ s1;
        continue;
        
    label3:
        /* Third candidate: shift operation */
        r4 = r3 << 2;  /* Constant shift, safe */
        temp3 = a[i] + b[i];
        b[i] = temp3 - r4;
        /* Mix in some floating point in alternate path to stress scheduler */
        if (dummy) {
            float ftemp = (float)temp3;
            dummy = (int)ftemp;
        }
        continue;
    }
    
    /* Additional jump patterns in nested loop for more scheduling contexts */
    for (i = 0; i < n; i += 2) {
        int *ptr1 = &a[i];
        int *ptr2 = &b[i];
        
        /* Create another jump-to-label pattern */
        if (__builtin_expect(*ptr1 > *ptr2, 1)) {
            goto label4;
        }
        
        /* Some arithmetic to create dependencies */
        temp4 = *ptr1 * 3;
        
    label4:
        /* Eligible instruction: simple assignment with no resource conflicts */
        s4 = s3 | 0xFF;  /* Bitwise OR with constant */
        *ptr2 = temp4 + s4;
        
        /* Complex control flow to inhibit merging */
        switch (i % 4) {
            case 0: dummy++; break;
            case 1: dummy--; break;
            case 2: asm volatile("" ::: "memory"); break;
            case 3: /* empty */ break;
        }
    }
}

/* Secondary function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void branch_dense_loop(int *arr, int size) {
    int i, j, acc = 0;
    
    /* Very tight loop with many branches */
    for (i = 0; i < size; i++) {
        /* Multiple conditional jumps to create scheduling pressure */
        if (arr[i] == 0) {
            goto zero_case;
        } else if (arr[i] < 0) {
            goto neg_case;
        } else {
            goto pos_case;
        }
        
    zero_case:
        /* Simple arithmetic that could go in delay slot */
        acc = acc + 1;  /* Uses 'acc', not conflicting with other resources */
        continue;
        
    neg_case:
        acc = acc - arr[i];  /* Simple subtraction */
        /* Insert a no-op asm to prevent sequence formation */
        asm volatile("" :::);
        continue;
        
    pos_case:
        acc = acc * 2;  /* Multiplication by 2 (shift in RTL) */
        /* Vary the pattern slightly */
        if (i & 1) {
            asm volatile("" ::: "memory");
        }
        continue;
    }
    
    /* Prevent dead code elimination */
    arr[0] = acc;
}

int main() {
    const int SIZE = 1000;
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern that creates branch variance */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 17) % 100 - 50;  /* Mix of positive and negative */
        array2[i] = (i * 23) % 100;
    }
    
    /* Execute kernels designed to trigger delay slot filling */
    delay_slot_kernel(array1, array2, SIZE);
    branch_dense_loop(array1, SIZE);
    
    /* Compute checksum to prevent optimization */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += array1[i] + array2[i];
    }
    
    printf("Result checksum: %d\n", sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
