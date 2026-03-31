#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
#define ARRAY_SIZE 512
static int32_t global_ints[ARRAY_SIZE];
static float global_floats[ARRAY_SIZE];
static volatile int32_t volatile_buffer[ARRAY_SIZE] = {0};
static volatile float volatile_float_buffer[ARRAY_SIZE] = {0};

/* Vector type definitions using GCC extensions */
typedef int32_t v4si __attribute__((vector_size(16)));
typedef int32_t v8si __attribute__((vector_size(32)));
typedef int32_t v16si __attribute__((vector_size(64)));
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef double v8df __attribute__((vector_size(64)));

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    unsigned int lcg = seed;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        lcg = lcg * 1103515245 + 12345;
        global_ints[i] = (int32_t)(lcg >> 16) & 0x7FFF;
        global_floats[i] = (float)((lcg >> 16) & 0xFF) / 256.0f;
    }
}

/* Function that uses __builtin_shuffle with many operands */
#ifdef __AVX2__
static v8si shuffle_8si_complex(v8si a, v8si b, v8si c, v8si d, 
                                volatile int* mask_ptr) {
    /* Create control mask from volatile memory */
    v8si mask;
    for (int i = 0; i < 8; i++) {
        mask[i] = mask_ptr[i] & 0x1F; /* Limit to 0-31 for 32-element space */
    }
    
    /* Complex control flow to prevent optimization */
    if (mask_ptr[0] & 1) {
        /* First shuffle pattern - uses 10 operands total */
        v8si temp1 = __builtin_shuffle(a, b, mask);
        v8si temp2 = __builtin_shuffle(c, d, mask);
        
        /* Nested shuffle with intermediate computation */
        v8si mask2 = mask + (v8si){1,2,3,4,5,6,7,8};
        v8si result = __builtin_shuffle(temp1, temp2, mask2);
        
        /* Store to volatile to prevent DCE */
        for (int i = 0; i < 8; i++) {
            volatile_buffer[i] = result[i];
        }
        
        return result;
    } else {
        /* Alternative path with different shuffle */
        v8si expanded_mask = mask * (v8si){2,2,2,2,2,2,2,2};
        v8si result = __builtin_shuffle(a, b, c, d, expanded_mask);
        
        /* Force memory barrier */
        asm volatile("" ::: "memory");
        
        return result;
    }
}
#endif

