/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force variables to be in memory to increase reload opportunities */
#define NO_INLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Complex struct to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to force memory operations */
int global_int USED = 42;
double global_double USED = 3.14159;
int global_array[256] USED;
struct nested global_struct USED;

/* Function to prevent optimization */
NO_INLINE void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

int main(void) {
    /* Declare diverse variables with different types and storage */
    volatile int vi = 123;
    volatile long long vll = 4567890123LL;
    volatile float vf = 2.71828f;
    volatile double vd = 1.41421356;
    volatile __m128i v128 = _mm_set_epi32(1, 2, 3, 4);
    volatile __m128 v128f = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Arrays for complex addressing */
    int array2d[16][16];
    double darray[64];
    struct nested local_struct;
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array2d[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 64; i++) {
        darray[i] = i * 0.1;
    }
    
    /* Pointer chains for complex address computation */
    struct nested *ptr1 = &local_struct;
    struct nested *ptr2 = &global_struct;
    
    /* Intermediate variables that will be used in reloads */
    int out1, out2, out3;
    double dout1, dout2;
    float fout;
    long long llout;
    __m128i vout;
    
    /* ============================================
       BLOCK A: Register Class Conflict Reload
       ============================================ */
    {
        /* Force integer to float register reload */
        int temp_int = vi * 2;
        double temp_double = vd * 2.0;
        
        /* This asm requires integer in float register - will force reload */
        asm volatile (
            /* Use both register classes to increase pressure */
            "movq %1, %%xmm0\n\t"      /* Move integer to XMM register */
            "cvtsi2sd %2, %%xmm1\n\t"  /* Convert integer to double */
            "addsd %3, %%xmm1\n\t"     /* Add double */
            "movq %%xmm1, %0\n\t"      /* Move result back */
            : "=m" (dout1)             /* Memory output */
            : "r" (temp_int),          /* Integer in general reg */
              "r" (vi),                /* Another integer */
              "x" (temp_double)        /* Double in XMM reg */
            : "xmm0", "xmm1", "memory"
        );
        
        /* Another conflict: float in integer register */
        float temp_float = vf * 3.0f;
        asm volatile (
            "movd %1, %%xmm0\n\t"
            "mulss %2, %%xmm0\n\t"
            "movd %%xmm0, %0\n\t"
            : "=r" (out1)              /* Integer register output */
            : "r" (*(int*)&temp_float), /* Float bitcast to int */
              "x" (vf)                 /* Float in XMM register */
            : "xmm0"
        );
    }
    
    /* ============================================
       BLOCK B: Complex Address Reload
       ============================================ */
    {
        /* Complex array indexing that may not fit in addressing mode */
        int i = vi & 0xF;
        int j = (vi >> 4) & 0xF;
        int k = (vi >> 8) & 0x3F;
        
        /* Complex address computation */
        int *addr1 = &array2d[i][j];
        double *addr2 = &darray[k];
        
        /* Force address into register then use in memory operand */
        asm volatile (
            "movl (%1, %2, 4), %0\n\t"  /* array2d[i][j] with scale */
            : "=r" (out2)
            : "r" (array2d),           /* Base address */
              "r" (i * 16 + j)         /* Complex index */
            : "memory"
        );
        
        /* Even more complex addressing with struct */
        int offset = (i * sizeof(struct nested)) / 4;
        asm volatile (
            "movl 0(%1, %2, 4), %0\n\t"  /* ptr->a[i] */
            : "=r" (out3)
            : "r" (ptr1),               /* Struct pointer */
              "r" (offset)              /* Computed offset */
            : "memory"
        );
        
        /* Multi-dimensional with variable indices */
        asm volatile (
            "movsd (%[base], %[idx], 8), %%xmm0\n\t"
            "addsd %[addend], %%xmm0\n\t"
            "movsd %%xmm0, %[result]\n\t"
            : [result] "=m" (dout2)
            : [base] "r" (darray),
              [idx] "r" ((long)(i * j + k)),
              [addend] "x" (vd)
            : "xmm0", "memory"
        );
    }
    
    /* ============================================
       BLOCK C: Early-Clobber Multiple Outputs
       ============================================ */
    {
        int in1 = vi + 1;
        int in2 = vi + 2;
        int in3 = vi + 3;
        int early1, early2;
        
        /* Early clobber with multiple outputs that conflict with inputs */
        asm volatile (
            "movl %2, %0\n\t"          /* out1 = in1 */
            "addl %3, %0\n\t"          /* out1 += in2 */
            "movl %0, %1\n\t"          /* out2 = out1 (early clobber!) */
            "imull %4, %1\n\t"         /* out2 *= in3 */
            : "=&r" (early1), "=&r" (early2)  /* Both early-clobber */
            : "r" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        /* Use the results to prevent optimization */
        out1 += early1 + early2;
        
        /* Another early-clobber with floating point */
        double din1 = vd;
        double din2 = vd * 2.0;
        double dout_a, dout_b;
        
        asm volatile (
            "movsd %2, %%xmm0\n\t"
            "addsd %3, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            "mulsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %1\n\t"
            : "=&x" (dout_a), "=&x" (dout_b)  /* Early-clobber XMM */
            : "x" (din1), "x" (din2)
            : "xmm0"
        );
        
        dout1 += dout_a + dout_b;
    }
    
    /* ============================================
       BLOCK D: Secondary Reload Patterns
       ============================================ */
    {
        /* Pattern that often requires secondary reloads:
           Moving between different register files with constraints */
        long long large_constant = 0x123456789ABCDEF0LL;
        __m128i vec_constant = _mm_set_epi64x(0x1122334455667788LL, 
                                              0x99AABBCCDDEEFF00LL);
        
        /* This pattern may require a temporary register on some arches */
        asm volatile (
            "movq %1, %%rax\n\t"       /* Might need temp GPR */
            "movq %%rax, %%xmm0\n\t"   /* Then to XMM */
            "paddq %2, %%xmm0\n\t"     /* Vector add */
            "movq %%xmm0, %0\n\t"      /* Back to memory */
            : "=m" (llout)
            : "r" (large_constant),    /* 64-bit immediate might need reload */
              "x" (vec_constant)       /* Vector constant */
            : "rax", "xmm0", "memory"
        );
        
        /* Another secondary reload pattern: 
           Using a constant in an addressing mode that needs a register */
        int constant_index = 256;  /* Too large for some addressing modes */
        asm volatile (
            "movl (%1, %2, 4), %%eax\n\t"  /* array[constant_index] */
            "addl %%eax, %0\n\t"
            : "+r" (out1)
            : "r" (global_array),
              "r" (constant_index)    /* Might need reload if constant is large */
            : "eax", "memory"
        );
    }
    
    /* ============================================
       BLOCK E: Mixed Mode Reloads
       ============================================ */
    {
        /* Different machine modes in same asm */
        char c = (char)vi;
        short s = (short)vi;
        int i = vi;
        long long ll = vll;
        
        asm volatile (
            "movsbl %1, %0\n\t"        /* Sign extend byte to int */
            : "=r" (out1)
            : "r" (c)
        );
        
        asm volatile (
            "movswl %1, %0\n\t"        /* Sign extend short to int */
            : "=r" (out2)
            : "r" (s)
        );
        
        asm volatile (
            "addq %1, %0\n\t"          /* 64-bit add */
            : "+r" (ll)
            : "r" (vll)
            : "cc"
        );
        
        llout = ll;
    }
    
    /* ============================================
       BLOCK F: High Register Pressure
       ============================================ */
    {
        /* Many live variables to increase register pressure */
        int r1 = vi + 1;
        int r2 = vi + 2;
        int r3 = vi + 3;
        int r4 = vi + 4;
        int r5 = vi + 5;
        int r6 = vi + 6;
        int r7 = vi + 7;
        int r8 = vi + 8;
        int r9 = vi + 9;
        int r10 = vi + 10;
        
        /* Use all in one asm to force spills and reloads */
        asm volatile (
            "addl %1, %0\n\t"
            "addl %2, %0\n\t"
            "addl %3, %0\n\t"
            "addl %4, %0\n\t"
            "addl %5, %0\n\t"
            "addl %6, %0\n\t"
            "addl %7, %0\n\t"
            "addl %8, %0\n\t"
            "addl %9, %0\n\t"
            "addl %10, %0\n\t"
            : "+r" (r1)
            : "r" (r2), "r" (r3), "r" (r4), "r" (r5),
              "r" (r6), "r" (r7), "r" (r8), "r" (r9), "r" (r10)
            : "cc"
        );
        
        out1 += r1;
    }
    
    /* ============================================
       Final computation to use all results
       ============================================ */
    int checksum = out1 + out2 + out3 + (int)llout + (int)dout1 + (int)dout2;
    checksum += vi + (int)vll + (int)vf + (int)vd;
    
    /* Use all variables to prevent optimization */
    use(&checksum);
    use(&out1);
    use(&out2);
    use(&out3);
    use(&llout);
    use(&dout1);
    use(&dout2);
    use(&v128);
    use(&v128f);
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
