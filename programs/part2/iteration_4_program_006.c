/* 
 * Program to trigger uncovered delay slot filling logic in GCC's reorg.cc
 * Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -o reorg_test reorg_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively on MIPS */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_kernel(int *result, const int *data, int size) {
    int temp1 = 0, temp2 = 0, temp3 = 0;
    int a = 1, b = 2, c = 3, d = 4;
    volatile int barrier = 0; /* Prevent optimization */
    
    /* Multiple distinct variables for resource separation */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    for (int i = 0; i < size; i++) {
        /* Create pressure for delay slot filling */
        if (__builtin_expect(data[i] > 100, 0)) {
            /* Pattern 1: Jump to label with simple arithmetic follower */
            if (a > b) {
                /* Use goto to create simplejump to label */
                goto label_alpha;
            }
            
            /* Some arithmetic to use registers */
            r1 = a + b;
            r2 = c - d;
            
            /* Memory barrier to constrain scheduling */
            barrier = r1;
            
            continue;
            
        label_alpha:
            /* Candidate for delay slot: simple, non-trapping arithmetic */
            /* Uses different registers than the jump's context */
            s1 = s2 + s3;  /* Simple add, no trap possible */
            /* This instruction should be eligible for moving into delay slot */
            
            /* More operations to prevent merging */
            r3 = r4 ^ 0xFF;
            result[i] = s1 + r3;
            
            /* Another jump pattern */
            if (__builtin_expect(temp1 < temp2, 1)) {
                goto label_beta;
            }
            
            temp1 = data[i] * 2;
            continue;
            
        label_beta:
            /* Another candidate instruction - different resources */
            s2 = s3 & s4;  /* Bitwise AND, safe */
            result[i] = s2 + temp1;
        }
        else if (__builtin_expect(data[i] < 0, 0)) {
            /* Pattern 2: Nested jumps with different register sets */
            if (temp3 == 0) {
                goto label_gamma;
            }
            
            /* Use completely different registers */
            int x = 5, y = 6, z = 7;
            x = y * z;  /* Multiplication is safe with small constants */
            
            continue;
            
        label_gamma:
            /* Candidate: simple assignment with constant */
            s3 = 42;  /* Very simple, no dependencies */
            result[i] = s3 + i;
            
            /* Create another jump opportunity */
            if (r1 != r2) {
                goto label_delta;
            }
            
            barrier = x;
            continue;
            
        label_delta:
            /* Another simple operation */
            s4 = ~s1;  /* Bitwise complement */
            result[i] = s4 ^ 0x55;
        }
        else {
            /* Default case with mixed operations */
            result[i] = data[i] + a;
            
            /* Occasional jump to create more patterns */
            if ((i & 3) == 0) {
                if (c > a) {
                    goto label_epsilon;
                }
            }
            
            /* Floating point in different path to diversify resources */
            float f1 = (float)data[i];
            float f2 = f1 * 1.5f;
            barrier = (int)f2;
            
            continue;
            
        label_epsilon:
            /* Simple arithmetic with local variables */
            a = b + 1;  /* Different variable than jump context uses */
            result[i] = a * 2;
        }
        
        /* Loop-carried dependencies to prevent over-optimization */
        temp1 = temp2;
        temp2 = temp3;
        temp3 = result[i] & 0xFF;
        
        /* Memory barrier to separate iterations */
        __asm__ volatile ("" ::: "memory");
    }
    
    /* Final computation using all temporaries to prevent dead code elimination */
    result[0] = temp1 + temp2 + temp3 + r1 + r2 + s1 + s2;
}

/* Secondary function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void secondary_pattern(int *arr, int n) {
    int x = 0, y = 0, z = 0;
    
    for (int i = 0; i < n; i++) {
        /* Switch-like pattern with goto labels */
        switch (arr[i] % 4) {
            case 0:
                if (x > y) goto case0_label;
                x = arr[i];
                break;
                
        case0_label:
                y = z + 1;  /* Simple increment - delay slot candidate */
                arr[i] = y;
                break;
                
            case 1:
                if (z != 0) goto case1_label;
                z = arr[i] >> 2;
                break;
                
        case1_label:
                x = y | 0x01;  /* Bitwise OR with constant */
                arr[i] = x;
                break;
                
            case 2:
                /* Direct goto without condition check */
                goto case2_label;
                
                /* Unreachable code - creates interesting control flow */
                x = x * x;
                
        case2_label:
                z = x - y;  /* Simple subtraction */
                arr[i] = z;
                break;
                
            default:
                /* Small loop inside to create more basic blocks */
                for (int j = 0; j < 2; j++) {
                    if (j == 1) goto inner_label;
                    y = arr[i] + j;
                }
                
        inner_label:
                x = y & 0xF0;  /* Bitwise AND with constant */
                arr[i] = x;
                break;
        }
        
        /* Cross-iteration dependency */
        arr[i] += arr[(i + 1) % n] % 256;
    }
}

int main() {
    const int SIZE = 256;
    int *data = (int*)malloc(SIZE * sizeof(int));
    int *result = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pattern that triggers various branches */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 73) % 199 - 50;  /* Mix of positive and negative */
    }
    
    /* Call the kernel function multiple times */
    for (int iter = 0; iter < 10; iter++) {
        delay_slot_kernel(result, data, SIZE);
        secondary_pattern(result, SIZE);
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            data[i] = (data[i] + result[i]) % 1000;
        }
    }
    
    /* Compute checksum to ensure computation isn't optimized away */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= result[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Result checksum: %d\n", checksum);
    
    free(data);
    free(result);
    
    return 0;
}
