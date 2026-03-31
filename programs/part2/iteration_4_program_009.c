/* Target architecture: MIPS (has delay slots) */
/* Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -S -o reorg_test.s reorg_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_kernel(int *data, int *result, int size) {
    /* Use distinct register sets to avoid resource conflicts */
    int reg_a = 0, reg_b = 0, reg_c = 0;
    int reg_x = 0, reg_y = 0, reg_z = 0;
    int temp1 = 0, temp2 = 0, temp3 = 0;
    
    /* Initialize with non-zero values to avoid constant propagation */
    reg_a = data[0] ^ 0x1234;
    reg_b = data[1] ^ 0x5678;
    reg_c = data[2] ^ 0x9ABC;
    
    for (int i = 0; i < size; i++) {
        /* Create multiple basic blocks with simple jumps to labels */
        if (__builtin_expect((data[i] & 1) != 0, 0)) {
            /* Jump pattern 1: simple goto to label with eligible follower */
            goto label1;
        } else {
            /* Alternate path to create scheduling pressure */
            reg_x = reg_y + reg_z;
            goto label2;
        }
        
    label1:
        /* Candidate for delay slot: simple, non-trapping integer operation
           Uses different registers than those set before the jump */
        temp1 = reg_x + 1;  /* Simple add, won't trap */
        /* Follow with more operations to prevent merging into SEQUENCE */
        reg_a = reg_b ^ reg_c;
        continue;
        
    label2:
        /* Another candidate with different register set */
        temp2 = reg_a * 2;  /* Multiplication by constant, won't trap */
        reg_y = reg_z | 0xFF;
        
        /* Nested conditional to create more jump opportunities */
        if (__builtin_expect((data[i] & 2) != 0, 0)) {
            goto label3;
        }
        reg_b = reg_c + i;
        continue;
        
    label3:
        /* Third candidate pattern */
        temp3 = reg_y - reg_x;
        reg_z = reg_a & 0x0F;
        
        /* Memory barrier to constrain scheduling in alternate paths */
        __asm__ volatile ("" ::: "memory");
    }
    
    /* Mix integer and floating point in different blocks */
    float f1 = 0.0f, f2 = 0.0f;
    for (int i = 0; i < 4; i++) {
        f1 += i * 0.5f;
        if (i % 2 == 0) {
            f2 = f1 * 2.0f;
            goto float_label;
        }
    float_label:
        /* Integer operation after float block - diversifies resource usage */
        reg_a = reg_b + 1;
    }
    
    /* Store results to prevent dead code elimination */
    result[0] = reg_a + temp1;
    result[1] = reg_b + temp2;
    result[2] = reg_c + temp3;
}

/* Secondary function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void branch_dense_loop(int *arr, int n) {
    int a = 1, b = 2, c = 3, d = 4;
    int e = 5, f = 6, g = 7, h = 8;
    
    /* Unroll partially to create more scheduling contexts */
    for (int i = 0; i < n; i += 2) {
        /* Pattern A: jump to label with simple arithmetic follower */
        if (arr[i] > 0) {
            goto target_a;
        }
        a = b + c;
        continue;
        
    target_a:
        /* Eligible instruction: uses registers not in previous set */
        d = e + f;  /* Simple add, non-trapping */
        g = h ^ 0xAA;
        
        /* Pattern B: another jump chain */
        if (arr[i] < 100) {
            goto target_b;
        }
        continue;
        
    target_b:
        /* Another eligible candidate */
        h = g * 3;  /* Multiplication by constant */
        a = b - c;
        
        /* Complex control flow to stress the scheduler */
        switch (arr[i] % 4) {
            case 0: goto target_a;
            case 1: goto target_b;
            case 2: 
                /* Direct jump with immediate follower */
                goto target_c;
            default:
                d = e + 1;
        }
        continue;
        
    target_c:
        /* Candidate that should pass all checks:
           - Simple operation (bitwise AND)
           - Uses fresh registers
           - Non-trapping
           - Not a SEQUENCE */
        f = g & 0x0F;
        e = d | 0xF0;
    }
    
    /* Prevent optimization */
    arr[0] = a + b + c + d + e + f + g + h;
}

/* Main computational kernel */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
int main() {
    const int SIZE = 256;
    int *data = (int*)malloc(SIZE * sizeof(int));
    int *result = (int*)malloc(3 * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Run the delay slot patterns */
    delay_slot_kernel(data, result, SIZE);
    
    int *arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (i * 1664525 + 1013904223) & 0xFF;
    }
    
    branch_dense_loop(arr, SIZE);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += result[i];
    }
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    
    printf("Result checksum: %d\n", sum);
    
    free(data);
    free(result);
    free(arr);
    
    return 0;
}
