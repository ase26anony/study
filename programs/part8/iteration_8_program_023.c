/* reload_test.c - Test program to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Force no optimization on specific variables */
#define VOL(var) (*(volatile typeof(var)*)&(var))

/* Complex structure to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to increase register pressure */
int global_array[256];
double global_doubles[128];
__m128i global_vecs[64];

int main(void) {
    /* ========== 1. VARIABLE DECLARATIONS WITH DIFFERENT TYPES ========== */
    /* Scalars of different types and sizes */
    int i1 = 123, i2 = 456, i3 = 789;
    long long ll1 = 0x123456789ABCDEF0LL, ll2 = 0xFEDCBA9876543210LL;
    float f1 = 3.14159f, f2 = 2.71828f;
    double d1 = 1.41421356, d2 = 1.73205080;
    
    /* Pointers and arrays for complex addressing */
    int array2d[16][16];
    struct nested nested_array[8];
    struct nested *nptr = &nested_array[0];
    
    /* Vector types */
    __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    
    /* Volatile variables to prevent optimization */
    volatile int vi1 = 0, vi2 = 0;
    volatile double vd1 = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array2d[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            nested_array[i].a[j] = i * 10 + j;
        }
        for (int j = 0; j < 4; j++) {
            nested_array[i].b[j] = i * 1.5 + j * 0.25;
        }
        nested_array[i].next = (i < 7) ? &nested_array[i+1] : NULL;
    }
    
    /* ========== 2. BLOCK A: REGISTER CLASS CONFLICT ========== */
    /* Force integer to float register reload */
    asm volatile (
        /* Request float register for integer value */
        "movss %1, %%xmm0\n\t"
        "cvtsi2ss %2, %%xmm1\n\t"
        "addss %%xmm1, %%xmm0\n\t"
        "movss %%xmm0, %0"
        : "=m" (vi1)          /* Memory output */
        : "f" (f1),           /* Float register constraint for float */
          "r" (i1)            /* General register for integer - will need reload */
        : "xmm0", "xmm1", "memory"
    );
    
    /* ========== 3. BLOCK B: COMPLEX ADDRESS RELOAD ========== */
    /* Complex addressing mode that may need reloading */
    int idx1 = 5, idx2 = 10, idx3 = 3;
    asm volatile (
        /* Access with complex addressing */
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "+m" (array2d[idx1 * 2 + idx3][idx2 - idx1 * idx3])  /* Complex address */
        : "ri" (i2)  /* Register or immediate - may force address reload */
        : "eax", "cc", "memory"
    );
    
    /* Even more complex: pointer chain with offset */
    int chain_result = 0;
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"  /* base + index*4 */
        "addl 16(%1), %%eax\n\t"       /* base + 16 */
        "movl %%eax, %0"
        : "=r" (chain_result)
        : "r" (&nested_array[idx1].a[0]),  /* Base pointer */
          "r" (idx2)                       /* Index - may need reload */
        : "eax", "memory"
    );
    
    /* ========== 4. BLOCK C: EARLY-CLOBBER MULTIPLE OUTPUTS ========== */
    /* Multiple outputs with early clobber */
    int out1, out2, out3;
    asm volatile (
        /* out2 is early-clobber, written before all inputs consumed */
        "movl %3, %0\n\t"      /* out1 = in1 */
        "imull %4, %0\n\t"     /* out1 *= in2 */
        "movl %0, %1\n\t"      /* out2 = out1 (early!) */
        "addl %5, %1\n\t"      /* out2 += in3 */
        "movl %1, %2\n\t"      /* out3 = out2 */
        "subl %3, %2"          /* out3 -= in1 */
        : "=&r" (out1),        /* Early clobber - conflicts with inputs */
          "=&r" (out2),        /* Early clobber */
          "=r" (out3)
        : "r" (i1),            /* Input 1 */
          "r" (i2),            /* Input 2 */
          "r" (i3)             /* Input 3 */
        : "cc"
    );
    
    /* ========== 5. BLOCK D: SECONDARY RELOAD PATTERNS ========== */
    /* Pattern that may require secondary reloads on some architectures */
    long long large_const = 0x1234567890ABCDEFLL;
    long long ll_result;
    
    /* Large constant that might need temporary register on some arches */
    asm volatile (
        "movq %1, %%rax\n\t"
        "addq %2, %%rax\n\t"
        "movq %%rax, %0"
        : "=r" (ll_result)
        : "r" (ll1),
          "n" (0x100000000LL)  /* Large constant - may need temp reg */
        : "rax", "cc"
    );
    
    /* Vector operation that might need general-purpose register as intermediate */
    __m128i v_result;
    asm volatile (
        "movdqa %1, %%xmm0\n\t"
        "paddd %2, %%xmm0\n\t"
        "movdqa %%xmm0, %0"
        : "=x" (v_result)      /* XMM register constraint */
        : "x" (v1),            /* XMM input */
          "xm" (v2)            /* XMM or memory - may need reload */
        : "xmm0"
    );
    
    /* ========== 6. ADDITIONAL COMPLEX RELOAD SCENARIOS ========== */
    /* Mixed register classes in single asm */
    double d_result;
    asm volatile (
        "cvtsi2sd %2, %%xmm0\n\t"   /* Convert int to double */
        "addsd %1, %%xmm0\n\t"      /* Add double */
        "movsd %%xmm0, %0"
        : "=m" (vd1)                /* Memory output */
        : "f" (d1),                 /* Float register */
          "r" (i3)                  /* General register - will need reload */
        : "xmm0", "memory"
    );
    
    /* Multiple memory inputs with different addressing */
    int mem_result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (mem_result)
        : "m" (array2d[5][6]),      /* Simple address */
          "m" (nested_array[2].a[3]) /* Complex address - may need reload */
        : "eax", "memory"
    );
    
    /* ========== 7. PREVENT DEAD CODE ELIMINATION ========== */
    /* Compute checksum from all modified variables */
    int checksum = 0;
    checksum += vi1;
    checksum += chain_result;
    checksum += out1 + out2 + out3;
    checksum += (int)ll_result;
    checksum += ((int*)&v_result)[0] + ((int*)&v_result)[1];
    checksum += (int)vd1;
    checksum += mem_result;
    
    /* Use all variables to prevent optimization */
    VOL(i1) = checksum;
    VOL(f1) = checksum * 0.01f;
    VOL(d1) = checksum * 0.01;
    
    printf("Reload test checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
