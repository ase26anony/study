#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
#define ARRAY_SIZE 512
static int32_t global_int_array[ARRAY_SIZE];
static float global_float_array[ARRAY_SIZE];
static volatile int32_t volatile_buffer[ARRAY_SIZE];
static volatile float volatile_float_buffer[ARRAY_SIZE];

/* Vector type definitions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef float v8sf __attribute__((vector_size(32)));        /* 256-bit float */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */
typedef double v4df __attribute__((vector_size(32)));       /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));       /* 512-bit double */

/* Accumulator arrays */
static v8si int_acc_256[4];
static v16si int_acc_512[4];
static v8sf float_acc_256[4];
static v16sf float_acc_512[4];

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Simple deterministic pseudo-random sequence */
        int32_t val = (i * 1103515245 + seed) & 0x7fffffff;
        global_int_array[i] = val % 1000;
        global_float_array[i] = (float)(val % 1000) * 0.1f;
        
        /* Initialize volatile buffers too */
        volatile_buffer[i] = global_int_array[i];
        volatile_float_buffer[i] = global_float_array[i];
    }
    
    /* Initialize accumulators to zero */
    memset(int_acc_256, 0, sizeof(int_acc_256));
    memset(int_acc_512, 0, sizeof(int_acc_512));
    memset(float_acc_256, 0, sizeof(float_acc_256));
    memset(float_acc_512, 0, sizeof(float_acc_512));
}

