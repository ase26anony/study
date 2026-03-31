/* reload_test.c - Comprehensive test to trigger various reload scenarios */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force noinline to prevent optimization */
#define NOINLINE __attribute__((noinline))

/* Complex structure to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to increase register pressure */
int global_int = 42;
double global_double = 3.14159;
__m128i global_vec;

/* Function to create complex addressing modes */
NOINLINE int complex_address(struct nested *arr, int i, int j, int k) {
    volatile int result = 0;
    
    /* Block 1: Register class conflict - integer in FP register */
    {
        int int_val = i * j + k;
        double fp_val = 2.71828;
        
        /* Force integer into floating-point register */
        asm volatile (
            "mov %1, %%eax\n\t"          /* Load integer */
            "cvtsi2sd %%eax, %%xmm0\n\t" /* Convert to double */
            "addsd %2, %%xmm0\n\t"       /* Add floating value */
            "movsd %%xmm0, %0"           /* Store result */
            : "=m" (fp_val)
            : "r" (int_val), "m" (global_double)
            : "%eax", "%xmm0", "memory"
        );
        
        /* Use both values to keep them live */
        result += (int)fp_val + int_val;
    }
    
    /* Block 2: Complex memory address reload */
    {
        /* Create complex addressing: arr[i].a[j] + arr[k].b[2] */
        int idx1 = i * 8 + j;
        int idx2 = k * 4 + 2;
        
        /* Force address computation that needs reloading */
        asm volatile (
            "mov (%[ptr1]), %%eax\n\t"    /* Load arr[i].a[j] */
            "movsd (%[ptr2]), %%xmm0\n\t" /* Load arr[k].b[2] */
            "cvtsi2sd %%eax, %%xmm1\n\t"  /* Convert to double */
            "addsd %%xmm0, %%xmm1\n\t"    /* Add */
            "cvttsd2si %%xmm1, %%eax\n\t" /* Convert back */
            "mov %%eax, %[out]"           /* Store result */
            : [out] "=r" (result)
            : [ptr1] "r" (&arr[i].a[j]),
              [ptr2] "r" (&arr[k].b[2]),
              "m" (arr[i].a[j]),
              "m" (arr[k].b[2])
            : "%eax", "%xmm0", "%xmm1", "memory"
        );
    }
    
    return result;
}

/* Function with early-clobber and multiple outputs */
NOINLINE void early_clobber_test(int a, int b, int c, int *out1, int *out2) {
    int tmp1, tmp2;
    
    /* Block 3: Early-clobber with multiple outputs */
    asm volatile (
        "mov %2, %0\n\t"      /* out1 = a */
        "imul %3, %0\n\t"     /* out1 *= b */
        "mov %0, %1\n\t"      /* out2 = out1 */
        "add %4, %1\n\t"      /* out2 += c */
        "sub %3, %0"          /* out1 -= b */
        : "=&r" (tmp1), "=&r" (tmp2)  /* Early-clobber outputs */
        : "r" (a), "r" (b), "r" (c)
        : "cc"
    );
    
    *out1 = tmp1;
    *out2 = tmp2;
}

/* Function requiring secondary reloads */
NOINLINE __m128i vector_reload_test(__m128i a, __m128i b, int shift) {
    __m128i result;
    
    /* Block 4: Vector operations with different modes */
    asm volatile (
        "movdqa %1, %0\n\t"           /* result = a */
        "paddq %2, %0\n\t"            /* result += b */
        "movd %3, %%xmm1\n\t"         /* Load shift count */
        "psllq %%xmm1, %0"            /* Shift left */
        : "=x" (result)
        : "x" (a), "x" (b), "r" (shift)
        : "%xmm1"
    );
    
    return result;
}

