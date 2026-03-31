/* Target: MIPS architecture with delay slots */
/* Compile with: -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if supported */
#ifdef __GNUC__
#define MIPS_TARGET __attribute__((target("arch=mips")))
#else
#define MIPS_TARGET
#endif

/* Volatile to prevent optimization of delay slot candidates */
static volatile int dummy_volatile = 0;

MIPS_TARGET
void delay_slot_test(int *arr1, int *arr2, int size) {
    int i, j;
    int acc1 = 0, acc2 = 0;
    int temp1, temp2, temp3;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4;
    int s1 = 5, s2 = 6, s3 = 7, s4 = 8;
    
    /* Create complex control flow with many jumps */
    for (i = 0; i < size; i++) {
        /* Force branch prediction hints */
        if (__builtin_expect((i & 1) == 0, 1)) {
            /* Jump pattern 1: Simple goto to label with eligible follower */
            if (arr1[i] > 100) {
                /* This jump will become simplejump_p in RTL */
                goto label_arithmetic_1;
            }
            
            /* Some arithmetic to create scheduling pressure */
            temp1 = r1 + r2;
            r1 = temp1 ^ r3;
            
            /* Memory barrier to constrain scheduling */
            asm volatile("" ::: "memory");
            
            continue;
            
        label_arithmetic_1:
            /* ELIGIBLE DELAY SLOT CANDIDATE:
               - Simple integer arithmetic (non-trapping)
               - Uses distinct registers (r4, s1) not used in delay slot set
               - Not a SEQUENCE, not a jump
               - Can be split by try_split */
            r4 = s1 + s2;  /* This is next_trial */
            
            /* Continue with more operations */
            acc1 += arr1[i];
            r2 = r4 * 2;
        }
        
        /* Nested loop for more scheduling contexts */
        for (j = 0; j < 3; j++) {
            /* Jump pattern 2: Another goto with different register set */
            if (__builtin_expect(arr2[j] < 50, 0)) {
                goto label_arithmetic_2;
            }
            
            /* Mixed operations to diversify resource usage */
            temp2 = s3 - s4;
            s3 = temp2 | s1;
            
            /* Another memory barrier */
            __sync_synchronize();
            
            continue;
            
        label_arithmetic_2:
            /* Another eligible candidate with different registers */
            s4 = r3 & r2;  /* Simple bitwise operation */
            
            acc2 += arr2[j];
            s1 = s4 + 1;
            
            /* Conditional jump to create more scheduling opportunities */
            if (s4 > 10) {
                /* This creates another jump for the scheduler */
                goto label_safe_op;
            }
        }
        
        /* More jump patterns in the outer loop */
        if (i % 3 == 0) {
            goto label_bitwise_op;
        }
        
        /* Floating point in alternate path to stress scheduler */
        float f1 = i * 0.5f;
        float f2 = f1 + 1.0f;
        dummy_volatile = (int)f2;
        
        continue;
        
    label_bitwise_op:
        /* Eligible operation: shift (non-trapping) */
        temp3 = r1 << 2;
        r3 = temp3;
        acc1 += r3;
        
    label_safe_op:
        /* Safe multiply (won't trap for small values) */
        int safe_mul = r2 * 3;  /* Constant multiply won't trap */
        acc2 += safe_mul;
    }
    
    /* Use results to prevent elimination */
    arr1[0] = acc1;
    arr2[0] = acc2;
}

/* Secondary test function with different patterns */
MIPS_TARGET
void nested_jump_patterns(int *data, int n) {
    int a = 1, b = 2, c = 3, d = 4;
    int x = 5, y = 6, z = 7, w = 8;
    
    while (n-- > 0) {
        /* Pattern 3: Jump to label followed by simple assignment */
        if (data[n] == 0) {
            goto label_assign;
        }
        
        /* Some computations that use different resources */
        a = b + c;
        b = c - d;
        
        /* Insert a call to create basic block boundaries */
        if (n % 5 == 0) {
            dummy_volatile = n;
        }
        
        continue;
        
    label_assign:
        /* Perfect delay slot candidate:
           - Simple assignment
           - No memory references
           - No function calls
           - Non-trapping */
        x = y + 1;  /* This should be eligible */
        
        /* Follow with more operations */
        data[n] = x;
        z = w ^ 0xFF;
    }
    
    /* Force register usage */
    data[0] = a + b + c + d + x + y + z + w;
}

/* Main function to drive the test */
int main(void) {
    const int SIZE = 100;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        return 1;
    }
    
    /* Initialize with pattern that creates varied branch behavior */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 7;
    }
    
    /* Run tests multiple times to increase scheduling pressure */
    for (int iter = 0; iter < 10; iter++) {
        delay_slot_test(array1, array2, SIZE);
        nested_jump_patterns(array1, SIZE / 2);
        
        /* Modify data to change branch patterns */
        for (int i = 0; i < SIZE; i++) {
            array1[i] += iter;
            array2[i] -= iter;
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Result1: %d, Result2: %d\n", array1[0], array2[0]);
    
    free(array1);
    free(array2);
    
    return 0;
}
