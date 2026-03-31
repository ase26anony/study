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
void delay_slot_pattern(int *arr1, int *arr2, int size) {
    int i, j;
    int temp1 = 0, temp2 = 0, temp3 = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int s1 = 10, s2 = 20, s3 = 30, s4 = 40, s5 = 50;
    
    /* Create multiple basic blocks with label-oriented jumps */
    for (i = 0; i < size; i++) {
        /* First pattern: simple jump to label with eligible follower */
        if (__builtin_expect(arr1[i] > 100, 0)) {
            /* Jump to label_target1 - this should be simplejump_p */
            goto label_target1;
        }
        
        /* Some arithmetic to create scheduling pressure */
        temp1 = arr1[i] * r1 + r2;
        temp2 = arr2[i] * r3 - r4;
        
        /* Another conditional jump pattern */
        if (__builtin_expect(temp1 < temp2, 1)) {
            goto label_target2;
        }
        
        /* Continue normal execution */
        acc1 += temp1;
        acc2 += temp2;
        
        /* Skip to avoid executing label code out of order */
        goto skip_label1;
        
    label_target1:
        /* Candidate instruction for delay slot: simple, non-trapping arithmetic */
        /* Uses distinct registers (s1-s5) to avoid resource conflicts with &set */
        s1 = s2 + s3;  /* Simple add - should pass try_split and eligible_for_delay */
        /* Continue with more operations */
        acc3 += s1;
        arr1[i] = s1;
        goto continue_main;
        
    label_target2:
        /* Another candidate: bitwise operation */
        s4 = s5 & 0xFF;  /* Non-trapping, simple operation */
        acc1 ^= s4;
        arr2[i] = s4;
        goto continue_main;
        
    skip_label1:
        /* Alternative path with memory barrier to constrain scheduling */
        asm volatile("" ::: "memory");
        r1 = (r1 * 3) % 7;  /* Change register values */
        
    continue_main:
        /* Mix integer and floating point in different paths */
        if (i % 3 == 0) {
            float ftemp = (float)acc1 * 0.5f;
            acc2 += (int)ftemp;
        }
        
        /* Nested loop to create complex control flow */
        for (j = 0; j < 3; j++) {
            if (__builtin_expect((i + j) % 5 == 0, 0)) {
                /* Another jump to label pattern */
                goto label_target3;
            }
            temp3 = arr1[i] + arr2[j % size];
            acc3 += temp3;
            
            if (j == 1) {
                /* Force a jump back */
                goto end_inner;
            }
            
        label_target3:
            /* Candidate: subtraction with distinct registers */
            r5 = r3 - r2;  /* Simple, non-trapping */
            acc1 += r5;
            continue;
            
        end_inner:
            /* Small computation */
            r4 = r4 ^ 0x55;
        }
    }
    
    /* Final computation to prevent dead code elimination */
    arr1[0] = acc1 + acc2 + acc3;
    arr2[0] = temp1 + temp2 + temp3;
}

/* Secondary function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void branch_dense_loop(int *data, int n) {
    int i, a = 0, b = 0, c = 0, d = 0;
    int x = 1, y = 2, z = 3, w = 4;  /* Distinct register set */
    
    for (i = 0; i < n; i++) {
        /* Multiple conditional jumps in tight loop */
        if (__builtin_expect(data[i] & 0x01, 0)) {
            goto lbl_a;
        }
        if (__builtin_expect(data[i] & 0x02, 1)) {
            goto lbl_b;
        }
        if (__builtin_expect(data[i] & 0x04, 0)) {
            goto lbl_c;
        }
        
        /* Default path */
        a += data[i];
        goto loop_end;
        
    lbl_a:
        /* Candidate: simple shift */
        x = y << 1;  /* Eligible for delay slot */
        a += x;
        goto loop_end;
        
    lbl_b:
        /* Candidate: addition */
        z = w + 1;  /* Simple, non-trapping */
        b += z;
        goto loop_end;
        
    lbl_c:
        /* Candidate: bitwise OR */
        y = x | 0x0F;  /* Non-trapping operation */
        c += y;
        /* Fall through */
        
    loop_end:
        /* Vary register usage */
        d ^= data[i];
        
        /* Memory barrier occasionally */
        if (i % 7 == 0) {
            __sync_synchronize();
        }
    }
    
    data[0] = a + b + c + d;
}

int main() {
    const int SIZE = 100;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    /* Initialize with pattern that creates branch variance */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 37) % 101;
        array2[i] = (i * 53) % 103;
    }
    
    /* Execute functions with delay slot patterns */
    delay_slot_pattern(array1, array2, SIZE);
    
    int *data = malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        data[i] = i;
    }
    
    branch_dense_loop(data, SIZE);
    
    /* Print results to prevent optimization */
    printf("Result1: %d, Result2: %d, Result3: %d\n", 
           array1[0], array2[0], data[0]);
    
    free(array1);
    free(array2);
    free(data);
    
    return 0;
}
