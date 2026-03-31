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
        accumulator_int[i] = 0;
        accumulator_float[i] = 0.0f;
    }
}

/* Function with complex control flow and 10+ operand shuffle */
#ifdef __AVX2__
void shuffle_avx2_int(v8si *result, const int *data, volatile int mask_idx) {
    v8si a = *(const v8si *)(data);
    v8si b = *(const v8si *)(data + 8);
    v8si c = *(const v8si *)(data + 16);
    
    /* Complex control flow to stress expander */
    if (mask_idx & 1) {
        /* Use volatile variable to prevent constant folding */
        volatile int idx = mask_idx;
        
        /* Create a shuffle mask with 8 elements from runtime values */
        int mask_arr[8];
        for (int i = 0; i < 8; i++) {
            mask_arr[i] = (idx + i) % 24;
        }
        
        /* Load mask as vector - this creates many operands */
        v8si mask = *(v8si *)mask_arr;
        
        /* __builtin_shuffle with 10 operands: a, b, mask */
        v8si shuffled = __builtin_shuffle(a, b, mask);
        
        /* Nested control with another shuffle */
        switch (mask_idx % 4) {
            case 0: {
                /* Another shuffle with different vectors */
                v8si mask2 = {7,6,5,4,3,2,1,0};
                v8si shuffled2 = __builtin_shuffle(c, shuffled, mask2);
                *result = shuffled2 + a;
                break;
            }
            case 1: {
                /* Shuffle with 11 conceptual operands through multiple steps */
                int mask3_arr[8];
                for (int i = 0; i < 8; i++) {
                    mask3_arr[i] = (mask_idx * i) % 16;
                }
                v8si mask3 = *(v8si *)mask3_arr;
                v8si temp1 = __builtin_shuffle(a, b, mask3);
                v8si temp2 = __builtin_shuffle(c, temp1, mask);
                *result = temp2;
                break;
            }
            default:
                *result = shuffled;
        }
    } else {
        /* Alternative path with different shuffle pattern */
        v8si mask = {0,8,1,9,2,10,3,11};
        *result = __builtin_shuffle(a, b, mask);
    }
}
#endif

#ifdef __AVX512F__
void shuffle_avx512_float(v16sf *result, const float *data, volatile int pattern) {
    v16sf a = *(const v16sf *)(data);
    v16sf b = *(const v16sf *)(data + 16);
    v16sf c = *(const v16sf *)(data + 32);
    
    /* Force runtime evaluation of mask */
    volatile int base = pattern;
    
    /* Large shuffle mask array - 16 elements */
    int mask_arr[16];
    for (int i = 0; i < 16; i++) {
        mask_arr[i] = (base + i * 3) % 32;
    }
    
    /* Convert to proper type for float shuffle */
    unsigned short mask_short[16];
    for (int i = 0; i < 16; i++) {
        mask_short[i] = mask_arr[i] & 0x1F;
    }
    
    /* Using __builtin_shufflevector which can take many arguments */
    /* This should create many operands during expansion */
    v16sf shuffled;
    
    if (pattern & 2) {
        /* Complex expression with potential for many operands */
        shuffled = __builtin_shufflevector(a, b, 
            mask_short[0], mask_short[1], mask_short[2], mask_short[3],
            mask_short[4], mask_short[5], mask_short[6], mask_short[7],
            mask_short[8], mask_short[9], mask_short[10], mask_short[11],
            mask_short[12], mask_short[13], mask_short[14], mask_short[15]);
    } else {
        /* Different shuffle pattern */
        shuffled = __builtin_shufflevector(b, a,
            16,17,18,19,20,21,22,23,
            24,25,26,27,28,29,30,31);
    }
    
    /* Mix with third vector using arithmetic */
    *result = shuffled + c * 2.0f;
}

void shuffle_avx512_double(v8df *result, const double *data, volatile int idx) {
    v8df a = *(const v8df *)(data);
    v8df b = *(const v8df *)(data + 8);
    
    /* Create complex mask from runtime value */
    long long mask_arr[8];
    for (int i = 0; i < 8; i++) {
        mask_arr[i] = (idx + i * 5) % 16;
    }
    
    /* Multiple shuffle operations in control flow */
    for (int i = 0; i < 3; i++) {
        if (i == (idx % 3)) {
            /* Shuffle with explicit indices - many operands */
            v8df temp = __builtin_shufflevector(a, b,
                mask_arr[0], mask_arr[1], mask_arr[2], mask_arr[3],
                mask_arr[4], mask_arr[5], mask_arr[6], mask_arr[7]);
            
            /* Another operation that might expand to many operands */
            v8df temp2 = __builtin_shufflevector(temp, a,
                0,8,1,9,2,10,3,11);
            
            *result = temp2;
            return;
        }
    }
    
    /* Default shuffle */
    *result = __builtin_shufflevector(a, b, 0,8,1,9,2,10,3,11);
}
#endif

