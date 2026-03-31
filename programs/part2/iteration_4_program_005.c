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
    int temp1, temp2, temp3;
    volatile int barrier = 0; /* Use for memory barriers */
    
    /* Initialize distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0, s5 = 0;
    
    /* Main computational loop with dense branching */
    for (i = 0; i < size; i++) {
        /* Create multiple basic blocks with simple jumps to labels */
        if (__builtin_expect((a[i] & 1) == 0, 1)) {
            /* Jump pattern 1: Simple goto to label with eligible follower */
            goto label1;
        } else {
            /* Alternate path with different operations */
            r1 = a[i] + b[i];
            goto label2;
        }
        
    label1:
        /* Candidate for delay slot filling: Simple non-trapping arithmetic */
        /* Uses distinct register set (s1-s3) to avoid resource conflicts */
        s1 = s2 + s3;  /* Simple add - no trap possible */
        /* Continue with other operations */
        r1 = a[i] * 2;
        goto continue_loop;
        
    label2:
        /* Another candidate location */
        s4 = s5 ^ 0xFF;  /* Bitwise operation - no trap */
        r2 = b[i] - a[i];
        /* No goto here - falls through */
        
    continue_loop:
        /* Complex enough to prevent instruction merging */
        temp1 = r1 * r2;
        temp2 = temp1 >> 3;
        
        /* Memory barrier to constrain scheduling */
        barrier = temp2;
        asm volatile("" ::: "memory");
        
        /* Nested loop to increase scheduling complexity */
        for (j = 0; j < 4; j++) {
            /* More label-oriented jumps */
            if (__builtin_expect((temp2 & (1 << j)) != 0, 0)) {
                goto nested_label;
            }
            
            /* Default path */
            temp3 = temp1 + j;
            goto nested_continue;
            
        nested_label:
            /* Another delay slot candidate */
            s1 = s2 - s3;  /* Simple subtract */
            temp3 = temp1 - j;
            
        nested_continue:
            /* Mix operations to diversify resource usage */
            a[i] += temp3;
            b[i] ^= temp3;
            
            /* Floating point in alternate path to stress scheduler */
            if (j % 2 == 0) {
                float ftemp = (float)temp3;
                /* Cast back to avoid elimination */
                barrier = (int)ftemp;
            }
        }
        
        /* Another unconditional jump pattern */
        if (__builtin_expect(i % 3 == 0, 0)) {
            goto jump_to_arith;
        }
        
        /* Fall through path */
        r3 = r4 | r5;
        goto loop_end;
        
    jump_to_arith:
        /* Target label with simple arithmetic follower */
        /* This should be eligible for delay slot filling */
        s2 = s3 * 2;  /* Multiplication by constant - safe */
        r3 = r4 & r5;
        
    loop_end:
        /* Final accumulation */
        a[i] = (a[i] + r3) & 0xFFFF;
    }
}

/* Secondary function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void branch_dense_pattern(int *arr, int n) {
    int i, acc = 0;
    
    /* Unrolled loop to create many basic blocks */
    for (i = 0; i < n; i += 2) {
        int x = arr[i];
        int y = arr[i + 1];
        
        /* Pattern of jumps to labels with simple followers */
        if (x > y) {
            goto compute_diff;
        }
        
        /* Default path */
        acc += x + y;
        goto next_pair;
        
    compute_diff:
        /* Candidate instruction: simple subtraction */
        /* Uses local variables only, no memory ops */
        int diff = x - y;  /* Safe, non-trapping */
        acc += diff;
        
    next_pair:
        /* Another branching opportunity */
        if ((x ^ y) & 1) {
            goto bit_op;
        }
        
        acc ^= 0xAA;
        goto pair_done;
        
    bit_op:
        /* Another simple operation candidate */
        int bits = x & 0xF;  /* Simple mask operation */
        acc |= bits;
        
    pair_done:
        /* Prevent dead code elimination */
        arr[i] = acc;
    }
}

/* Hash computation with many branches */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
int branching_hash(const char *str, int len) {
    int hash = 5381;
    int i;
    
    for (i = 0; i < len; i++) {
        char c = str[i];
        
        /* Multiple branch targets */
        if (c < 'A') {
            goto handle_special;
        }
        
        /* Normal path */
        hash = ((hash << 5) + hash) + c;
        goto next_char;
        
    handle_special:
        /* Simple arithmetic after label */
        int code = (int)c;  /* Just a cast - very simple */
        hash = ((hash << 3) + hash) + code;
        
    next_char:
        /* Occasionally jump to another label */
        if ((i & 3) == 0) {
            goto adjust_hash;
        }
        
        /* Fall through */
        hash ^= i;
        goto loop_continue;
        
    adjust_hash:
        /* Another candidate for delay slot */
        hash = hash + 1;  /* Simple increment */
        
    loop_continue:
        /* Memory barrier occasionally */
        if ((i & 7) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    return hash;
}

int main() {
    const int SIZE = 256;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    char *test_str = "Test string for hash computation with branches";
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 5 + 1;
    }
    
    /* Execute kernels to trigger reorg logic */
    delay_slot_kernel(array1, array2, SIZE);
    branch_dense_pattern(array1, SIZE / 2);
    
    int hash = branching_hash(test_str, 45);
    
    /* Use results to prevent optimization */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += array1[i] + array2[i];
    }
    
    printf("Result: sum = %d, hash = %d\n", sum, hash);
    
    free(array1);
    free(array2);
    
    return 0;
}