#ifdef __AVX512F__
/* Function targeting 11+ operand expansion */
static v16si shuffle_16si_mega(v16si a, v16si b, v16si c, v16si d,
                               v16si e, v16si f, volatile int* mask_ptr) {
    /* Create multiple control masks */
    v16si mask1, mask2;
    for (int i = 0; i < 16; i++) {
        mask1[i] = (mask_ptr[i] + i) & 0x3F;
        mask2[i] = (mask_ptr[i] * 2) & 0x3F;
    }
    
    /* Switch statement to create complex CFG */
    switch (mask_ptr[0] & 3) {
        case 0: {
            /* Pattern requiring 11 operands during expansion */
            v16si temp = __builtin_shuffle(a, b, c, mask1);
            v16si temp2 = __builtin_shuffle(d, e, f, mask2);
            
            /* Combined shuffle with arithmetic */
            v16si combined_mask = mask1 + mask2;
            v16si result = __builtin_shuffle(temp, temp2, combined_mask);
            
            /* Store intermediate results */
            for (int i = 0; i < 16; i++) {
                volatile_buffer[i * 2] = result[i];
            }
            
            return result;
        }
        case 1: {
            /* Different shuffle pattern */
            v16si wide_mask;
            for (int i = 0; i < 16; i++) {
                wide_mask[i] = (mask_ptr[i] + mask_ptr[i+16]) & 0x3F;
            }
            
            v16si result = __builtin_shuffle(a, b, c, d, e, wide_mask);
            return result + (v16si){1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
        }
        default: {
            /* Fallback with simple operation */
            return a + b + c;
        }
    }
}
#endif

/* Mixed floating-point shuffle with narrowing/widening */
#ifdef __AVX2__
static v8sf shuffle_float_complex(v8sf a, v8sf b, v8sf c, 
                                  volatile float* float_mask_ptr) {
    /* Create integer mask from float data */
    v8si int_mask;
    for (int i = 0; i < 8; i++) {
        int_mask[i] = (int)(float_mask_ptr[i] * 100.0f) & 0xF;
    }
    
    /* Conditional shuffle patterns */
    if (float_mask_ptr[0] > 0.5f) {
        /* Narrowing: 256-bit to 128-bit via shuffle */
        v4sf low = __builtin_shufflevector(a, b, 0, 2, 4, 6);
        v4sf high = __builtin_shufflevector(c, (v4sf){0}, 1, 3, 5, 7);
        
        /* Expand back to 256-bit */
        v8sf expanded = __builtin_shufflevector(low, high, 
                                                0,1,2,3,4,5,6,7);
        
        /* Store to volatile float buffer */
        for (int i = 0; i < 8; i++) {
            volatile_float_buffer[i] = expanded[i];
        }
        
        return expanded;
    } else {
        /* Complex shuffle with many input vectors */
        v8sf mask_vec;
        for (int i = 0; i < 8; i++) {
            mask_vec[i] = float_mask_ptr[i];
        }
        
        /* This may require many operands during expansion */
        v8sf temp = __builtin_shuffle(a, b, (v8si)int_mask);
        v8sf result = __builtin_shuffle(temp, c, mask_vec);
        
        return result;
    }
}
#endif

/* Main test driver */
int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    /* Accumulator arrays */
    int32_t int_acc[ARRAY_SIZE] = {0};
    float float_acc[ARRAY_SIZE] = {0.0f};
    
    /* Main test loop */
    for (int iter = 0; iter < 10; iter++) {
        volatile int mask_source[64];
        volatile float float_mask_source[64];
        
        /* Initialize masks with loop-dependent values */
        for (int i = 0; i < 64; i++) {
            mask_source[i] = (global_ints[i] + iter) & 0x3F;
            float_mask_source[i] = global_floats[i] + (float)iter * 0.1f;
        }
        
#ifdef __AVX2__
        /* Test 256-bit integer shuffles */
        for (int offset = 0; offset < ARRAY_SIZE - 32; offset += 8) {
            v8si vec1 = *(v8si*)&global_ints[offset];
            v8si vec2 = *(v8si*)&global_ints[offset + 8];
            v8si vec3 = *(v8si*)&global_ints[offset + 16];
            v8si vec4 = *(v8si*)&global_ints[offset + 24];
            
            v8si result = shuffle_8si_complex(vec1, vec2, vec3, vec4, 
                                             (int*)mask_source);
            
            /* Accumulate results */
            for (int i = 0; i < 8; i++) {
                int_acc[offset + i] += result[i];
            }
        }
        
        /* Test floating-point shuffles */
        for (int offset = 0; offset < ARRAY_SIZE - 32; offset += 8) {
            v8sf fvec1 = *(v8sf*)&global_floats[offset];
            v8sf fvec2 = *(v8sf*)&global_floats[offset + 8];
            v8sf fvec3 = *(v8sf*)&global_floats[offset + 16];
            
            v8sf fresult = shuffle_float_complex(fvec1, fvec2, fvec3,
                                                (float*)float_mask_source);
            
            for (int i = 0; i < 8; i++) {
                float_acc[offset + i] += fresult[i];
            }
        }
#endif

#ifdef __AVX512F__
        /* Test 512-bit shuffles for 11+ operand case */
        for (int offset = 0; offset < ARRAY_SIZE - 96; offset += 16) {
            v16si vec1 = *(v16si*)&global_ints[offset];
            v16si vec2 = *(v16si*)&global_ints[offset + 16];
            v16si vec3 = *(v16si*)&global_ints[offset + 32];
            v16si vec4 = *(v16si*)&global_ints[offset + 48];
            v16si vec5 = *(v16si*)&global_ints[offset + 64];
            v16si vec6 = *(v16si*)&global_ints[offset + 80];
            
            v16si result = shuffle_16si_mega(vec1, vec2, vec3, vec4,
                                            vec5, vec6, (int*)mask_source);
            
            for (int i = 0; i < 16; i++) {
                int_acc[offset + i] += result[i];
            }
        }
#endif
        
        /* SSE2 fallback for wider compatibility */
#ifdef __SSE2__
        if (iter % 3 == 0) {
            for (int offset = 0; offset < ARRAY_SIZE - 16; offset += 4) {
                v4si sse_vec1 = *(v4si*)&global_ints[offset];
                v4si sse_vec2 = *(v4si*)&global_ints[offset + 4];
                
                /* Simple shuffle to ensure some code generation */
                v4si mask = {3, 2, 1, 0};
                v4si result = __builtin_shuffle(sse_vec1, sse_vec2, mask);
                
                for (int i = 0; i < 4; i++) {
                    int_acc[offset + i] += result[i];
                }
            }
        }
#endif
    }
    
    /* Compute final checksum */
    int64_t int_sum = 0;
    float float_sum = 0.0f;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_sum += int_acc[i];
        float_sum += float_acc[i];
    }
    
    /* Also include volatile buffers in checksum */
    for (int i = 0; i < 32; i++) {
        int_sum += volatile_buffer[i];
        float_sum += volatile_float_buffer[i];
    }
    
    printf("Checksum - Integer: %lld, Float: %f\n", 
           (long long)int_sum, float_sum);
    
    return 0;
}
