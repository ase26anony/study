/* Target architecture: MIPS (has delay slots) */
/* Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -S -o reorg_test.s reorg_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_pattern(int *arr1, int *arr2, int size) {
    int i, j;
    int acc = 0;
    int temp1, temp2, temp3;
    
    /* Use distinct registers/variables to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Create multiple jump-to-label patterns */
    for (i = 0; i < size; i++) {
        /* First pattern: simple conditional jump to label */
        if (__builtin_expect(arr1[i] > 100, 0)) {
            /* Jump to label with simple arithmetic after it */
            goto label_arithmetic_1;
        }
        
        /* Some computation to create scheduling context */
        r1 = arr1[i] * 3;
        r2 = arr2[i] + 7;
        
        /* Second pattern */
        if (__builtin_expect(r1 < r2, 1)) {
            goto label_arithmetic_2;
        }
        
        /* Continue normal execution */
        r3 = r1 - r2;
        arr1[i] = r3;
        continue;
        
        /* Label 1: Simple non-trapping arithmetic (eligible for delay slot) */
    label_arithmetic_1:
        /* This is the candidate instruction for delay slot filling */
        /* Simple integer add - no trapping, no memory access */
        s1 = s2 + s3;  /* Uses different resource set than r1-r4 */
        
        /* Continue execution */
        r4 = s1 * 2;
        arr2[i] = r4;
        continue;
        
        /* Label 2: Another candidate pattern */
    label_arithmetic_2:
        /* Another simple arithmetic operation */
        s4 = s1 ^ s2;  /* Bitwise XOR - non-trapping */
        
        /* More computation */
        temp1 = s4 << 2;
        arr1[i] = temp1;
    }
    
    /* Nested loop with more complex control flow */
    for (i = 0; i < size - 1; i++) {
        for (j = i + 1; j < size; j++) {
            /* Create pressure for delay slot filling */
            if (__builtin_expect(arr1[i] > arr1[j], 0)) {
                /* Jump to label with simple operation */
                goto label_swap_check;
            }
            
            /* Alternate path with memory barrier to constrain scheduling */
            if (arr1[i] == arr1[j]) {
                __asm__ volatile("" ::: "memory");
                goto label_equal;
            }
            
            /* Normal computation continues */
            temp2 = arr1[i] - arr1[j];
            continue;
            
        label_swap_check:
            /* Candidate for delay slot: simple logical operation */
            temp3 = arr1[i] & 0xFF;  /* Non-trapping, no memory access */
            
            /* Actual swap logic */
            if (temp3 > arr1[j]) {
                int tmp = arr1[i];
                arr1[i] = arr1[j];
                arr1[j] = tmp;
            }
            continue;
            
        label_equal:
            /* Another candidate */
            s2 = s3 | 0x1;  /* Simple bitwise OR */
            arr2[i] += s2;
        }
    }
}

/* Mix integer and float operations in different paths */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void mixed_operations(int *arr, float *farr, int size) {
    int i;
    int local_int1 = 0, local_int2 = 0;
    float local_float1 = 0.0f, local_float2 = 0.0f;
    
    for (i = 0; i < size; i++) {
        /* Create jump pattern that might be scheduled */
        if (__builtin_expect(arr[i] % 2 == 0, 1)) {
            goto label_int_op;
        } else {
            goto label_float_op;
        }
        
        /* This should never be reached directly */
        continue;
        
    label_int_op:
        /* Simple integer operation - delay slot candidate */
        local_int1 = local_int2 + i;  /* Non-trapping, uses different resources */
        
        /* Follow-up computation */
        arr[i] = local_int1 * 2;
        continue;
        
    label_float_op:
        /* Different type of operation to diversify resource usage */
        local_float1 = local_float2 + (float)i;
        farr[i] = local_float1;
    }
}

/* Main computational kernel */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
int main() {
    const int SIZE = 100;
    int array1[SIZE];
    int array2[SIZE];
    float farray[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = rand() % 200;
        array2[i] = rand() % 200;
        farray[i] = (float)(rand() % 100) / 10.0f;
    }
    
    /* Call functions with jump patterns */
    delay_slot_pattern(array1, array2, SIZE);
    mixed_operations(array1, farray, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array1[i] + array2[i] + (int)farray[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
