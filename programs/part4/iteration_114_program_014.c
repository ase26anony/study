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
void test_control_flow_reloads(int mode, int count, int *results);

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_mode = 0;

/* Complex inline assembly with many operands */
static inline void complex_asm_5ops(int a, int b, int c, int *d, long e, int *result)
{
    int tmp1, tmp2, tmp3;
    long tmp4;
    
    /* 5+ operands with mixed constraints */
    __asm__ volatile (
        "movl %[a_in], %[tmp1]\n\t"
        "addl %[b_in], %[tmp1]\n\t"
        "imull %[c_in], %[tmp1]\n\t"
        "movq %[e_in], %[tmp4]\n\t"
        "addq %[tmp4], %[tmp4]\n\t"
        "movl %[tmp1], %[tmp2]\n\t"
        "subl $1, %[tmp2]\n\t"
        "leal (%[tmp1],%[tmp2],2), %[tmp3]\n\t"
        "movl %[tmp3], (%[d_ptr])\n\t"
        "movl %[tmp1], %[out]"
        : [out] "=r" (*result), [tmp1] "=&r" (tmp1), [tmp2] "=&r" (tmp2),
          [tmp3] "=&r" (tmp3), [tmp4] "=&r" (tmp4)
        : [a_in] "rm" (a), [b_in] "rm" (b), [c_in] "rm" (c),
          [d_ptr] "r" (d), [e_in] "rm" (e)
        : "memory", "cc"
    );
}

/* Assembly with different machine modes */
static inline void mixed_mode_asm(char *b, short *s, int *i, long *l, float *f, double *d)
{
    char b_out;
    short s_out;
    int i_out;
    long l_out;
    float f_out;
    double d_out;
    
    __asm__ volatile (
        "movb (%[bin]), %%al\n\t"
        "movb %%al, %[bout]\n\t"
        "movw (%[sin]), %%ax\n\t"
        "movw %%ax, %[sout]\n\t"
        "movl (%[iin]), %%eax\n\t"
        "movl %%eax, %[iout]\n\t"
        "movq (%[lin]), %%rax\n\t"
        "movq %%rax, %[lout]\n\t"
        "movss (%[fin]), %%xmm0\n\t"
        "movss %%xmm0, %[fout]\n\t"
        "movsd (%[din]), %%xmm1\n\t"
        "movsd %%xmm1, %[dout]"
        : [bout] "=q" (b_out), [sout] "=r" (s_out), [iout] "=r" (i_out),
          [lout] "=r" (l_out), [fout] "=t" (f_out), [dout] "=t" (d_out)
        : [bin] "r" (b), [sin] "r" (s), [iin] "r" (i),
          [lin] "r" (l), [fin] "r" (f), [din] "r" (d)
        : "rax", "xmm0", "xmm1", "memory"
    );
    
    *b = b_out;
    *s = s_out;
    *i = i_out;
    *l = l_out;
    *f = f_out;
    *d = d_out;
}

/* Assembly requiring secondary reloads */
static inline void secondary_reload_asm(int *mem, int reg, int *out)
{
    int tmp;
    
    /* "a" constraint forces accumulator, may need secondary reload */
    __asm__ volatile (
        "movl %[reg_in], %%eax\n\t"
        "addl (%[mem_in]), %%eax\n\t"
        "movl %%eax, %[tmp]\n\t"
        "movl %[tmp], %%ebx\n\t"  /* Force move to another register */
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, (%[out_ptr])"
        : [tmp] "=&r" (tmp), [out] "=m" (*out)
        : [reg_in] "a" (reg), [mem_in] "rm" (mem), [out_ptr] "r" (out)
        : "rax", "rbx", "memory", "cc"
    );
}

/* Assembly with optional constraints */
static inline int optional_reload_asm(int a, int b, int *c, int d)
{
    int result;
    int optional_out;
    
    __asm__ volatile (
        "testl %[a_in], %[a_in]\n\t"
        "jz 1f\n\t"
        "movl %[b_in], %[opt_out]\n\t"
        "jmp 2f\n\t"
        "1:\n\t"
        "movl $0, %[opt_out]\n\t"
        "2:\n\t"
        "addl %[opt_out], %[d_in]\n\t"
        "movl %[d_in], %[result]"
        : [result] "=r" (result), [opt_out] "=?r" (optional_out)
        : [a_in] "rm" (a), [b_in] "rm" (b), [d_in] "0" (d)
        : "cc"
    );
    
    if (c) *c = optional_out;
    return result;
}

