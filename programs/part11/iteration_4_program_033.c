#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef int64_t int64x8_t __attribute__((vector_size(64)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Function to create non-constant mask vectors */
static inline int32x16_t create_complex_mask(volatile int offset) {
    int32x16_t mask = {0};
    for (int i = 0; i < 16; i++) {
        mask[i] = (i + offset) % 32;
    }
    return mask;
}

/* Function with conditional mask generation */
static inline int32x16_t create_conditional_mask(volatile int condition) {
    int32x16_t mask;
    if (condition & 1) {
        for (int i = 0; i < 16; i++) {
            mask[i] = (i * 3) % 32;
        }
    } else {
        for (int i = 0; i < 16; i++) {
            mask[i] = (i * 5 + 7) % 32;
        }
    }
    return mask;
}

int main(void) {
    /* Initialize large arrays with pseudo-random data */
    int32_t data_a[256], data_b[256], data_c[256];
    float float_data[256];
    
    for (int i = 0; i < 256; i++) {
        data_a[i] = (int32_t)lcg_rand();
        data_b[i] = (int32_t)lcg_rand();
        data_c[i] = (int32_t)lcg_rand();
        float_data[i] = (float)lcg_rand() / (float)UINT32_MAX;
    }
    
    /* Cast to various vector types */
    int32x16_t *va = (int32x16_t *)data_a;
    int32x16_t *vb = (int32x16_t *)data_b;
    int32x16_t *vc = (int32x16_t *)data_c;
    float32x16_t *vf = (float32x16_t *)float_data;
    
    /* Result storage */
    int32x16_t results[4];
    float32x16_t float_results[2];
    
    volatile int control = 0;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 1: Complex shuffle with computed mask vector */
    /* This should generate many operands due to the complex shuffle */
    for (int iter = 0; iter < 100; iter++) {
        control = iter;
        
        /* Create non-constant mask using volatile variable */
        int32x16_t mask1 = create_complex_mask(control);
        
        /* Complex shuffle operation that may require many operands */
        results[0] = __builtin_shuffle(va[0], vb[0], mask1);
        
        /* Another shuffle with different sources */
        int32x16_t mask2 = create_conditional_mask(control);
        results[1] = __builtin_shuffle(vb[0], vc[0], mask2);
        
        /* Chain shuffles together */
        results[2] = __builtin_shuffle(results[0], results[1], mask1);
    }
    
    asm volatile("" ::: "memory");
    
    /* KERNEL 2: Chain of shuffles with mixed types */
    /* This creates dependency chains that increase operand count */
    for (int iter = 0; iter < 50; iter++) {
        control = iter * 3;
        
        /* Create multiple mask vectors */
        int32x16_t m1 = create_complex_mask(control);
        int32x16_t m2 = create_complex_mask(control + 1);
        int32x16_t m3 = create_complex_mask(control + 2);
        
        /* Chain of vector operations */
        int32x16_t t1 = __builtin_shuffle(va[1], vb[1], m1);
        int32x16_t t2 = __builtin_shuffle(vb[1], vc[1], m2);
        int32x16_t t3 = __builtin_shuffle(t1, t2, m3);
        
        /* Mix with float vectors */
        float32x16_t float_mask = (float32x16_t)m1;
        float_results[0] = __builtin_shuffle(vf[0], vf[1], 
            (int32x16_t){0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});
        
        /* Another operation to increase operand usage */
        results[3] = t3 + (int32x16_t)float_results[0];
    }
    
    asm volatile("" ::: "memory");
    
    /* KERNEL 3: Conditional vector permutations */
    /* Uses ternary operator with vector shuffle results */
    for (int iter = 0; iter < 30; iter++) {
        control = iter;
        
        int32x16_t mask_a = create_complex_mask(control);
        int32x16_t mask_b = create_conditional_mask(control);
        
        /* Conditional selection between two different shuffle results */
        int32x16_t shuffle_a = __builtin_shuffle(va[2], vb[2], mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(vb[2], vc[2], mask_b);
        
        int32x16_t selected = (control & 2) ? shuffle_a : shuffle_b;
        
        /* Use selected result in another operation */
        float_results[1] = (float32x16_t)selected * 0.5f;
    }
    
    asm volatile("" ::: "memory");
    
    /* KERNEL 4: Inline assembly with many vector operands */
    /* This directly creates many operands for the RTL expander */
#ifdef __x86_64__
    for (int iter = 0; iter < 10; iter++) {
        int32x8_t v1, v2, v3, v4, v5, v6, v7, v8;
        
        /* Load data into vectors */
        v1 = ((int32x8_t *)data_a)[iter];
        v2 = ((int32x8_t *)data_b)[iter];
        v3 = ((int32x8_t *)data_c)[iter];
        v4 = v1 + v2;
        v5 = v2 + v3;
        v6 = v3 + v1;
        v7 = v4 + v5;
        v8 = v5 + v6;
        
        /* Inline assembly with many vector operands */
        asm volatile(
            /* Hypothetical multi-operand vector operation */
            "vpaddd %[v4], %[v1], %[v2]\n\t"
            "vpaddd %[v5], %[v2], %[v3]\n\t"
            "vpaddd %[v6], %[v3], %[v1]\n\t"
            "vpaddd %[v7], %[v4], %[v5]\n\t"
            "vpaddd %[v8], %[v5], %[v6]"
            : [v4] "+x" (v4), [v5] "+x" (v5), [v6] "+x" (v6),
              [v7] "+x" (v7), [v8] "+x" (v8)
            : [v1] "x" (v1), [v2] "x" (v2), [v3] "x" (v3)
            : "memory"
        );
        
        /* Store results back */
        ((int32x8_t *)data_a)[iter] = v7;
        ((int32x8_t *)data_b)[iter] = v8;
    }
#endif
    
    /* Target-specific builtins for different architectures */
#ifdef __ARM_NEON
    {
        typedef int32_t int32x4_t __attribute__((vector_size(16)));
        int32x4_t neon_vec = {1, 2, 3, 4};
        /* Use ARM-specific shuffle builtin */
        int32x4_t rev = __builtin_neon_vrev64q_s32(neon_vec);
        (void)rev;
    }
#endif
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            checksum += results[i][j];
        }
    }
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 16; j++) {
            checksum += (int64_t)float_results[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
