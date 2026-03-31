/* Target: MIPS architecture with delay slots */
/* Compile with: -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if supported */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_kernel(int *a, int *b, int n) {
    int i, j;
    int temp1, temp2, temp3, temp4;
    int acc = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Main loop with multiple jump patterns */
    for (i = 0; i < n; i++) {
        /* Create pressure for delay slot filling */
        if (__builtin_expect(a[i] > 0, 1)) {
            /* Pattern 1: Jump to label with simple arithmetic follower */
            if (a[i] & 1) {
                goto label_arithmetic_1;
            } else {
                goto label_arithmetic_2;
            }
            
            /* These should NOT be reached directly */
            temp1 = a[i] + b[i];  /* Simple non-trapping arithmetic */
            continue;
            
        label_arithmetic_1:
            /* Candidate for delay slot: simple integer operation */
            /* Uses distinct resources (r1-r4) from jump context */
            r1 = r2 + r3;  /* Non-trapping, splittable operation */
            /* Follow with more operations to create scheduling context */
            r4 = r1 ^ 0x55;
            acc += r4;
            continue;
            
        label_arithmetic_2:
            /* Another candidate with different resources */
            s1 = s2 | s3;  /* Bitwise OR - safe, non-trapping */
            s4 = s1 << 2;
            acc += s4;
            continue;
        }
        
        /* Alternate path with different jump pattern */
        if (__builtin_expect(b[i] < 0, 0)) {
            /* Pattern 2: Nested jumps to create complex flow */
            if (acc & 1) {
                goto label_bitwise_1;
            }
            
            /* Memory barrier to constrain scheduling */
            __asm__ volatile("" ::: "memory");
            
            temp2 = a[i] * 3;  /* Multiplication is safe for integers */
            acc += temp2;
            continue;
            
        label_bitwise_1:
            /* Another eligible candidate */
            temp3 = temp4 & 0xFF;  /* Simple bitwise AND */
            acc += temp3;
        }
    }
    
    /* Second loop with different patterns */
    for (i = 0; i < n; i += 2) {
        /* Create conditional jumps to labels */
        int cond = a[i] ^ b[i];
        
        /* Pattern 3: Multiple jumps in switch-like pattern */
        switch (cond & 3) {
            case 0:
                goto label_case_0;
            case 1:
                goto label_case_1;
            case 2:
                goto label_case_2;
            default:
                goto label_case_3;
        }
        
        /* These labels must be immediately followed by simple operations */
    label_case_0:
        r1 = r2 - r3;  /* Subtraction - safe, non-trapping */
        acc += r1;
        continue;
        
    label_case_1:
        s1 = s3 ^ s4;  /* XOR operation */
        acc += s1;
        continue;
        
    label_case_2:
        temp1 = temp2 + 1;  /* Increment */
        acc += temp1;
        continue;
        
    label_case_3:
        temp3 = ~temp4;  /* Bitwise NOT */
        acc += temp3;
        continue;
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = acc;
}

/* Mixed integer/float operations to stress scheduler */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void mixed_operations(int *arr, float *farr, int n) {
    int i;
    
    for (i = 0; i < n; i++) {
        /* Jump pattern with float context */
        if (__builtin_expect(arr[i] > 100, 0)) {
            goto label_float_mix;
        }
        
        arr[i] = arr[i] * 2;
        continue;
        
    label_float_mix:
        /* Simple integer operation (candidate for delay slot) */
        int tmp = arr[i] / 2;  /* Division by constant - safe */
        /* Follow with float op in different basic block */
        farr[i] = (float)tmp;
    }
}

/* Main computational kernel */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
int main() {
    const int SIZE = 1024;
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    float *farray = (float*)malloc(SIZE * sizeof(float));
    
    /* Initialize with pattern to trigger various branches */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 17) % 256;
        array2[i] = (i * 23) % 256;
        farray[i] = (float)i;
    }
    
    /* Call kernels multiple times to increase optimization opportunities */
    for (int iter = 0; iter < 100; iter++) {
        delay_slot_kernel(array1, array2, SIZE);
        mixed_operations(array2, farray, SIZE);
    }
    
    /* Compute checksum to ensure computation happens */
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
