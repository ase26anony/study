/* Target: MIPS architecture with delay slots */
/* Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -S -o reorg_test.s reorg_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_kernel(int *a, int *b, int size) {
    int i, j;
    int temp1, temp2, temp3, temp4;
    int acc = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Create multiple jump-to-label patterns */
    for (i = 0; i < size; i++) {
        /* First jump pattern */
        if (__builtin_expect(a[i] > 100, 0)) {
            /* Jump to label with simple arithmetic follower */
            goto label1;
        } else {
            /* Alternate path with different resource usage */
            r1 = a[i] + b[i];
            r2 = r1 * 2;
            goto label2;
        }
        
    label1:
        /* Candidate for delay slot filling: simple, non-trapping arithmetic */
        /* Uses distinct registers from jump's context */
        s1 = s2 + s3;  /* Simple add - will pass try_split and eligible_for_delay */
        acc += s1;
        continue;
        
    label2:
        /* Another candidate instruction */
        s4 = s3 ^ s2;  /* Bitwise XOR - safe, non-trapping */
        acc += s4;
        
        /* Nested loop to increase scheduling pressure */
        for (j = 0; j < 4; j++) {
            /* Second jump pattern with different target */
            if (__builtin_expect(b[i] < 0, 1)) {
                goto label3;
            } else {
                temp1 = a[i] - b[i];
                goto label4;
            }
            
        label3:
            /* Another delay slot candidate */
            temp2 = temp3 & temp4;  /* Bitwise AND - resource compatible */
            acc += temp2;
            continue;
            
        label4:
            temp3 = temp1 | 0xFF;  /* Bitwise OR with constant */
            acc += temp3;
        }
        
        /* Memory barrier to constrain scheduling */
        __asm__ volatile ("" ::: "memory");
    }
    
    /* Third jump pattern in different control flow context */
    if (acc > 1000) {
        goto label5;
    } else {
        r3 = r4 << 2;
        goto label6;
    }
    
label5:
    /* Candidate: simple arithmetic with no resource conflicts */
    s2 = s1 + 1;  /* Increment - will pass insn_sets_resource_p checks */
    acc += s2;
    goto done;
    
label6:
    s3 = s4 - 1;  /* Decrement - also safe */
    acc += s3;
    
done:
    /* Use result to prevent dead code elimination */
    a[0] = acc;
}

/* Mixed integer/float operations to diversify resource patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void mixed_operations(int *arr, float *farr, int n) {
    int i;
    int local1 = 0, local2 = 0, local3 = 0;
    float flocal1 = 0.0f, flocal2 = 0.0f;
    
    for (i = 0; i < n; i++) {
        /* Create jump-to-label pattern in loop */
        if (__builtin_expect(arr[i] % 2 == 0, 0)) {
            goto mix_label1;
        }
        
        /* Different basic block with FP ops */
        flocal1 = farr[i] * 2.0f;
        flocal2 = flocal1 + 1.0f;
        goto mix_label2;
        
    mix_label1:
        /* Delay slot candidate: integer arithmetic */
        local1 = local2 + local3;  /* Uses different registers than FP ops */
        arr[i] = local1;
        continue;
        
    mix_label2:
        local2 = local3 ^ 0xAA;  /* Bitwise op - no trapping */
        arr[i] = local2;
    }
}

/* Main computational kernel with heavy branch density */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
int main() {
    const int SIZE = 256;
    int array1[SIZE], array2[SIZE];
    float farray[SIZE];
    int i, result = 0;
    
    /* Initialize with pattern to create predictable but varied branches */
    for (i = 0; i < SIZE; i++) {
        array1[i] = (i * 3) % 100;
        array2[i] = (i * 7) % 100 - 50;  /* Some negative values */
        farray[i] = (float)i / 10.0f;
    }
    
    /* Execute kernels multiple times to increase scheduling opportunities */
    for (i = 0; i < 100; i++) {
        delay_slot_kernel(array1, array2, SIZE);
        mixed_operations(array2, farray, SIZE);
    }
    
    /* Accumulate results (prevents dead code elimination) */
    for (i = 0; i < SIZE; i++) {
        result += array1[i] + array2[i];
    }
    
    printf("Result: %d\n", result);
    
    /* Additional complex control flow to stress reorg pass */
    volatile int trigger = 1;
    int x = 0, y = 0, z = 0;
    
    while (trigger) {
        /* Multiple consecutive jump-label patterns */
        if (x++ > 100) break;
        
        if (__builtin_expect(y % 3 == 0, 1)) {
            goto final_label1;
        }
        z = y * 2;
        goto final_label2;
        
    final_label1:
        /* Final delay slot candidate */
        y = z + 1;  /* Simple add, no resource conflicts with jump */
        continue;
        
    final_label2:
        y = z - 1;
    }
    
    return 0;
}
