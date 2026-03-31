/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout);
void test_secondary_reloads(int iterations, long *lin, long *lout, float *fin, float *fout);
void test_optional_reloads(int iterations, char *cin, char *cout, __m128i *vin, __m128i *vout);
void test_control_flow_reloads(int mode, int *data, int *result, int size);

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_mode = 0;

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 2;
    
    /* Parse command line arguments */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    /* Initialize arrays with mixed data types */
    int *int_in = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_out = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double *double_in = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *double_out = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    long *long_in = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    long *long_out = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    float *float_in = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *float_out = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    char *char_in = (char*)aligned_alloc(64, ARRAY_SIZE * sizeof(char));
    char *char_out = (char*)aligned_alloc(64, ARRAY_SIZE * sizeof(char));
    __m128i *vec_in = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    __m128i *vec_out = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    
    /* Fill arrays with data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_in[i] = i * 3 + 1;
        double_in[i] = i * 1.5;
        long_in[i] = i * 7L;
        float_in[i] = i * 2.3f;
        char_in[i] = (i % 256);
        vec_in[i] = _mm_set_epi32(i, i+1, i+2, i+3);
    }
    
    /* Execute test functions to trigger reloads */
    test_primary_reloads(iterations, int_in, int_out, double_in, double_out);
    test_secondary_reloads(iterations, long_in, long_out, float_in, float_out);
    test_optional_reloads(iterations, char_in, char_out, vec_in, vec_out);
    test_control_flow_reloads(mode, int_in, int_out, ARRAY_SIZE);
    
    /* Compute checksum to ensure all assembly executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_out[i];
        checksum += (unsigned long long)double_out[i];
        checksum += long_out[i];
        checksum += (unsigned int)(float_out[i] * 1000);
        checksum += char_out[i];
        checksum += _mm_extract_epi32(vec_out[i], 0);
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(int_in); free(int_out);
    free(double_in); free(double_out);
    free(long_in); free(long_out);
    free(float_in); free(float_out);
    free(char_in); free(char_out);
    free(vec_in); free(vec_out);
    
    return 0;
}

/* Primary reloads with register pressure */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout) {
    /* Create many live variables to exhaust registers */
    int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
    double d0, d1, d2, d3, d4, d5, d6, d7;
    long l0, l1, l2, l3;
    
    /* Unrolled loop with complex inline assembly */
    for (int i = 0; i < iterations; i++) {
        /* Load many values into registers */
        a0 = in[i*16 + 0]; a1 = in[i*16 + 1]; a2 = in[i*16 + 2]; a3 = in[i*16 + 3];
        a4 = in[i*16 + 4]; a5 = in[i*16 + 5]; a6 = in[i*16 + 6]; a7 = in[i*16 + 7];
        a8 = in[i*16 + 8]; a9 = in[i*16 + 9]; a10 = in[i*16 + 10]; a11 = in[i*16 + 11];
        a12 = in[i*16 + 12]; a13 = in[i*16 + 13]; a14 = in[i*16 + 14]; a15 = in[i*16 + 15];
        
        d0 = din[i*8 + 0]; d1 = din[i*8 + 1]; d2 = din[i*8 + 2]; d3 = din[i*8 + 3];
        d4 = din[i*8 + 4]; d5 = din[i*8 + 5]; d6 = din[i*8 + 6]; d7 = din[i*8 + 7];
        
        /* Complex inline assembly with multiple operands and constraints */
        __asm__ volatile (
            /* Mixed constraints: r=register, m=memory, i=immediate, a=accumulator, d=data */
            "movl %[a0], %%eax\n\t"
            "addl %[a1], %%eax\n\t"
            "imull %[imm], %%eax\n\t"
            "movl %%eax, %[out0]\n\t"
            
            "movq %[d0], %%xmm0\n\t"
            "addsd %[d1], %%xmm0\n\t"
            "mulsd %[d2], %%xmm0\n\t"
            "movq %%xmm0, %[dout0]\n\t"
            
            /* Early clobber and matching constraints */
            "movl %[a2], %[out1]\n\t"
            "addl %[a3], %[out1]\n\t"
            
            /* Byte register constraint */
            "movb %[byte], %%al\n\t"
            "addb $0x10, %%al\n\t"
            "movb %%al, %[byteout]\n\t"
            
            /* Top of stack constraint for x87 */
            "fldl %[d3]\n\t"
            "faddl %[d4]\n\t"
            "fstpl %[dout1]\n\t"
            
            : [out0] "=r" (out[i*16 + 0]),    /* General register output */
              [out1] "=&r" (out[i*16 + 1]),   /* Early clobber */
              [dout0] "=m" (dout[i*8 + 0]),   /* Memory output */
              [dout1] "=m" (dout[i*8 + 1]),   /* Memory output for x87 */
              [byteout] "=q" (out[i*16 + 2])  /* Byte register output */
            : [a0] "r" (a0), [a1] "r" (a1), [a2] "r" (a2), [a3] "r" (a3),
              [d0] "r" (d0), [d1] "r" (d1), [d2] "r" (d2),
              [d3] "m" (d3), [d4] "m" (d4),  /* Memory constraints for x87 */
              [imm] "i" (42),                 /* Immediate constraint */
              [byte] "r" ((char)a4)           /* Byte-sized input */
            : "eax", "edx", "xmm0", "xmm1", "st", "st(1)", "memory", "cc"
        );
        
        /* More assembly with different constraints */
        __asm__ volatile (
            "leal (%[a5], %[a6], 2), %%ecx\n\t"
            "addl %[a7], %%ecx\n\t"
            "movl %%ecx, %[out2]\n\t"
            
            "movl %[a8], %%edx\n\t"
            "shrl $3, %%edx\n\t"
            "movl %%edx, %[out3]\n\t"
            
            : [out2] "=r" (out[i*16 + 3]),
              [out3] "=r" (out[i*16 + 4])
            : [a5] "r" (a5), [a6] "r" (a6), [a7] "r" (a7), [a8] "r" (a8)
            : "ecx", "edx", "cc"
        );
    }
}

