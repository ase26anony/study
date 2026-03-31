/* reload_test.c - Test program to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force variables to be in memory to increase reload opportunities */
#define NO_INLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Complex global variables to force memory operations */
int global_array[256][256];
double global_doubles[128];
float global_floats[256];
long long global_longs[64];
__m128i global_vecs[32];

/* Function to prevent optimization */
NO_INLINE static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

int main(void) {
    /* Declare diverse variables with different types and storage */
    volatile int vi = 12345;
    volatile long long vll = 0x123456789ABCDEF0LL;
    volatile float vf = 3.14159f;
    volatile double vd = 2.718281828459045;
    volatile __m128i vvec = _mm_setzero_si128();
    
    /* Array indices that will be computed at runtime */
    volatile int i = 100, j = 200, k = 50;
    
    /* Pointers that will need address reloads */
    int *ptr1 = &global_array[0][0];
    double *ptr2 = &global_doubles[0];
    float *ptr3 = &global_floats[0];
    
    /* Output variables for asm statements */
    int out1, out2, out3;
    long long out_ll1, out_ll2;
    float out_f;
    double out_d;
    __m128i out_vec;
    
    /* Prevent initial optimizations */
    use(&vi); use(&vll); use(&vf); use(&vd); use(&vvec);
    
    /*** BLOCK A: Register Class Conflict ***/
    /* Force integer to float register reload */
    asm volatile (
        /* Request float register for integer value */
        "movss %1, %%xmm0\n\t"
        "cvtsi2ss %2, %%xmm1\n\t"
        "addss %%xmm1, %%xmm0\n\t"
        "movss %%xmm0, %0"
        : "=m" (out_f)                    /* Memory output */
        : "m" (vf), "r" (vi)              /* Float from memory, int from register */
        : "xmm0", "xmm1", "memory"
    );
    
    /*** BLOCK B: Complex Address Reload ***/
    /* Multi-dimensional array with complex addressing */
    int idx1 = i * 256 + j;
    int idx2 = k * 128 + (i & 127);
    
    asm volatile (
        /* Complex address computation forcing reload */
        "movl %[idx1], %%eax\n\t"
        "movl %[idx2], %%ebx\n\t"
        "movl %[arr](%%eax,4), %%ecx\n\t"  /* arr[idx1] */
        "addl %[arr](%%ebx,4), %%ecx\n\t"  /* + arr[idx2] */
        "movl %%ecx, %[out]"
        : [out] "=r" (out1)
        : [arr] "R" (global_array),        /* Base address */
          [idx1] "rm" (idx1),              /* Can be reg or mem */
          [idx2] "rm" (idx2)               /* Can be reg or mem */
        : "eax", "ebx", "ecx", "memory"
    );
    
    /* More complex: pointer chain with displacement */
    struct nested {
        int a;
        struct inner {
            long x;
            double y;
        } *inner_ptr;
    } *nptr = (void*)global_array;
    
    /* Force address reload with structure offset */
    asm volatile (
        "movq %[ptr], %%rax\n\t"
        "movq 8(%%rax), %%rbx\n\t"        /* Load inner_ptr */
        "movsd (%%rbx), %%xmm0\n\t"       /* Load inner_ptr->y */
        "cvtsd2ss %%xmm0, %%xmm0\n\t"
        "movss %%xmm0, %[out]"
        : [out] "=m" (out_f)
        : [ptr] "r" (nptr)
        : "rax", "rbx", "xmm0", "memory"
    );
    
    /*** BLOCK C: Early-Clobber Multiple Outputs ***/
    /* Multiple outputs with early clobber */
    int in1 = vi * 2;
    int in2 = vi + 100;
    int in3 = vi - 50;
    
    asm volatile (
        /* out1 gets in1+in2, out2 gets in1*in3, but out2 clobbers early */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"          /* out1 = in1 + in2 */
        "movl %%eax, %[out1]\n\t"
        "movl %[in1], %%ebx\n\t"
        "imull %[in3], %%ebx\n\t"         /* out2 = in1 * in3 */
        "movl %%ebx, %[out2]"
        : [out1] "=&r" (out1),            /* Early clobber */
          [out2] "=&r" (out2)             /* Early clobber */
        : [in1] "r" (in1),
          [in2] "r" (in2),
          [in3] "r" (in3)
        : "eax", "ebx", "cc"
    );
    
    /* Triple early-clobber with different machine modes */
    asm volatile (
        "mov %[in_ll], %%rax\n\t"
        "add $100, %%rax\n\t"
        "mov %%rax, %[out_ll1]\n\t"
        "mov %[in_ll], %%rbx\n\t"
        "sub $50, %%rbx\n\t"
        "mov %%rbx, %[out_ll2]\n\t"
        "pxor %%xmm0, %%xmm0\n\t"
        "movq %%rax, %%xmm0\n\t"
        "movdqu %%xmm0, %[out_vec]"
        : [out_ll1] "=&r" (out_ll1),      /* Early clobber 64-bit */
          [out_ll2] "=&r" (out_ll2),      /* Early clobber 64-bit */
          [out_vec] "=xm" (out_vec)       /* XMM register/memory */
        : [in_ll] "rm" (vll)              /* 64-bit input */
        : "rax", "rbx", "xmm0", "memory"
    );
    
    /*** BLOCK D: Secondary Reload Patterns ***/
    /* Pattern that often needs secondary reloads: 
       Moving between different register files */
    double dbl_input = vd * 2.0;
    
    asm volatile (
        /* Force move from general reg to xmm via memory */
        "movq %[in], %%rax\n\t"
        "movq %%rax, %[temp]\n\t"
        "movsd %[temp], %%xmm0\n\t"
        "addsd %[dbl], %%xmm0\n\t"
        "movsd %%xmm0, %[out]"
        : [out] "=m" (out_d),
          [temp] "=m" (out_ll1)           /* Temporary storage */
        : [in] "r" (vll),                 /* Integer in general reg */
          [dbl] "m" (dbl_input)           /* Double in memory */
        : "rax", "xmm0", "memory"
    );
    
    /* Vector reload pattern with different constraints */
    __m128i vec_input = _mm_set_epi32(vi, vi+1, vi+2, vi+3);
    
    asm volatile (
        /* Complex vector operation forcing reloads */
        "movdqu %[vec1], %%xmm0\n\t"
        "movdqu %[vec2], %%xmm1\n\t"
        "paddd %%xmm1, %%xmm0\n\t"
        "pslld $2, %%xmm0\n\t"
        "movdqu %%xmm0, %[out]"
        : [out] "=xm" (out_vec)           /* Must be xmm reg or memory */
        : [vec1] "xm" (vvec),             /* XMM reg or memory */
          [vec2] "xm" (vec_input)         /* XMM reg or memory */
        : "xmm0", "xmm1", "memory"
    );
    
    /*** Additional: Mixed mode reloads ***/
    /* Different machine modes in same asm */
    short sval = vi & 0xFFFF;
    char cval = vi & 0xFF;
    
    asm volatile (
        "movswl %[sval], %%eax\n\t"
        "movsbl %[cval], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "cltd\n\t"                        /* Sign extend eax to edx:eax */
        "idivl %[div], %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=r" (out3)
        : [sval] "r" (sval),              /* 16-bit value */
          [cval] "r" (cval),              /* 8-bit value */
          [div] "r" (vi)                  /* 32-bit divisor */
        : "eax", "ebx", "edx", "cc"
    );
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = out1 + out2 + out3 + (int)out_f + (int)out_d 
                   + (int)out_ll1 + (int)out_ll2;
    
    /* Use vector result */
    int vec_sum[4];
    _mm_storeu_si128((__m128i*)vec_sum, out_vec);
    checksum += vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3];
    
    /* Mix in global array accesses */
    checksum += global_array[0][0];
    checksum += (int)global_doubles[0];
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
