/* Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -o reorg_test reorg_test.c */
#include <stdio.h>
#include <stdlib.h>

/* Force MIPS architecture if not using -march=mips flag */
#ifdef __GNUC__
#define MIPS_TARGET __attribute__((target("arch=mips")))
#else
#define MIPS_TARGET
#endif

/* Volatile to prevent optimization of critical sections */
static volatile int memory_barrier;

/* Resource separation: Use distinct register sets for different operations */
MIPS_TARGET
void delay_slot_pattern(int *arr1, int *arr2, int size) {
    int i, j;
    int reg_a = 0, reg_b = 0, reg_c = 0;  /* Resources for main computation */
    int reg_x = 0, reg_y = 0, reg_z = 0;  /* Separate resources for delay slot candidates */
    int temp1, temp2, temp3;
    
    /* Initialize with non-zero values to avoid constant propagation */
    for (i = 0; i < size; i++) {
        arr1[i] = (i * 3) % 17;
        arr2[i] = (i * 7) % 23;
    }
    
    /* Main computational loop with dense branching */
    for (i = 0; i < size; i++) {
        /* Force multiple basic blocks with conditional jumps */
        if (__builtin_expect(arr1[i] > 8, 0)) {
            /* Pattern 1: Jump to label with simple arithmetic follower */
            reg_a = arr1[i] + arr2[i];
            reg_b = reg_a * 2;
            
            /* Use goto to create simplejump to label */
            if (reg_b > 20) {
                goto target_label_1;
            }
            
            /* Alternate path to avoid fall-through optimization */
            reg_c = reg_b - 5;
            continue;
            
        target_label_1:
            /* Candidate for delay slot filling: Simple, non-trapping arithmetic */
            /* Uses separate resource set (reg_x, reg_y) to avoid conflicts */
            reg_x = reg_y + 3;  /* Simple add, no trap possible */
            reg_y = reg_x ^ 0xFF;
            
            /* Memory barrier to constrain scheduling */
            memory_barrier = reg_y;
            continue;
        }
        
        /* Second pattern with different register usage */
        if (__builtin_expect(arr1[i] < 4, 1)) {
            reg_a = arr2[i] - arr1[i];
            
            if (reg_a != 0) {
                goto target_label_2;
            }
            
            reg_b = reg_a | 0x01;
            continue;
            
        target_label_2:
            /* Another delay slot candidate: bitwise operation */
            reg_z = reg_x & 0x0F;  /* Safe, non-trapping operation */
            reg_x = reg_z << 1;
            
            /* Insert scheduling barrier */
            asm volatile("" ::: "memory");
            continue;
        }
        
        /* Third pattern with loop-invariant computation */
        temp1 = arr1[i] * arr2[i];
        if (temp1 % 2 == 0) {
            goto target_label_3;
        }
        
        temp2 = temp1 / 3;  /* Note: integer division, safe */
        continue;
        
    target_label_3:
        /* Candidate: safe shift operation */
        temp3 = temp1 >> 2;  /* No trap, uses different temp register */
        arr1[i] = temp3;
        
        /* Force control flow complexity */
        if (i % 3 == 0) {
            memory_barrier = temp3;
        }
    }
    
    /* Nested loop to increase scheduling pressure */
    for (i = 0; i < size - 1; i++) {
        for (j = i + 1; j < size; j++) {
            int diff = arr1[i] - arr1[j];
            
            /* Multiple conditional jumps in tight loop */
            if (diff > 0) {
                goto target_label_4;
            } else if (diff < 0) {
                goto target_label_5;
            } else {
                goto target_label_6;
            }
            
        target_label_4:
            /* Simple arithmetic after label */
            arr2[j] = arr2[j] + 1;
            continue;
            
        target_label_5:
            /* Another simple operation */
            arr2[j] = arr2[j] - 1;
            continue;
            
        target_label_6:
            /* Bitwise operation */
            arr2[j] = arr2[j] ^ 1;
            continue;
        }
    }
}

/* Mixed integer/float operations to diversify resource usage */
MIPS_TARGET
void mixed_operations(int *arr, float *farr, int size) {
    int i;
    float ftemp;
    
    for (i = 0; i < size; i++) {
        /* Integer path with branch */
        if (arr[i] > 100) {
            int itemp = arr[i] * 2;
            
            if (itemp > 200) {
                goto float_label;
            }
            
            arr[i] = itemp % 127;
            continue;
            
        float_label:
            /* Switch to float ops in different basic block */
            ftemp = farr[i] * 2.0f;
            farr[i] = ftemp + 1.0f;
            
            /* Simple integer op as delay slot candidate */
            arr[i] = arr[i] & 0x7F;  /* Safe bitwise AND */
            continue;
        }
        
        /* Alternate path */
        farr[i] = farr[i] / 2.0f;
    }
}

/* Hash computation with many branches */
MIPS_TARGET
int compute_hash(int *data, int len) {
    int hash = 0;
    int i;
    
    for (i = 0; i < len; i++) {
        /* Multiple conditional jumps */
        if (data[i] & 0x01) {
            goto hash_update_1;
        }
        
        if (data[i] & 0x02) {
            goto hash_update_2;
        }
        
        if (data[i] & 0x04) {
            goto hash_update_3;
        }
        
        hash = hash ^ data[i];
        continue;
        
    hash_update_1:
        /* Simple arithmetic after label */
        hash = hash + (data[i] << 1);
        continue;
        
    hash_update_2:
        hash = hash ^ (data[i] >> 1);
        continue;
        
    hash_update_3:
        hash = hash * 33 + data[i];
        continue;
    }
    
    return hash;
}

MIPS_TARGET
int main() {
    const int SIZE = 256;
    int *array1, *array2;
    float *farray;
    int hash_result;
    
    /* Allocate and initialize arrays */
    array1 = (int*)malloc(SIZE * sizeof(int));
    array2 = (int*)malloc(SIZE * sizeof(int));
    farray = (float*)malloc(SIZE * sizeof(float));
    
    if (!array1 || !array2 || !farray) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i;
        array2[i] = SIZE - i;
        farray[i] = i * 0.5f;
    }
    
    /* Execute patterns to generate RTL sequences */
    delay_slot_pattern(array1, array2, SIZE);
    mixed_operations(array1, farray, SIZE);
    hash_result = compute_hash(array1, SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Hash result: %d\n", hash_result);
    printf("Sample values: %d, %d, %.2f\n", 
           array1[SIZE/2], array2[SIZE/3], farray[SIZE/4]);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(farray);
    
    return 0;
}
