#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define large vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize array with pseudo-random values */
static void init_array(int32_t *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr[i] = (int32_t)lcg_rand();
    }
}

/* Complex mask computation - prevents constant propagation */
static int32x16_t compute_dynamic_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control & 1) ? (i * 3) % 16 : (i * 5) % 16;
        control >>= 1;
    }
    return mask;
}

int main(void) {
    /* Large arrays to hold vector data */
    int32_t array_a[64] __attribute__((aligned(64)));
    int32_t array_b[64] __attribute__((aligned(64)));
    int32_t array_c[64] __attribute__((aligned(64)));
    int32_t result[64] __attribute__((aligned(64)));
    
    /* Volatile control variables to prevent optimization */
    volatile int control1 = 42;
    volatile int control2 = 73;
    
    /* Initialize with pseudo-random data */
    init_array(array_a, 64);
    init_array(array_b, 64);
    init_array(array_c, 64);
    
    /* Cast to vector types */
    int32x16_t *va = (int32x16_t*)array_a;
    int32x16_t *vb = (int32x16_t*)array_b;
    int32x16_t *vc = (int32x16_t*)array_c;
    int32x16_t *vr = (int32x16_t*)result;
    
    /* Kernel 1: Complex shuffle with computed mask */
    /* This should generate many operands during expansion */
    for (int i = 0; i < 2; i++) {
        int32x16_t mask = compute_dynamic_mask(control1 + i);
        
        /* Complex shuffle operation - may require many operands in RTL */
        vr[i] = __builtin_shuffle(va[i], vb[i], mask);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 2: Chain of shuffles to accumulate operand count */
    {
        int32x16_t temp1, temp2, temp3;
        
        /* First shuffle */
        int32x16_t mask1 = compute_dynamic_mask(control2);
        temp1 = __builtin_shuffle(va[0], vb[0], mask1);
        
        /* Second shuffle using result of first */
        int32x16_t mask2 = compute_dynamic_mask(control2 ^ 0x55);
        temp2 = __builtin_shuffle(temp1, vc[0], mask2);
        
        /* Third shuffle chaining more vectors */
        int32x16_t mask3 = compute_dynamic_mask(control2 ^ 0xAA);
        temp3 = __builtin_shuffle(temp2, va[1], mask3);
        
        /* Final operation combining multiple vectors */
        int32x16_t mask4 = compute_dynamic_mask(control1);
        vr[2] = __builtin_shuffle(temp3, vb[1], mask4);
        
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 3: Conditional vector permutations */
    {
        int32x16_t mask_a = compute_dynamic_mask(control1);
        int32x16_t mask_b = compute_dynamic_mask(control2);
        
        int32x16_t shuffle_a = __builtin_shuffle(va[2], vb[2], mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(va[3], vb[3], mask_b);
        
        /* Conditional selection between two shuffle results */
        int32x16_t selector = (control1 > control2) ? 
            (int32x16_t){1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0} : 
            (int32x16_t){0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1};
            
        vr[3] = __builtin_shufflevector(shuffle_a, shuffle_b, 
            0,16,2,18,4,20,6,22,8,24,10,26,12,28,14,30);
        
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 4: Mixed vector types and widths */
    {
        /* Cast to different vector types */
        float32x16_t *fa = (float32x16_t*)array_a;
        float32x16_t *fb = (float32x16_t*)array_b;
        
        /* Create a complex mask using arithmetic */
        int32x16_t mix_mask;
        int32_t *mask_data = (int32_t*)&mix_mask;
        for (int i = 0; i < 16; i++) {
            mask_data[i] = (i * 7 + control1) % 32;
        }
        
        /* Shuffle with mixed-type vectors (cast to appropriate type) */
        float32x16_t float_result = __builtin_shuffle(
            fa[0], fb[0], 
            *(__builtin_convertvector(mix_mask, int32x16_t))
        );
        
        /* Store back through integer pointer */
        memcpy(&vr[0], &float_result, sizeof(float32x16_t));
        
        asm volatile("" ::: "memory");
    }
    
#ifdef __x86_64__
    /* Kernel 5: x86-specific builtins with many operands */
    {
        /* Use x86-specific shuffle intrinsics */
        int32x8_t x86_vec_a = *(int32x8_t*)&array_a[0];
        int32x8_t x86_vec_b = *(int32x8_t*)&array_b[0];
        
        /* __builtin_ia32_pshufd typically takes 2 operands, but we can chain */
        for (int i = 0; i < 4; i++) {
            /* Simulate multi-operand operation through chaining */
            int32x8_t temp = __builtin_ia32_pshufd(x86_vec_a, (i * 0x55) & 0xFF);
            temp = __builtin_ia32_paddd128(temp, x86_vec_b);
            *(int32x8_t*)&result[i*8] = temp;
        }
        
        asm volatile("" ::: "memory");
    }
#endif

#ifdef __ARM_NEON
    /* Kernel 6: ARM-specific builtins */
    {
        /* Use ARM NEON intrinsics */
        int32x4_t neon_vec_a = vld1q_s32(array_a);
        int32x4_t neon_vec_b = vld1q_s32(array_b);
        
        /* Chain multiple operations */
        int32x4_t rev_a = __builtin_neon_vrev64q_s32(neon_vec_a);
        int32x4_t rev_b = __builtin_neon_vrev64q_s32(neon_vec_b);
        int32x4_t combined = __builtin_neon_vaddq_s32(rev_a, rev_b);
        
        vst1q_s32(result, combined);
        
        asm volatile("" ::: "memory");
    }
#endif

    /* Kernel 7: Inline assembly with many vector operands */
    {
        /* Hypothetical multi-operand vector operation */
        asm volatile(
            "# Complex vector operation with many operands\n\t"
            "vmovdqa %0, %%ymm0\n\t"
            "vmovdqa %1, %%ymm1\n\t"
            "vmovdqa %2, %%ymm2\n\t"
            "vmovdqa %3, %%ymm3\n\t"
            "# Multi-operand shuffle/permute\n\t"
            "vpermd %%ymm0, %%ymm1, %%ymm2\n\t"
            "vmovdqa %%ymm2, %4"
            : "=m"(vr[0])
            : "m"(va[0]), "m"(vb[0]), "m"(vc[0]), "m"(vr[0])
            : "ymm0", "ymm1", "ymm2", "ymm3", "memory"
        );
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += result[i];
    }
    
    /* Also checksum inputs to ensure they're used */
    for (int i = 0; i < 64; i++) {
        checksum += array_a[i] + array_b[i] + array_c[i];
    }
    
    printf("Checksum: %ld\n", (long)checksum);
    
    return 0;
}