/* Mixed mode reloads */
NOINLINE double mixed_mode_test(int i, float f, double d, long long ll) {
    double result = d;
    
    /* Block 5: Mixed integer/float/vector modes */
    {
        __m128 vec1, vec2;
        float farr[4] = {1.0f, 2.0f, 3.0f, 4.0f};
        
        /* Force reloads between different register classes */
        asm volatile (
            "movd %[ival], %%xmm0\n\t"        /* int -> xmm */
            "cvtdq2ps %%xmm0, %%xmm0\n\t"     /* int -> float */
            "mulps %[farr], %%xmm0\n\t"       /* multiply */
            "movaps %%xmm0, %[vec1]\n\t"      /* store */
            "cvtps2pd %%xmm0, %%xmm1\n\t"     /* float -> double */
            "addsd %[dval], %%xmm1\n\t"       /* add double */
            "movsd %%xmm1, %[result]"         /* store result */
            : [vec1] "=x" (vec1),
              [result] "=m" (result)
            : [ival] "r" (i),
              [farr] "m" (farr[0]),
              [dval] "m" (d)
            : "%xmm0", "%xmm1", "memory"
        );
    }
    
    return result;
}

/* Main function with varied reload triggers */
int main() {
    int i, j, k;
    int out1, out2;
    double dresult;
    __m128i vresult;
    
    /* Initialize test data */
    struct nested arr[10];
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 8; j++) {
            arr[i].a[j] = i * 10 + j;
        }
        for (j = 0; j < 4; j++) {
            arr[i].b[j] = i * 1.5 + j * 0.25;
        }
        arr[i].next = (i < 9) ? &arr[i+1] : NULL;
    }
    
    /* Trigger various reload scenarios */
    int sum = 0;
    
    /* 1. Complex address reload */
    sum += complex_address(arr, 3, 4, 5);
    
    /* 2. Early-clobber test */
    early_clobber_test(100, 200, 300, &out1, &out2);
    sum += out1 + out2;
    
    /* 3. Vector reload test */
    __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    vresult = vector_reload_test(v1, v2, 2);
    
    /* Extract from vector to add to sum */
    int varr[4];
    _mm_storeu_si128((__m128i*)varr, vresult);
    sum += varr[0] + varr[1] + varr[2] + varr[3];
    
    /* 4. Mixed mode test */
    dresult = mixed_mode_test(42, 3.14f, 2.71828, 999999999LL);
    sum += (int)dresult;
    
    /* 5. Additional pressure with inline asm using many registers */
    {
        long long ll1 = 0x123456789ABCDEF0LL;
        long long ll2 = 0xFEDCBA9876543210LL;
        long long ll3, ll4;
        
        /* Force many register reloads */
        asm volatile (
            "mov %2, %%rax\n\t"
            "mov %3, %%rbx\n\t"
            "add %%rbx, %%rax\n\t"
            "mov %%rax, %0\n\t"
            "sub %%rbx, %%rax\n\t"
            "mov %%rax, %1\n\t"
            "imul %4, %0\n\t"
            "xor %5, %1"
            : "=&r" (ll3), "=&r" (ll4)
            : "r" (ll1), "r" (ll2), "r" (sum), "r" (0xFF)
            : "%rax", "%rbx", "cc"
        );
        
        sum += (int)(ll3 >> 32) + (int)(ll4 & 0xFFFFFFFF);
    }
    
    /* 6. Memory operand with displacement too large */
    {
        int large_array[1000];
        for (i = 0; i < 1000; i++) {
            large_array[i] = i * 2;
        }
        
        /* Access with large displacement that might need reload */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0"
            : "=r" (sum)
            : "m" (large_array[500]),  /* Large displacement */
              "r" (sum)
            : "%eax"
        );
    }
    
    /* 7. Force reload with volatile and memory clobber */
    {
        volatile int vol1 = 1234;
        volatile double vol2 = 5.678;
        
        asm volatile (
            "mov %1, %%eax\n\t"
            "cvtsi2sd %%eax, %%xmm0\n\t"
            "addsd %2, %%xmm0\n\t"
            "cvttsd2si %%xmm0, %%eax\n\t"
            "mov %%eax, %0"
            : "=m" (vol1)
            : "m" (vol1), "m" (vol2)
            : "%eax", "%xmm0", "memory"
        );
        
        sum += vol1;
    }
    
    printf("Result checksum: %d\n", sum);
    return sum;
}
