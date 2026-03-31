/* reload_test.c - Comprehensive test to trigger various reload scenarios */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Force no optimization on specific variables */
#define NOOPT __attribute__((optimize("O0")))

/* Complex structure to force address computations */
struct nested {
    int a[8][8];
    double b[4][4];
    struct nested *next;
};

/* Global variables to increase register pressure */
int global_array[256];
double global_doubles[128];
__m128i global_vecs[64];

NOOPT int main(void) {
    /* Diverse variable declarations */
    int i = 42, j = 73, k = 19;
    long long ll1 = 0x123456789ABCDEF0LL, ll2 = 0xFEDCBA9876543210LL;
    float f1 = 3.14159f, f2 = 2.71828f;
    double d1 = 1.41421356, d2 = 1.73205080;
    __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    
    /* Array and pointer variables for complex addressing */
    int array2d[16][16];
    struct nested nested_array[4];
    struct nested *nptr = &nested_array[0];
    void *label_ptr = &&target_label;
    
    /* Initialize arrays to prevent constant propagation */
    for (int idx = 0; idx < 16; idx++) {
        for (int jdx = 0; jdx < 16; jdx++) {
            array2d[idx][jdx] = idx * 100 + jdx;
        }
    }
    
    /* BLOCK A: Register class conflict reload */
    /* Force integer to float register reload */
    {
        int int_for_float = i * j;
        double float_result;
        
        /* Request float register for integer value - forces reload */
        asm volatile (
            "movq %1, %%xmm0\n\t"
            "movq %%xmm0, %0"
            : "=f" (float_result)      /* Output in float register */
            : "r" (int_for_float)      /* Input in general register */
            : "%xmm0"
        );
        
        d1 = float_result;  /* Use result to prevent elimination */
    }
    
    /* BLOCK B: Complex address reload with multi-dimensional array */
    /* Force address computation into register */
    {
        int complex_addr_result;
        /* Complex addressing: base + index1*stride1 + index2*stride2 */
        int idx1 = i & 0xF;
        int idx2 = j & 0xF;
        
        asm volatile (
            "movl (%1), %0\n\t"
            "addl %2, %0"
            : "=r" (complex_addr_result)
            : "r" (&array2d[idx1][idx2]),  /* Complex address forced into register */
              "r" (k)
            : "memory"
        );
        
        i = complex_addr_result;  /* Use result */
    }
    
    /* BLOCK C: Early-clobber multiple outputs */
    /* Force reloads due to early clobber constraint */
    {
        int out1, out2;
        int in1 = i + 1;
        int in2 = j + 2;
        int in3 = k + 3;
        
        /* Early clobber on out2 forces different register allocation */
        asm volatile (
            "movl %2, %0\n\t"      /* out1 = in1 */
            "imull %3, %0\n\t"     /* out1 *= in2 */
            "movl %4, %1\n\t"      /* out2 = in3 (early - clobbers reg early) */
            "addl %0, %1"          /* out2 += out1 */
            : "=&r" (out1), "=&r" (out2)  /* Both early-clobber */
            : "r" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        j = out1 + out2;  /* Use both outputs */
    }
    
    /* BLOCK D: Secondary reload pattern - vector to integer transfer */
    /* May require intermediate register on some architectures */
    {
        uint64_t vec_extract;
        __m128i vec_source = v1;
        
        /* Extract element from vector - may need secondary reload */
        asm volatile (
            "movq %1, %0"
            : "=r" (vec_extract)
            : "x" (vec_source)      /* Vector register constraint */
            : "%xmm0"               /* Clobber vector reg to force save/restore */
        );
        
        ll1 = vec_extract;  /* Use result */
    }
    
    /* BLOCK E: Memory operand with displacement too large */
    /* Force address reload due to large displacement */
    {
        int far_mem_result;
        /* Access far element in global array */
        int far_index = 200;  /* Large index */
        
        asm volatile (
            "movl %1, %0"
            : "=r" (far_mem_result)
            : "m" (global_array[far_index])  /* Large displacement may need reload */
            : "memory"
        );
        
        k = far_mem_result;
    }
    
    /* BLOCK F: Multiple register classes in single asm */
    {
        double float_from_int;
        int int_from_float;
        
        /* Mixed register classes force multiple reload types */
        asm volatile (
            "cvtsi2sd %2, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            "cvttsd2si %0, %1"
            : "=f" (float_from_int), "=r" (int_from_float)
            : "r" (i)
            : "%xmm0", "cc"
        );
        
        f1 = float_from_int;
        i = int_from_float;
    }
    
    /* BLOCK G: Computed goto with address reload */
    {
        /* Force address of label into register */
        void *dynamic_target = label_ptr;
        
        asm volatile (
            "jmp *%0"
            : 
            : "r" (dynamic_target)  /* Jump target address in register */
            : "memory"
        );
        
    target_label:
        /* Land here after jump */
        d2 += 1.0;
    }
    
    /* BLOCK H: Structure pointer chain forcing address reload */
    {
        int struct_field;
        /* Complex pointer chain */
        struct nested *current = nptr;
        current->next = &nested_array[1];
        current->next->next = &nested_array[2];
        
        asm volatile (
            "movl (%1), %0"
            : "=r" (struct_field)
            : "r" (&current->next->next->a[3][3])  /* Complex address */
            : "memory"
        );
        
        j = struct_field;
    }
    
    /* BLOCK I: 64-bit immediate reload pattern (AArch64 style) */
    {
        uint64_t big_constant = 0x123456789ABCDEF0ULL;
        uint64_t result;
        
        /* Moving 64-bit constant may require secondary reload on some arches */
        asm volatile (
            "mov %1, %0\n\t"
            "ror $32, %0"
            : "=r" (result)
            : "r" (big_constant)  /* Large constant may need temp register */
            : "cc"
        );
        
        ll2 = result;
    }
    
    /* BLOCK J: High register pressure to force spill/reload */
    {
        /* Use many variables in one asm to increase register pressure */
        asm volatile (
            "imull %1, %0\n\t"
            "addl %2, %0\n\t"
            "subl %3, %0\n\t"
            "xorl %4, %0"
            : "+r" (i)
            : "r" (j), "r" (k), "r" ((int)ll1), "r" ((int)ll2)
            : "cc"
        );
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = i + j + k + (int)ll1 + (int)ll2 + (int)f1 + (int)d1 + (int)d2;
    checksum += _mm_extract_epi32(v1, 0) + _mm_extract_epi32(v2, 0);
    
    /* Use computed goto label address in checksum */
    checksum += (uintptr_t)label_ptr & 0xFFFF;
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
