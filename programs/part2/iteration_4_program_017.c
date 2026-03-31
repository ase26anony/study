/* Target: MIPS architecture with delay slots */
/* Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -S -o reorg_test.s reorg_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_kernel(int *result, const int *data, int size) {
    int temp1 = 0, temp2 = 0, temp3 = 0;
    int a = 1, b = 2, c = 3, d = 4;
    volatile int barrier = 0; /* Prevent optimizations */
    
    /* Multiple basic blocks with label-oriented jumps */
    for (int i = 0; i < size; i++) {
        /* Create pressure for delay slot filling */
        if (__builtin_expect(data[i] > 100, 0)) {
            /* Block A: Jump to label with simple arithmetic follower */
            goto label_a;
        } else if (__builtin_expect(data[i] < 0, 0)) {
            /* Block B: Another jump pattern */
            goto label_b;
        } else {
            /* Block C: Regular computation */
            temp1 += data[i] * 2;
            continue;
        }
        
        /* Never reached directly, only via jumps */
        label_a:
        /* Candidate for delay slot: Simple, non-trapping arithmetic */
        /* Uses distinct registers from jump context */
        a = b + c;  /* Simple add, no trap possible */
        
        /* Follow with more computation to prevent merging */
        temp2 += a * 3;
        barrier = temp2; /* Memory barrier */
        continue;
        
        label_b:
        /* Another candidate with different registers */
        d = c ^ 0xFF;  /* Bitwise operation, no trap */
        
        temp3 += d >> 1;
        barrier = temp3;
        continue;
    }
    
    /* Mix integer and float to stress scheduler */
    float ftemp = 0.0f;
    for (int i = 0; i < size; i++) {
        if (i & 1) {
            ftemp += data[i] * 0.5f;
        }
    }
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 3; j++) {
            /* Conditional jump to label with eligible follower */
            if ((data[i] + j) % 5 == 0) {
                goto label_c;
            }
            temp1 += j;
        }
        
        if (i % 7 == 0) {
            /* Another jump pattern */
            goto label_d;
        }
        continue;
        
        label_c:
        /* Eligible instruction: subtraction with distinct vars */
        b = c - a;  /* Simple, non-trapping */
        temp2 += b;
        __asm__ volatile ("" ::: "memory"); /* Scheduling barrier */
        
        label_d:
        /* Bitwise operation candidate */
        a = d | 0x0F;  /* Safe operation */
        temp3 += a;
    }
    
    /* Final result computation */
    *result = temp1 + temp2 + temp3 + (int)ftemp;
}

/* Secondary function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void branch_dense_loop(int *arr, int n) {
    int x = 0, y = 0, z = 0;
    
    /* High density of branches */
    for (int i = 0; i < n; i++) {
        /* Multiple conditional jumps */
        if (arr[i] % 2 == 0) goto L1;
        if (arr[i] % 3 == 0) goto L2;
        if (arr[i] % 5 == 0) goto L3;
        
        x += arr[i];
        continue;
        
        L1:
        y = x + 1;  /* Eligible: simple assignment with add */
        arr[i] = y;
        continue;
        
        L2:
        z = y & 0x7F;  /* Eligible: bitwise AND */
        x = z;
        continue;
        
        L3:
        y = z ^ x;  /* Eligible: XOR operation */
        z = y;
    }
}

/* Main function to drive execution */
int main() {
    const int SIZE = 1000;
    int *data = (int*)malloc(SIZE * sizeof(int));
    int result1 = 0, result2 = 0;
    
    /* Initialize with pattern that creates branch variance */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 37) % 200 - 50; /* Range: -50 to 149 */
    }
    
    /* Execute kernels */
    delay_slot_kernel(&result1, data, SIZE);
    branch_dense_loop(data, SIZE);
    
    /* Compute final result to prevent elimination */
    for (int i = 0; i < SIZE; i++) {
        result2 += data[i];
    }
    
    printf("Result1: %d, Result2: %d\n", result1, result2);
    
    free(data);
    return 0;
}
