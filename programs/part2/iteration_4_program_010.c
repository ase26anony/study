/* Target: MIPS architecture with delay slots */
/* Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -S -o reorg_test.s reorg_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_kernel(int *a, int *b, int size) {
    int i, temp1, temp2, temp3, temp4;
    volatile int barrier = 0; /* Use for memory barriers */
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Main computational loop with multiple jump patterns */
    for (i = 0; i < size; i++) {
        /* Pattern 1: Conditional jump to label with simple arithmetic after label */
        if (__builtin_expect((a[i] & 0x1) != 0, 0)) {
            /* Jump to label L1 - will become simplejump in RTL */
            goto L1;
        }
        
        /* Some arithmetic to create scheduling pressure */
        r1 = a[i] * 3;
        r2 = b[i] + r1;
        
        /* Continue normal flow */
        a[i] = r2;
        continue;
        
    L1:
        /* Candidate for delay slot filling: Simple, non-trapping arithmetic */
        /* Uses different register set than operations before the jump */
        s1 = s2 + s3;  /* Simple add, no trapping possible */
        
        /* More operations to prevent dead code elimination */
        s4 = s1 ^ 0xFF;
        b[i] = s4;
        
        /* Memory barrier to constrain scheduling in other paths */
        barrier = 1;
        
        /* Pattern 2: Another jump pattern with different register usage */
        if (__builtin_expect((i % 3) == 0, 1)) {
            goto L2;
        }
        
        r3 = r4 << 2;
        a[i] += r3;
        continue;
        
    L2:
        /* Another delay slot candidate */
        temp1 = temp2 & temp3;  /* Bitwise AND, non-trapping */
        temp4 = temp1 | 0x55;
        b[i] ^= temp4;
        
        /* Pattern 3: Nested conditional jumps */
        if (__builtin_expect(a[i] > b[i], 0)) {
            if (__builtin_expect((i % 5) == 0, 0)) {
                goto L3;
            }
            goto L4;
        }
        
        r1 = r2 - r3;  /* Simple subtraction */
        continue;
        
    L3:
        /* Candidate after label L3 */
        s2 = s3 * 2;  /* Multiplication by 2 (shift), non-trapping */
        s1 = s2 + 1;
        continue;
        
    L4:
        /* Candidate after label L4 */
        temp2 = temp3 ^ temp4;  /* XOR operation */
        temp1 = temp2 + i;
        
        /* Pattern 4: Loop with multiple exit points */
        for (int j = 0; j < 4; j++) {
            if (__builtin_expect((a[i] >> j) & 1, 0)) {
                goto L5;
            }
            r4 = r1 + j;
        }
        continue;
        
    L5:
        /* Candidate after label L5 */
        s3 = s4 - s1;  /* Simple subtraction */
        b[i] += s3;
    }
    
    /* Final computation to use all temporaries and prevent elimination */
    int sum = 0;
    for (i = 0; i < size; i++) {
        sum += a[i] + b[i];
    }
    
    /* Use the result */
    a[0] = sum;
}

/* Secondary function with different jump patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void secondary_patterns(int *arr, int n) {
    int x = 0, y = 0, z = 0;
    
    /* Switch-like pattern using goto labels */
    for (int i = 0; i < n; i++) {
        int val = arr[i] % 7;
        
        if (val == 0) goto CASE0;
        if (val == 1) goto CASE1;
        if (val == 2) goto CASE2;
        
        /* Default case */
        x = y + z;
        continue;
        
    CASE0:
        /* Simple arithmetic after label - delay slot candidate */
        y = z * 3;  /* Safe multiplication */
        arr[i] = y;
        continue;
        
    CASE1:
        /* Another candidate */
        z = x & y;  /* Bitwise AND */
        arr[i] = z ^ 0xAA;
        continue;
        
    CASE2:
        /* Another candidate */
        x = y - z;  /* Simple subtraction */
        arr[i] = x | 0x55;
        continue;
    }
}

/* Mix integer and float operations to stress scheduler */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void mixed_operations(float *farr, int *iarr, int n) {
    float f1 = 1.0f, f2 = 2.0f;
    int i1 = 0, i2 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Integer path with jump to label */
        if (__builtin_expect(iarr[i] > 100, 0)) {
            goto INT_LABEL;
        }
        
        /* Float path */
        f1 = f2 * 3.14f;
        farr[i] = f1;
        continue;
        
    INT_LABEL:
        /* Integer operation after label - delay slot candidate */
        i1 = i2 + 5;  /* Simple add, non-trapping */
        iarr[i] = i1;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
}

int main() {
    const int SIZE = 1000;
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    float *farray = (float*)malloc(SIZE * sizeof(float));
    
    /* Initialize with pattern to trigger various branches */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 7;
        farray[i] = i * 0.5f;
    }
    
    /* Execute kernels to generate RTL patterns */
    delay_slot_kernel(array1, array2, SIZE);
    secondary_patterns(array1, SIZE);
    mixed_operations(farray, array2, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array1[i] + array2[i] + (int)farray[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    
    free(array1);
    free(array2);
    free(farray);
    
    return 0;
}
