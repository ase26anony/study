/* 
 * This program is designed to trigger the uncovered delay slot filling logic
 * in GCC's reorg.cc (lines 2135-2149). It creates patterns where unconditional
 * jumps to labels are followed by simple, non-trapping arithmetic operations
 * that can be moved into delay slots.
 *
 * Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-sched -o reorg_test reorg_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively on MIPS */
#ifdef __GNUC__
#define MIPS_TARGET __attribute__((target("arch=mips")))
#else
#define MIPS_TARGET
#endif

/* Volatile to prevent optimization of delay slot candidates */
static volatile int guard = 0;

MIPS_TARGET
void delay_slot_pattern(int *arr1, int *arr2, int size) {
    int i, j;
    int temp1, temp2, temp3;
    
    /* Use distinct registers/variables to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Create multiple basic blocks with jumps to labels */
    for (i = 0; i < size; i++) {
        /* Pattern 1: Jump to label with simple arithmetic follower */
        if (__builtin_expect(arr1[i] > arr2[i], 0)) {
            /* Jump to label L1 */
            goto L1;
        } else {
            /* Alternate path with memory barrier to constrain scheduling */
            asm volatile("" ::: "memory");
            r1 += arr1[i];
            continue;
        }
        
    L1:
        /* Candidate for delay slot filling: simple integer operation
           that doesn't trap and uses different resources than the jump */
        s1 = s2 + s3;  /* Uses s1-s3, not r1-r4 */
        
        /* Continue with loop body */
        r2 += arr2[i];
        
        /* Pattern 2: Another jump pattern in the same loop */
        if (__builtin_expect((i & 1) == 0, 1)) {
            goto L2;
        }
        
        r3 += arr1[i] * 2;
        continue;
        
    L2:
        /* Another delay slot candidate */
        s4 = s1 ^ s2;  /* Bitwise operation - non-trapping */
        
        r4 += arr2[i] * 3;
    }
    
    /* Nested loop with different pattern */
    for (i = 0; i < size; i++) {
        for (j = 0; j < 4; j++) {
            /* Pattern 3: Conditional jump to label */
            if (__builtin_expect(arr1[i] + j > arr2[i], 0)) {
                goto L3;
            }
            
            /* Mix in floating point to diversify resource usage */
            float ftemp = (float)arr1[i] / 2.0f;
            guard = (int)ftemp;
            continue;
            
        L3:
            /* Delay slot candidate: safe arithmetic */
            temp1 = temp2 - temp3;
            
            arr1[i] += j;
        }
    }
    
    /* Pattern 4: Switch-like structure with goto labels */
    for (i = 0; i < size && i < 10; i++) {
        int selector = arr1[i] % 4;
        
        if (selector == 0) goto CASE0;
        if (selector == 1) goto CASE1;
        if (selector == 2) goto CASE2;
        goto CASEDEFAULT;
        
    CASE0:
        /* Simple assignment - good delay slot candidate */
        temp2 = temp1 | 0xFF;
        arr2[i] += 1;
        continue;
        
    CASE1:
        temp3 = temp2 & 0x0F;
        arr2[i] += 2;
        continue;
        
    CASE2:
        /* Avoid division (could trap) - use shift instead */
        temp1 = temp3 << 2;
        arr2[i] += 3;
        continue;
        
    CASEDEFAULT:
        arr2[i] += 4;
        continue;
    }
}

/* Secondary function with different patterns */
MIPS_TARGET
void secondary_patterns(int *arr, int size) {
    int i;
    int a = 1, b = 2, c = 3, d = 4;
    
    /* Create tight loop with many branches */
    for (i = 0; i < size; i++) {
        /* Unconditional jump pattern */
        if (__builtin_expect(arr[i] > 100, 0)) {
            goto PROCESS_HIGH;
        }
        
        /* Low value processing */
        a = b + c;
        arr[i] = a;
        continue;
        
    PROCESS_HIGH:
        /* Candidate instruction after label */
        d = a ^ b;  /* Uses different variables than the jump path */
        
        arr[i] = d * 2;
        
        /* Another immediate jump */
        if (__builtin_expect(arr[i] < 0, 0)) {
            goto NEGATIVE;
        }
        continue;
        
    NEGATIVE:
        /* Another simple operation */
        c = d + 1;
        arr[i] = -arr[i];
    }
}

MIPS_TARGET
int main() {
    const int SIZE = 100;
    int *array1, *array2;
    int result = 0;
    int i;
    
    /* Allocate and initialize arrays */
    array1 = (int*)malloc(SIZE * sizeof(int));
    array2 = (int*)malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        return 1;
    }
    
    /* Initialize with pattern to create predictable branches */
    for (i = 0; i < SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 2 + (i % 3);
    }
    
    /* Execute the delay slot patterns */
    delay_slot_pattern(array1, array2, SIZE);
    secondary_patterns(array1, SIZE);
    
    /* Compute result to prevent dead code elimination */
    for (i = 0; i < SIZE; i++) {
        result += array1[i] + array2[i];
    }
    
    /* Use result to ensure no optimization */
    printf("Result: %d\n", result);
    printf("Guard: %d\n", guard);
    
    free(array1);
    free(array2);
    
    return 0;
}
