/* 
 * Target: Trigger specific delay slot filling logic in GCC's reorg.cc
 * Compile with: -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-sched
 */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS architecture if supported */
#ifdef __GNUC__
#define MIPS_TARGET __attribute__((target("arch=mips")))
#else
#define MIPS_TARGET
#endif

/* Avoid optimization removing critical patterns */
static volatile int keep_alive = 0;

MIPS_TARGET
static void delay_slot_pattern(int *arr1, int *arr2, int size) {
    int i, temp1, temp2, temp3, temp4;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4;
    int s1 = 5, s2 = 6, s3 = 7, s4 = 8;
    
    /* Labels for jump targets - must be immediately before simple operations */
    target_label_1:
        /* Simple non-trapping integer operation - candidate for delay slot */
        r1 = r2 + r3;  /* Uses different registers than surrounding code */
    
    target_label_2:
        s1 = s2 ^ s3;  /* Bitwise operation, no trapping */
    
    target_label_3:
        r4 = r1 - r2;  /* Simple subtraction */
    
    target_label_4:
        s4 = s3 << 2;  /* Shift operation */
    
    /* Main loop with multiple jump patterns */
    for (i = 0; i < size; i++) {
        /* Create pressure for delay slot filling */
        if (__builtin_expect(arr1[i] > 0, 1)) {
            /* Pattern 1: Simple jump to label with eligible follower */
            goto target_label_1;
        after_label_1:
            /* Operations using different resources than the candidate */
            temp1 = arr2[i] * r4;  /* Different register set */
            acc1 += temp1;
            
            /* Another jump pattern */
            if (__builtin_expect(arr1[i] < 100, 0)) {
                goto target_label_2;
            after_label_2:
                temp2 = arr1[i] & s4;  /* Different resource set */
                acc2 ^= temp2;
            }
            
            /* Third pattern with different register usage */
            if (__builtin_expect((i & 1) == 0, 1)) {
                goto target_label_3;
            after_label_3:
                temp3 = r1 | s1;  /* Mix registers from both sets */
                acc3 |= temp3;
            }
            
            /* Fourth pattern */
            if (__builtin_expect(arr2[i] != 0, 1)) {
                goto target_label_4;
            after_label_4:
                temp4 = s2 + r2;  /* Another mix */
                acc4 -= temp4;
            }
        }
        
        /* Memory barrier to constrain scheduling in alternate path */
        if (i % 16 == 0) {
            __sync_synchronize();
        }
        
        /* Additional operations to create resource pressure */
        arr1[i] = acc1 + acc2;
        arr2[i] = acc3 ^ acc4;
    }
    
    /* Prevent dead code elimination */
    keep_alive = acc1 + acc2 + acc3 + acc4;
}

/* Secondary function with different patterns */
MIPS_TARGET
static void nested_jump_pattern(int *arr, int size) {
    int i, j;
    int a = 1, b = 2, c = 3, d = 4;
    
    /* More target labels */
    nested_label_1:
        a = b + c;
    
    nested_label_2:
        d = a ^ b;
    
    for (i = 0; i < size; i++) {
        for (j = 0; j < 4; j++) {
            /* Nested jumps increase scheduling complexity */
            if (__builtin_expect((i + j) % 3 == 0, 0)) {
                goto nested_label_1;
            after_nested_1:
                arr[i] += a;
                
                if (__builtin_expect(arr[i] > 50, 1)) {
                    goto nested_label_2;
                after_nested_2:
                    arr[i] ^= d;
                }
            }
            
            /* Simple arithmetic to alternate with jumps */
            b = c * 2;
            c = d + 1;
        }
        
        /* Varying trip counts create complex CFG */
        switch (i % 4) {
            case 0: a = b << 1; break;
            case 1: a = c >> 1; break;
            case 2: a = d & 0xFF; break;
            default: a = b ^ c; break;
        }
    }
}

/* Mix integer and float operations in different paths */
MIPS_TARGET
static void mixed_operations(int *arr, float *farr, int size) {
    int i;
    float f1 = 1.0f, f2 = 2.0f;
    int x = 1, y = 2, z = 3;
    
    mix_label_1:
        x = y + z;  /* Integer op for delay slot candidate */
    
    for (i = 0; i < size; i++) {
        if (__builtin_expect(i % 2 == 0, 1)) {
            /* Integer path with jump */
            goto mix_label_1;
        after_mix_1:
            arr[i] = x * y;
        } else {
            /* Floating path - different resource usage */
            f1 = f2 * 3.14f;
            farr[i] = f1;
        }
        
        /* Cross-path dependency */
        z = arr[i] + (int)farr[i];
    }
}

MIPS_TARGET
int main() {
    const int SIZE = 256;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    float *farray = malloc(SIZE * sizeof(float));
    
    if (!array1 || !array2 || !farray) {
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 7;
        farray[i] = i * 0.5f;
    }
    
    /* Execute patterns to generate RTL sequences */
    delay_slot_pattern(array1, array2, SIZE);
    nested_jump_pattern(array1, SIZE);
    mixed_operations(array2, farray, SIZE);
    
    /* Compute final result to prevent optimization */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += array1[i] + array2[i] + (int)farray[i];
    }
    
    printf("Result: %d (keep_alive=%d)\n", final_sum, keep_alive);
    
    free(array1);
    free(array2);
    free(farray);
    
    return 0;
}
