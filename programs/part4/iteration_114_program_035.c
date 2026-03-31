/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 256

/* Test functions */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout);
void test_secondary_reloads(int iterations, long long *in, long long *out, float *fin, float *fout);
void test_optional_reloads(int iterations, char *in, char *out, __m128i *vin, __m128i *vout);
void test_control_flow_reloads(int mode, int *data, int *result);

/* Global variables to increase register pressure */
volatile int global_counter = 0;
volatile double global_double = 3.14159;

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 2;
    
    /* Parse command line */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    /* Allocate and initialize arrays with mixed types */
    int *int_in = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_out = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    long long *ll_in = (long long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long long));
    long long *ll_out = (long long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long long));
    double *double_in = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *double_out = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *float_in = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *float_out = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    char *char_in = (char*)aligned_alloc(64, ARRAY_SIZE * sizeof(char));
    char *char_out = (char*)aligned_alloc(64, ARRAY_SIZE * sizeof(char));
    __m128i *vec_in = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    __m128i *vec_out = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_in[i] = i * 3 + 1;
        ll_in[i] = i * 5LL + 2;
        double_in[i] = i * 1.5 + 3.14;
        float_in[i] = i * 0.75f + 1.618f;
        char_in[i] = (i % 26) + 'A';
        vec_in[i] = _mm_set_epi32(i, i+1, i+2, i+3);
    }
    
    /* Execute test functions with heavy register pressure */
    test_primary_reloads(iterations, int_in, int_out, double_in, double_out);
    test_secondary_reloads(iterations, ll_in, ll_out, float_in, float_out);
    test_optional_reloads(iterations, char_in, char_out, vec_in, vec_out);
    
    /* Control flow dependent reloads */
    int *cf_data = (int*)aligned_alloc(64, 128 * sizeof(int));
    int *cf_result = (int*)aligned_alloc(64, 128 * sizeof(int));
    for (int i = 0; i < 128; i++) cf_data[i] = i * 7;
    test_control_flow_reloads(mode, cf_data, cf_result);
    
    /* Compute checksum to ensure all asm executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_out[i];
        checksum += ll_out[i];
        checksum += (unsigned long long)double_out[i];
        checksum += (unsigned long long)float_out[i];
        checksum += char_out[i];
        
        /* Extract from vector */
        unsigned int *v = (unsigned int*)&vec_out[i];
        checksum += v[0] + v[1] + v[2] + v[3];
    }
    for (int i = 0; i < 128; i++) {
        checksum += cf_result[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(int_in); free(int_out);
    free(ll_in); free(ll_out);
    free(double_in); free(double_out);
    free(float_in); free(float_out);
    free(char_in); free(char_out);
    free(vec_in); free(vec_out);
    free(cf_data); free(cf_result);
    
    return 0;
}

/* Primary reloads with many operands and mixed constraints */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout) {
    /* Many live variables to create register pressure */
    int a = in[0], b = in[1], c = in[2], d = in[3];
    int e = in[4], f = in[5], g = in[6], h = in[7];
    double da = din[0], db = din[1], dc = din[2], dd = din[3];
    
    /* Unrolled loop with complex asm */
    for (int i = 0; i < iterations; i++) {
        int idx = i % (ARRAY_SIZE - 8);
        
        /* Complex asm with 7 operands, mixed constraints and modes */
        __asm__ volatile (
            /* Outputs with different register classes and modes */
            "=r" (a),        /* General register, word mode */
            "=&q" (b),       /* Byte register (earlyclobber) */
            "=t" (da),       /* Top of FP stack */
            "=a" (c),        /* Accumulator */
            "=d" (d),        /* Data register */
            "=r" (e),        /* General register */
            "=m" (out[idx])  /* Memory output */
            
            /* Inputs with mixed constraints */
            : "0" (a),       /* Matching constraint with output 0 */
            "i" (0x1234),    /* Immediate */
            "r" (b),         /* General register */
            "m" (in[idx]),   /* Memory */
            "r" (c),         /* General register */
            "g" (d),         /* General or memory */
            "r" (e),         /* General register */
            "a" (f),         /* Accumulator */
            "d" (g),         /* Data register */
            "t" (da),        /* Top of FP stack */
            "u" (db)         /* Second FP register */
            
            /* Extensive clobber list */
            : "cc", "memory", "r8", "r9", "r10", "r11", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        
        /* Another asm with different constraints to prevent combining */
        __asm__ volatile (
            "movl %1, %0\n\t"
            "addl %%ecx, %0\n\t"
            "imull %2, %0"
            : "=r" (f), "=&r" (g), "=r" (h)
            : "1" (in[idx+1]), "r" (global_counter), "m" (in[idx+2])
            : "cc", "ecx"
        );
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Use results to prevent dead code elimination */
        out[idx] = a + b + c + d + e + f + g + h;
        dout[idx] = da + db;
        
        /* Update many live variables */
        a = (a * 3) ^ b;
        b = (b + c) | d;
        c = c ^ e;
        d = d + f;
        e = e * g;
        da = da * 1.1;
        db = db + 0.5;
    }
}

/* Secondary reload patterns with mismatched constraints */
void test_secondary_reloads(int iterations, long long *in, long long *out, float *fin, float *fout) {
    /* Variables that will force secondary reloads */
    long long r8_val, r9_val, r10_val;
    float xmm0_val, xmm1_val, xmm2_val;
    
    for (int i = 0; i < iterations; i++) {
        int idx = i % (ARRAY_SIZE - 4);
        
        /* Force secondary reload by using "R" constraint (legacy register)
           with values that might be allocated to R8-R15 */
        __asm__ volatile (
            /* Try to force operand into legacy register */
            "movq %[input], %%rax\n\t"
            "addq $1, %%rax\n\t"
            "movq %%rax, %[output]"
            : [output] "=R" (r8_val)  /* Legacy register constraint */
            : [input] "rm" (in[idx]),  /* Register or memory - may need secondary reload */
              "r" (global_counter)
            : "rax", "cc"
        );
        
        /* Another pattern: "a" constraint followed by "b" constraint */
        int ax_val, bx_val;
        __asm__ volatile (
            "movl %2, %%eax\n\t"
            "addl $100, %%eax"
            : "=a" (ax_val)
            : "0" (in[idx] & 0xFFFFFFFF), "m" (fin[idx])
            : "cc"
        );
        
        /* Force move from eax to ebx via secondary reload */
        __asm__ volatile (
            "movl %%eax, %%ebx\n\t"
            "imull $3, %%ebx"
            : "=b" (bx_val)
            : "a" (ax_val)
            : "cc"
        );
        
        /* XMM register pressure with mixed constraints */
        __asm__ volatile (
            "addss %1, %0\n\t"
            "mulss %2, %0"
            : "=x" (xmm0_val), "=&x" (xmm1_val)
            : "0" (fin[idx]), "x" (fin[idx+1]), "m" (fin[idx+2])
            : "cc"
        );
        
        /* Use results */
        out[idx] = r8_val + bx_val;
        fout[idx] = xmm0_val + xmm1_val;
        
        /* Create more register pressure */
        __m128 v1 = _mm_set_ps(fin[idx], fin[idx+1], fin[idx+2], fin[idx+3]);
        __m128 v2 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
        __m128 v3 = _mm_add_ps(v1, v2);
        __m128 v4 = _mm_mul_ps(v3, v2);
        
        /* Extract to increase live range */
        float farr[4];
        _mm_store_ps(farr, v4);
        fout[idx+1] = farr[0] + farr[1] + farr[2] + farr[3];
    }
}

/* Optional reloads and nocombine patterns */
void test_optional_reloads(int iterations, char *in, char *out, __m128i *vin, __m128i *vout) {
    char opt1, opt2, opt3;
    __m128i vtmp1, vtmp2;
    
    for (int i = 0; i < iterations; i++) {
        int idx = i % (ARRAY_SIZE - 8);
        
        /* Optional constraints with '?' modifier */
        __asm__ volatile (
            "testb %2, %2\n\t"
            "jz 1f\n\t"
            "movb %3, %b0\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "movb $0, %b0\n\t"
            "2:\n\t"
            "addb $1, %b0"
            : "=?r" (opt1), "=?q" (opt2)  /* Optional outputs */
            : "r" (i & 1), "m" (in[idx]), "i" ('A')
            : "cc"
        );
        
        /* Volatile barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that could be combined but won't due to barrier */
        __asm__ volatile (
            "movb %1, %b0\n\t"
            "subb $32, %b0"
            : "=q" (opt3)
            : "rm" (in[idx+1])
            : "cc"
        );
        
        /* Vector operations with many live values */
        __m128i v1 = vin[idx];
        __m128i v2 = vin[idx+1];
        __m128i v3, v4, v5, v6;
        
        /* Multiple asm blocks that could combine but have different clobbers */
        __asm__ volatile (
            "paddd %1, %0"
            : "+x" (v1)
            : "xm" (v2)
            : "cc"
        );
        
        __asm__ volatile (
            "pxor %1, %0"
            : "=x" (vtmp1)
            : "xm" (v1), "0" (v2)
            : "cc"
        );
        
        /* Different clobber prevents combination */
        __asm__ volatile (
            "pslld $2, %0"
            : "+x" (vtmp1)
            :: "cc"
        );
        
        /* Use all optional results */
        out[idx] = opt1 + opt2 + opt3;
        vout[idx] = _mm_add_epi32(v1, vtmp1);
        
        /* Create many live vector variables */
        v3 = _mm_slli_epi32(vout[idx], 1);
        v4 = _mm_srli_epi32(v3, 2);
        v5 = _mm_add_epi32(v4, vtmp1);
        v6 = _mm_sub_epi32(v5, v1);
        
        /* Use them to prevent optimization */
        char *vbytes = (char*)&v6;
        for (int j = 0; j < 8; j++) {
            out[idx + j] ^= vbytes[j];
        }
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int mode, int *data, int *result) {
    int temp1, temp2, temp3, temp4;
    int a = data[0], b = data[1], c = data[2], d = data[3];
    
    /* Complex control flow */
    for (int i = 0; i < 128; i++) {
        if (mode & 1) {
            /* Branch 1: asm with specific constraints */
            __asm__ volatile (
                "movl %1, %%eax\n\t"
                "leal (%%eax, %%eax, 2), %0"
                : "=r" (temp1)
                : "rm" (data[i]), "r" (i)
                : "eax", "cc"
            );
        } else {
            /* Branch 2: different asm pattern */
            __asm__ volatile (
                "imull $3, %1, %0"
                : "=r" (temp1)
                : "r" (data[i])
                : "cc"
            );
        }
        
        /* Nested conditionals */
        for (int j = 0; j < 4; j++) {
            if ((i + j) % 3 == 0) {
                __asm__ volatile (
                    "addl %%ecx, %0\n\t"
                    "xorl %%edx, %0"
                    : "+r" (temp1)
                    : "c" (j), "d" (mode)
                    : "cc"
                );
            } else if ((i + j) % 3 == 1) {
                __asm__ volatile (
                    "subl %1, %0\n\t"
                    "negl %0"
                    : "+r" (temp1)
                    : "r" (global_counter)
                    : "cc"
                );
            }
            
            /* Loop-carried dependency */
            __asm__ volatile (
                "rorl $3, %0"
                : "+r" (temp1)
                :: "cc"
            );
        }
        
        /* Switch statement with asm in cases */
        switch (i % 4) {
            case 0:
                __asm__ volatile ("addl $100, %0" : "+r" (temp1) :: "cc");
                break;
            case 1:
                __asm__ volatile ("subl $50, %0" : "+r" (temp1) :: "cc");
                break;
            case 2:
                __asm__ volatile ("imull $2, %0, %0" : "+r" (temp1) :: "cc");
                break;
            case 3:
                __asm__ volatile ("xorl $0xFF, %0" : "+r" (temp1) :: "cc");
                break;
        }
        
        result[i] = temp1 + a + b + c + d;
        
        /* Update live variables across iterations */
        a = (a + b) ^ c;
        b = (b + temp1) | d;
        c = c * 3;
        d = d + i;
    }
}
