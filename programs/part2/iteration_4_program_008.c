/* reorg_coverage.c
 * Designed to trigger uncovered delay slot filling logic in GCC's reorg.cc
 * Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -S reorg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS architecture if supported */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
static void delay_slot_patterns(int *arr1, int *arr2, int n) {
    int i, j;
    int a = 0, b = 0, c = 0, d = 0;
    int x = 1, y = 2, z = 3;
    volatile int barrier = 0; /* Use for memory barriers */
    
    /* Pattern 1: Simple jump to label with eligible follower */
    for (i = 0; i < n; i++) {
        /* Create register pressure */
        int t1 = arr1[i];
        int t2 = arr2[i];
        
        /* Conditional that often branches */
        if (__builtin_expect(t1 > t2, 1)) {
            /* Jump pattern that should become simplejump_p */
            if (t1 > 100) {
                goto label1;
            }
            a += t1;
        } else {
            b += t2;
        }
        
        /* Continue normal flow */
        c = a + b;
        continue;
        
    label1:
        /* This instruction must be eligible for delay slot:
           - Simple integer operation
           - No trapping
           - Uses different registers than delay slot resources
           - Not a SEQUENCE */
        d = x + y;  /* Simple add, no trap possible */
        a += d;
        
        /* Memory barrier to constrain scheduling */
        barrier = 1;
    }
    
    /* Pattern 2: Nested loops with multiple jump-label patterns */
    for (i = 0; i < n; i++) {
        for (j = 0; j < 4; j++) {
            int val = arr1[i] + j;
            
            /* Multiple conditional jumps to different labels */
            if (val & 1) {
                if (__builtin_expect((val % 3) == 0, 0)) {
                    goto label2;
                }
                x = x ^ y;
            } else {
                if (__builtin_expect(val > 50, 1)) {
                    goto label3;
                }
                y = y | z;
            }
            
            /* Default path */
            z = z << 1;
            continue;
            
        label2:
            /* Candidate for delay slot filling */
            a = b + c;  /* Uses different vars than surrounding code */
            x = x + 1;
            continue;
            
        label3:
            /* Another candidate */
            c = d ^ 0xFF;  /* Bitwise op, no trap */
            y = y - 1;
        }
        
        /* Mix in floating point to diversify resource usage */
        float ftemp = (float)arr2[i];
        if (ftemp > 0.0f) {
            /* Another jump pattern */
            if (i & 1) {
                goto label4;
            }
        }
        barrier = ftemp;  /* Use volatile store */
        continue;
        
    label4:
        /* Safe arithmetic for delay slot candidate */
        d = a + 1;  /* Simple increment */
        b = b * 2;
    }
    
    /* Pattern 3: Switch-like structure with computed goto simulation */
    int state = 0;
    for (i = 0; i < n; i++) {
        state = (state + arr1[i]) & 3;
        
        switch (state) {
            case 0:
                if (__builtin_expect(arr2[i] > 0, 1)) {
                    goto case0_label;
                }
                break;
            case 1:
                if (__builtin_expect(arr2[i] < 0, 0)) {
                    goto case1_label;
                }
                break;
            default:
                break;
        }
        
        /* Fall through */
        a = a + arr1[i];
        continue;
        
    case0_label:
        /* Delay slot candidate - uses fresh variables */
        {
            int temp1 = z;
            int temp2 = x;
            temp1 = temp1 + temp2;  /* Simple operation */
            z = temp1;
        }
        a = a + 1;
        continue;
        
    case1_label:
        /* Another candidate */
        y = y & 0x0F;  /* Masking operation, no trap */
        b = b + 2;
    }
}

/* Pattern 4: Complex control flow with multiple basic blocks */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
static void complex_control_flow(int *arr, int n) {
    int i, sum1 = 0, sum2 = 0, sum3 = 0;
    int r1 = 5, r2 = 10, r3 = 15;  /* Distinct register-like variables */
    
    for (i = 0; i < n; i++) {
        int val = arr[i];
        
        /* Chain of conditions creating multiple basic blocks */
        if (val > 100) {
            if (__builtin_expect((val % 7) == 0, 0)) {
                goto big_multiple_of_7;
            }
            sum1 += val;
            
            /* Inline asm memory barrier to affect scheduling */
            __asm__ volatile ("" ::: "memory");
        } else if (val > 50) {
            if (__builtin_expect((val & 3) == 0, 1)) {
                goto medium_multiple_of_4;
            }
            sum2 += val;
        } else {
            if (__builtin_expect(val < 0, 0)) {
                goto negative_val;
            }
            sum3 += val;
        }
        
        /* Common continuation */
        r1 = r1 ^ val;
        continue;
        
    big_multiple_of_7:
        /* Ideal delay slot candidate:
           - Uses r2, r3 which aren't used in delay slot resources
           - Simple non-trapping operation */
        r2 = r3 + 1;  /* Simple increment via different register */
        sum1 += 100;
        continue;
        
    medium_multiple_of_4:
        /* Another good candidate */
        r3 = r1 & 0xFF;  /* Masking operation */
        sum2 += 50;
        continue;
        
    negative_val:
        /* Safe arithmetic */
        r1 = r2 - r3;  /* Subtraction, no overflow trap expected */
        sum3 += val;
    }
    
    /* Use results to prevent elimination */
    arr[0] = sum1 + sum2 + sum3 + r1 + r2 + r3;
}

/* Main computational kernel */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
int main() {
    const int SIZE = 1000;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        return 1;
    }
    
    /* Initialize with pattern that creates varied branch behavior */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 37) % 123;
        array2[i] = (i * 73) % 456;
    }
    
    /* Execute patterns multiple times to increase scheduling pressure */
    for (int iter = 0; iter < 10; iter++) {
        delay_slot_patterns(array1, array2, SIZE);
        complex_control_flow(array1, SIZE);
        
        /* Modify arrays slightly to change branch patterns */
        for (int i = 0; i < SIZE; i++) {
            array1[i] = (array1[i] + iter) % 256;
        }
    }
    
    /* Compute final result to ensure no dead code elimination */
    int final_result = 0;
    for (int i = 0; i < SIZE; i++) {
        final_result ^= array1[i];
        final_result += array2[i];
    }
    
    printf("Result: %d\n", final_result);
    
    free(array1);
    free(array2);
    
    return 0;
}