/* Secondary reload patterns */
void test_secondary_reloads(int iterations, long *lin, long *lout, float *fin, float *fout) {
    for (int i = 0; i < iterations; i++) {
        long l0 = lin[i*4 + 0];
        long l1 = lin[i*4 + 1];
        long l2 = lin[i*4 + 2];
        long l3 = lin[i*4 + 3];
        float f0 = fin[i*4 + 0];
        float f1 = fin[i*4 + 1];
        float f2 = fin[i*4 + 2];
        float f3 = fin[i*4 + 3];
        
        /* Assembly requiring secondary reloads due to constraint mismatches */
        __asm__ volatile (
            /* 'a' constraint (accumulator) forcing specific register */
            "movq %[l0], %%rax\n\t"
            "addq %[l1], %%rax\n\t"
            "movq %%rax, %[lout0]\n\t"
            
            /* Result used later with different constraint */
            "movq %[lout0], %%rbx\n\t"
            "subq %[l2], %%rbx\n\t"
            "movq %%rbx, %[lout1]\n\t"
            
            /* 'R' constraint for legacy register (may need secondary reload) */
            "movl %[f0], %%eax\n\t"
            "addl %[f1], %%eax\n\t"
            "movl %%eax, %[fout0]\n\t"
            
            : [lout0] "=a" (lout[i*4 + 0]),   /* Must be in RAX */
              [lout1] "=b" (lout[i*4 + 1]),   /* Must be in RBX */
              [fout0] "=R" (fout[i*4 + 0])    /* Legacy register constraint */
            : [l0] "rm" (l0),                 /* Register or memory - may need secondary */
              [l1] "rm" (l1),
              [l2] "r" (l2),
              [f0] "r" (*(int*)&f0),          /* Bitcast float to int for integer reg */
              [f1] "r" (*(int*)&f1)
            : "rax", "rbx", "rcx", "memory", "cc"
        );
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* More complex case with multiple constraints */
        __asm__ volatile (
            "movss %[f2], %%xmm0\n\t"
            "addss %[f3], %%xmm0\n\t"
            "movss %%xmm0, %[fout1]\n\t"
            
            "movq %[l3], %%rcx\n\t"
            "imulq $0x12345678, %%rcx, %%rcx\n\t"
            "movq %%rcx, %[lout2]\n\t"
            
            : [fout1] "=m" (fout[i*4 + 1]),   /* Memory output */
              [lout2] "=r" (lout[i*4 + 2])    /* Register output */
            : [f2] "x" (f2),                  /* SSE register constraint */
              [f3] "x" (f3),
              [l3] "r" (l3)
            : "xmm0", "xmm1", "rcx", "memory", "cc"
        );
    }
}

