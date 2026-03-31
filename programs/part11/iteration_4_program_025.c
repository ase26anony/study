#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef int32_t int32x4_t __attribute__((vector_size(16)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef float float32x8_t __attribute__((vector_size(32)));
typedef double float64x8_t __attribute__((vector_size(64)));
typedef double float64x4_t __attribute__((vector_size(32)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize array with pseudo-random data */
static void init_array(int32_t *arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = (int32_t)(lcg_rand() % 1000);
    }
}

/* Complex shuffle mask computation - prevents constant propagation */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control + i * 3) % 32;
        if (i % 4 == 0) mask_data[i] += control % 5;
        if (i % 3 == 0) mask_data[i] -= control % 7;
    }
    
    /* Additional non-linear transformation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (mask_data[i] * 13 + 7) % 32;
        mask_data[i] = mask_data[i] < 0 ? 31 - mask_data[i] : mask_data[i];
    }
    
    return mask;
}

/* Another mask computation with different pattern */
static int32x16_t compute_alternate_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control * i + 17) % 32;
        mask_data[i] ^= (control << 3);
        mask_data[i] = (mask_data[i] & 31) | ((i % 2) ? 16 : 0);
    }
    
    return mask;
}

int main(void) {
    /* Large arrays for vector operations */
    int32_t array_a[256];
    int32_t array_b[256];
    int32_t array_c[256];
    float float_array[256];
    double double_array[256];
    
    /* Initialize arrays */
    init_array(array_a, 256);
    init_array(array_b, 256);
    init_array(array_c, 256);
    
    for (int i = 0; i < 256; i++) {
        float_array[i] = (float)(lcg_rand() % 1000) / 10.0f;
        double_array[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
    
    /* Volatile control variables to prevent compile-time optimization */
    volatile int control1 = 42;
    volatile int control2 = 73;
    volatile int control3 = 19;
    
    /* Result vectors */
    int32x16_t results[4];
    float32x16_t float_results[2];
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /*******************************************************************
     * KERNEL 1: Complex shuffle with computed mask
     * This should generate many operands during RTL expansion
     *******************************************************************/
    {
        /* Load vectors from arrays */
        int32x16_t vec_a = *(int32x16_t*)&array_a[0];
        int32x16_t vec_b = *(int32x16_t*)&array_a[16];
        int32x16_t vec_c = *(int32x16_t*)&array_b[0];
        
        /* Compute complex mask - prevents constant propagation */
        int32x16_t mask1 = compute_complex_mask(control1);
        int32x16_t mask2 = compute_alternate_mask(control2);
        
        /* Complex shuffle chain - each operation adds to operand count */
        int32x16_t temp1 = __builtin_shuffle(vec_a, vec_b, mask1);
        int32x16_t temp2 = __builtin_shuffle(vec_c, temp1, mask2);
        
        /* Another shuffle with mixed sources */
        int32x16_t mask3;
        int32_t *mask3_data = (int32_t*)&mask3;
        for (int i = 0; i < 16; i++) {
            mask3_data[i] = (control3 + i * 5) % 48;
            if (mask3_data[i] >= 32) mask3_data[i] = 63 - mask3_data[i];
        }
        
        /* This shuffle with 3 vectors and computed mask may require
         * many operands during expansion */
        results[0] = __builtin_shuffle(vec_a, vec_b, vec_c, mask3);
        
        /* Store intermediate results */
        results[1] = temp2;
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /*******************************************************************
     * KERNEL 2: Chain of shuffles with mixed types
     * Chaining increases operand count in intermediate RTL
     *******************************************************************/
    {
        /* Load float vectors */
        float32x16_t fvec_a = *(float32x16_t*)&float_array[0];
        float32x16_t fvec_b = *(float32x16_t*)&float_array[16];
        float32x16_t fvec_c = *(float32x16_t*)&float_array[32];
        
        /* Create complex mask for float shuffle */
        int32x16_t float_mask;
        int32_t *fmask_data = (int32_t*)&float_mask;
        for (int i = 0; i < 16; i++) {
            fmask_data[i] = (control1 * i + control2) % 32;
            fmask_data[i] = (fmask_data[i] + (i % 8) * 4) % 32;
        }
        
        /* Chain of float shuffles - each result feeds into next */
        float32x16_t ftemp1 = __builtin_shuffle(fvec_a, fvec_b, float_mask);
        
        /* Modify mask based on previous result (data-dependent) */
        for (int i = 0; i < 16; i++) {
            fmask_data[i] = (fmask_data[i] + 
                           (int)(((float*)&ftemp1)[i] * 10)) % 32;
        }
        
        float32x16_t ftemp2 = __builtin_shuffle(fvec_c, ftemp1, float_mask);
        
        /* Another mask variation */
        for (int i = 0; i < 16; i++) {
            fmask_data[i] = (fmask_data[i] * 7 + 3) % 32;
            fmask_data[i] ^= (control3 << (i % 4));
        }
        
        float_results[0] = __builtin_shuffle(ftemp1, ftemp2, float_mask);
        
        /* Convert float result to int for checksum */
        int32x16_t *int_view = (int32x16_t*)&float_results[0];
        results[2] = *int_view;
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /*******************************************************************
     * KERNEL 3: Conditional vector permutations
     * Uses ?: operator on vectors with shuffle results
     *******************************************************************/
    {
        int32x16_t vec_d = *(int32x16_t*)&array_c[0];
        int32x16_t vec_e = *(int32x16_t*)&array_c[16];
        int32x16_t vec_f = *(int32x16_t*)&array_c[32];
        
        /* Compute two different masks */
        int32x16_t mask_a = compute_complex_mask(control1 + control2);
        int32x16_t mask_b = compute_alternate_mask(control1 - control2);
        
        /* Generate two different shuffle results */
        int32x16_t shuffle_a = __builtin_shuffle(vec_d, vec_e, mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(vec_e, vec_f, mask_b);
        
        /* Conditional selection between shuffle results */
        int32x16_t selector;
        int32_t *sel_data = (int32_t*)&selector;
        for (int i = 0; i < 16; i++) {
            sel_data[i] = (control3 + i) % 100 > 50 ? -1 : 0;
        }
        
        /* This conditional operation with vector selects may require
         * expanding both shuffle operations before selection */
        results[3] = selector ? shuffle_a : shuffle_b;
        
        /* Additional complex operation mixing results */
        int32x16_t mask_c;
        int32_t *mask_c_data = (int32_t*)&mask_c;
        for (int i = 0; i < 16; i++) {
            mask_c_data[i] = ((int32_t*)&results[0])[i] % 32;
            mask_c_data[i] += ((int32_t*)&results[1])[i] % 32;
            mask_c_data[i] %= 32;
        }
        
        /* Final complex shuffle with many input vectors */
        int32x16_t final_shuffle = __builtin_shuffle(
            results[0], results[1], results[2], results[3], mask_c);
        
        /* Store back to array to prevent elimination */
        *(int32x16_t*)&array_a[64] = final_shuffle;
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /*******************************************************************
     * KERNEL 4: Inline assembly with many vector operands
     * Directly specifies many input/output operands
     *******************************************************************/
    {
        int32x8_t vec1 = *(int32x8_t*)&array_a[0];
        int32x8_t vec2 = *(int32x8_t*)&array_a[8];
        int32x8_t vec3 = *(int32x8_t*)&array_a[16];
        int32x8_t vec4 = *(int32x8_t*)&array_a[24];
        int32x8_t vec5 = *(int32x8_t*)&array_b[0];
        int32x8_t vec6 = *(int32x8_t*)&array_b[8];
        
        int32x8_t out1, out2, out3, out4;
        
        /* Inline assembly with many vector operands */
        asm volatile(
            "# Complex vector operation with many operands\n\t"
            "mov %0, %1\n\t"
            "mov %2, %3\n\t"
            "mov %4, %5\n\t"
            "mov %6, %7"
            : "=v"(out1), "=v"(out2), "=v"(out3), "=v"(out4)
            : "v"(vec1), "v"(vec2), "v"(vec3), "v"(vec4),
              "v"(vec5), "v"(vec6)
            : "memory"
        );
        
        /* Use results to prevent elimination */
        *(int32x8_t*)&array_c[64] = out1;
        *(int32x8_t*)&array_c[72] = out2;
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /*******************************************************************
     * Architecture-specific builtins (conditional compilation)
     *******************************************************************/
#ifdef __x86_64__
    {
        int32x4_t x86_vec1 = *(int32x4_t*)&array_a[0];
        int32x4_t x86_vec2 = *(int32x4_t*)&array_a[4];
        int32x4_t x86_vec3 = *(int32x4_t*)&array_a[8];
        
        /* Use x86-specific shuffle intrinsic */
        int32x4_t x86_result;
        /* __builtin_ia32_pshufd takes immediate mask, so we use variable
         * control to prevent constant propagation */
        int mask = control1 % 255;
        x86_result = __builtin_ia32_pshufd(x86_vec1, mask);
        
        /* Chain with another operation */
        mask = (control2 + mask) % 255;
        x86_result = __builtin_ia32_pshufd(x86_result, mask);
        
        *(int32x4_t*)&array_b[64] = x86_result;
    }
#endif
    
#ifdef __ARM_NEON
    {
        int32x4_t neon_vec1 = *(int32x4_t*)&array_a[0];
        int32x4_t neon_vec2 = *(int32x4_t*)&array_a[4];
        
        /* ARM NEON reversal intrinsic */
        int32x4_t neon_result = __builtin_neon_vrev64q_s32(neon_vec1);
        
        /* Additional NEON operation */
        neon_result = __builtin_neon_vrev32q_s32(neon_result);
        
        *(int32x4_t*)&array_b[68] = neon_result;
    }
#endif
    
    /*******************************************************************
     * Compute checksum to prevent dead code elimination
     *******************************************************************/
    uint64_t checksum = 0;
    
    /* Horizontal addition of result vectors */
    for (int i = 0; i < 4; i++) {
        int32_t *data = (int32_t*)&results[i];
        for (int j = 0; j < 16; j++) {
            checksum += (uint64_t)data[j];
        }
    }
    
    /* Add array data to checksum */
    for (int i = 0; i < 100; i++) {
        checksum += (uint64_t)array_a[i];
        checksum += (uint64_t)array_b[i];
        checksum += (uint64_t)array_c[i];
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
