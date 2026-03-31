/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force no optimization on specific variables */
#define VOLATILE_VAR volatile

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

/* Function to create complex addressing patterns */
int complex_address(int i, int j, int k) {
    /* Multi-dimensional array with non-constant indices */
    VOLATILE_VAR int md_array[10][20][30];
    
    /* This address computation should trigger address reloads */
    int result;
    int *addr = &md_array[i][j][k];
    
    /* Block B: Complex address reload with memory constraint */
    asm volatile (
        "movl (%1), %0\n\t"
        : "=r" (result)
        : "m" (*addr)
        : "memory"
    );
    
    return result;
}

/* Function with early-clobber and multiple outputs */
void early_clobber_test(int a, int b, int c, int *out1, int *out2) {
    int tmp1, tmp2;
    
    /* Block C: Early-clobber with multiple outputs */
    asm volatile (
        "movl %2, %0\n\t"      /* out1 gets a */
        "addl %3, %0\n\t"      /* out1 += b */
        "movl %0, %1\n\t"      /* out2 gets out1 (early clobber!) */
        "imull %4, %1\n\t"     /* out2 *= c */
        : "=&r" (tmp1), "=&r" (tmp2)  /* Both early-clobber */
        : "r" (a), "r" (b), "r" (c)
        : "cc"
    );
    
    *out1 = tmp1;
    *out2 = tmp2;
}

/* Function to force register class conflicts */
void register_class_conflict(double d, float f, int i) {
    double d_result;
    float f_result;
    int i_result;
    
    /* Block A: Register class conflict - integer in FP register */
    asm volatile (
        "cvtsi2sd %2, %0\n\t"   /* Convert int to double */
        "cvtss2sd %3, %1\n\t"   /* Convert float to double */
        : "=f" (d_result), "=f" (f_result)
        : "r" (i), "f" (f)      /* 'i' in general reg, 'f' in FP reg */
        : 
    );
    
    /* Use results to prevent optimization */
    global_doubles[0] = d_result + f_result;
    
    /* Another conflict: FP value in integer register */
    asm volatile (
        "movd %1, %0\n\t"       /* Move float to integer register */
        : "=r" (i_result)
        : "f" (f)
        : 
    );
    
    global_array[0] = i_result;
}

/* Function with secondary reload patterns */
void secondary_reload_test(void) {
    __m128i vec1, vec2;
    long long large_imm = 0x123456789ABCDEF0LL;
    
    /* Block D: Pattern that may require secondary reloads */
    /* Loading large immediate may need temporary register */
    asm volatile (
        "movq %1, %%rax\n\t"    /* Load 64-bit immediate to RAX */
        "movq %%rax, %0\n\t"    /* Move to output (may need reload) */
        : "=r" (large_imm)
        : "i" (0x123456789ABCDEF0LL)
        : "rax", "cc"
    );
    
    /* Vector operations that might need secondary reloads */
    vec1 = _mm_set_epi32(1, 2, 3, 4);
    vec2 = _mm_set_epi32(5, 6, 7, 8);
    
    /* Complex vector operation with memory operand */
    asm volatile (
        "paddd %1, %0\n\t"
        : "+x" (vec1)
        : "xm" (vec2)  /* May need reload if vec2 not in register */
        : 
    );
    
    global_vecs[0] = vec1;
}

/* Function with high register pressure */
void high_pressure_test(void) {
    /* Many live variables to force spills and reloads */
    VOLATILE_VAR int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    VOLATILE_VAR int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    VOLATILE_VAR double d1 = 1.0, d2 = 2.0, d3 = 3.0;
    VOLATILE_VAR float f1 = 1.0f, f2 = 2.0f;
    VOLATILE_VAR __m128i vec1, vec2, vec3;
    
    /* Use all variables in complex inline asm */
    asm volatile (
        "/* Start high pressure block */\n\t"
        "movl %0, %%eax\n\t"
        "addl %1, %%eax\n\t"
        "movl %%eax, %2\n\t"
        "cvtsi2sd %3, %%xmm0\n\t"
        "addsd %4, %%xmm0\n\t"
        "movsd %%xmm0, %5\n\t"
        "/* End high pressure block */\n\t"
        : "+r" (v1), "+r" (v2), "=m" (global_array[0]),
          "+r" (v3), "+f" (d1), "=m" (global_doubles[0])
        : 
        : "rax", "xmm0", "cc", "memory"
    );
}

/* Main function orchestrating all tests */
int main(void) {
    int result = 0;
    int out1, out2;
    struct nested ns[4];
    
    /* Initialize globals */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Test 1: Complex address reloads */
    result += complex_address(1, 2, 3);
    
    /* Test 2: Early clobber with multiple outputs */
    early_clobber_test(10, 20, 30, &out1, &out2);
    result += out1 + out2;
    
    /* Test 3: Register class conflicts */
    register_class_conflict(3.14, 2.71f, 42);
    result += global_array[0];
    
    /* Test 4: Secondary reload patterns */
    secondary_reload_test();
    
    /* Test 5: High register pressure */
    high_pressure_test();
    
    /* Test 6: Mixed types in asm with constraints */
    {
        long long ll1 = 0xFFFFFFFF, ll2 = 0xAAAAAAAA;
        double dd1 = 3.14159, dd2 = 2.71828;
        __m128i vv1, vv2;
        
        /* Mixed constraints forcing different reload types */
        asm volatile (
            "movq %1, %%rax\n\t"
            "addq %2, %%rax\n\t"
            "movq %%rax, %0\n\t"
            "/* Switch to FP */\n\t"
            "movsd %3, %%xmm0\n\t"
            "addsd %4, %%xmm0\n\t"
            "movsd %%xmm0, %5\n\t"
            : "=r" (ll1), "+r" (ll2), "+f" (dd1), "=f" (dd2)
            : "3" (dd1), "4" (dd2)  /* Matching constraints add complexity */
            : "rax", "xmm0", "cc"
        );
        
        result += (int)ll1 + (int)dd1;
    }
    
    /* Test 7: Memory operand with complex addressing */
    {
        VOLATILE_VAR int idx = 100;
        /* Create complex addressing: global_array[idx*2 + idx/2] */
        asm volatile (
            "movl %1, %%eax\n\t"
            "leal (%%eax,%%eax,2), %%ecx\n\t"  /* idx * 3 */
            "movl global_array(,%%ecx,4), %0\n\t"  /* global_array[idx*3] */
            : "=r" (result)
            : "r" (idx)
            : "rax", "rcx", "memory"
        );
    }
    
    /* Final checksum to prevent optimization */
    printf("Result checksum: %d\n", result);
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