/* Optional reloads and nocombine patterns */
void test_optional_reloads(int iterations, char *cin, char *cout, __m128i *vin, __m128i *vout) {
    for (int i = 0; i < iterations; i++) {
        char c0 = cin[i*8 + 0];
        char c1 = cin[i*8 + 1];
        char c2 = cin[i*8 + 2];
        char c3 = cin[i*8 + 3];
        __m128i v0 = vin[i*2 + 0];
        __m128i v1 = vin[i*2 + 1];
        
        /* Optional constraints with '?' modifier */
        __asm__ volatile (
            "movd %[c0], %%mm0\n\t"
            "paddb %[c1], %%mm0\n\t"
            "movd %%mm0, %[cout0]\n\t"
            
            : [cout0] "=?r" (cout[i*8 + 0])   /* Optional output */
            : [c0] "r" ((int)c0),
              [c1] "r" ((int)c1)
            : "mm0", "mm1", "cc"
        );
        
        /* Volatile barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar but not identical asm - won't combine due to different clobbers */
        __asm__ volatile (
            "movd %[c2], %%mm1\n\t"
            "psubb %[c3], %%mm1\n\t"
            "movd %%mm1, %[cout1]\n\t"
            
            : [cout1] "=r" (cout[i*8 + 1])
            : [c2] "r" ((int)c2),
              [c3] "r" ((int)c3)
            : "mm1", "mm2", "cc"  /* Different clobber list */
        );
        
        /* Vector operations with many operands */
        __asm__ volatile (
            "movdqa %[v0], %%xmm2\n\t"
            "paddd %[v1], %%xmm2\n\t"
            "pslld $2, %%xmm2\n\t"
            "movdqa %%xmm2, %[vout0]\n\t"
            
            "pmuludq %[v0], %%xmm2\n\t"
            "movdqa %%xmm2, %[vout1]\n\t"
            
            : [vout0] "=x" (vout[i*2 + 0]),   /* SSE register output */
              [vout1] "=x" (vout[i*2 + 1])
            : [v0] "x" (v0),                  /* SSE register input */
              [v1] "x" (v1)
            : "xmm2", "xmm3", "xmm4", "cc"
        );
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int mode, int *data, int *result, int size) {
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    
    for (int i = 0; i < size - 8; i++) {
        /* Load multiple values that will be live across control flow */
        temp1 = data[i];
        temp2 = data[i+1];
        temp3 = data[i+2];
        temp4 = data[i+3];
        temp5 = data[i+4];
        temp6 = data[i+5];
        temp7 = data[i+6];
        temp8 = data[i+7];
        
        /* Conditional execution paths with inline assembly */
        if (mode & 0x1) {
            __asm__ volatile (
                "movl %[t1], %%eax\n\t"
                "addl %[t2], %%eax\n\t"
                "movl %%eax, %[r1]\n\t"
                
                "movl %[t3], %%ebx\n\t"
                "subl %[t4], %%ebx\n\t"
                "movl %%ebx, %[r2]\n\t"
                
                : [r1] "=r" (result[i]),
                  [r2] "=r" (result[i+1])
                : [t1] "r" (temp1),
                  [t2] "r" (temp2),
                  [t3] "r" (temp3),
                  [t4] "r" (temp4)
                : "eax", "ebx", "cc"
            );
        } else {
            __asm__ volatile (
                "movl %[t1], %%ecx\n\t"
                "imull %[t2], %%ecx\n\t"
                "movl %%ecx, %[r1]\n\t"
                
                : [r1] "=r" (result[i])
                : [t1] "r" (temp1),
                  [t2] "r" (temp2)
                : "ecx", "cc"
            );
        }
        
        /* Loop with assembly inside */
        for (int j = 0; j < 4; j++) {
            if (mode & (1 << j)) {
                __asm__ volatile (
                    "movl %[t5], %%edx\n\t"
                    "addl $0x%[imm], %%edx\n\t"
                    "movl %%edx, %[r3]\n\t"
                    
                    : [r3] "=r" (result[i+2+j])
                    : [t5] "r" (temp5 + j),
                      [imm] "i" (j * 10)      /* Immediate varies with loop */
                    : "edx", "cc"
                );
            }
        }
        
        /* Switch statement with different asm blocks */
        switch (mode % 4) {
            case 0:
                __asm__ volatile (
                    "xorl %%eax, %%eax\n\t"
                    "addl %[t6], %%eax\n\t"
                    "movl %%eax, %[r4]\n\t"
                    
                    : [r4] "=r" (result[i+6])
                    : [t6] "r" (temp6)
                    : "eax", "cc"
                );
                break;
            case 1:
                __asm__ volatile (
                    "movl $1, %%eax\n\t"
                    "orl %[t7], %%eax\n\t"
                    "movl %%eax, %[r5]\n\t"
                    
                    : [r5] "=r" (result[i+6])
                    : [t7] "r" (temp7)
                    : "eax", "cc"
                );
                break;
            default:
                __asm__ volatile (
                    "movl %[t8], %%eax\n\t"
                    "andl $0xFF, %%eax\n\t"
                    "movl %%eax, %[r6]\n\t"
                    
                    : [r6] "=r" (result[i+6])
                    : [t8] "r" (temp8)
                    : "eax", "cc"
                );
                break;
        }
    }
}
