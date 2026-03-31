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
    
    /* Create tight loops with many branches */
    for (i = 0; i < size; i++) {
        /* First branch pattern - simple conditional jump to label */
        if (__builtin_expect((a[i] & 1) != 0, 0)) {
            /* Jump to label L1 */
            goto L1;
        } else {
            /* Some arithmetic that sets resources */
            r1 = a[i] + b[i];
            r2 = r1 * 2;
            /* Another jump opportunity */
            if (__builtin_expect(r2 > 100, 0)) {
                goto L2;
            }
            continue;
        }
        
    L1:
        /* Candidate instruction for delay slot filling */
        /* Simple, non-trapping integer operation */
        s1 = s1 + 1;  /* This should be eligible for delay slot */
        
        /* Continue with more operations */
        r3 = b[i] - a[i];
        acc += r3;
        continue;
        
    L2:
        /* Another candidate instruction */
        s2 = s2 ^ 0x55;  /* Bitwise operation - safe, non-trapping */
        
        r4 = a[i] * 3;
        acc -= r4;
        continue;
    }
    
    /* Nested loop with different pattern */
    for (i = 0; i < size; i += 2) {
        for (j = 0; j < 4; j++) {
            /* Complex conditional with multiple jumps */
            if (__builtin_expect((a[i] >> j) & 1, 0)) {
                /* Jump to label L3 */
                goto L3;
            }
            
            /* Some operations that might set resources */
            temp1 = a[i] + j;
            temp2 = b[i] - j;
            
            if (__builtin_expect(temp1 > temp2, 1)) {
                /* Jump to label L4 */
                goto L4;
            }
            
            /* Continue normal execution */
            temp3 = temp1 * temp2;
            acc += temp3;
            continue;
            
        L3:
            /* Another delay slot candidate */
            s3 = s3 | 0xFF;  /* Bitwise OR - safe operation */
            
            temp4 = a[i] ^ b[i];
            acc ^= temp4;
            continue;
            
        L4:
            /* Yet another candidate */
            s4 = s4 & ~0x0F;  /* Bitwise AND - safe operation */
            
            acc += a[i] * b[i];
            continue;
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (r1=%d, r2=%d, r3=%d, r4=%d)\n", 
           acc, r1, r2, r3, r4);
    printf("Side results: s1=%d, s2=%d, s3=%d, s4=%d\n",
           s1, s2, s3, s4);
}

/* Additional test function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void test_pattern2(int *arr, int n) {
    int i;
    int x = 0, y = 0, z = 0;
    
    /* Pattern with computed goto-like behavior */
    for (i = 0; i < n; i++) {
        switch (arr[i] % 4) {
            case 0:
                /* Jump to CASE0 label */
                goto CASE0;
            case 1:
                /* Jump to CASE1 label */
                goto CASE1;
            case 2:
                /* Jump to CASE2 label */
                goto CASE2;
            default:
                /* Some computation */
                x += arr[i];
                continue;
        }
        
    CASE0:
        /* Delay slot candidate - simple increment */
        y = y + 2;  /* Eligible for delay slot */
        
        x += arr[i] * 2;
        continue;
        
    CASE1:
        /* Another candidate - bit shift */
        z = z << 1;  /* Safe operation */
        
        x -= arr[i];
        continue;
        
    CASE2:
        /* Another candidate - subtraction */
        y = y - 1;  /* Safe operation */
        
        x ^= arr[i];
        continue;
    }
    
    /* Memory barrier to constrain scheduling */
    __sync_synchronize();
    
    printf("Pattern2: x=%d, y=%d, z=%d\n", x, y, z);
}

/* Main function that sets up test data */
int main() {
    const int SIZE = 100;
    int array1[SIZE];
    int array2[SIZE];
    int i;
    
    /* Initialize test data */
    for (i = 0; i < SIZE; i++) {
        array1[i] = i * 3 + 7;
        array2[i] = i * 5 - 2;
    }
    
    /* Run the delay slot test kernel */
    delay_slot_kernel(array1, array2, SIZE);
    
    /* Run second pattern test */
    test_pattern2(array1, SIZE);
    
    /* Additional test with volatile to prevent optimization */
    volatile int check = 0;
    for (i = 0; i < 10; i++) {
        /* Simple jump pattern in a small loop */
        if (__builtin_expect(array1[i] > 50, 0)) {
            goto SMALL_JUMP;
        }
        check += i;
        continue;
        
    SMALL_JUMP:
        /* Candidate for delay slot */
        check = check | 0x1;  /* Simple bitwise operation */
        
        check *= 2;
    }
    
    printf("Final check: %d\n", check);
    
    return 0;
}
