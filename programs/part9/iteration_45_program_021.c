#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int global_seed;
int global_int_array[512];
float global_float_array[512];
int accumulator[512] = {0};

/* Vector type definitions using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef int v16si __attribute__((vector_size(64)));
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef double v8df __attribute__((vector_size(64)));

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    unsigned int lcg = seed;
    for (int i = 0; i < 512; i++) {
        lcg = lcg * 1103515245 + 12345;
        global_int_array[i] = (int)(lcg % 1000);
        global_float_array[i] = (float)(lcg % 1000) * 0.1f;
    }
}

/* Function 1: Complex shuffle with 10+ operands using __builtin_shuffle */
#ifdef __AVX2__
void shuffle_10_operand_int(v16si *result, const int *data, volatile int mask_idx) {
    /* Load 4 vectors of data */
    v16si v1 = *(const v16si*)(data);
    v16si v2 = *(const v16si*)(data + 16);
    v16si v3 = *(const v16si*)(data + 32);
    v16si v4 = *(const v16si*)(data + 48);
    
    /* Create control mask with runtime-dependent indices */
    volatile int mask_base = mask_idx;
    int control[16];
    for (int i = 0; i < 16; i++) {
        control[i] = (mask_base + i * 3) % 64;
    }
    
    /* Complex shuffle pattern that may require many operands during expansion */
    v16si mask = *(v16si*)control;
    
    /* This shuffle with 3 input vectors and complex mask may expand to many operands */
    v16si shuffled = __builtin_shuffle(v1, v2, v3, mask);
    
    /* Additional arithmetic to ensure the result is used */
    *result = shuffled + v4;
}
#endif

/* Function 2: Mixed float/double shuffle with potential 11 operands */
#ifdef __AVX512F__
void shuffle_11_operand_mixed(v8df *result, const float *fdata, const double *ddata, 
                              volatile int pattern_selector) {
    /* Load different vector types */
    v16sf fvec1 = *(const v16sf*)(fdata);
    v16sf fvec2 = *(const v16sf*)(fdata + 16);
    v8df dvec1 = *(const v8df*)(ddata);
    v8df dvec2 = *(const v8df*)(ddata + 8);
    
    /* Control flow that selects different shuffle patterns */
    volatile int selector = pattern_selector;
    int control[8];
    
    if (selector & 1) {
        /* Pattern A: Interleaved indices */
        for (int i = 0; i < 8; i++) {
            control[i] = (i * 2) % 16;
        }
    } else {
        /* Pattern B: Reversed with offset */
        for (int i = 0; i < 8; i++) {
            control[i] = (15 - i + selector) % 16;
        }
    }
    
    v8df mask = *(v8df*)control;
    
    /* Complex shuffle operation that may expand to 11 operands */
    v8df temp;
    switch (selector % 4) {
        case 0:
            temp = __builtin_shuffle(fvec1, fvec2, dvec1, mask);
            break;
        case 1:
            temp = __builtin_shuffle(dvec1, dvec2, fvec1, mask);
            break;
        case 2:
            temp = __builtin_shufflevector(fvec1, fvec2, 
                0, 2, 4, 6, 8, 10, 12, 14,
                1, 3, 5, 7, 9, 11, 13, 15);
            break;
        default:
            temp = dvec1 + dvec2;
    }
    
    /* Store with volatile to prevent elimination */
    volatile v8df *volatile_ptr = result;
    *volatile_ptr = temp;
}
#endif

/* Function 3: Nested shuffles with control flow */
#ifdef __SSE2__
void complex_nested_shuffle(v8si *acc, const int *data, volatile int iter) {
    v8si v1 = *(const v8si*)(data);
    v8si v2 = *(const v8si*)(data + 8);
    v8si v3 = *(const v8si*)(data + 16);
    v8si v4 = *(const v8si*)(data + 24);
    
    /* Runtime-dependent control mask */
    int control[8];
    for (int i = 0; i < 8; i++) {
        control[i] = (iter + i * 5) % 32;
    }
    
    v8si mask = *(v8si*)control;
    v8si result;
    
    /* Complex if-else chain with different shuffle patterns */
    if (iter % 3 == 0) {
        /* Pattern requiring potentially many operands */
        result = __builtin_shuffle(v1, v2, v3, v4, mask);
    } else if (iter % 3 == 1) {
        /* Alternative pattern with __builtin_shufflevector */
        result = __builtin_shufflevector(v1, v2, v3,
            0, 2, 4, 6, 8, 10, 12, 14,
            1, 3, 5, 7, 9, 11, 13, 15);
    } else {
        /* Chain of operations that might be combined */
        v8si t1 = __builtin_shuffle(v1, v2, mask);
        v8si t2 = __builtin_shuffle(v3, v4, mask);
        result = t1 + t2;
    }
    
    /* Accumulate result */
    *acc = *acc + result;
}
#endif

/* Function 4: Vector size conversion through shuffles */
void vector_size_conversion(int *output, const int *input, volatile int offset) {
    /* Start with 256-bit vector */
    v8si wide_vec = *(const v8si*)(input + offset);
    
    /* Narrow to 128-bit through complex shuffle pattern */
    int narrow_ctrl[4] = {offset % 8, (offset + 2) % 8, 
                         (offset + 4) % 8, (offset + 6) % 8};
    v4si narrow_mask = *(v4si*)narrow_ctrl;
    
    /* This shuffle might require many operands during expansion */
    v4si narrow = __builtin_shuffle(wide_vec, narrow_mask);
    
    /* Expand back to 256-bit */
    int expand_ctrl[8];
    for (int i = 0; i < 8; i++) {
        expand_ctrl[i] = (i < 4) ? i : 0;
    }
    v8si expand_mask = *(v8si*)expand_ctrl;
    v8si expanded = __builtin_shuffle(narrow, expand_mask);
    
    /* Store result */
    *(v8si*)output = expanded;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    global_seed = seed;
    
    init_arrays(seed);
    
    /* Convert float array to double for mixed operations */
    double double_array[512];
    for (int i = 0; i < 512; i++) {
        double_array[i] = (double)global_float_array[i];
    }
    
    /* Main processing loop */
    for (int iter = 0; iter < 10; iter++) {
        volatile int mask_idx = iter;
        
#ifdef __AVX2__
        /* Call 10-operand shuffle function */
        v16si int_result;
        shuffle_10_operand_int(&int_result, &global_int_array[iter * 16], mask_idx);
        
        /* Accumulate results */
        for (int i = 0; i < 16; i++) {
            accumulator[iter * 16 + i] += int_result[i];
        }
#endif

#ifdef __AVX512F__
        /* Call 11-operand mixed shuffle function */
        v8df double_result;
        shuffle_11_operand_mixed(&double_result, 
                                &global_float_array[iter * 32],
                                &double_array[iter * 16],
                                mask_idx);
        
        /* Convert and accumulate */
        for (int i = 0; i < 8; i++) {
            accumulator[iter * 8 + i] += (int)double_result[i];
        }
#endif

#ifdef __SSE2__
        /* Call nested shuffle function */
        v8si acc_vec = {0};
        complex_nested_shuffle(&acc_vec, &global_int_array[iter * 32], mask_idx);
        
        /* Store accumulated results */
        for (int i = 0; i < 8; i++) {
            accumulator[iter * 8 + i + 256] += acc_vec[i];
        }
#endif
        
        /* Vector size conversion test */
        vector_size_conversion(&accumulator[iter * 8 + 384],
                              global_int_array,
                              mask_idx);
    }
    
    /* Compute final checksum */
    long long checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += accumulator[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    printf("Seed used: %d\n", seed);
    
    return 0;
}
