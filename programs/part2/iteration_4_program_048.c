/* reorg_coverage.c - Target GCC's reorg.cc delay slot filling logic */
/* Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -S reorg_coverage.c */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_patterns(int *arr1, int *arr2, int size) {
    int i, j;
    int temp1 = 0, temp2 = 0, temp3 = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int s1 = 6, s2 = 7, s3 = 8, s4 = 9, s5 = 10;
    
    /* Pattern 1: Simple jump to label with eligible follower */
    for (i = 0; i < size; i++) {
        if (__builtin_expect(arr1[i] > 100, 0)) {
            /* This should become a simplejump to label L1 */
            goto TARGET_LABEL_1;
        }
        
        /* Some computation to create scheduling pressure */
        temp1 = arr1[i] * r1 + r2;
        temp2 = arr2[i] * r3 - r4;
        
        /* Avoid merging with the label's instruction */
        asm volatile("" : : : "memory");
        
        continue;
        
    TARGET_LABEL_1:
        /* Eligible instruction: simple integer operation, non-trapping */
        /* This is the 'next_trial' candidate */
        acc1 = acc1 + r5;  /* Simple add, uses different register set */
        
        /* More computation to prevent dead code elimination */
        arr1[i] = temp1 ^ temp2;
    }
    
    /* Pattern 2: Nested loops with multiple jump-label patterns */
    for (i = 0; i < size - 1; i++) {
        for (j = i + 1; j < size; j++) {
            if (__builtin_expect(arr1[i] == arr2[j], 0)) {
                /* Another simplejump to label */
                goto TARGET_LABEL_2;
            }
            
            /* Different register usage pattern */
            s1 = arr1[i] & 0xFF;
            s2 = arr2[j] | 0x80;
            
            /* Memory barrier to constrain scheduling */
            __sync_synchronize();
            
            continue;
            
        TARGET_LABEL_2:
            /* Another eligible candidate instruction */
            /* Uses completely different resources than the jump's context */
            acc2 = s3 ^ s4;  /* Bitwise op, non-trapping */
            
            /* Ensure this isn't optimized away */
            arr2[j] = s1 + s2;
        }
    }
    
    /* Pattern 3: Switch-like pattern with goto labels */
    for (i = 0; i < size; i++) {
        int val = arr1[i] % 8;  /* Modulo by constant - safe */
        
        switch (val) {
            case 0:
                goto LABEL_CASE_0;
            case 1:
                goto LABEL_CASE_1;
            case 2:
                goto LABEL_CASE_2;
            default:
                /* Default computation */
                temp3 = arr1[i] * 3;
                break;
        }
        
        /* These labels should be immediate targets of simple jumps */
        LABEL_CASE_0:
            /* Candidate: simple subtraction */
            acc3 = acc3 - 1;  /* Safe, non-trapping */
            arr1[i] = temp3 + 1;
            continue;
            
        LABEL_CASE_1:
            /* Candidate: bitwise AND with constant */
            acc3 = acc3 & 0x7FFFFFFF;  /* Safe, non-trapping */
            arr1[i] = temp3 + 2;
            continue;
            
        LABEL_CASE_2:
            /* Candidate: shift operation */
            acc3 = acc3 << 1;  /* Safe, non-trapping */
            arr1[i] = temp3 + 3;
            continue;
    }
    
    /* Pattern 4: Complex control flow with multiple eligible candidates */
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    int limit = size / 2;
    
    while (limit-- > 0) {
        if (__builtin_expect(*ptr1 > *ptr2, 0)) {
            goto PTR_LABEL_A;
        }
        
        /* Alternate path with different resource usage */
        r1 = *ptr1 + *ptr2;
        r2 = *ptr1 - *ptr2;
        
        /* Force a basic block boundary */
        if (r1 != r2) {
            asm volatile("" : : : "memory");
        }
        
        ptr1++;
        ptr2++;
        continue;
        
    PTR_LABEL_A:
        /* Eligible candidate: multiplication by constant (safe) */
        acc1 = acc1 * 2;  /* Simple multiplication, non-trapping */
        
        /* Use different registers than the jump context */
        s1 = *ptr1;
        s2 = *ptr2;
        ptr1++;
        ptr2++;
    }
    
    /* Final computation using all accumulators to prevent elimination */
    arr1[0] = acc1 + acc2 + acc3;
}

/* Secondary function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void more_patterns(int *data, int n) {
    int i;
    int x = 0, y = 0, z = 0;
    
    /* Create a chain of jumps */
    for (i = 0; i < n; i++) {
        if (data[i] == 0) {
            goto CHAIN_LABEL_1;
        }
        
        x = data[i] + i;
        if (x > 1000) {
            goto CHAIN_LABEL_2;
        }
        
        y = x * 2;
        continue;
        
    CHAIN_LABEL_1:
        /* First candidate in chain */
        z = z | 0x01;  /* Bitwise OR, safe */
        data[i] = -1;
        continue;
        
    CHAIN_LABEL_2:
        /* Second candidate in chain */
        z = z & 0xFE;  /* Bitwise AND, safe */
        data[i] = 1000;
    }
    
    /* Use result */
    data[n-1] = z;
}

int main() {
    const int SIZE = 100;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        return 1;
    }
    
    /* Initialize with pattern that creates varied branch behavior */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 17) % 123;
        array2[i] = (i * 23) % 456;
    }
    
    /* Call the functions with delay slot patterns */
    delay_slot_patterns(array1, array2, SIZE);
    more_patterns(array1, SIZE);
    
    /* Compute and print checksum to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += array1[i] + array2[i];
    }
    
    printf("Result checksum: %d\n", sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
