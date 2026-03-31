/* Target: MIPS architecture with delay slots */
#ifndef __mips__
#error "This code is designed for MIPS architecture with delay slots. Compile with -march=mips"
#endif

#include <stdio.h>
#include <stdlib.h>

/* Force delay slot usage */
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

/* Avoid optimization removing our labels */
#define KEEP_LABEL(label) asm volatile ("#" #label ":" ::: "memory")

/* Resource separation - use distinct variable sets */
static int set1_var1, set1_var2, set1_var3;
static int set2_var1, set2_var2, set2_var3;
static int set3_var1, set3_var2, set3_var3;

/* Non-trapping arithmetic operations */
static inline int safe_add(int a, int b) {
    return a + b;  /* Never traps */
}

static inline int safe_sub(int a, int b) {
    return a - b;  /* Never traps */
}

static inline int safe_and(int a, int b) {
    return a & b;  /* Never traps */
}

/* Main computational kernel with label-oriented jumps */
void compute_kernel(int *input, int *output, int size) {
    int i, j;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    /* Initialize resource-separated variables */
    set1_var1 = 1; set1_var2 = 2; set1_var3 = 3;
    set2_var1 = 4; set2_var2 = 5; set2_var3 = 6;
    set3_var1 = 7; set3_var2 = 8; set3_var3 = 9;
    
    /* Complex loop structure to create scheduling pressure */
    for (i = 0; i < size; i++) {
        int val = input[i];
        
        /* Pattern 1: Jump to label with simple arithmetic follower */
        if (LIKELY(val > 0)) {
            goto label_arithmetic_1;
        } else {
            goto label_arithmetic_2;
        }
        
    label_arithmetic_1:
        KEEP_LABEL(label_arithmetic_1);
        /* Simple, non-trapping, splittable operation */
        /* Uses set1 variables - separate resource set */
        set1_var1 = safe_add(set1_var2, set1_var3);
        acc1 += set1_var1;
        
        /* Continue normal flow */
        if (UNLIKELY(val % 2 == 0)) {
            goto label_bitwise_1;
        }
        /* Fall through */
        
    label_arithmetic_2:
        KEEP_LABEL(label_arithmetic_2);
        /* Another simple operation with different resource set */
        set2_var1 = safe_sub(set2_var3, set2_var2);
        acc2 += set2_var1;
        
        if (LIKELY(val < size/2)) {
            goto label_arithmetic_3;
        }
        /* Fall through */
        
    label_bitwise_1:
        KEEP_LABEL(label_bitwise_1);
        /* Bitwise operation - never traps */
        set3_var1 = safe_and(set3_var2, set3_var3);
        acc3 += set3_var1;
        
        /* Nested loop to increase branch density */
        for (j = 0; j < 3; j++) {
            int temp = val + j;
            
            /* More label jumps in tight loop */
            if (LIKELY(temp > 0)) {
                goto label_nested_1;
            } else {
                goto label_nested_2;
            }
            
        label_nested_1:
            KEEP_LABEL(label_nested_1);
            /* Simple operation - delay slot candidate */
            set1_var2 = safe_add(set1_var1, 1);
            acc1 += set1_var2;
            
            if (UNLIKELY(j == 1)) {
                goto label_nested_3;
            }
            continue;
            
        label_nested_2:
            KEEP_LABEL(label_nested_2);
            set2_var2 = safe_sub(set2_var1, 1);
            acc2 += set2_var2;
            continue;
            
        label_nested_3:
            KEEP_LABEL(label_nested_3);
            set3_var2 = safe_and(set3_var1, 0xFF);
            acc3 += set3_var2;
            continue;
        }
        
    label_arithmetic_3:
        KEEP_LABEL(label_arithmetic_3);
        /* Final simple operation in this path */
        set1_var3 = safe_add(set1_var1, set1_var2);
        acc1 += set1_var3;
    }
    
    /* Store results to prevent dead code elimination */
    output[0] = acc1;
    output[1] = acc2;
    output[2] = acc3;
    
    /* Memory barrier to constrain scheduling */
    __sync_synchronize();
}

