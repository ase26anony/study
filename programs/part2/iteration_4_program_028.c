/* Target: MIPS architecture with delay slots */
/* Compile with: -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all */
/* This program creates patterns that should trigger the specific uncovered
   delay slot filling logic in reorg.cc lines 2135-2149 */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if supported */
#ifdef __GNUC__
#define MIPS_TARGET __attribute__((target("arch=mips")))
#else
#define MIPS_TARGET
#endif

/* Avoid optimization removing our carefully crafted patterns */
#define KEEP(expr) do { asm volatile("" : : "r"(expr)); } while(0)

MIPS_TARGET
static void delay_slot_pattern(int *arr1, int *arr2, int size) {
    int i, j;
    int temp1, temp2, temp3, temp4;
    int acc = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Create multiple basic blocks with label-oriented jumps */
    for (i = 0; i < size; i++) {
        /* Branch prediction hints to influence scheduling */
        if (__builtin_expect((arr1[i] & 1) != 0, 1)) {
            /* Pattern 1: Simple jump to label with eligible follower */
            goto label1;
            
        back_from_label1:
            /* Continue with different operations to create scheduling pressure */
            r1 = arr1[i] + arr2[i];
            r2 = r1 * 3;
            KEEP(r2);
            continue;
        }
        
        if (__builtin_expect((arr1[i] & 2) != 0, 0)) {
            /* Pattern 2: Another jump pattern */
            goto label2;
            
        back_from_label2:
            s1 = arr1[i] - arr2[i];
            s2 = s1 << 2;
            KEEP(s2);
            continue;
        }
        
        /* Default path with memory barrier to constrain scheduling */
        asm volatile("" ::: "memory");
        arr1[i] = arr2[i] * 2;
        continue;
        
        /* LABEL PATTERNS - These should be candidates for delay slot filling */
        
        /* Label 1: Simple non-trapping arithmetic */
        label1:
            /* This instruction should be eligible for delay slot:
               - Simple integer operation
               - No memory access (avoid trapping)
               - Uses different registers than jump context
               - Not a sequence or jump */
            temp1 = r3 + r4;  /* Simple add, cannot trap */
            r3 = temp1 ^ 0x55; /* Bitwise op, also safe */
            goto back_from_label1;
        
        /* Label 2: Another eligible pattern */
        label2:
            /* Different register set to avoid resource conflicts */
            temp2 = s3 | s4;  /* Bitwise OR, cannot trap */
            s3 = temp2 & 0xFF; /* Mask operation, safe */
            goto back_from_label2;
    }
    
    /* Nested loop to increase scheduling complexity */
    for (i = 0; i < size; i++) {
        for (j = 0; j < 4; j++) {
            /* More jump patterns in nested context */
            if (__builtin_expect((arr1[i] + j) > 100, 0)) {
                goto label3;
                
            back_from_label3:
                acc += arr1[i] * j;
                continue;
            }
            
            /* Alternate path with floating point to diversify resources */
            float ftemp = (float)arr2[i];
            KEEP(ftemp);
            continue;
            
            label3:
                /* Another delay slot candidate */
                temp3 = acc + 1;  /* Simple increment */
                acc = temp3;      /* Assignment */
                goto back_from_label3;
        }
    }
    
    /* Final computation to prevent dead code elimination */
    for (i = 0; i < size; i++) {
        if (__builtin_expect(arr1[i] != 0, 1)) {
            goto label4;
            
        back_from_label4:
            arr2[i] = arr1[i] >> 1;
            continue;
            
        label4:
            /* Final delay slot candidate pattern */
            temp4 = i * 2;
            KEEP(temp4);
            goto back_from_label4;
        }
    }
    
    KEEP(acc);
}

/* Secondary function with different patterns */
MIPS_TARGET
static void complex_branch_pattern(int *data, int n) {
    int a = 0, b = 0, c = 0, d = 0;
    int i;
    
    /* Initialize distinct register sets */
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4;
    int y1 = 5, y2 = 6, y3 = 7, y4 = 8;
    
    for (i = 0; i < n; i++) {
        /* Multiple conditional jumps to create scheduling pressure */
        switch (data[i] & 3) {
            case 0:
                if (__builtin_expect((i & 1) == 0, 1)) {
                    goto case0_label;
                }
                a = data[i] + x1;
                break;
                
            case0_label:
                /* Eligible instruction: simple arithmetic with local vars */
                x1 = x2 + x3;  /* Different registers than surrounding code */
                a = x1 * 2;
                break;
                
            case 1:
                if (__builtin_expect(data[i] > 50, 0)) {
                    goto case1_label;
                }
                b = data[i] - y1;
                break;
                
            case1_label:
                /* Another eligible pattern */
                y1 = y2 ^ y3;  /* Bitwise operation, safe */
                b = y1 | 0x0F;
                break;
                
            case 2:
                /* Force a jump chain */
                if (__builtin_expect((data[i] % 7) == 0, 0)) {
                    goto chain_label1;
                }
                c = data[i] * 3;
                break;
                
            chain_label1:
                /* First in chain */
                x2 = x4 - 1;
                goto chain_label2;
                
            chain_label2:
                /* Second in chain - eligible if reached from jump */
                y2 = y4 + 1;
                c = x2 + y2;
                break;
                
            default:
                d = data[i] / 2;  /* Division but by constant 2, safe */
                break;
        }
        
        /* Mix in memory barriers */
        if (i % 8 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    KEEP(a + b + c + d);
}

MIPS_TARGET
int main(void) {
    const int SIZE = 256;
    int *array1, *array2;
    int i;
    
    /* Allocate and initialize arrays */
    array1 = (int*)malloc(SIZE * sizeof(int));
    array2 = (int*)malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern that creates varied branch behavior */
    for (i = 0; i < SIZE; i++) {
        array1[i] = (i * 37) & 0xFF;  /* Pseudorandom pattern */
        array2[i] = (i * 73) & 0xFF;
    }
    
    /* Execute patterns designed to trigger delay slot filling */
    delay_slot_pattern(array1, array2, SIZE);
    complex_branch_pattern(array1, SIZE);
    
    /* Compute and print result to prevent optimization */
    int sum = 0;
    for (i = 0; i < SIZE; i++) {
        sum += array1[i] + array2[i];
    }
    
    printf("Result: %d\n", sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
