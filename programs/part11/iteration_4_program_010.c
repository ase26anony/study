#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Vector type definitions */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));

/* Volatile control variables to prevent constant propagation */
static volatile int shuffle_mode = 0;
static volatile int permutation_seed = 42;

int main(void) {
    /* Initialize large arrays with pseudo-random data */
    int32_t data_int[256];
    float data_float[256];
    double data_double[128];
    
    for (int i = 0; i < 256; i++) {
        data_int[i] = (int32_t)lcg_rand();
        data_float[i] = (float)lcg_rand() / 1000.0f;
        if (i < 128) {
            data_double[i] = (double)lcg_rand() / 1000.0;
        }
    }
    
    /* Cast array segments to vector types */
    int32x16_t* vec_int16 = (int32x16_t*)data_int;
    int32x8_t* vec_int8 = (int32x8_t*)data_int;
    float32x16_t* vec_float16 = (float32x16_t*)data_float;
    float64x8_t* vec_double8 = (float64x8_t*)data_double;
    
    /* Result vectors */
    int32x16_t result_int16[4];
    float32x16_t result_float16[4];
    float64x8_t result_double8[4];
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 1: Complex shuffle with computed mask vector */
    {
        /* Create a non-constant mask using arithmetic and volatile variables */
        int32x16_t mask;
        for (int i = 0; i < 16; i++) {
            mask[i] = (i * permutation_seed + shuffle_mode) % 32;
        }
        
        /* Shuffle with two source vectors - potentially requiring many operands */
        result_int16[0] = __builtin_shuffle(vec_int16[0], vec_int16[1], mask);
        
        /* Another shuffle with mixed types */
        int32x16_t temp_int = vec_int16[2];
        float32x16_t temp_float = vec_float16[0];
        
        /* Complex expression that might expand to many operands */
        result_int16[1] = __builtin_shuffle(
            temp_int,
            __builtin_convertvector(temp_float, int32x16_t),
            mask + (int32x16_t){0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}
        );
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 2: Chain of shuffles accumulating operand count */
    {
        /* Initial shuffle */
        int32x16_t mask1;
        for (int i = 0; i < 16; i++) {
            mask1[i] = (i + permutation_seed) % 16;
        }
        
        int32x16_t intermediate1 = __builtin_shuffle(vec_int16[3], mask1);
        
        /* Second shuffle using result of first */
        int32x16_t mask2;
        for (int i = 0; i < 16; i++) {
            mask2[i] = (i * 3 + shuffle_mode) % 16;
        }
        
        int32x16_t intermediate2 = __builtin_shuffle(
            intermediate1,
            vec_int16[4],
            mask2
        );
        
        /* Third shuffle chaining more results */
        int32x16_t mask3;
        for (int i = 0; i < 16; i++) {
            mask3[i] = (mask1[i] + mask2[i]) % 32;
        }
        
        result_int16[2] = __builtin_shuffle(
            intermediate2,
            vec_int16[5],
            mask3
        );
        
        /* This chain of operations may require handling many operands during expansion */
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 3: Conditional vector permutation */
    {
        /* Create two different shuffle results */
        int32x16_t mask_a, mask_b;
        for (int i = 0; i < 16; i++) {
            mask_a[i] = (i * 2) % 16;
            mask_b[i] = (i * 3) % 16;
        }
        
        int32x16_t shuffle_a = __builtin_shuffle(vec_int16[6], mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(vec_int16[7], mask_b);
        
        /* Conditional selection between shuffle results */
        int32x16_t condition;
        for (int i = 0; i < 16; i++) {
            condition[i] = (vec_int16[0][i] > 0) ? -1 : 0;
        }
        
        result_int16[3] = condition ? shuffle_a : shuffle_b;
        
        /* Mixed type conditional shuffle */
        float64x8_t mask_double;
        for (int i = 0; i < 8; i++) {
            mask_double[i] = (double)((i + shuffle_mode) % 16);
        }
        
        /* Using __builtin_shufflevector which can take many arguments */
        result_double8[0] = __builtin_shufflevector(
            vec_double8[0], vec_double8[1],
            0, 2, 4, 6, 8, 10, 12, 14
        );
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 4: Inline assembly with many vector operands */
    {
        /* This inline asm block has 10 operands total */
        asm volatile (
            "# Complex vector operation with many operands\n\t"
            "vmovdqa %0, %%ymm0\n\t"
            "vmovdqa %1, %%ymm1\n\t"
            "vmovdqa %2, %%ymm2\n\t"
            "vmovdqa %3, %%ymm3\n\t"
            "# Some hypothetical multi-operand operation\n\t"
            "vpermq $0x1B, %%ymm0, %%ymm4\n\t"
            "vpermq $0x39, %%ymm1, %%ymm5\n\t"
            "vpblendd $0xF0, %%ymm4, %%ymm5, %%ymm6\n\t"
            "vmovdqa %%ymm6, %4\n\t"
            : "=m"(result_int16[0])
            : "m"(vec_int16[0]), "m"(vec_int16[1]), 
              "m"(vec_int16[2]), "m"(vec_int16[3])
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "memory"
        );
    }
    
#ifdef __x86_64__
    /* KERNEL 5: Target-specific builtins for x86 */
    {
        /* Use x86-specific shuffle intrinsics */
        typedef int32_t v8si __attribute__((vector_size(32)));
        v8si x86_vec = {0,1,2,3,4,5,6,7};
        
        /* __builtin_ia32_pshufd takes immediate mask - but we can make it complex */
        for (int i = 0; i < 100; i++) {
            /* Loop-dependent operation prevents optimization */
            int mask = (i + permutation_seed) % 256;
            v8si shuffled = __builtin_ia32_pshufd(x86_vec, mask);
            
            /* Chain with other operations */
            v8si result = shuffled + x86_vec;
            
            /* Use result to prevent elimination */
            if (i == 99) {
                int32x8_t* ptr = (int32x8_t*)&result;
                result_int16[0][0] = (*ptr)[0];
            }
        }
    }
#endif
    
#ifdef __ARM_NEON
    /* KERNEL 6: ARM-specific builtins */
    {
        typedef int32_t int32x4_t __attribute__((vector_size(16)));
        int32x4_t neon_vec = {0,1,2,3};
        
        /* ARM reversal intrinsic */
        int32x4_t reversed = __builtin_neon_vrev64q_s32(neon_vec);
        
        /* Chain with shuffle */
        int32x4_t mask = {3,2,1,0};
        int32x4_t final = __builtin_shuffle(reversed, mask);
        
        /* Store to global to prevent elimination */
        result_int16[0][0] = final[0];
    }
#endif
    
    /* Loop-dependent vector operations to prevent optimization */
    for (int iter = 0; iter < 10; iter++) {
        shuffle_mode = iter;
        
        /* Data-dependent shuffle mask */
        int32x16_t dynamic_mask;
        for (int i = 0; i < 16; i++) {
            dynamic_mask[i] = (vec_int16[iter % 4][i] % 16) + i;
        }
        
        /* Shuffle that depends on loop variable */
        float32x16_t temp_result = __builtin_shuffle(
            vec_float16[iter % 2],
            vec_float16[(iter + 1) % 2],
            __builtin_convertvector(dynamic_mask, int32x16_t)
        );
        
        /* Store result */
        result_float16[iter % 4] = temp_result;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            checksum += result_int16[i][j];
        }
        for (int j = 0; j < 16; j++) {
            checksum += (int64_t)result_float16[i][j];
        }
        if (i < 2) {
            for (int j = 0; j < 8; j++) {
                checksum += (int64_t)result_double8[i][j];
            }
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