void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout)
{
    int i, j;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    double dtmp1, dtmp2, dtmp3, dtmp4;
    
    /* Create many live variables to increase register pressure */
    int live_vars[UNROLL_FACTOR];
    double live_doubles[UNROLL_FACTOR];
    
    for (i = 0; i < UNROLL_FACTOR; i++) {
        live_vars[i] = i * 7;
        live_doubles[i] = i * 3.14159;
    }
    
    for (i = 0; i < iterations; i++) {
        /* Unrolled loop with many asm statements */
        for (j = 0; j < UNROLL_FACTOR; j += 4) {
            /* Complex asm with 5+ operands */
            complex_asm_5ops(in[i+j], live_vars[j], global_counter, 
                           &out[i+j], (long)din[i+j], &tmp1);
            
            /* Mixed mode asm */
            mixed_mode_asm((char*)&in[i+j+1], (short*)&in[i+j+1], 
                          &in[i+j+1], (long*)&in[i+j+1],
                          (float*)&din[i+j+1], &dout[i+j+1]);
            
            /* Another complex asm with earlyclobber */
            __asm__ volatile (
                "movl %[a], %%eax\n\t"
                "addl %[b], %%eax\n\t"
                "movl %%eax, %[c]\n\t"
                "movl %[d], %%ebx\n\t"
                "imull %%ebx, %%eax\n\t"
                "movl %%eax, %[out1]\n\t"
                "movl %[e], %%ecx\n\t"
                "addl %%ecx, %%ebx\n\t"
                "movl %%ebx, %[out2]"
                : [out1] "=&r" (tmp2), [out2] "=&r" (tmp3), [c] "=&r" (tmp4)
                : [a] "rm" (live_vars[j]), [b] "rm" (live_vars[j+1]),
                  [d] "rm" (live_vars[j+2]), [e] "rm" (live_vars[j+3])
                : "rax", "rbx", "rcx", "cc"
            );
            
            out[i+j] = tmp1 + tmp2 + tmp3;
            
            /* Use matching constraints to force specific allocation */
            __asm__ volatile (
                "movl %[in1], %[out1]\n\t"
                "addl %[in2], %[out1]\n\t"
                "movl %[out1], %[out2]"
                : [out1] "=r" (tmp5), [out2] "=0" (tmp6)
                : [in1] "rm" (in[i+j]), [in2] "rm" (global_counter)
                : "cc"
            );
            
            live_vars[j] = tmp5;
            live_vars[j+1] = tmp6;
        }
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* More operations to keep values live */
        dtmp1 = din[i] * 2.0;
        dtmp2 = dtmp1 + 1.0;
        dtmp3 = dtmp2 * dtmp2;
        dtmp4 = dtmp3 - dtmp1;
        
        __asm__ volatile (
            "movsd %[din], %%xmm0\n\t"
            "addsd %[dtmp1], %%xmm0\n\t"
            "movsd %%xmm0, %[dout]"
            : [dout] "=m" (dout[i])
            : [din] "m" (din[i]), [dtmp1] "m" (dtmp1)
            : "xmm0", "memory"
        );
        
        global_counter++;
    }
}

void test_secondary_reloads(int iterations, long *lin, long *lout, float *fin, float *fout)
{
    int i;
    long ltmp;
    float ftmp;
    
    for (i = 0; i < iterations; i++) {
        /* Force secondary reload by using specific register constraints */
        __asm__ volatile (
            "movq %[lin], %%rax\n\t"
            "addq $1, %%rax\n\t"
            "movq %%rax, %[ltmp]\n\t"
            /* Force move to another register class */
            "movq %[ltmp], %%rbx\n\t"
            "imulq %%rbx, %%rax\n\t"
            "movq %%rax, %[lout]"
            : [lout] "=m" (lout[i]), [ltmp] "=&r" (ltmp)
            : [lin] "a" (lin[i])  /* 'a' constraint forces rax */
            : "rax", "rbx", "memory", "cc"
        );
        
        /* Another secondary reload pattern */
        __asm__ volatile (
            "movl %[fin], %%eax\n\t"
            "movd %%eax, %%xmm0\n\t"
            "addss %[const], %%xmm0\n\t"
            "movd %%xmm0, %%eax\n\t"
            "movl %%eax, %[ftmp]\n\t"
            "movl %[ftmp], %%ebx\n\t"  /* Secondary move */
            "movl %%ebx, %[fout]"
            : [fout] "=m" (fout[i]), [ftmp] "=&r" (ftmp)
            : [fin] "m" (fin[i]), [const] "X" (1.0f)
            : "rax", "rbx", "xmm0", "memory"
        );
        
        /* Use legacy register constraint that may need secondary reload */
        __asm__ volatile (
            "movl %[val], %%eax\n\t"
            "shrl $1, %%eax\n\t"
            "movl %%eax, %[out]"
            : [out] "=R" (lin[i])  /* Legacy register constraint */
            : [val] "r" ((int)lin[i])
            : "rax", "cc"
        );
    }
}

