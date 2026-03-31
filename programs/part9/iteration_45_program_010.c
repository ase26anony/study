#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int global_seed;
int global_int_array[512];
float global_float_array[512];
int accumulator_int[512];
float accumulator_float[512];

/* Vector type definitions */
typedef int v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef float v8sf __attribute__((vector_size(32)));    /* 256-bit float */
typedef float v16sf __attribute__((vector_size(64)));   /* 512-bit float */
typedef double v4df __attribute__((vector_size(32)));   /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));   /* 512-bit double */

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    unsigned int r = seed;
    for (int i = 0; i < 512; i++) {
        /* Simple LCG */
        r = r * 1103515245 + 12345;
        global_int_array[i] = (int)(r % 1000);
        global_float_array[i] = (float)(r % 1000) * 0.1f;
        accumulator_int[i] = 0;
        accumulator_float[i] = 0.0f;
    }
}

/* Function 1: Complex shuffle with 10+ operands using __builtin_shuffle */
#ifdef __AVX2__
void shuffle_v8si_complex(v8si *result, const int *data, volatile int *mask_indices) {
    v8si a = *(const v8si*)(data);
    v8si b = *(const v8si*)(data + 8);
    v8si c = *(const v8si*)(data + 16);
    v8si d = *(const v8si*)(data + 24);
    
    /* Volatile to prevent constant folding */
    volatile int idx0 = mask_indices[0] & 31;
    volatile int idx1 = mask_indices[1] & 31;
    volatile int idx2 = mask_indices[2] & 31;
    volatile int idx3 = mask_indices[3] & 31;
    volatile int idx4 = mask_indices[4] & 31;
    volatile int idx5 = mask_indices[5] & 31;
    volatile int idx6 = mask_indices[6] & 31;
    volatile int idx7 = mask_indices[7] & 31;
    
    /* Create control vector with volatile-derived indices */
    v8si control = {idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7};
    
    /* Complex control flow to stress expander */
    if (global_seed % 3 == 0) {
        /* This shuffle uses 10 operands during expansion:
           - 4 input vectors (a, b, c, d)
           - 1 control vector (control)
           Total: 5 vector operands = 10+ scalar operands in RTL */
        v8si temp = __builtin_shuffle(a, b, c, d, control);
        *result = temp + (v8si){1, 2, 3, 4, 5, 6, 7, 8};
    } else if (global_seed % 3 == 1) {
        /* Alternative shuffle pattern */
        v8si temp = __builtin_shuffle(b, a, d, c, control);
        *result = temp - (v8si){8, 7, 6, 5, 4, 3, 2, 1};
    } else {
        /* Third pattern with different operand order */
        v8si temp = __builtin_shuffle(d, c, b, a, control);
        *result = temp * (v8si){2, 2, 2, 2, 2, 2, 2, 2};
    }
}
#endif

/* Function 2: __builtin_shufflevector with many operands */
#ifdef __AVX512F__
void shuffle_v16si_many_operands(v16si *result, const int *data, volatile int *mask_indices) {
    v16si a = *(const v16si*)(data);
    v16si b = *(const v16si*)(data + 16);
    v16si c = *(const v16si*)(data + 32);
    v16si d = *(const v16si*)(data + 48);
    
    /* Use volatile indices to prevent compile-time folding */
    volatile int idx[16];
    for (int i = 0; i < 16; i++) {
        idx[i] = mask_indices[i] & 63;
    }
    
    /* Switch statement with different shuffle patterns */
    switch (global_seed % 4) {
        case 0: {
            /* This can generate 11+ operands:
               - 4 input vectors (a, b, c, d)
               - 16 scalar indices
               Total: 20 scalar operands in RTL */
            v16si temp = __builtin_shufflevector(a, b, c, d,
                idx[0], idx[1], idx[2], idx[3], idx[4], idx[5], idx[6], idx[7],
                idx[8], idx[9], idx[10], idx[11], idx[12], idx[13], idx[14], idx[15]);
            *result = temp + a;
            break;
        }
        case 1: {
            v16si temp = __builtin_shufflevector(b, a, d, c,
                idx[15], idx[14], idx[13], idx[12], idx[11], idx[10], idx[9], idx[8],
                idx[7], idx[6], idx[5], idx[4], idx[3], idx[2], idx[1], idx[0]);
            *result = temp - b;
            break;
        }
        case 2: {
            v16si temp = __builtin_shufflevector(c, d, a, b,
                idx[0]+1, idx[1]+1, idx[2]+1, idx[3]+1, idx[4]+1, idx[5]+1, idx[6]+1, idx[7]+1,
                idx[8]+1, idx[9]+1, idx[10]+1, idx[11]+1, idx[12]+1, idx[13]+1, idx[14]+1, idx[15]+1);
            *result = temp * c;
            break;
        }
        default: {
            v16si temp = __builtin_shufflevector(d, c, b, a,
                idx[0]*2, idx[1]*2, idx[2]*2, idx[3]*2, idx[4]*2, idx[5]*2, idx[6]*2, idx[7]*2,
                idx[8]*2, idx[9]*2, idx[10]*2, idx[11]*2, idx[12]*2, idx[13]*2, idx[14]*2, idx[15]*2);
            *result = temp | d;
            break;
        }
    }
}
#endif

