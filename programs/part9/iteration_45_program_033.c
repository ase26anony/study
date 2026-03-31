#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
static int32_t global_ints[512];
static double global_floats[512];
static volatile int32_t volatile_buffer[512];
static int32_t accumulator[512];

/* Vector type definitions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef double v4df __attribute__((vector_size(32)));       /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));       /* 512-bit double */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */

/* Initialize with deterministic pseudo-random sequence */
void init_arrays(int seed) {
    srand(seed);
    for (int i = 0; i < 512; i++) {
        global_ints[i] = rand() % 1000;
        global_floats[i] = (rand() % 1000) / 10.0;
        accumulator[i] = 0;
    }
}

/* Function using __builtin_shuffle with many operands - targeting 10+ operands */
#ifdef __AVX2__
static v8si shuffle_10_operand_int(v8si a, v8si b, v8si c, v8si d, 
                                   v8si mask1, v8si mask2, v8si mask3) {
    /* Complex shuffle pattern that should require many operand slots */
    v8si temp1 = __builtin_shuffle(a, b, mask1);
    v8si temp2 = __builtin_shuffle(c, d, mask2);
    
    /* Nested shuffle with intermediate computation */
    v8si sum = temp1 + temp2;
    v8si result = __builtin_shuffle(sum, a + b, mask3);
    
    return result;
}
#endif

#ifdef __AVX512F__
/* Function targeting exactly 11 operands during expansion */
static v16si shuffle_11_operand_int(v16si a, v16si b, v16si c, v16si d,
                                    v16si e, v16si f, v16si mask) {
    /* This complex expression should require 11 operand slots:
       1-6: input vectors a-f
       7: mask
       8-11: intermediate computation results */
    v16si t1 = __builtin_shuffle(a, b, mask);
    v16si t2 = __builtin_shuffle(c, d, mask);
    v16si t3 = __builtin_shuffle(e, f, mask);
    
    /* Combined operation that uses all shuffled results */
    v16si result = (t1 * t2) + (t3 << 2);
    
    return result;
}

