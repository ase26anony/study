/* reload_test.c - Test program to trigger multiple reload scenarios in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>  /* For SSE intrinsics */

/* Force variables to be in memory to increase reload opportunities */
#define NO_REGISTER __attribute__((noinline, used))

/* Complex structure to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to force memory operations */
int global_array[256];
double global_doubles[128];
struct nested nested_array[16];

NO_REGISTER int compute_checksum(int a, int b, int c, int d) {
    return a ^ b ^ c ^ d;
}

int main(void) {
    /* Declare diverse variables with different types and storage */
    volatile int int_var = 12345;
    volatile long long ll_var = 9876543210LL;
    volatile float float_var = 3.14159f;
    volatile double double_var = 2.718281828459045;
    volatile __m128i vec_var = _mm_set_epi32(1, 2, 3, 4);
    
    /* Arrays for complex addressing */
    int multi_array[8][16];
    double *ptr_array[32];
    
    /* Initialize arrays */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            multi_array[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 32; i++) {
        ptr_array[i] = &global_doubles[i % 128];
    }
    
    /* Results from asm blocks */
    int result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    long long result_ll = 0;
    double result_double = 0.0;
    __m128i result_vec;
    
    /* ============================================
       BLOCK A: Register Class Conflict
       Force reload between integer and floating-point registers
       ============================================ */
    {
        int temp_int = int_var;
        double temp_double = double_var;
        
        /* This asm requires integer in floating-point register - will force reload */
        asm volatile (
            /* Move integer to floating-point register (simulated) */
            "movd %1, %%xmm0\n\t"
            "movd %%xmm0, %0\n\t"
            : "=r" (result1)          /* Output in integer register */
            : "f" (temp_int)          /* Input in floating-point register - CONFLICT! */
            : "%xmm0"
        );
        
        /* Use both results to prevent optimization */
        result1 += temp_int;
    }
    
    /* ============================================
       BLOCK B: Complex Address Reload
       Force address computation reload with complex indexing
       ============================================ */
    {
        int i = int_var & 7;      /* 0-7 */
        int j = (int_var >> 3) & 15; /* 0-15 */
        int k = (int_var >> 7) & 31; /* 0-31 */
        
        /* Complex addressing: multi_array[i][j] + ptr_array[k] dereference */
        /* The address computation may need reloading into a register */
        asm volatile (
            "movl (%[addr1]), %%eax\n\t"
            "movsd (%[addr2]), %%xmm0\n\t"
            "cvtsd2si %%xmm0, %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %[result]\n\t"
            : [result] "=r" (result2)
            : [addr1] "r" (&multi_array[i][j]),  /* Complex address 1 */
              [addr2] "r" (ptr_array[k])         /* Complex address 2 */
            : "%eax", "%ebx", "%xmm0", "memory"
        );
        
        /* Additional complex address computation */
        struct nested *nptr = &nested_array[i];
        for (int m = 0; m < 3; m++) {
            if (nptr) {
                asm volatile (
                    "addl (%[ptr]), %[res]\n\t"
                    : [res] "+r" (result2)
                    : [ptr] "r" (&nptr->a[j])
                    : "memory"
                );
                nptr = nptr->next;  /* Could be NULL, but keeps compiler guessing */
            }
        }
    }
    
    /* ============================================
       BLOCK C: Early-Clobber Multiple Outputs
       Force reloads due to early-clobber constraints
       ============================================ */
    {
        int in1 = int_var + 1;
        int in2 = int_var + 2;
        int in3 = int_var + 3;
        int out1, out2;
        
        /* Early-clobber on out2 means it's written before all inputs are read,
           forcing reloads if inputs share registers with outputs */
        asm volatile (
            "movl %[in1], %[out1]\n\t"    /* out1 gets in1 */
            "imull %[in2], %[out1]\n\t"   /* out1 *= in2 */
            "movl %[out1], %[out2]\n\t"   /* out2 gets out1 EARLY */
            "addl %[in3], %[out2]\n\t"    /* out2 += in3 */
            "subl %[in1], %[out1]\n\t"    /* out1 -= in1 (uses in1 again) */
            : [out1] "=&r" (out1),        /* Early-clobber */
              [out2] "=&r" (out2)         /* Early-clobber */
            : [in1] "r" (in1),
              [in2] "r" (in2),
              [in3] "r" (in3)
            : "cc"
        );
        
        result3 = out1 + out2;
    }
    
    /* ============================================
       BLOCK D: Secondary Reload Patterns
       Force secondary reloads through complex operations
       ============================================ */
    {
        /* Mix of different modes: SImode, DImode, SFmode, DFmode */
        int int_val = int_var;
        long long ll_val = ll_var;
        float float_val = float_var;
        double double_val = double_var;
        
        /* Multiple operations requiring different register classes */
        asm volatile (
            /* Integer to float conversion chain */
            "cvtsi2ssl %[intv], %%xmm0\n\t"
            "cvtss2sd %%xmm0, %%xmm1\n\t"
            /* Double to integer (may need intermediate) */
            "cvttsd2si %%xmm1, %%eax\n\t"
            /* Long long operation */
            "addq %[llv], %%rax\n\t"
            "movq %%rax, %[llout]\n\t"
            /* Vector operation */
            "movd %[intv], %%xmm2\n\t"
            "paddd %[vecv], %%xmm2\n\t"
            "movdqa %%xmm2, %[vecout]\n\t"
            : [llout] "=r" (result_ll),
              [vecout] "=x" (result_vec)
            : [intv] "r" (int_val),
              [llv] "r" (ll_val),
              [vecv] "x" (vec_var)
            : "%rax", "%xmm0", "%xmm1", "%xmm2", "cc"
        );
        
        /* Additional forced spill/reload with memory constraint */
        double temp_double;
        asm volatile (
            "movsd %[in], %%xmm0\n\t"
            "addsd %[in2], %%xmm0\n\t"
            "movsd %%xmm0, %[out]\n\t"
            : [out] "=m" (temp_double)  /* Memory output constraint */
            : [in] "m" (double_val),    /* Memory input constraint */
              [in2] "m" (global_doubles[0])
            : "%xmm0"
        );
        
        result_double = temp_double;
        result4 = (int)result_ll + (int)result_double;
    }
    
    /* ============================================
       BLOCK E: Additional Pressure with Many Clobbers
       Force many register spills/reloads
       ============================================ */
    {
        int a = int_var, b = a + 1, c = a + 2, d = a + 3;
        int e = a + 4, f = a + 5, g = a + 6, h = a + 7;
        
        /* Clobber many registers to force spills */
        asm volatile (
            "movl %[a], %%eax\n\t"
            "movl %[b], %%ebx\n\t"
            "movl %[c], %%ecx\n\t"
            "movl %[d], %%edx\n\t"
            "movl %[e], %%esi\n\t"
            "movl %[f], %%edi\n\t"
            "addl %%ebx, %%eax\n\t"
            "addl %%ecx, %%eax\n\t"
            "addl %%edx, %%eax\n\t"
            "addl %%esi, %%eax\n\t"
            "addl %%edi, %%eax\n\t"
            "movl %%eax, %[res]\n\t"
            : [res] "=r" (result4)
            : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
              [e] "r" (e), [f] "r" (f)
            : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi", "cc"
        );
    }
    
    /* Final computation to use all results and prevent dead code elimination */
    int final_checksum = compute_checksum(result1, result2, result3, result4);
    final_checksum += (int)result_ll;
    final_checksum += (int)result_double;
    
    /* Use vector result to prevent optimization */
    int vec_elems[4];
    _mm_storeu_si128((__m128i*)vec_elems, result_vec);
    for (int i = 0; i < 4; i++) {
        final_checksum += vec_elems[i];
    }
    
    printf("Final checksum: %d\n", final_checksum);
    return final_checksum & 255;  /* Return non-zero to indicate execution */
}