/* Function 3: Mixed floating-point shuffles */
#ifdef __AVX2__
void shuffle_v8df_complex(v8df *result, const double *data, volatile int *mask_indices) {
    v4df a = *(const v4df*)(data);
    v4df b = *(const v4df*)(data + 4);
    v4df c = *(const v4df*)(data + 8);
    v4df d = *(const v4df*)(data + 12);
    
    volatile int idx0 = mask_indices[0] & 15;
    volatile int idx1 = mask_indices[1] & 15;
    volatile int idx2 = mask_indices[2] & 15;
    volatile int idx3 = mask_indices[3] & 15;
    volatile int idx4 = mask_indices[4] & 15;
    volatile int idx5 = mask_indices[5] & 15;
    volatile int idx6 = mask_indices[6] & 15;
    volatile int idx7 = mask_indices[7] & 15;
    
    v8df control = {idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7};
    
    /* Loop with conditional shuffle execution */
    for (int i = 0; i < 4; i++) {
        if (mask_indices[i] % 2 == 0) {
            /* Complex shuffle operation */
            v8df temp = __builtin_shuffle(a, b, c, d, control);
            *result = temp + (v8df){1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
            
            /* Volatile store to prevent optimization */
            volatile v8df *volatile_ptr = result;
            *volatile_ptr = *result;
        } else {
            v8df temp = __builtin_shuffle(b, a, d, c, control);
            *result = temp - (v8df){8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
        }
    }
}
#endif

/* Function 4: Narrowing and expanding with shuffles */
#ifdef __SSE2__
void mixed_size_shuffle(int *result, const int *data, volatile int *mask_indices) {
    typedef int v4si __attribute__((vector_size(16)));
    
    v4si a = *(const v4si*)(data);
    v4si b = *(const v4si*)(data + 4);
    v4si c = *(const v4si*)(data + 8);
    v4si d = *(const v4si*)(data + 12);
    
    volatile int idx[8];
    for (int i = 0; i < 8; i++) {
        idx[i] = mask_indices[i] & 15;
    }
    
    /* Multiple shuffle operations in sequence */
    v4si temp1 = __builtin_shufflevector(a, b, idx[0], idx[1], idx[2], idx[3]);
    v4si temp2 = __builtin_shufflevector(c, d, idx[4], idx[5], idx[6], idx[7]);
    
    /* Combine results */
    v4si combined = temp1 + temp2;
    
    /* Store to global accumulator */
    *(v4si*)result = combined;
}
#endif

int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    global_seed = seed;
    init_arrays(seed);
    
    volatile int mask_buffer[64];
    for (int i = 0; i < 64; i++) {
        mask_buffer[i] = (i * seed + 123) % 64;
    }
    
    /* Main computation loop */
    for (int iter = 0; iter < 10; iter++) {
        int base_idx = (iter * 37) % 256;
        
#ifdef __AVX2__
        /* Use v8si shuffle */
        v8si v8si_result;
        shuffle_v8si_complex(&v8si_result, 
                           &global_int_array[base_idx],
                           &mask_buffer[iter * 4]);
        
        /* Accumulate result */
        v8si *acc_ptr = (v8si*)&accumulator_int[base_idx];
        *acc_ptr = *acc_ptr + v8si_result;
#endif

#ifdef __AVX512F__
        /* Use v16si shuffle */
        if (iter % 2 == 0) {
            v16si v16si_result;
            shuffle_v16si_many_operands(&v16si_result,
                                      &global_int_array[base_idx * 2],
                                      &mask_buffer[iter * 8]);
            
            /* Store with volatile to prevent DCE */
            volatile v16si *vol_ptr = &v16si_result;
            v16si temp = *vol_ptr;
            
            v16si *acc_ptr16 = (v16si*)&accumulator_int[base_idx * 2];
            *acc_ptr16 = *acc_ptr16 + temp;
        }
#endif

#ifdef __AVX2__
        /* Use v8df shuffle */
        if (iter % 3 == 0) {
            v8df v8df_result;
            /* Convert int array to double for testing */
            double double_buffer[16];
            for (int i = 0; i < 16; i++) {
                double_buffer[i] = (double)global_int_array[base_idx + i];
            }
            
            shuffle_v8df_complex(&v8df_result,
                               double_buffer,
                               &mask_buffer[iter * 4]);
            
            /* Convert back and accumulate */
            for (int i = 0; i < 8; i++) {
                accumulator_float[base_idx + i] += (float)v8df_result[i];
            }
        }
#endif

#ifdef __SSE2__
        /* Mixed size shuffle */
        mixed_size_shuffle(&accumulator_int[base_idx + 64],
                         &global_int_array[base_idx + 64],
                         &mask_buffer[iter * 8]);
#endif
    }
    
    /* Compute checksum */
    long long int_checksum = 0;
    float float_checksum = 0.0f;
    
    for (int i = 0; i < 512; i++) {
        int_checksum += accumulator_int[i];
        float_checksum += accumulator_float[i];
    }
    
    printf("Integer checksum: %lld\n", int_checksum);
    printf("Float checksum: %f\n", float_checksum);
    
    return 0;
}