/* Alternative path with different patterns */
void alternate_kernel(int *input, int *output, int size) {
    int i;
    int res1 = 0, res2 = 0;
    
    /* Different variable sets to avoid resource conflicts */
    int local_a = 1, local_b = 2, local_c = 3;
    int local_x = 4, local_y = 5, local_z = 6;
    
    for (i = 0; i < size; i++) {
        int val = input[i] ^ 0x55AA;  /* Simple transform */
        
        /* Jump pattern with immediate label follower */
        if (LIKELY((val & 1) == 0)) {
            goto alt_label_1;
        }
        
        /* Some intermediate computation */
        local_a = local_b + local_c;
        res1 += local_a;
        
        if (UNLIKELY(val > 1000)) {
            goto alt_label_2;
        }
        continue;
        
    alt_label_1:
        KEEP_LABEL(alt_label_1);
        /* Eligible delay slot candidate */
        local_x = local_y - local_z;  /* Simple, non-trapping */
        res2 += local_x;
        
        /* Another jump to keep pattern */
        if (LIKELY(val < 500)) {
            goto alt_label_3;
        }
        continue;
        
    alt_label_2:
        KEEP_LABEL(alt_label_2);
        local_b = local_a * 2;  /* Multiplication is safe with integers */
        res1 += local_b;
        continue;
        
    alt_label_3:
        KEEP_LABEL(alt_label_3);
        local_y = local_x | 0x0F;  /* Bitwise OR - never traps */
        res2 += local_y;
        continue;
    }
    
    output[0] = res1;
    output[1] = res2;
}

/* Mix integer and float to stress scheduler */
void mixed_kernel(int *input, float *foutput, int size) {
    int i;
    float facc = 0.0f;
    int iacc = 0;
    
    int local1 = 10, local2 = 20, local3 = 30;
    
    for (i = 0; i < size; i++) {
        /* Integer path with label jumps */
        if (LIKELY(input[i] > 0)) {
            goto mixed_label_int;
        }
        
        /* Float computation in different basic block */
        facc += 1.5f;
        goto mixed_continue;
        
    mixed_label_int:
        KEEP_LABEL(mixed_label_int);
        /* Simple integer operation - delay slot candidate */
        local1 = local2 + local3;
        iacc += local1;
        
        /* Conditional jump */
        if (UNLIKELY(input[i] % 3 == 0)) {
            goto mixed_label_float;
        }
        
    mixed_continue:
        /* Empty for fallthrough */
        continue;
        
    mixed_label_float:
        KEEP_LABEL(mixed_label_float);
        /* Different type of operation */
        facc *= 1.1f;
        continue;
    }
    
    foutput[0] = facc;
    foutput[1] = (float)iacc;
}

int main() {
    const int SIZE = 1000;
    int *input = malloc(SIZE * sizeof(int));
    int *output = malloc(10 * sizeof(int));
    float *foutput = malloc(10 * sizeof(float));
    
    if (!input || !output || !foutput) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern to trigger various paths */
    for (int i = 0; i < SIZE; i++) {
        input[i] = (i * 17) % 123;  /* Semi-random pattern */
    }
    
    /* Execute all kernels to maximize coverage */
    compute_kernel(input, output, SIZE);
    alternate_kernel(input, output + 3, SIZE);
    mixed_kernel(input, foutput, SIZE);
    
    /* Use results to prevent optimization */
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total += output[i];
    }
    total += (int)foutput[0];
    
    printf("Result: %d\n", total);
    printf("Output[0]=%d, Output[1]=%d, Output[2]=%d\n", 
           output[0], output[1], output[2]);
    printf("Float output: %.2f, %.2f\n", foutput[0], foutput[1]);
    
    free(input);
    free(output);
    free(foutput);
    
    return 0;
}
