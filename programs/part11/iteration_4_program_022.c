#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));
typedef int64_t int64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize array with pseudo-random values */
static void init_array(void *array, size_t size) {
    uint32_t *ptr = (uint32_t *)array;
    size_t words = size / sizeof(uint32_t);
    for (size_t i = 0; i < words; i++) {
        ptr[i] = lcg_rand();
    }
}

/* Horizontal sum for checksum */
static int64_t horizontal_sum_i32(int32x16_t v) {
    int64_t sum = 0;
    int32_t *p = (int32_t *)&v;
    for (int i = 0; i < 16; i++) {
        sum += p[i];
    }
    return sum;
}

static double horizontal_sum_f64(float64x8_t v) {
    double sum = 0.0;
    double *p = (double *)&v;
    for (int i = 0; i < 8; i++) {
        sum += p[i];
    }
    return sum;
}

int main(void) {
    /* Large arrays for vector operations */
    int32_t data_i32[256];
    float data_f32[256];
    double data_f64[128];
    
    /* Initialize with pseudo-random data */
    init_array(data_i32, sizeof(data_i32));
    init_array(data_f32, sizeof(data_f32));
    init_array(data_f64, sizeof(data_f64));
    
    /* Volatile variables to prevent constant propagation */
    volatile int mask_seed = lcg_rand() % 100;
    volatile int permutation_mode = lcg_rand() % 3;
    
    /* Cast array segments to vector types */
    int32x16_t *vec_i32_a = (int32x16_t *)&data_i32[0];
    int32x16_t *vec_i32_b = (int32x16_t *)&data_i32[16];
    int32x16_t *vec_i32_c = (int32x16_t *)&data_i32[32];
    int32x16_t *vec_i32_d = (int32x16_t *)&data_i32[48];
    
    float32x16_t *vec_f32_a = (float32x16_t *)&data_f32[0];
    float32x16_t *vec_f32_b = (float32x16_t *)&data_f32[16];
    
    float64x8_t *vec_f64_a = (float64x8_t *)&data_f64[0];
    float64x8_t *vec_f64_b = (float64x8_t *)&data_f64[8];
    
    /* Result vectors */
    int32x16_t result_i32[4];
    float32x16_t result_f32[2];
    float64x8_t result_f64[2];
    
    int64_t checksum = 0;
    
    /* KERNEL 1: Complex shuffle with computed mask vector */
    {
        /* Create a non-constant mask using arithmetic */
        int32x16_t mask;
        int32_t *mask_ptr = (int32_t *)&mask;
        for (int i = 0; i < 16; i++) {
            mask_ptr[i] = (i + mask_seed) % 32;
        }
        
        /* Complex shuffle with 3 vectors and mask - may require many operands */
        result_i32[0] = __builtin_shuffle(*vec_i32_a, *vec_i32_b, mask);
        
        /* Another shuffle with different sources */
        for (int i = 0; i < 16; i++) {
            mask_ptr[i] = (i * 3 + mask_seed) % 32;
        }
        result_i32[1] = __builtin_shuffle(*vec_i32_c, *vec_i32_d, mask);
        
        checksum += horizontal_sum_i32(result_i32[0]);
        checksum += horizontal_sum_i32(result_i32[1]);
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 2: Chain of shuffles accumulating operand count */
    {
        int32x16_t temp1, temp2, temp3;
        
        /* First shuffle */
        int32x16_t mask1;
        int32_t *mask1_ptr = (int32_t *)&mask1;
        for (int i = 0; i < 16; i++) {
            mask1_ptr[i] = (i * 2 + mask_seed) % 32;
        }
        temp1 = __builtin_shuffle(*vec_i32_a, *vec_i32_b, mask1);
        
        /* Second shuffle using result of first */
        int32x16_t mask2;
        int32_t *mask2_ptr = (int32_t *)&mask2;
        for (int i = 0; i < 16; i++) {
            mask2_ptr[i] = (i * 5 + mask_seed) % 32;
        }
        temp2 = __builtin_shuffle(temp1, *vec_i32_c, mask2);
        
        /* Third shuffle chaining more results */
        int32x16_t mask3;
        int32_t *mask3_ptr = (int32_t *)&mask3;
        for (int i = 0; i < 16; i++) {
            mask3_ptr[i] = (i * 7 + mask_seed) % 32;
        }
        temp3 = __builtin_shuffle(temp2, *vec_i32_d, mask3);
        
        /* Final shuffle with all accumulated vectors */
        int32x16_t mask4;
        int32_t *mask4_ptr = (int32_t *)&mask4;
        for (int i = 0; i < 16; i++) {
            mask4_ptr[i] = (i * 11 + mask_seed) % 32;
        }
        result_i32[2] = __builtin_shuffle(temp3, temp1, mask4);
        
        checksum += horizontal_sum_i32(result_i32[2]);
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 3: Conditional vector permutation */
    {
        /* Create two different mask vectors conditionally */
        int32x16_t mask_a, mask_b;
        int32_t *mask_a_ptr = (int32_t *)&mask_a;
        int32_t *mask_b_ptr = (int32_t *)&mask_b;
        
        for (int i = 0; i < 16; i++) {
            mask_a_ptr[i] = (i + permutation_mode) % 32;
            mask_b_ptr[i] = (i * 3 - permutation_mode) % 32;
        }
        
        /* Conditional selection between two shuffle results */
        int32x16_t shuffle_a = __builtin_shuffle(*vec_i32_a, *vec_i32_b, mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(*vec_i32_c, *vec_i32_d, mask_b);
        
        /* Use conditional operator on vectors */
        int32x16_t selector;
        int32_t *sel_ptr = (int32_t *)&selector;
        for (int i = 0; i < 16; i++) {
            sel_ptr[i] = (mask_seed >> (i & 7)) & 1 ? -1 : 0;
        }
        
        /* This creates complex conditional vector operations */
        result_i32[3] = selector ? shuffle_a : shuffle_b;
        
        checksum += horizontal_sum_i32(result_i32[3]);
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 4: Mixed vector types and widths */
    {
        /* Shuffle with float vectors */
        int32x16_t float_mask;
        int32_t *fmask_ptr = (int32_t *)&float_mask;
        for (int i = 0; i < 16; i++) {
            fmask_ptr[i] = (i * 2 + mask_seed) % 32;
        }
        
        /* Using shufflevector for more complex patterns */
        result_f32[0] = __builtin_shufflevector(*vec_f32_a, *vec_f32_b, 
            0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30);
        
        /* Another shufflevector with different pattern */
        result_f32[1] = __builtin_shufflevector(*vec_f32_a, *vec_f32_b,
            1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31);
        
        /* Double precision shuffles */
        int64x8_t double_mask;
        int64_t *dmask_ptr = (int64_t *)&double_mask;
        for (int i = 0; i < 8; i++) {
            dmask_ptr[i] = (i + mask_seed) % 16;
        }
        
        /* Shuffle with double vectors */
        result_f64[0] = __builtin_shuffle(*vec_f64_a, *vec_f64_b, 
            (int64x8_t){0, 2, 4, 6, 8, 10, 12, 14});
        
        checksum += (int64_t)horizontal_sum_f64(result_f64[0]);
    }
    
#ifdef __x86_64__
    /* KERNEL 5: Target-specific builtins for x86 */
    {
        /* Use x86-specific shuffle intrinsics if available */
        int32x8_t sse_vec1 = *((int32x8_t *)&data_i32[0]);
        int32x8_t sse_vec2 = *((int32x8_t *)&data_i32[8]);
        
        /* These builtins often require multiple operands */
        #ifdef __SSE4_1__
        /* Simulate complex operation with inline asm */
        int32x8_t sse_result;
        asm volatile (
            "pshufd $0x1B, %1, %0\n\t"
            "paddd %2, %0"
            : "=x"(sse_result)
            : "x"(sse_vec1), "x"(sse_vec2)
            : "memory"
        );
        
        /* Accumulate to checksum */
        int32_t *sr_ptr = (int32_t *)&sse_result;
        for (int i = 0; i < 8; i++) {
            checksum += sr_ptr[i];
        }
        #endif
    }
#endif
    
#ifdef __aarch64__
    /* KERNEL 6: ARM-specific builtins */
    {
        /* ARM NEON operations */
        int32x4_t neon_vec1 = *((int32x4_t *)&data_i32[0]);
        int32x4_t neon_vec2 = *((int32x4_t *)&data_i32[4]);
        
        /* Complex inline asm with multiple operands */
        int32x4_t neon_result;
        asm volatile (
            "rev64 %0.4s, %1.4s\n\t"
            "add %0.4s, %0.4s, %2.4s"
            : "=w"(neon_result)
            : "w"(neon_vec1), "w"(neon_vec2)
            : "memory"
        );
        
        /* Accumulate to checksum */
        int32_t *nr_ptr = (int32_t *)&neon_result;
        for (int i = 0; i < 4; i++) {
            checksum += nr_ptr[i];
        }
    }
#endif
    
    /* KERNEL 7: Loop-dependent vector operations */
    {
        int32x16_t accum = {0};
        
        /* Loop with data-dependent shuffles */
        for (int iter = 0; iter < 4; iter++) {
            /* Compute mask based on loop iteration and volatile seed */
            int32x16_t loop_mask;
            int32_t *lmask_ptr = (int32_t *)&loop_mask;
            for (int i = 0; i < 16; i++) {
                lmask_ptr[i] = (i * iter + mask_seed) % 32;
            }
            
            /* Select source vectors based on iteration */
            int32x16_t *src1, *src2;
            switch (iter % 4) {
                case 0: src1 = vec_i32_a; src2 = vec_i32_b; break;
                case 1: src1 = vec_i32_b; src2 = vec_i32_c; break;
                case 2: src1 = vec_i32_c; src2 = vec_i32_d; break;
                default: src1 = vec_i32_d; src2 = vec_i32_a; break;
            }
            
            /* Perform shuffle */
            int32x16_t shuffled = __builtin_shuffle(*src1, *src2, loop_mask);
            
            /* Accumulate results */
            accum += shuffled;
        }
        
        checksum += horizontal_sum_i32(accum);
    }
    
    /* Final checksum output to prevent dead code elimination */
    printf("Final checksum: %ld\n", (long)checksum);
    
    return 0;
}
