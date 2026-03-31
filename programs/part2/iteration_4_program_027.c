/* Target: MIPS architecture with delay slots */
/* Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -S -o reorg_test.s reorg_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_kernel(int *a, int *b, int n) {
    int i, j;
    int temp1, temp2, temp3;
    volatile int dummy = 0; /* Prevent over-optimization */
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    for (i = 0; i < n; i++) {
        /* Create multiple basic blocks with jumps to labels */
        if (__builtin_expect((i & 1) == 0, 1)) {
            /* Jump pattern 1 - simple unconditional jump to label */
            goto label_arithmetic_1;
            
            /* This should never be reached directly */
            dummy = 1;
        }
        
        /* Alternate path to create control flow complexity */
        r1 = a[i] + b[i];
        s1 = r1 * 2;
        
        /* Another conditional jump */
        if (__builtin_expect((i & 3) == 0, 0)) {
            goto label_arithmetic_2;
        }
        
        continue;
        
        /* Label with simple, non-trapping arithmetic that can be moved into delay slot */
        label_arithmetic_1:
            /* Simple integer operation - no trapping, no memory access */
            r2 = r3 + r4;  /* Uses different registers than surrounding code */
            /* Follow with another operation to ensure it's not merged */
            s2 = s3 ^ s4;
            /* Continue normal flow */
            a[i] = r2;
            continue;
            
        label_arithmetic_2:
            /* Another simple operation using different registers */
            r3 = r1 - r2;
            s3 = s1 | s2;
            b[i] = r3;
            continue;
    }
    
    /* Nested loop to increase scheduling pressure */
    for (i = 0; i < n; i++) {
        for (j = 0; j < 4; j++) {
            /* More jump-to-label patterns */
            if (__builtin_expect((j & 1) == 0, 1)) {
                goto label_bitwise;
            }
            
            /* Some floating point to diversify resource usage */
            float f1 = (float)a[i];
            float f2 = f1 * 2.0f;
            dummy = (int)f2;
            
            if (__builtin_expect(j == 2, 0)) {
                goto label_shift;
            }
            
            continue;
            
            label_bitwise:
                /* Bitwise operations are safe and non-trapping */
                r4 = r1 & r2;
                s4 = s3 << 2;
                a[i] ^= r4;
                continue;
                
            label_shift:
                /* Shift operation - also safe */
                temp1 = b[i] >> 1;
                temp2 = temp1 + i;
                b[i] = temp2;
                continue;
        }
    }
}

/* Complex control flow with multiple jumps to stress the scheduler */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
int complex_branch_pattern(int x, int y) {
    int result = 0;
    
    /* Multiple labels with simple arithmetic followers */
    if (x > y) {
        goto add_operation;
    }
    
    if (x < 0) {
        goto sub_operation;
    }
    
    /* Memory barrier to constrain scheduling */
    __sync_synchronize();
    
    result = x * y;
    goto end;
    
    add_operation:
        /* Candidate for delay slot filling */
        result = x + y;  /* Simple add, no trapping */
        /* Follow with another operation */
        result ^= 0xFF;
        goto end;
        
    sub_operation:
        /* Another candidate */
        result = x - y;
        result &= 0xFFFF;
        goto end;
        
    end:
    return result;
}

/* Main computational kernel with dense branch patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void compute_hash(int *data, int size, int *hash) {
    int i;
    int h1 = 0x5A5A5A5A;
    int h2 = 0xA5A5A5A5;
    
    for (i = 0; i < size; i++) {
        /* Create pressure for delay slot filling */
        if (__builtin_expect((data[i] & 0x01) != 0, 1)) {
            goto hash_update1;
        }
        
        if (__builtin_expect((data[i] & 0x02) != 0, 0)) {
            goto hash_update2;
        }
        
        /* Default path */
        h1 = h1 ^ data[i];
        continue;
        
        hash_update1:
            /* Simple arithmetic after label - good delay slot candidate */
            h2 = h2 + data[i];
            h1 = h1 ^ h2;
            continue;
            
        hash_update2:
            /* Another candidate */
            h2 = h2 - data[i];
            h1 = h1 | h2;
            continue;
    }
    
    /* Final mixing */
    *hash = h1 ^ h2;
    
    /* Force another jump pattern */
    if (__builtin_expect(h1 > h2, 1)) {
        goto final_adjust;
    }
    
    *hash = *hash << 1;
    return;
    
    final_adjust:
        *hash = *hash >> 1;
        return;
}

int main() {
    const int SIZE = 100;
    int array1[SIZE], array2[SIZE];
    int hash_result = 0;
    
    /* Initialize arrays with pattern to create predictable branches */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 7;
    }
    
    /* Run kernels that create jump-to-label patterns */
    delay_slot_kernel(array1, array2, SIZE);
    
    int complex_result = complex_branch_pattern(array1[0], array2[0]);
    
    compute_hash(array1, SIZE, &hash_result);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d, %d\n", array1[SIZE/2], complex_result, hash_result);
    
    /* Additional loop with mixed operations to keep reorg pass busy */
    for (int i = 0; i < SIZE; i++) {
        /* Varying trip counts create complex CFG */
        for (int j = 0; j < (i % 8); j++) {
            /* More goto patterns */
            if ((i + j) % 3 == 0) {
                goto mix_op;
            }
            
            array1[i] += array2[j];
            continue;
            
            mix_op:
                array2[j] -= array1[i];
                continue;
        }
    }
    
    printf("Final array1[10] = %d\n", array1[10]);
    
    return 0;
}
