/* reload_test.c - Comprehensive test to trigger various reload scenarios */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force no optimization on specific variables */
#define NOOPT __attribute__((optimize("O0")))

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

NOOPT int main(void) {
    /* Diverse variable declarations with different types and modes */
    int int_var = 1;
    long long_var = 2;
    float float_var = 3.0f;
    double double_var = 4.0;
    __m128i vec_var = _mm_set_epi32(1, 2, 3, 4);
    __m128 vec_float = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Arrays for complex addressing */
    int array_1d[100] = {0};
    int array_2d[10][10];
    double dbl_array[50];
    struct nested nested_array[5];
    
    /* Pointers for indirection */
    int *ptr1 = &int_var;
    double *ptr2 = &double_var;
    struct nested *nptr = &nested_array[0];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) array_1d[i] = i;
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
            array_2d[i][j] = i * 10 + j;
    
    /* Initialize nested struct */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 8; j++) nested_array[i].a[j] = i * 8 + j;
        nested_array[i].next = (i < 4) ? &nested_array[i + 1] : NULL;
    }
    
    volatile int result = 0; /* Prevent optimization */
    
    /*******************************************************************
     * BLOCK A: Register Class Conflict Reload
     * Force integer to float register reload
     *******************************************************************/
    {
        int int_input = 12345;
        double float_output;
        
        /* Request float register for integer value - forces reload */
        asm volatile (
            "mov %1, %%eax\n\t"          /* Move int to eax */
            "cvtsi2sd %%eax, %0\n\t"     /* Convert to double in float reg */
            : "=f" (float_output)        /* Output in float register */
            : "r" (int_input)            /* Input in general register */
            : "%eax", "memory"
        );
        
        double_var += float_output;
        result += (int)float_output;
    }
    
    /*******************************************************************
     * BLOCK B: Complex Address Reload with Multiple Indexing
     * Force address computation reload
     *******************************************************************/
    {
        int idx1 = 3, idx2 = 7, idx3 = 2;
        int addr_result;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl %c[array](%[i1],%[i2],4), %0\n\t"  /* array[i1 + i2*4] */
            : "=r" (addr_result)
            : [array] "i" (array_1d),    /* Immediate displacement */
              [i1] "r" (idx1),           /* Base register */
              [i2] "r" (idx2)            /* Index register with scale 4 */
            : "memory"
        );
        
        /* Even more complex: array_2d[idx1][idx2] + nested_array[idx3].a[idx1] */
        int complex_addr;
        asm volatile (
            "movl %c[base](%[i1],%[i2],4), %%eax\n\t"    /* 2D array access */
            "addl %c[nested](%[i3],%[i1],4), %%eax\n\t"  /* Struct array access */
            "movl %%eax, %0\n\t"
            : "=r" (complex_addr)
            : [base] "i" (array_2d),
              [nested] "i" (nested_array[0].a),
              [i1] "r" (idx1),
              [i2] "r" (idx2),
              [i3] "r" (idx3)
            : "%eax", "memory"
        );
        
        result += addr_result + complex_addr;
    }
    
    /*******************************************************************
     * BLOCK C: Early-Clobber Multiple Output Reloads
     * Force reloads due to early clobber constraints
     *******************************************************************/
    {
        int in1 = 100, in2 = 200, in3 = 300;
        int out1, out2, out3;
        
        /* Multiple outputs with early clobber */
        asm volatile (
            "movl %2, %0\n\t"        /* out1 = in1 */
            "imull %3, %0\n\t"       /* out1 *= in2 (clobbers early) */
            "addl %4, %0\n\t"        /* out1 += in3 */
            "movl %0, %1\n\t"        /* out2 = out1 */
            "shrl $1, %1\n\t"        /* out2 >>= 1 */
            : "=&r" (out1), "=&r" (out2), "=r" (out3)  /* Two early-clobber */
            : "r" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        /* Use all outputs to prevent dead code elimination */
        result += out1 + out2 + out3;
    }
    
    /*******************************************************************
     * BLOCK D: Secondary Reload Patterns
     * Force secondary reloads through complex constraints
     *******************************************************************/
    {
        /* Pattern 1: Large immediate to vector register (may need GPR intermediate) */
        __m128i vec_const;
        asm volatile (
            "mov $0x12345678, %%eax\n\t"
            "movd %%eax, %0\n\t"
            "pshufd $0, %0, %0\n\t"
            : "=x" (vec_const)       /* XMM register constraint */
            : /* no inputs */
            : "%eax", "memory"
        );
        
        /* Pattern 2: Memory to vector with complex address */
        __m128i loaded_vec;
        int offset = 16;
        asm volatile (
            "movdqu %c[mem](%[off]), %0\n\t"
            : "=x" (loaded_vec)
            : [mem] "i" (dbl_array),  /* Immediate base */
              [off] "r" (offset)      /* Register offset */
            : "memory"
        );
        
        /* Pattern 3: Mixed register classes with multiple constraints */
        double dbl_result;
        long long ll_input = 0x1122334455667788LL;
        
        asm volatile (
            "movq %1, %%rax\n\t"
            "shrq $32, %%rax\n\t"
            "cvtsi2sd %%rax, %0\n\t"
            : "=f" (dbl_result)      /* Float register output */
            : "r" (ll_input)         /* General register input */
            : "%rax", "memory"
        );
        
        vec_var = _mm_add_epi32(vec_const, loaded_vec);
        double_var += dbl_result;
        result += (int)double_var;
    }
    
    /*******************************************************************
     * BLOCK E: High Register Pressure with Multiple Clobbers
     * Force many reloads by clobbering most registers
     *******************************************************************/
    {
        int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
        int r1, r2, r3, r4;
        
        /* Clobber many registers to force spills and reloads */
        asm volatile (
            "movl %4, %%eax\n\t"
            "addl %5, %%eax\n\t"
            "movl %%eax, %0\n\t"
            "movl %6, %%ebx\n\t"
            "subl %7, %%ebx\n\t"
            "movl %%ebx, %1\n\t"
            "movl %8, %%ecx\n\t"
            "imull %9, %%ecx\n\t"
            "movl %%ecx, %2\n\t"
            "movl %10, %%edx\n\t"
            "xorl %11, %%edx\n\t"
            "movl %%edx, %3\n\t"
            : "=r" (r1), "=r" (r2), "=r" (r3), "=r" (r4)
            : "r" (a), "r" (b), "r" (c), "r" (d),
              "r" (e), "r" (f), "r" (g), "r" (h)
            : "%eax", "%ebx", "%ecx", "%edx", "cc", "memory"
        );
        
        result += r1 + r2 + r3 + r4;
    }
    
    /*******************************************************************
     * BLOCK F: Volatile Memory Operations with Complex Addressing
     * Force address reloads for volatile accesses
     *******************************************************************/
    {
        /* Chain pointer chasing through struct */
        volatile int chain_result = 0;
        struct nested *current = nptr;
        
        for (int i = 0; i < 3 && current != NULL; i++) {
            /* Complex addressing: current->a[current->a[0] % 8] */
            int index;
            asm volatile (
                "movl (%1), %%eax\n\t"
                "andl $7, %%eax\n\t"
                "movl (%1,%%eax,4), %0\n\t"
                : "=r" (index)
                : "r" (current->a)
                : "%eax", "memory"
            );
            
            chain_result += index;
            current = current->next;
        }
        
        result += chain_result;
    }
    
    /*******************************************************************
     * BLOCK G: Mixed Size Operations (Different Machine Modes)
     * Trigger reloads with different mode requirements
     *******************************************************************/
    {
        char char_var = 'A';
        short short_var = 1000;
        int int_var2 = 100000;
        long long ll_var2 = 0xFFFFFFFFLL;
        
        /* Mixed size operations forcing mode conversions */
        long long mixed_result;
        asm volatile (
            "movsbl %1, %%eax\n\t"       /* Sign extend char to int */
            "movswl %2, %%ebx\n\t"       /* Sign extend short to int */
            "addl %%ebx, %%eax\n\t"
            "cltq\n\t"                   /* Sign extend eax to rax */
            "addq %3, %%rax\n\t"
            "addq %4, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=r" (mixed_result)
            : "r" (char_var), "r" (short_var),
              "r" (int_var2), "r" (ll_var2)
            : "%rax", "%rbx", "%eax", "%ebx", "cc", "memory"
        );
        
        result += (int)mixed_result;
    }
    
    /* Final computation to use all modified variables */
    int final_result = result 
                     + int_var 
                     + (int)double_var 
                     + array_1d[0]
                     + array_2d[0][0]
                     + nested_array[0].a[0];
    
    /* Print to prevent optimization */
    printf("Result: %d\n", final_result);
    
    return final_result & 0xFF; /* Return non-zero to indicate execution */
}
