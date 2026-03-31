/* Target: MIPS architecture with delay slots */
/* Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -S -o reorg_test.s reorg_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_kernel(int *arr1, int *arr2, int size) {
    int i, j;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int temp1, temp2, temp3;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Main computational loop with dense branching */
    for (i = 0; i < size; i++) {
        /* Create multiple basic blocks with goto patterns */
        if (__builtin_expect(arr1[i] > 100, 0)) {
            /* Jump pattern 1: simple jump to label with eligible follower */
            goto label_alpha;
            
            /* This should never execute due to goto */
            r1 = r1 + 1;
            
        label_alpha:
            /* ELIGIBLE CANDIDATE: Simple non-trapping arithmetic */
            /* Uses distinct registers from delay slot resources */
            s1 = s2 + s3;  /* Simple add, no trap possible */
            acc1 += s1;
            
            /* Additional operation to create scheduling pressure */
            temp1 = arr1[i] * 2;
            continue;
        }
        
        if (__builtin_expect(arr1[i] < -100, 0)) {
            /* Jump pattern 2: another simple jump */
            goto label_beta;
            
            r2 = r2 + 1;
            
        label_beta:
            /* Another eligible candidate */
            s2 = s3 ^ s4;  /* Bitwise op, non-trapping */
            acc2 += s2;
            
            temp2 = arr1[i] / 3;  /* Division by constant, safe */
            continue;
        }
        
        /* Nested loop to increase control flow complexity */
        for (j = 0; j < 3; j++) {
            if (__builtin_expect((i + j) % 5 == 0, 1)) {
                /* Jump pattern 3: inside nested loop */
                goto label_gamma;
                
                r3 = r3 + 1;
                
            label_gamma:
                /* Eligible candidate with different operation */
                s3 = s4 | 0xFF;  /* Bitwise OR with constant */
                acc3 += s3;
                
                /* Memory barrier to constrain scheduling */
                asm volatile("" ::: "memory");
                
                /* Conditional with another goto */
                if (__builtin_expect(arr2[j] > 50, 0)) {
                    goto label_delta;
                    
                    r4 = r4 + 1;
                    
                label_delta:
                    /* Another simple arithmetic candidate */
                    s4 = s1 & 0x0F;  /* Bitwise AND */
                    acc1 += s4;
                }
            }
        }
        
        /* Mix in floating point to diversify resource usage */
        float f1 = (float)arr1[i];
        float f2 = f1 * 1.5f;
        
        /* Another goto pattern with arithmetic follower */
        if (__builtin_expect(f2 > 200.0f, 0)) {
            goto label_epsilon;
            
            /* Unreachable code */
            temp3 = 0;
            
        label_epsilon:
            /* Simple arithmetic that doesn't conflict */
            int t = r1 + r2;  /* Uses different register set */
            acc2 += t;
        }
    }
    
    /* Use results to prevent dead code elimination */
    arr1[0] = acc1 + acc2 + acc3;
}

/* Alternative function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void branch_dense_pattern(int *data, int n) {
    int i;
    int a = 0, b = 0, c = 0, d = 0;
    int x = 1, y = 2, z = 3, w = 4;
    
    /* Create many basic blocks with goto-label patterns */
    for (i = 0; i < n; i++) {
        switch (data[i] & 0x7) {
            case 0:
                goto case0_label;
                /* Never reached */
                a = a + 100;
                
            case0_label:
                /* Simple arithmetic after label */
                b = c + d;  /* Non-trapping, uses distinct vars */
                break;
                
            case 1:
                goto case1_label;
                a = a + 200;
                
            case1_label:
                c = d ^ x;  /* Bitwise XOR */
                break;
                
            case 2:
                if (__builtin_expect(data[i] > 1000, 0)) {
                    goto case2_label;
                    /* Unreachable */
                    y = y * 2;
                    
                case2_label:
                    d = x + y;  /* Simple addition */
                }
                break;
                
            case 3:
                /* No goto here - creates variation */
                a = b + c;
                break;
                
            case 4:
                goto case4_label;
                z = z - 1;
                
            case4_label:
                w = a << 2;  /* Shift operation */
                break;
                
            default:
                /* Complex enough to not be a SEQUENCE */
                a = (b + c) * (d - x);
        }
        
        /* Create data dependencies */
        data[i] = a + b + c + d;
        
        /* Occasionally insert memory barrier */
        if ((i & 0xF) == 0) {
            __sync_synchronize();
        }
    }
}

int main() {
    const int SIZE = 1000;
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pattern that creates varied branch behavior */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 37) % 1000 - 500;  /* Range: -500 to 499 */
        array2[i] = (i * 73) % 100;
    }
    
    /* Execute kernels designed to trigger delay slot filling */
    delay_slot_kernel(array1, array2, SIZE);
    branch_dense_pattern(array1, SIZE);
    
    /* Compute checksum to ensure computation happens */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= array1[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