void test_optional_reloads(int iterations, char *cin, char *cout, __m128i *vin, __m128i *vout)
{
    int i;
    char ctmp;
    __m128i vtmp;
    
    for (i = 0; i < iterations; i++) {
        /* Optional output constraint */
        optional_reload_asm(cin[i], cout[i], &global_counter, i);
        
        /* Memory barrier between similar asm statements */
        __asm__ volatile ("" ::: "memory");
        
        /* Another optional constraint usage */
        __asm__ volatile (
            "testb %[cin], %[cin]\n\t"
            "setnz %[ctmp]\n\t"
            "addb $32, %[ctmp]"
            : [ctmp] "=?r" (ctmp)
            : [cin] "rm" (cin[i])
            : "cc"
        );
        
        cout[i] = ctmp;
        
        /* Vector operations to increase register pressure */
        vtmp = _mm_add_epi32(vin[i], _mm_set1_epi32(i));
        vtmp = _mm_mullo_epi32(vtmp, _mm_set1_epi32(3));
        
        /* Inline asm with vector register */
        __asm__ volatile (
            "movdqa %[vin], %%xmm0\n\t"
            "paddd %[vtmp], %%xmm0\n\t"
            "movdqa %%xmm0, %[vout]"
            : [vout] "=x" (vout[i])
            : [vin] "x" (vin[i]), [vtmp] "x" (vtmp)
            : "xmm0"
        );
        
        /* Prevent combination with different clobber list */
        __asm__ volatile (
            "movdqa %[vout], %%xmm1\n\t"
            "psrld $1, %%xmm1\n\t"
            "movdqa %%xmm1, %[vout]"
            : [vout] "+x" (vout[i])
            :
            : "xmm1", "cc"
        );
    }
}

void test_control_flow_reloads(int mode, int count, int *results)
{
    int i, temp;
    
    for (i = 0; i < count; i++) {
        /* Different asm based on control flow */
        if (mode & 1) {
            __asm__ volatile (
                "movl %[i], %%eax\n\t"
                "andl $0xF, %%eax\n\t"
                "movl %%eax, %[temp]"
                : [temp] "=r" (temp)
                : [i] "rm" (i)
                : "rax", "cc"
            );
        } else {
            __asm__ volatile (
                "movl %[i], %%ebx\n\t"
                "shrl $4, %%ebx\n\t"
                "movl %%ebx, %[temp]"
                : [temp] "=r" (temp)
                : [i] "rm" (i)
                : "rbx", "cc"
            );
        }
        
        /* Nested conditional with asm */
        if (temp & 1) {
            __asm__ volatile (
                "addl $100, %[temp]\n\t"
                "movl %[temp], %[out]"
                : [out] "=r" (results[i])
                : [temp] "0" (temp)
                : "cc"
            );
        } else {
            __asm__ volatile (
                "subl $50, %[temp]\n\t"
                "movl %[temp], %[out]"
                : [out] "=r" (results[i])
                : [temp] "0" (temp)
                : "cc"
            );
        }
        
        /* Loop-dependent asm */
        __asm__ volatile (
            "cmpl $0, %[temp]\n\t"
            "jg 1f\n\t"
            "movl $1, %[mode]\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "movl $0, %[mode]\n\t"
            "2:\n\t"
            "nop"
            : [mode] "+r" (global_mode)
            : [temp] "rm" (temp)
            : "cc"
        );
    }
}

int main(int argc, char **argv)
{
    int iterations = 100;
    int mode = 1;
    
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    /* Allocate and initialize arrays with mixed types */
    int *int_in = malloc(ARRAY_SIZE * sizeof(int));
    int *int_out = malloc(ARRAY_SIZE * sizeof(int));
    long *long_in = malloc(ARRAY_SIZE * sizeof(long));
    long *long_out = malloc(ARRAY_SIZE * sizeof(long));
    float *float_in = malloc(ARRAY_SIZE * sizeof(float));
    float *float_out = malloc(ARRAY_SIZE * sizeof(float));
    double *double_in = malloc(ARRAY_SIZE * sizeof(double));
    double *double_out = malloc(ARRAY_SIZE * sizeof(double));
    char *char_in = malloc(ARRAY_SIZE * sizeof(char));
    char *char_out = malloc(ARRAY_SIZE * sizeof(char));
    __m128i *vec_in = malloc(ARRAY_SIZE * sizeof(__m128i));
    __m128i *vec_out = malloc(ARRAY_SIZE * sizeof(__m128i));
    int *results = malloc(ARRAY_SIZE * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_in[i] = i * 3;
        long_in[i] = i * 5L;
        float_in[i] = i * 1.5f;
        double_in[i] = i * 2.71828;
        char_in[i] = (i % 26) + 'A';
        vec_in[i] = _mm_set_epi32(i, i+1, i+2, i+3);
    }
    
    /* Run tests to trigger reloads */
    test_primary_reloads(iterations, int_in, int_out, double_in, double_out);
    test_secondary_reloads(iterations, long_in, long_out, float_in, float_out);
    test_optional_reloads(iterations, char_in, char_out, vec_in, vec_out);
    test_control_flow_reloads(mode, iterations, results);
    
    /* Compute checksum to ensure all asm executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_out[i];
        checksum += long_out[i];
        checksum += (int)float_out[i];
        checksum += (long long)double_out[i];
        checksum += char_out[i];
        checksum += ((int*)&vec_out[i])[0];
        if (i < iterations) checksum += results[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global mode: %d\n", global_mode);
    
    /* Cleanup */
    free(int_in); free(int_out);
    free(long_in); free(long_out);
    free(float_in); free(float_out);
    free(double_in); free(double_out);
    free(char_in); free(char_out);
    free(vec_in); free(vec_out);
    free(results);
    
    return 0;
}