/* Function 1: Large integer shuffle with 10+ operands */
#ifdef __AVX2__
void shuffle_large_int_vectors(int idx, volatile int mask_source) {
    /* Load data using volatile to prevent constant folding */
    v16si vec1 = *(v16si*)&global_int_array[idx * 16];
    v16si vec2 = *(v16si*)&global_int_array[idx * 16 + 16];
    v16si vec3 = *(v16si*)&global_int_array[idx * 16 + 32];
    
    /* Create control mask from volatile source - prevents compile-time folding */
    int mask_val = mask_source % 16;
    v16si control_mask = {
        mask_val, mask_val + 1, mask_val + 2, mask_val + 3,
        mask_val + 4, mask_val + 5, mask_val + 6, mask_val + 7,
        mask_val + 8, mask_val + 9, mask_val + 10, mask_val + 11,
        mask_val + 12, mask_val + 13, mask_val + 14, mask_val + 15
    };
    
    /* Force memory barrier */
    asm volatile("" ::: "memory");
    
    /* Complex control flow to stress expander */
    if (mask_val & 1) {
        /* This shuffle uses 11 operands: 3 input vectors + 8-element control mask */
        v16si result = __builtin_shufflevector(vec1, vec2, vec3,
            0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
        
        /* Perform arithmetic to prevent dead code elimination */
        result = result + control_mask;
        
        /* Store to accumulator */
        int_acc_512[idx % 4] = int_acc_512[idx % 4] + result;
    } else {
        /* Alternative path with different shuffle pattern */
        v16si result = __builtin_shufflevector(vec2, vec3, vec1,
            8, 24, 9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31);
        
        result = result - control_mask;
        int_acc_512[idx % 4] = int_acc_512[idx % 4] + result;
    }
    
    /* Store intermediate result to volatile memory */
    *(volatile v16si*)&volatile_buffer[idx * 16] = int_acc_512[idx % 4];
}
#endif

/* Function 2: Mixed float/double shuffle with complex control flow */
#ifdef __AVX512F__
void shuffle_mixed_float_vectors(int idx, volatile int pattern) {
    v8df dvec1 = *(v8df*)&global_float_array[idx * 8];
    v8df dvec2 = *(v8df*)&global_float_array[idx * 8 + 8];
    v16sf fvec1 = *(v16sf*)&global_float_array[idx * 16];
    
    /* Create mask from volatile pattern */
    int mask_base = pattern % 8;
    v8df dcontrol = {mask_base, mask_base+1, mask_base+2, mask_base+3,
                     mask_base+4, mask_base+5, mask_base+6, mask_base+7};
    
    /* Switch statement to create complex CFG */
    switch (pattern & 0x7) {
        case 0:
        case 1:
        case 2: {
            /* Large shuffle with 10 operands */
            v8df result = __builtin_shuffle(dvec1, dvec2, 
                (v8di){0, 8, 1, 9, 2, 10, 3, 11});
            
            /* Convert and mix with float vector */
            v16sf fresult = __builtin_convertvector(result, v16sf);
            v16sf mixed = __builtin_shuffle(fresult, fvec1,
                (v16si){0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23});
            
            float_acc_512[idx % 4] = float_acc_512[idx % 4] + mixed;
            break;
        }
        case 3:
        case 4: {
            /* Different shuffle pattern */
            v8df result = __builtin_shuffle(dvec2, dvec1,
                (v8di){4, 12, 5, 13, 6, 14, 7, 15});
            
            result = result * dcontrol;
            
            /* Store to volatile */
            *(volatile v8df*)&volatile_float_buffer[idx * 8] = result;
            break;
        }
        default: {
            /* Complex nested shuffle */
            v16sf temp = __builtin_shuffle(fvec1, fvec1,
                (v16si){15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0});
            
            float_acc_512[idx % 4] = float_acc_512[idx % 4] + temp;
            break;
        }
    }
}
#endif

/* Function 3: SSE2-compatible shuffle with many operands */
#ifdef __SSE2__
void shuffle_sse2_vectors(int idx, volatile int mask) {
    typedef int32_t v4si __attribute__((vector_size(16)));
    v4si vec1 = *(v4si*)&global_int_array[idx * 4];
    v4si vec2 = *(v4si*)&global_int_array[idx * 4 + 4];
    v4si vec3 = *(v4si*)&global_int_array[idx * 4 + 8];
    v4si vec4 = *(v4si*)&global_int_array[idx * 4 + 12];
    
    /* Create control from volatile mask */
    int m = mask % 4;
    v4si control = {m, m+1, m+2, m+3};
    
    /* Loop with conditional shuffle execution */
    for (int i = 0; i < 4; i++) {
        if ((m + i) & 1) {
            /* Shuffle with multiple vector inputs - may expand to many operands */
            v4si result = __builtin_shufflevector(vec1, vec2, vec3, vec4,
                0, 4, 8, 12);
            result = result + control;
            
            /* Accumulate */
            int_acc_256[0] = int_acc_256[0] + __builtin_convertvector(result, v8si);
        }
    }
    
    /* Volatile store */
    *(volatile v4si*)&volatile_buffer[idx * 4] = *(v4si*)&int_acc_256[0];
}
#endif

/* Function 4: AVX2 256-bit integer shuffle chain */
#ifdef __AVX2__
void shuffle_avx2_chain(int idx, volatile int selector) {
    v8si vec1 = *(v8si*)&global_int_array[idx * 8];
    v8si vec2 = *(v8si*)&global_int_array[idx * 8 + 8];
    v8si vec3 = *(v8si*)&global_int_array[idx * 8 + 16];
    
    /* Multiple shuffle stages with conditional execution */
    v8si stage1, stage2, stage3;
    
    if (selector & 0x1) {
        stage1 = __builtin_shuffle(vec1, vec2, 
            (v8si){0, 8, 1, 9, 2, 10, 3, 11});
    } else {
        stage1 = __builtin_shuffle(vec2, vec1,
            (v8si){4, 12, 5, 13, 6, 14, 7, 15});
    }
    
    if (selector & 0x2) {
        stage2 = __builtin_shuffle(stage1, vec3,
            (v8si){0, 8, 2, 10, 4, 12, 6, 14});
    } else {
        stage2 = __builtin_shuffle(vec3, stage1,
            (v8si){1, 9, 3, 11, 5, 13, 7, 15});
    }
    
    /* Final complex shuffle with many operands */
    stage3 = __builtin_shuffle(stage1, stage2, vec3,
        (v8si){0, 8, 16, 1, 9, 17, 2, 10});
    
    /* Arithmetic and accumulation */
    stage3 = stage3 * (selector & 0xFF);
    int_acc_256[idx % 4] = int_acc_256[idx % 4] + stage3;
}
#endif

/* Main function with architecture-specific dispatch */
int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    /* Volatile variable to prevent compile-time optimization */
    volatile int runtime_mask = seed;
    
    /* Main processing loop */
    for (int iter = 0; iter < 10; iter++) {
        int idx = iter * 3 % 16;  /* Vary index */
        
        /* Call architecture-specific functions */
#ifdef __SSE2__
        shuffle_sse2_vectors(idx, runtime_mask + iter);
#endif
        
#ifdef __AVX2__
        shuffle_large_int_vectors(idx, runtime_mask + iter);
        shuffle_avx2_chain(idx, runtime_mask ^ iter);
#endif
        
#ifdef __AVX512F__
        shuffle_mixed_float_vectors(idx, runtime_mask * iter);
#endif
        
        /* Update volatile mask to affect control flow */
        runtime_mask = (runtime_mask * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Compute checksum from all accumulators */
    uint64_t checksum = 0;
    
    /* Sum integer accumulators */
    for (int i = 0; i < 4; i++) {
        int32_t *ptr = (int32_t*)&int_acc_256[i];
        for (int j = 0; j < 8; j++) {
            checksum += ptr[j];
        }
    }
    
    for (int i = 0; i < 4; i++) {
        int32_t *ptr = (int32_t*)&int_acc_512[i];
        for (int j = 0; j < 16; j++) {
            checksum += ptr[j];
        }
    }
    
    /* Sum float accumulators */
    for (int i = 0; i < 4; i++) {
        float *ptr = (float*)&float_acc_256[i];
        for (int j = 0; j < 8; j++) {
            checksum += (uint64_t)ptr[j];
        }
    }
    
    for (int i = 0; i < 4; i++) {
        float *ptr = (float*)&float_acc_512[i];
        for (int j = 0; j < 16; j++) {
            checksum += (uint64_t)ptr[j];
        }
    }
    
    /* Also include volatile buffers in checksum */
    for (int i = 0; i < 32; i++) {
        checksum += volatile_buffer[i];
        checksum += (uint64_t)volatile_float_buffer[i];
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    return 0;
}
