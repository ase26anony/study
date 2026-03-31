#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
#define ARRAY_SIZE 512
static volatile int global_seed;
static int32_t global_int_array[ARRAY_SIZE];
static double global_float_array[ARRAY_SIZE];
static int32_t accumulator_int[ARRAY_SIZE];
static double accumulator_float[ARRAY_SIZE];

/* Vector type definitions using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef double v4df __attribute__((vector_size(32)));       /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));       /* 512-bit double */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    global_seed = seed;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Simple LCG for deterministic values */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        global_int_array[i] = (int32_t)seed;
        global_float_array[i] = (double)(seed % 1000) / 10.0;
        accumulator_int[i] = 0;
        accumulator_float[i] = 0.0;
    }
}

/* Function 1: Complex shuffle with 10+ operands using __builtin_shuffle */
__attribute__((noinline))
v16si shuffle_int_512(v16si a, v16si b, v16si mask) {
    /* This should trigger the 11-operand case when expanded:
       __builtin_shuffle(a, b, mask) becomes 3 vector operands
       Additional arithmetic operations add more operands */
    
    volatile int control = global_seed & 0xF;
    v16si result;
    
    /* Complex control flow to prevent optimization */
    if (control < 8) {
        /* First shuffle pattern - requires many operands during expansion */
        v16si shuffled = __builtin_shuffle(a, b, mask);
        result = shuffled + a - b;
    } else {
        /* Second shuffle pattern */
        v16si temp1 = __builtin_shuffle(a, mask, b);
        v16si temp2 = __builtin_shuffle(b, mask, a);
        result = temp1 * temp2 + a;
    }
    
    /* Additional shuffle to ensure multiple expander calls */
    v16si final_mask = mask + (v16si){1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    result = __builtin_shuffle(result, final_mask);
    
    return result;
}

/* Function 2: Mixed float/int operations with large shufflevector */
__attribute__((noinline))
v8df shuffle_float_512(v8df a, v8df b, v8df c, v8df mask) {
    volatile int pattern = global_seed & 0x3;
    v8df result;
    
    switch (pattern) {
        case 0: {
            /* __builtin_shufflevector with many arguments */
            v8df shuffled = __builtin_shufflevector(a, b, c,
                0, 8, 1, 9, 2, 10, 3, 11,
                4, 12, 5, 13, 6, 14, 7, 15);
            result = shuffled * mask + a;
            break;
        }
        case 1: {
            /* Another complex shuffle pattern */
            v8df temp = __builtin_shuffle(a, b, (v8df){0,1,2,3,4,5,6,7});
            result = __builtin_shufflevector(temp, c, mask,
                7,6,5,4,3,2,1,0,
                15,14,13,12,11,10,9,8);
            break;
        }
        default: {
            /* Mix of shuffle operations */
            v8df s1 = __builtin_shuffle(a, mask);
            v8df s2 = __builtin_shuffle(b, (v8df){7,6,5,4,3,2,1,0});
            result = s1 + s2 * c;
        }
    }
    
    return result;
}

/* Function 3: Narrowing and widening with shuffles */
__attribute__((noinline))
v8si shuffle_narrow_wide(v8si a, v8si b, v8si c, v8si d) {
    /* Convert 256-bit to 512-bit via multiple shuffles */
    v16si wide_a = __builtin_shufflevector(a, b,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    v16si wide_b = __builtin_shufflevector(c, d,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    
    volatile int idx = global_seed % 16;
    v16si mask = {idx, idx+1, idx+2, idx+3, idx+4, idx+5, idx+6, idx+7,
                  idx+8, idx+9, idx+10, idx+11, idx+12, idx+13, idx+14, idx+15};
    
    v16si result = __builtin_shuffle(wide_a, wide_b, mask);
    
    /* Narrow back to 256-bit */
    v8si narrowed = __builtin_shufflevector(result, (v16si){},
        0,2,4,6,8,10,12,14);
    
    return narrowed;
}

/* Function 4: AVX2-specific shuffle patterns */
#ifdef __AVX2__
__attribute__((noinline))
v8si avx2_complex_shuffle(v8si a, v8si b, v8si mask1, v8si mask2) {
    /* Multiple shuffle operations in sequence */
    v8si s1 = __builtin_shuffle(a, b, mask1);
    v8si s2 = __builtin_shuffle(b, a, mask2);
    
    /* Interleave results with another shuffle */
    v8si result = __builtin_shufflevector(s1, s2,
        0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15);
    
    /* Conditional shuffle based on volatile */
    volatile int cond = global_seed & 1;
    if (cond) {
        result = __builtin_shuffle(result, (v8si){7,6,5,4,3,2,1,0});
    }
    
    return result;
}
#endif

/* Function 5: AVX512-specific with many operands */
#ifdef __AVX512F__
__attribute__((noinline))
v16si avx512_mega_shuffle(v16si a, v16si b, v16si c, v16si d, v16si mask) {
    /* This should stress the expander with many vector operands */
    v16si temp1 = __builtin_shuffle(a, b, mask);
    v16si temp2 = __builtin_shuffle(c, d, mask + (v16si){1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1});
    
    /* Complex expression with multiple shuffle operands */
    v16si result = __builtin_shuffle(temp1, temp2,
        (v16si){0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23});
    
    /* Additional shuffle to ensure we hit the 10+ operand case */
    result = result + __builtin_shuffle(a, mask) - __builtin_shuffle(b, mask);
    
    return result;
}
#endif

/* Main test function that calls all shuffle variants */
void run_shuffle_tests(int iterations) {
    for (int i = 0; i < iterations; i++) {
        int base_idx = (i * 16) % (ARRAY_SIZE - 64);
        
        /* Load data into vectors */
        v16si int_vec1 = *(v16si*)(&global_int_array[base_idx]);
        v16si int_vec2 = *(v16si*)(&global_int_array[base_idx + 16]);
        v16si int_vec3 = *(v16si*)(&global_int_array[base_idx + 32]);
        v16si int_vec4 = *(v16si*)(&global_int_array[base_idx + 48]);
        
        v8df float_vec1 = *(v8df*)(&global_float_array[base_idx]);
        v8df float_vec2 = *(v8df*)(&global_float_array[base_idx + 8]);
        v8df float_vec3 = *(v8df*)(&global_float_array[base_idx + 16]);
        v8df float_vec4 = *(v8df*)(&global_float_array[base_idx + 24]);
        
        /* Create mask from loop-dependent values */
        v16si int_mask;
        v8df float_mask;
        for (int j = 0; j < 16; j++) {
            int_mask[j] = (i + j) % 16;
            if (j < 8) {
                float_mask[j] = (double)((i + j) % 8);
            }
        }
        
        /* Call shuffle functions */
        v16si int_result = shuffle_int_512(int_vec1, int_vec2, int_mask);
        
        /* Store results to accumulator */
        for (int j = 0; j < 16; j++) {
            accumulator_int[base_idx + j] += int_result[j];
        }
        
        /* Float shuffles */
        v8df float_result = shuffle_float_512(float_vec1, float_vec2, float_vec3, float_mask);
        for (int j = 0; j < 8; j++) {
            accumulator_float[base_idx + j] += float_result[j];
        }
        
        /* Narrow/wide shuffles */
        v8si narrow_a = *(v8si*)(&global_int_array[base_idx]);
        v8si narrow_b = *(v8si*)(&global_int_array[base_idx + 8]);
        v8si narrow_c = *(v8si*)(&global_int_array[base_idx + 16]);
        v8si narrow_d = *(v8si*)(&global_int_array[base_idx + 24]);
        v8si narrow_result = shuffle_narrow_wide(narrow_a, narrow_b, narrow_c, narrow_d);
        
        /* Architecture-specific tests */
#ifdef __AVX2__
        v8si avx2_mask1 = {7,6,5,4,3,2,1,0};
        v8si avx2_mask2 = {0,1,2,3,4,5,6,7};
        v8si avx2_result = avx2_complex_shuffle(narrow_a, narrow_b, avx2_mask1, avx2_mask2);
        for (int j = 0; j < 8; j++) {
            accumulator_int[base_idx + 32 + j] += avx2_result[j];
        }
#endif

#ifdef __AVX512F__
        v16si avx512_result = avx512_mega_shuffle(int_vec1, int_vec2, int_vec3, int_vec4, int_mask);
        for (int j = 0; j < 16; j++) {
            accumulator_int[base_idx + 48 + j] += avx512_result[j];
        }
#endif
        
        /* Volatile memory barrier to prevent optimization */
        volatile int barrier = global_seed;
        (void)barrier;
    }
}

/* Compute checksum of results */
int64_t compute_checksum(void) {
    int64_t checksum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += accumulator_int[i];
        checksum += (int64_t)accumulator_float[i];
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    printf("Initializing with seed: %d\n", seed);
    init_arrays(seed);
    
    printf("Running shuffle tests...\n");
    run_shuffle_tests(10);
    
    int64_t checksum = compute_checksum();
    printf("Final checksum: %lld\n", (long long)checksum);
    
    return 0;
}
