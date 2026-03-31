#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
static volatile int global_seed;
static int global_data_int[512];
static float global_data_float[512];
static int accumulator_int[512];
static float accumulator_float[512];

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

/* Initialize data with deterministic pseudo-random sequence */
void init_data(int seed) {
    global_seed = seed;
    for (int i = 0; i < 512; i++) {
        global_data_int[i] = (i * seed + 12345) % 1000;
        global_data_float[i] = ((i * seed + 6789) % 1000) * 0.001f;
        accumulator_int[i] = 0;
        accumulator_float[i] = 0.0f;
    }
}

/* Function using __builtin_shuffle with many operands (10+ total) */
#ifdef __AVX2__
static void shuffle_large_int_vector(v16si *acc, int offset, volatile int mask_idx) {
    v16si a, b, c, d;
    
    /* Load data from global arrays */
    for (int i = 0; i < 16; i++) {
        a[i] = global_data_int[offset + i];
        b[i] = global_data_int[offset + i + 16];
        c[i] = global_data_int[offset + i + 32];
        d[i] = global_data_int[offset + i + 48];
    }
    
    /* Complex control flow to prevent optimization */
    if (mask_idx & 1) {
        /* Create a complex shuffle mask using runtime values */
        v16si mask;
        for (int i = 0; i < 16; i++) {
            mask[i] = (mask_idx + i) % 32;  /* Runtime-dependent indices */
        }
        
        /* This shuffle uses 3 operands: a, b, mask (but internally expands to many) */
        v16si shuffled = __builtin_shuffle(a, b, mask);
        
        /* Another shuffle with different inputs */
        v16si mask2;
        for (int i = 0; i < 16; i++) {
            mask2[i] = (mask_idx * 2 + i) % 32;
        }
        
        v16si shuffled2 = __builtin_shuffle(c, d, mask2);
        
        /* Combine results */
        v16si result = shuffled + shuffled2;
        
        /* Store to accumulator */
        for (int i = 0; i < 16; i++) {
            (*acc)[i] += result[i];
        }
    } else {
        /* Alternative path with different shuffle pattern */
        v16si mask3;
        for (int i = 0; i < 16; i++) {
            mask3[i] = (mask_idx * 3 + i * 2) % 64;
        }
        
        /* Use __builtin_shufflevector which can take many arguments */
        /* This creates a 32-element shuffle from 4 input vectors */
        v16si combined = __builtin_shufflevector(a, b, c, d,
            0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23,
            8, 24, 9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31);
        
        /* Store result */
        for (int i = 0; i < 16; i++) {
            (*acc)[i] += combined[i];
        }
    }
}
#endif

#ifdef __AVX512F__
/* Function specifically designed to require many operands */
static void complex_double_shuffle(v8df *acc, int offset, volatile int pattern) {
    v8df v1, v2, v3, v4;
    
    /* Load data */
    for (int i = 0; i < 8; i++) {
        v1[i] = global_data_float[offset + i] * 2.0;
        v2[i] = global_data_float[offset + i + 8] * 3.0;
        v3[i] = global_data_float[offset + i + 16] * 4.0;
        v4[i] = global_data_float[offset + i + 24] * 5.0;
    }
    
    /* Switch statement with different shuffle patterns */
    switch (pattern % 4) {
        case 0: {
            /* Complex shuffle pattern requiring many operands */
            v8df mask;
            for (int i = 0; i < 8; i++) {
                mask[i] = (double)((pattern + i * 3) % 16);
            }
            
            /* Nested shuffles */
            v8df t1 = __builtin_shuffle(v1, v2, (v8di)mask);
            v8df t2 = __builtin_shuffle(v3, v4, (v8di)mask);
            v8df result = t1 * t2;
            
            for (int i = 0; i < 8; i++) {
                (*acc)[i] += result[i];
            }
            break;
        }
        case 1: {
            /* Even more complex pattern with __builtin_shufflevector */
            v8df result = __builtin_shufflevector(v1, v2, v3, v4,
                0, 8, 1, 9, 2, 10, 3, 11,
                4, 12, 5, 13, 6, 14, 7, 15);
            
            /* Additional operation to prevent elimination */
            for (int i = 0; i < 8; i++) {
                result[i] = result[i] * (pattern + 1);
            }
            
            for (int i = 0; i < 8; i++) {
                (*acc)[i] += result[i];
            }
            break;
        }
        case 2: {
            /* Pattern that might trigger 11-operand expansion */
            v8df mask1, mask2;
            for (int i = 0; i < 8; i++) {
                mask1[i] = (double)((i + pattern) % 8);
                mask2[i] = (double)((i * 2 + pattern) % 8);
            }
            
            v8df shuffled1 = __builtin_shuffle(v1, v2, (v8di)mask1);
            v8df shuffled2 = __builtin_shuffle(v3, v4, (v8di)mask2);
            
            /* Interleave results */
            v8df result = __builtin_shufflevector(shuffled1, shuffled2,
                0, 8, 1, 9, 2, 10, 3, 11,
                4, 12, 5, 13, 6, 14, 7, 15);
            
            for (int i = 0; i < 8; i++) {
                (*acc)[i] += result[i];
            }
            break;
        }
        default: {
            /* Simple pattern as fallback */
            for (int i = 0; i < 8; i++) {
                (*acc)[i] += v1[i] + v2[i] + v3[i] + v4[i];
            }
            break;
        }
    }
}
#endif

