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

/* Initialize array with pseudo-random values */
static void init_array(void *array, size_t size) {
    uint32_t *ptr = (uint32_t *)array;
    size_t count = size / sizeof(uint32_t);
    for (size_t i = 0; i < count; i++) {
        ptr[i] = lcg_rand();
    }
}

/* Complex mask computation - prevents constant propagation */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t *)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control + i * 3) % 32;
        if (mask_data[i] >= 16) {
            mask_data[i] = 31 - mask_data[i];
        }
    }
    
    return mask;
}

/* Another mask with different pattern */
static int32x16_t compute_alternate_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t *)&mask;
    
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control * 7 + i * 5) % 32;
        mask_data[i] = mask_data[i] < 0 ? -mask_data[i] : mask_data[i];
    }
    
    return mask;
}

int main(void) {
    /* Large arrays for vector operations */
    int32_t array_a[64] __attribute__((aligned(64)));
    int32_t array_b[64] __attribute__((aligned(64)));
    int32_t array_c[64] __attribute__((aligned(64)));
    int32_t result[64] __attribute__((aligned(64)));
    
    /* Initialize with pseudo-random data */
    init_array(array_a, sizeof(array_a));
    init_array(array_b, sizeof(array_b));
    init_array(array_c, sizeof(array_c));
    
    /* Volatile control variables to prevent optimization */
    volatile int control1 = 42;
    volatile int control2 = 17;
    volatile int control3 = 99;
    
    /* Cast to vector types */
    int32x16_t *vec_a = (int32x16_t *)array_a;
    int32x16_t *vec_b = (int32x16_t *)array_b;
    int32x16_t *vec_c = (int32x16_t *)array_c;
    int32x16_t *vec_result = (int32x16_t *)result;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /*** KERNEL 1: Complex shuffle with computed mask ***/
    /* This should generate many operands due to complex mask computation */
    for (int iter = 0; iter < 10; iter++) {
        int32x16_t mask1 = compute_complex_mask(control1 + iter);
        int32x16_t mask2 = compute_alternate_mask(control2 + iter);
        
        /* Complex shuffle chain - results depend on previous operations */
        int32x16_t temp1 = __builtin_shuffle(vec_a[0], vec_b[0], mask1);
        int32x16_t temp2 = __builtin_shuffle(vec_c[0], temp1, mask2);
        
        /* Mix with arithmetic to prevent simplification */
        temp2 = temp2 + (vec_a[0] >> 1);
        
        /* Another shuffle with mixed sources */
        int32x16_t temp3 = __builtin_shuffle(temp2, vec_b[0], mask1);
        
        vec_result[iter % 4] = temp3;
        
        /* Modify control to change mask computation */
        control1 = control1 ^ (iter * 7);
    }
    
    asm volatile("" ::: "memory");
    
    /*** KERNEL 2: Chain of shuffles accumulating operand count ***/
    {
        /* Create a complex mask using multiple operations */
        int32x16_t base_mask = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        int32x16_t offset_mask = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
        
        /* Data-dependent mask selection */
        int32x16_t dynamic_mask;
        if (control3 > 50) {
            dynamic_mask = base_mask + (control1 % 8);
        } else {
            dynamic_mask = offset_mask - (control2 % 8);
        }
        
        /* Chain of operations that may require many operands during expansion */
        int32x16_t chain1 = __builtin_shuffle(vec_a[1], vec_b[1], dynamic_mask);
        int32x16_t chain2 = __builtin_shuffle(chain1, vec_c[1], dynamic_mask + 1);
        int32x16_t chain3 = __builtin_shuffle(vec_b[1], chain2, dynamic_mask * 2);
        int32x16_t chain4 = __builtin_shuffle(chain3, vec_a[1], dynamic_mask / 2);
        
        /* Final shuffle with all previous results involved */
        int32x16_t final_result = __builtin_shuffle(
            chain1, 
            chain2,
            __builtin_shuffle(chain3, chain4, dynamic_mask)
        );
        
        vec_result[0] = final_result;
    }
    
    asm volatile("" ::: "memory");
    
    /*** KERNEL 3: Conditional vector permutation ***/
    {
        /* Compute two different masks */
        int32x16_t mask_a = compute_complex_mask(control1);
        int32x16_t mask_b = compute_alternate_mask(control2);
        
        /* Create two different shuffle results */
        int32x16_t shuffle_a = __builtin_shuffle(vec_a[2], vec_b[2], mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(vec_a[2], vec_c[2], mask_b);
        
        /* Conditional selection between shuffle results */
        int32x16_t selected = (control3 > 75) ? shuffle_a : shuffle_b;
        
        /* Use selected result in another shuffle */
        int32x16_t mask_c = mask_a ^ mask_b;
        int32x16_t final_shuffle = __builtin_shuffle(selected, vec_b[2], mask_c);
        
        vec_result[1] = final_shuffle;
    }
    
    asm volatile("" ::: "memory");
    
    /*** KERNEL 4: Inline assembly with many operands ***/
    {
        /* Use inline assembly with vector constraints */
        int32x16_t asm_input1 = vec_a[3];
        int32x16_t asm_input2 = vec_b[3];
        int32x16_t asm_input3 = vec_c[3];
        int32x16_t asm_output1, asm_output2;
        
        /* Complex inline assembly that may require many operands */
        asm volatile (
            /* Hypothetical multi-operand vector operation */
            "vmovdqa %[in1], %%ymm0\n\t"
            "vmovdqa %[in2], %%ymm1\n\t"
            "vmovdqa %[in3], %%ymm2\n\t"
            "vpshufd $0x1B, %%ymm0, %%ymm3\n\t"
            "vpshufd $0x39, %%ymm1, %%ymm4\n\t"
            "vpblendd $0xF0, %%ymm3, %%ymm4, %%ymm5\n\t"
            "vmovdqa %%ymm5, %[out1]\n\t"
            "vpermq $0x4E, %%ymm5, %[out2]\n\t"
            : [out1] "=v" (asm_output1),
              [out2] "=v" (asm_output2)
            : [in1] "v" (asm_input1),
              [in2] "v" (asm_input2),
              [in3] "v" (asm_input3)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "memory"
        );
        
        vec_result[2] = asm_output1 + asm_output2;
    }
    
    asm volatile("" ::: "memory");
    
    /*** KERNEL 5: Mixed vector types and widths ***/
    {
        /* Use different vector types */
        float64x8_t *double_vec_a = (float64x8_t *)array_a;
        float64x8_t *double_vec_b = (float64x8_t *)array_b;
        
        /* Create a mask for 64-bit elements */
        int64x8_t double_mask = {0,2,4,6,1,3,5,7};
        
        /* Shuffle with 64-bit vectors */
        float64x8_t double_shuffle = __builtin_shufflevector(
            double_vec_a[0],
            double_vec_b[0],
            0, 9, 2, 11, 4, 13, 6, 15
        );
        
        /* Convert and mix with integer vectors */
        int64x8_t int_shuffle = (int64x8_t)double_shuffle;
        int32x16_t mixed_result = __builtin_shuffle(
            vec_result[0],
            (int32x16_t)int_shuffle,
            compute_complex_mask(control3)
        );
        
        vec_result[3] = mixed_result;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += result[i];
    }
    
    printf("Checksum: %ld\n", (long)checksum);
    
    return 0;
}
