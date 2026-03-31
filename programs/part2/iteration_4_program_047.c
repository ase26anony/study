/* Target: MIPS architecture with delay slots */
/* Compile with: -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all */
/* This program creates patterns to trigger delay slot filling logic in reorg.cc */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if supported */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void compute_hash(int *input, int *output, int size) {
    int temp1 = 0, temp2 = 0, temp3 = 0;
    int a = 0, b = 0, c = 0, d = 0;
    int x = 0, y = 0, z = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4;
    int s1 = 5, s2 = 6, s3 = 7, s4 = 8;
    
    for (int i = 0; i < size; i++) {
        /* Create multiple basic blocks with jumps to labels */
        if (__builtin_expect((input[i] & 1) != 0, 0)) {
            /* Jump pattern 1: simple goto to label with eligible follower */
            goto label1;
            
            /* This should not be executed */
            temp1 = input[i] * 2;
        }
        
        /* Alternate path */
        a = b + c;  /* Simple non-trapping arithmetic */
        continue;
        
    label1:
        /* ELIGIBLE CANDIDATE for delay slot filling */
        /* Simple, non-trapping, splittable operation */
        /* Uses distinct registers from jump context */
        r1 = r2 + r3;  /* This is next_trial in the uncovered code */
        
        /* Follow with more operations to create scheduling context */
        d = a ^ b;
        output[i] = input[i] + r1;
        
        /* Another jump pattern */
        if (__builtin_expect((input[i] & 2) != 0, 0)) {
            goto label2;
        }
        
        x = y | z;
        continue;
        
    label2:
        /* Another eligible candidate */
        s1 = s2 - s3;  /* Simple subtraction, non-trapping */
        
        /* Mix in memory barriers to constrain scheduling */
        asm volatile("" ::: "memory");
        
        output[i] = output[i] * s1;
    }
}

/* Second function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void matrix_manipulate(int *mat1, int *mat2, int n) {
    int i, j;
    int t1 = 0, t2 = 0, t3 = 0;
    int u1 = 10, u2 = 20, u3 = 30;
    int v1 = 40, v2 = 50, v3 = 60;
    
    /* Nested loops create complex control flow */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            int idx = i * n + j;
            
            /* Create jump-to-label pattern in loop body */
            if (__builtin_expect(mat1[idx] > mat2[idx], 0)) {
                goto process_label;
            }
            
            /* Normal path */
            t1 = t2 * t3;
            mat1[idx] = mat2[idx] + t1;
            continue;
            
        process_label:
            /* Candidate instruction for delay slot */
            /* Uses completely different register set */
            u1 = u2 ^ u3;  /* Bitwise operation, non-trapping */
            
            /* Follow with arithmetic using different registers */
            v1 = v2 + v3;
            mat1[idx] = (mat1[idx] * u1) / (v1 + 1);  /* +1 prevents division by zero */
            
            /* Another conditional jump */
            if (__builtin_expect((mat1[idx] & 0xFF) == 0, 1)) {
                goto adjust_label;
            }
            
            t3 = t1 << 2;
            continue;
            
        adjust_label:
            /* Another candidate */
            v2 = v3 & 0x7F;  /* Simple bitwise AND */
            
            /* Memory barrier to separate scheduling regions */
            __sync_synchronize();
            
            mat2[idx] = mat1[idx] - v2;
        }
        
        /* Outer loop has its own jump patterns */
        if (__builtin_expect(i % 3 == 0, 0)) {
            goto outer_label;
        }
        
        t2 = t1 | 0x01;
        continue;
        
    outer_label:
        /* Outer loop candidate */
        u3 = u1 + u2;  /* Simple addition */
        
        /* Force register pressure */
        asm volatile("" ::: 
            "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9",
            "$10", "$11", "$12", "$13", "$14", "$15", "$16");
    }
}

/* Third function with pure integer operations */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void integer_workload(int *data, int iterations) {
    /* Use many local variables to create register pressure */
    int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    int b1 = 6, b2 = 7, b3 = 8, b4 = 9, b5 = 10;
    int c1 = 11, c2 = 12, c3 = 13, c4 = 14, c5 = 15;
    
    int result = 0;
    
    while (iterations-- > 0) {
        /* Create a chain of jump-label patterns */
        switch (data[iterations] % 5) {
            case 0:
                goto case0_label;
            case 1:
                goto case1_label;
            case 2:
                goto case2_label;
            case 3:
                goto case3_label;
            default:
                goto default_label;
        }
        
        /* These should be unreachable but create scheduling contexts */
        a1 = a2 * a3;
        continue;
        
    case0_label:
        /* Eligible candidate - simple operation with distinct registers */
        b1 = b2 + b3;  /* next_trial candidate */
        result += b1;
        continue;
        
    case1_label:
        c1 = c2 - c3;  /* Another candidate */
        result ^= c1;
        
        /* Insert a jump back to create loop */
        if (__builtin_expect((result & 1) == 0, 0)) {
            goto skip_label;
        }
        continue;
        
    case2_label:
        a4 = a5 << 1;  /* Shift operation */
        result |= a4;
        continue;
        
    case3_label:
        b4 = b5 >> 1;  /* Another shift */
        result &= b4;
        continue;
        
    default_label:
        c4 = c5 + 1;  /* Simple increment */
        result *= c4;
        continue;
        
    skip_label:
        /* Final candidate in this chain */
        a5 = b5 ^ c5;  /* XOR operation */
        result += a5;
    }
    
    /* Prevent dead code elimination */
    data[0] = result;
}

int main() {
    const int SIZE = 100;
    int *input = malloc(SIZE * sizeof(int));
    int *output = malloc(SIZE * sizeof(int));
    int *matrix1 = malloc(SIZE * SIZE * sizeof(int));
    int *matrix2 = malloc(SIZE * SIZE * sizeof(int));
    
    /* Initialize with pattern to trigger various branches */
    for (int i = 0; i < SIZE; i++) {
        input[i] = i * 1103515245 + 12345;  /* Simple LCG */
        output[i] = 0;
    }
    
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix1[i] = i;
        matrix2[i] = SIZE * SIZE - i;
    }
    
    /* Call functions with jump-label patterns */
    compute_hash(input, output, SIZE);
    matrix_manipulate(matrix1, matrix2, 10);
    
    /* Another workload */
    integer_workload(input, SIZE);
    
    /* Print results to prevent optimization */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += output[i];
    }
    printf("Result: %d\n", sum);
    
    free(input);
    free(output);
    free(matrix1);
    free(matrix2);
    
    return 0;
}