/* Mixed SIMD width operations */
#ifdef __SSE2__
static void mixed_width_shuffles(int offset, volatile int idx) {
    v4si a, b;
    v8si c, d;
    
    /* Load data for different vector sizes */
    for (int i = 0; i < 4; i++) {
        a[i] = global_data_int[offset + i];
        b[i] = global_data_int[offset + i + 4];
    }
    
    for (int i = 0; i < 8; i++) {
        c[i] = global_data_int[offset + i + 8];
        d[i] = global_data_int[offset + i + 16];
    }
    
    /* Shuffle with runtime-dependent mask */
    v4si mask;
    for (int i = 0; i < 4; i++) {
        mask[i] = (idx + i * 2) % 8;
    }
    
    v4si shuffled = __builtin_shuffle(a, b, mask);
    
    /* Convert and combine with larger vectors */
    for (int i = 0; i < 4; i++) {
        accumulator_int[offset + i] += shuffled[i];
    }
    
    /* Another shuffle with larger vectors */
    v8si mask2;
    for (int i = 0; i < 8; i++) {
        mask2[i] = (idx * 3 + i) % 16;
    }
    
    v8si shuffled2 = __builtin_shuffle(c, d, mask2);
    
    for (int i = 0; i < 8; i++) {
        accumulator_int[offset + i + 8] += shuffled2[i];
    }
}
#endif

/* Main test function */
int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_data(seed);
    
    volatile int loop_counter = 0;
    
    /* Main test loop */
    for (int iter = 0; iter < 10; iter++) {
        loop_counter = iter;
        
        #ifdef __AVX2__
        {
            v16si acc_int = {0};
            for (int block = 0; block < 4; block++) {
                shuffle_large_int_vector(&acc_int, block * 64, loop_counter + block);
            }
            /* Store accumulated results */
            for (int i = 0; i < 16; i++) {
                accumulator_int[iter * 16 + i] += acc_int[i];
            }
        }
        #endif
        
        #ifdef __AVX512F__
        {
            v8df acc_double = {0.0};
            for (int block = 0; block < 4; block++) {
                complex_double_shuffle(&acc_double, block * 32, loop_counter + block * 2);
            }
            /* Store accumulated results */
            for (int i = 0; i < 8; i++) {
                accumulator_float[iter * 8 + i] += (float)acc_double[i];
            }
        }
        #endif
        
        #ifdef __SSE2__
        {
            for (int block = 0; block < 8; block++) {
                mixed_width_shuffles(block * 32, loop_counter + block * 3);
            }
        }
        #endif
        
        /* Volatile memory operations to prevent optimization */
        volatile int dummy = global_data_int[iter];
        (void)dummy;
    }
    
    /* Compute checksum to prevent dead code elimination */
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