/* Mixed float/double shuffle with many operands */
static v8df shuffle_mixed_10_operand(v8df a, v8df b, v4df c, v4df d,
                                     v16si int_mask, v8si smaller_mask) {
    /* Convert between vector sizes to stress the expander */
    v8df wide_c = __builtin_shufflevector(c, c, 0, 1, 2, 3, 0, 1, 2, 3);
    v8df wide_d = __builtin_shufflevector(d, d, 3, 2, 1, 0, 3, 2, 1, 0);
    
    /* Use integer mask for floating-point shuffle (requires conversion) */
    v8df temp1 = __builtin_shuffle(a, b, int_mask);
    v8df temp2 = __builtin_shuffle(wide_c, wide_d, smaller_mask);
    
    return temp1 * temp2 + (v8df){1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
}
#endif

/* Function with control flow around vector operations */
void conditional_shuffle_operations(int mode, volatile int* control) {
#ifdef __AVX2__
    v8si mask1 = {0, 7, 1, 6, 2, 5, 3, 4};
    v8si mask2 = {7, 0, 6, 1, 5, 2, 4, 3};
    v8si mask3 = {3, 4, 2, 5, 1, 6, 0, 7};
    
    /* Load data with volatile access to prevent optimization */
    v8si data1, data2, data3, data4;
    for (int i = 0; i < 8; i++) {
        data1[i] = volatile_buffer[i];
        data2[i] = volatile_buffer[i + 8];
        data3[i] = volatile_buffer[i + 16];
        data4[i] = volatile_buffer[i + 24];
    }
    
    /* Complex control flow with multiple shuffle patterns */
    if (mode == 0) {
        v8si result = shuffle_10_operand_int(data1, data2, data3, data4,
                                            mask1, mask2, mask3);
        /* Store back through volatile to force computation */
        for (int i = 0; i < 8; i++) {
            volatile_buffer[i] = result[i];
        }
    } else if (mode == 1 && *control > 0) {
        /* Alternative shuffle pattern */
        v8si alt_mask = {*control % 8, (*control + 1) % 8, (*control + 2) % 8,
                         (*control + 3) % 8, (*control + 4) % 8,
                         (*control + 5) % 8, (*control + 6) % 8,
                         (*control + 7) % 8};
        v8si result = __builtin_shuffle(data1, data2, alt_mask);
        result = result + __builtin_shuffle(data3, data4, mask2);
        
        for (int i = 0; i < 8; i++) {
            accumulator[i] += result[i];
        }
    }
#endif
}

/* Loop with varying shuffle patterns based on runtime values */
void shuffle_loop(int iterations, int start_idx) {
    volatile int control = start_idx;
    
    for (int i = 0; i < iterations; i++) {
        /* Update volatile control variable */
        control = (control * 1103515245 + 12345) & 0x7fffffff;
        
        /* Switch between different operation modes */
        switch (control % 4) {
            case 0:
                conditional_shuffle_operations(0, &control);
                break;
            case 1:
                conditional_shuffle_operations(1, &control);
                break;
            case 2:
#ifdef __AVX512F__
                /* Load 512-bit vectors */
                v16si big_mask = {0, 15, 1, 14, 2, 13, 3, 12,
                                  4, 11, 5, 10, 6, 9, 7, 8};
                v16si data_a, data_b, data_c, data_d, data_e, data_f;
                
                for (int j = 0; j < 16; j++) {
                    int idx = (start_idx + i * 16 + j) % 512;
                    data_a[j] = global_ints[idx];
                    data_b[j] = global_ints[(idx + 16) % 512];
                    data_c[j] = global_ints[(idx + 32) % 512];
                    data_d[j] = global_ints[(idx + 48) % 512];
                    data_e[j] = global_ints[(idx + 64) % 512];
                    data_f[j] = global_ints[(idx + 80) % 512];
                }
                
                v16si result = shuffle_11_operand_int(data_a, data_b, data_c,
                                                     data_d, data_e, data_f,
                                                     big_mask);
                
                /* Accumulate results */
                for (int j = 0; j < 16; j++) {
                    accumulator[(i * 16 + j) % 512] += result[j];
                }
#endif
                break;
            case 3:
#ifdef __AVX512F__
                /* Mixed float/double operations */
                v8df double_data1, double_data2;
                v4df double_data3, double_data4;
                v16si int_mask = {0, 8, 1, 9, 2, 10, 3, 11,
                                  4, 12, 5, 13, 6, 14, 7, 15};
                v8si smaller_mask = {0, 7, 1, 6, 2, 5, 3, 4};
                
                for (int j = 0; j < 8; j++) {
                    int idx = (start_idx + i * 8 + j) % 512;
                    double_data1[j] = global_floats[idx];
                    double_data2[j] = global_floats[(idx + 8) % 512];
                    if (j < 4) {
                        double_data3[j] = global_floats[(idx + 16) % 512];
                        double_data4[j] = global_floats[(idx + 20) % 512];
                    }
                }
                
                v8df mixed_result = shuffle_mixed_10_operand(double_data1,
                                                           double_data2,
                                                           double_data3,
                                                           double_data4,
                                                           int_mask,
                                                           smaller_mask);
                
                /* Convert and accumulate */
                for (int j = 0; j < 8; j++) {
                    accumulator[(i * 8 + j) % 512] += (int32_t)mixed_result[j];
                }
#endif
                break;
        }
        
        /* Memory barrier via volatile store */
        volatile_buffer[i % 512] = control;
    }
}

int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    /* Copy initial data to volatile buffer */
    memcpy((void*)volatile_buffer, global_ints, sizeof(global_ints));
    
    /* Perform shuffle operations in loops with varying parameters */
    for (int phase = 0; phase < 3; phase++) {
        shuffle_loop(10, phase * 50);
    }
    
    /* Compute checksum */
    int64_t checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += accumulator[i];
        checksum += volatile_buffer[i];  /* Include volatile data */
    }
    
    printf("Final checksum: %lld\n", (long long)checksum);
    
    return 0;
}