/* SSE2 fallback for wider compatibility */
#ifdef __SSE2__
void shuffle_sse2_mixed(v4si *int_result, v4sf *float_result, 
                       const int *int_data, const float *float_data, 
                       volatile int mask) {
    v4si a_int = *(const v4si *)(int_data);
    v4si b_int = *(const v4si *)(int_data + 4);
    v4sf a_float = *(const v4sf *)(float_data);
    v4sf b_float = *(const v4sf *)(float_data + 4);
    
    /* Multiple shuffle patterns based on control flow */
    int int_mask[4];
    unsigned short float_mask[4];
    
    for (int i = 0; i < 4; i++) {
        int_mask[i] = (mask + i) % 8;
        float_mask[i] = (mask + i * 2) % 8;
    }
    
    /* Chain shuffles to increase operand count in expansion */
    v4si shuffled_int = __builtin_shuffle(a_int, b_int, 
        int_mask[0], int_mask[1], int_mask[2], int_mask[3]);
    
    v4sf shuffled_float = __builtin_shuffle(a_float, b_float,
        float_mask[0], float_mask[1], float_mask[2], float_mask[3]);
    
    /* Cross-type operations */
    v4si int_from_float = __builtin_convertvector(shuffled_float, v4si);
    
    *int_result = shuffled_int + int_from_float;
    *float_result = shuffled_float;
}
#endif

/* Generic function using GCC vector extensions */
void shuffle_generic_large(void) {
    /* Use very large vector types */
    typedef int v32si __attribute__((vector_size(128)));
    typedef float v32sf __attribute__((vector_size(128)));
    
    /* Initialize from global arrays */
    v32si large_int = *(v32si *)&global_int_array[0];
    v32si large_int2 = *(v32si *)&global_int_array[32];
    
    /* Complex shuffle pattern that might require many operands */
    int large_mask[32];
    for (int i = 0; i < 32; i++) {
        large_mask[i] = (i * 7) % 64;
    }
    
    /* This would ideally trigger large operand expansion */
    /* Note: Actual shuffle might be implemented differently by compiler */
    v32si result;
    
    /* Simulate complex operation by breaking into smaller pieces */
    for (int i = 0; i < 32; i += 8) {
        v8si chunk1 = *(v8si *)&((int*)&large_int)[i];
        v8si chunk2 = *(v8si *)&((int*)&large_int2)[i];
        int chunk_mask[8];
        for (int j = 0; j < 8; j++) {
            chunk_mask[j] = large_mask[i + j] % 16;
        }
        v8si shuffled = __builtin_shuffle(chunk1, chunk2,
            chunk_mask[0], chunk_mask[1], chunk_mask[2], chunk_mask[3],
            chunk_mask[4], chunk_mask[5], chunk_mask[6], chunk_mask[7]);
        memcpy(&((int*)&result)[i], &shuffled, sizeof(v8si));
    }
    
    /* Store to accumulator */
    for (int i = 0; i < 32; i++) {
        accumulator_int[i] += ((int*)&result)[i];
    }
}

int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    global_seed = seed;
    
    int iterations = 10;
    volatile int control = seed;
    
    for (int iter = 0; iter < iterations; iter++) {
        volatile int pattern = (control + iter * 17) % 256;
        
#ifdef __AVX2__
        /* AVX2 integer shuffles */
        v8si avx2_result;
        shuffle_avx2_int(&avx2_result, &global_int_array[iter * 8], pattern);
        
        /* Accumulate results */
        for (int i = 0; i < 8; i++) {
            accumulator_int[iter * 8 + i] += ((int*)&avx2_result)[i];
        }
#endif
        
#ifdef __AVX512F__
        /* AVX-512 float shuffles */
        if (iter % 3 == 0) {
            v16sf avx512_float_result;
            shuffle_avx512_float(&avx512_float_result, 
                               &global_float_array[iter * 16], 
                               pattern);
            
            for (int i = 0; i < 16; i++) {
                accumulator_float[iter * 16 + i] += ((float*)&avx512_float_result)[i];
            }
        }
        
        /* AVX-512 double shuffles */
        if (iter % 4 == 1) {
            v8df avx512_double_result;
            /* Use double data from integer array converted on the fly */
            double double_data[16];
            for (int i = 0; i < 16; i++) {
                double_data[i] = global_int_array[iter * 16 + i] * 0.5;
            }
            
            shuffle_avx512_double(&avx512_double_result, double_data, pattern);
            
            /* Store to float accumulator */
            for (int i = 0; i < 8; i++) {
                accumulator_float[iter * 8 + i] += ((double*)&avx512_double_result)[i];
            }
        }
#endif
        
#ifdef __SSE2__
        /* SSE2 mixed-type shuffles */
        v4si sse_int_result;
        v4sf sse_float_result;
        shuffle_sse2_mixed(&sse_int_result, &sse_float_result,
                         &global_int_array[iter * 4],
                         &global_float_array[iter * 4],
                         pattern);
        
        for (int i = 0; i < 4; i++) {
            accumulator_int[iter * 4 + i] += ((int*)&sse_int_result)[i];
            accumulator_float[iter * 4 + i] += ((float*)&sse_float_result)[i];
        }
#endif
        
        /* Generic large vector shuffles */
        if (iter % 5 == 0) {
            shuffle_generic_large();
        }
        
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
    }
    
    /* Compute checksums */
    long long int_checksum = 0;
    double float_checksum = 0.0;
    
    for (int i = 0; i < 512; i++) {
        int_checksum += accumulator_int[i];
        float_checksum += accumulator_float[i];
    }
    
    printf("Integer checksum: %lld\n", int_checksum);
    printf("Float checksum: %f\n", float_checksum);
    
    return 0;
}
