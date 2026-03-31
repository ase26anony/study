/* reload_test.c - Test program to trigger multiple reload scenarios in GCC */

#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>  /* For SSE intrinsics */

/* Force noinline to prevent optimization */
#define NOINLINE __attribute__((noinline))

/* Complex struct to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to increase register pressure */
int global_int = 42;
double global_double = 3.14159;
__m128i global_vec = {0};

/* Function to create register pressure */
NOINLINE int create_register_pressure(int a, int b, int c, int d, int e, int f) {
    /* Many live variables to force spills */
    int x1 = a * b;
    int x2 = c * d;
    int x3 = e * f;
    int x4 = x1 + x2;
    int x5 = x3 * x4;
    volatile int dummy = x5; /* Prevent optimization */
    return x5;
}

int main(void) {
    /* Diverse variable declarations with different types and modes */
    int int_var1 = 1, int_var2 = 2, int_var3 = 3;
    long long ll_var1 = 100, ll_var2 = 200;
    float float_var1 = 1.5f, float_var2 = 2.5f;
    double double_var1 = 3.14159, double_var2 = 2.71828;
    __m128i vec_var1, vec_var2;
    
    /* Arrays for complex addressing */
    int multi_array[4][8] = {{0}};
    double dbl_array[16] = {0};
    struct nested nested_array[4];
    struct nested *nested_ptr = &nested_array[0];
    
    /* Initialize arrays */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            multi_array[i][j] = i * 8 + j;
        }
        for (int j = 0; j < 4; j++) {
            nested_array[i].b[j] = i * 4.0 + j;
        }
        nested_array[i].next = &nested_array[(i + 1) % 4];
    }
    
    /* BLOCK A: Register Class Conflict */
    /* Force integer to float register reload */
    printf("Block A - Register class conflict:\n");
    {
        int int_input = 12345;
        double float_output;
        
        /* Request float register for integer input - will need reload */
        asm volatile (
            "mov %1, %%eax\n\t"           /* Move integer to eax */
            "cvtsi2sd %%eax, %0\n\t"      /* Convert to double in xmm register */
            : "=f" (float_output)         /* Output in floating-point register */
            : "r" (int_input)             /* Input in general-purpose register */
            : "%eax", "memory"
        );
        
        double_var1 = float_output;
        printf("  Converted %d to %f\n", int_input, float_output);
    }
    
    /* Create register pressure between blocks */
    int_var1 = create_register_pressure(int_var1, int_var2, int_var3, 
                                       global_int, 5, 6);
    
    /* BLOCK B: Complex Address Reload */
    /* Force address computation reload with multi-dimensional array */
    printf("Block B - Complex address reload:\n");
    {
        int i = 2, j = 3;
        int result;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+r" (result)
            : "m" (multi_array[i][j])     /* Complex address computation */
            : "%eax", "memory"
        );
        
        /* Even more complex address with pointer chain */
        double dbl_result;
        asm volatile (
            "movsd %1, %0\n\t"
            : "=x" (dbl_result)
            : "m" (nested_ptr->next->next->b[2])  /* Deep pointer chain */
            : "memory"
        );
        
        printf("  Result from complex address: %d, %f\n", result, dbl_result);
    }
    
    /* BLOCK C: Early-Clobber Multiple Outputs */
    /* Force reloads due to early clobber */
    printf("Block C - Early-clobber multiple outputs:\n");
    {
        int in1 = 100, in2 = 200, in3 = 300;
        int out1, out2, out3;
        
        /* Early clobber on out2 - written before all inputs consumed */
        asm volatile (
            "movl %3, %0\n\t"            /* out1 = in1 */
            "imull %4, %0\n\t"           /* out1 *= in2 (uses out1 as temp) */
            "movl %0, %1\n\t"            /* out2 = out1 (early clobber!) */
            "addl %5, %1\n\t"            /* out2 += in3 */
            "movl %4, %2\n\t"            /* out3 = in2 */
            "subl %3, %2\n\t"            /* out3 -= in1 */
            : "=&r" (out1), "=&r" (out2), "=r" (out3)  /* Two early-clobber */
            : "r" (in1), "r" (in2), "r" (in3)
            : "memory"
        );
        
        printf("  Early-clobber results: %d, %d, %d\n", out1, out2, out3);
    }
    
    /* BLOCK D: Secondary Reload Pattern */
    /* Force secondary reloads with 64-bit constants */
    printf("Block D - Secondary reload patterns:\n");
    {
        uint64_t large_const = 0x123456789ABCDEF0ULL;
        uint64_t result64;
        
        /* Large constant that may need secondary reload on some arches */
        asm volatile (
            "movq %1, %0\n\t"
            "rorq $32, %0\n\t"
            : "=r" (result64)
            : "ri" (large_const)         /* May need reload if constant too large */
            : "memory"
        );
        
        /* Mixed size operations to trigger different machine modes */
        int truncated;
        asm volatile (
            "movl %1, %0\n\t"
            "andl $0xFFFF, %0\n\t"
            : "=r" (truncated)
            : "r" (result64)             /* 64-bit to 32-bit truncation */
            : "memory"
        );
        
        printf("  64-bit operation result: %llx, truncated: %x\n", 
               (unsigned long long)result64, truncated);
    }
    
    /* BLOCK E: Vector/SIMD Reloads */
    /* Force vector register reloads */
    printf("Block E - Vector/SIMD reloads:\n");
    {
        __m128i vec1, vec2;
        int align_dummy __attribute__((aligned(16)));
        
        /* Load aligned vector - may need address reload */
        asm volatile (
            "movdqa %1, %0\n\t"
            : "=x" (vec1)
            : "m" (global_vec)           /* May need address computation */
            : "memory"
        );
        
        /* Vector operation with memory operand */
        asm volatile (
            "paddq %1, %0\n\t"
            : "+x" (vec1)
            : "xm" (vec1)                /* May need reload for same register */
            : "memory"
        );
        
        printf("  Vector operation completed\n");
    }
    
    /* BLOCK F: Memory-to-Memory with Intermediate Register */
    /* Force reload for memory-to-memory operations */
    printf("Block F - Memory-to-memory via register:\n");
    {
        int src = 999, dst;
        
        /* Memory-to-memory via register reload */
        asm volatile (
            "movl %1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=m" (dst)                 /* Memory output */
            : "m" (src)                  /* Memory input */
            : "%eax", "memory"
        );
        
        printf("  Memory copy: %d -> %d\n", src, dst);
    }
    
    /* BLOCK G: Multiple Constraint Alternatives */
    /* Force reload selection among alternatives */
    printf("Block G - Multiple constraint alternatives:\n");
    {
        int value = 777;
        int squared;
        
        /* Multiple constraints - compiler chooses best */
        asm volatile (
            "imull %1, %0\n\t"
            : "=r,r,m" (squared)         /* Multiple output alternatives */
            : "0,rm,r" (value)           /* Multiple input alternatives */
            : "memory"
        );
        
        printf("  Squared value: %d\n", squared);
    }
    
    /* Final checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    checksum += int_var1 + int_var2 + int_var3;
    checksum += (unsigned long long)(double_var1 * 1000);
    checksum += (unsigned long long)(double_var2 * 1000);
    checksum += ll_var1 + ll_var2;
    
    printf("\nFinal checksum: %llu\n", checksum);
    
    return (int)(checksum % 1000);
}
