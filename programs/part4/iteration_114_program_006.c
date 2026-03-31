/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 256

/* Test functions */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout);
void test_secondary_reloads(int iterations, long *in, long *out, float *fin, float *fout);
void test_optional_reloads(int iterations, char *in, char *out, __m128i *vin, __m128i *vout);
void test_control_flow_reloads(int mode, int count, int *data, int *result);

/* Global variables to increase register pressure */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* Primary reloads with register pressure */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout) {
    int i, j;
    int a, b, c, d, e, f, g, h;
    double da, db, dc, dd, de, df, dg, dh;
    
    /* Unrolled loop with many live variables */
    for (i = 0; i < iterations; i++) {
        /* Load many values into registers */
        a = in[i * 8 + 0];
        b = in[i * 8 + 1];
        c = in[i * 8 + 2];
        d = in[i * 8 + 3];
        e = in[i * 8 + 4];
        f = in[i * 8 + 5];
        g = in[i * 8 + 6];
        h = in[i * 8 + 7];
        
        da = din[i * 4 + 0];
        db = din[i * 4 + 1];
        dc = din[i * 4 + 2];
        dd = din[i * 4 + 3];
        
        /* Complex inline assembly with multiple constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (a),    /* General register */
            "=q" (b),    /* Byte-addressable register (eax, ebx, ecx, edx) */
            "=r" (c),
            "=a" (d),    /* Accumulator */
            "=d" (e),    /* Data register */
            "=&r" (f),   /* Earlyclobber */
            "=r" (g),
            "=t" (da)    /* Top of FP stack */
            
            /* Inputs with mixed constraints */
            : "0" (a),   /* Matching constraint - same as output 0 */
            "i" (123),   /* Immediate */
            "m" (in[i]), /* Memory */
            "r" (b),
            "a" (c),
            "d" (d),
            "g" (e),     /* General register, memory or immediate */
            "rm" (f),    /* Register or memory */
            "r" (g),
            "r" (h),
            "m" (db),
            "f" (dc)     /* Floating point register */
            
            /* Clobber many registers */
            : "memory", "cc",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15"
        );
        
        /* More assembly with different mode constraints */
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "imull %3, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (out[i])
            : "r" (a), "m" (b), "i" (456)
            : "eax", "cc"
        );
        
        /* Store results to force spills */
        out[i * 8 + 0] = a;
        out[i * 8 + 1] = b;
        out[i * 8 + 2] = c;
        out[i * 8 + 3] = d;
        out[i * 8 + 4] = e;
        out[i * 8 + 5] = f;
        out[i * 8 + 6] = g;
        out[i * 8 + 7] = h;
        
        dout[i * 2 + 0] = da;
        dout[i * 2 + 1] = db;
    }
}

/* Secondary reload patterns */
void test_secondary_reloads(int iterations, long *in, long *out, float *fin, float *fout) {
    int i;
    long x, y, z;
    float fx, fy, fz;
    
    for (i = 0; i < iterations; i++) {
        x = in[i * 3 + 0];
        y = in[i * 3 + 1];
        z = in[i * 3 + 2];
        
        fx = fin[i * 3 + 0];
        fy = fin[i * 3 + 1];
        fz = fin[i * 3 + 2];
        
        /* Assembly requiring specific register classes */
        __asm__ volatile (
            /* Force accumulator constraint then use in different constraint */
            "mov %1, %%rax\n\t"
            "add %2, %%rax\n\t"
            : "=a" (x)      /* Output in accumulator */
            : "a" (x),      /* Input must be in accumulator */
              "b" (y)       /* Input in base register - may need secondary reload */
            : "rbx", "cc"
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Use result with different constraint */
        __asm__ volatile (
            "mov %1, %%rbx\n\t"
            "sub %2, %%rbx\n\t"
            : "=b" (y)      /* Output in base register */
            : "a" (x),      /* Input from accumulator - may need move */
              "r" (z)
            : "rax", "cc"
        );
        
        /* Legacy register constraint that may need secondary reload for R8-R15 */
        __asm__ volatile (
            "mov %1, %%eax\n\t"
            "xor %%ecx, %%ecx\n\t"
            "cpuid\n\t"
            : "=R" (z)      /* Legacy register constraint */
            : "r" (y)
            : "rax", "rbx", "rcx", "rdx", "cc"
        );
        
        /* Floating point with mismatched constraints */
        __asm__ volatile (
            "addss %1, %0\n\t"
            "mulss %2, %0\n\t"
            : "=x" (fx)     /* XMM register */
            : "x" (fx), "m" (fy)  /* Memory constraint for float may need reload */
            : "cc"
        );
        
        out[i * 3 + 0] = x;
        out[i * 3 + 1] = y;
        out[i * 3 + 2] = z;
        
        fout[i * 3 + 0] = fx;
        fout[i * 3 + 1] = fy;
        fout[i * 3 + 2] = fz;
    }
}

/* Optional reloads and nocombine patterns */
void test_optional_reloads(int iterations, char *in, char *out, __m128i *vin, __m128i *vout) {
    int i;
    char c1, c2, c3, c4;
    __m128i v1, v2, v3;
    
    for (i = 0; i < iterations; i++) {
        c1 = in[i * 4 + 0];
        c2 = in[i * 4 + 1];
        c3 = in[i * 4 + 2];
        c4 = in[i * 4 + 3];
        
        v1 = vin[i * 2 + 0];
        v2 = vin[i * 2 + 1];
        
        /* Optional constraints */
        __asm__ volatile (
            "mov %1, %%al\n\t"
            "add %2, %%al\n\t"
            : "=?r" (c1),   /* Optional output */
              "=r" (c2)     /* Required output */
            : "0" (c1),     /* Matching optional */
              "r" (c2),
              "i" (1)       /* Immediate */
            : "al", "cc"
        );
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar but not identical asm to prevent combine */
        __asm__ volatile (
            "mov %1, %%bl\n\t"
            "sub %2, %%bl\n\t"
            : "=r" (c3)
            : "r" (c3),
              "r" (c4)
            : "bl", "cc"
        );
        
        /* Vector operations to increase register pressure */
        v3 = _mm_add_epi32(v1, v2);
        v3 = _mm_mullo_epi32(v3, _mm_set1_epi32(2));
        
        /* Mixed scalar/vector in same asm */
        __asm__ volatile (
            "paddd %1, %0\n\t"
            "movd %0, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movd %%eax, %0\n\t"
            : "+x" (v3)     /* Read-write XMM */
            : "x" (v1),
              "r" (c1)      /* Scalar in register */
            : "eax", "cc"
        );
        
        out[i * 4 + 0] = c1;
        out[i * 4 + 1] = c2;
        out[i * 4 + 2] = c3;
        out[i * 4 + 3] = c4;
        
        vout[i] = v3;
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int mode, int count, int *data, int *result) {
    int i, temp;
    double dtemp;
    
    for (i = 0; i < count; i++) {
        temp = data[i];
        dtemp = (double)temp;
        
        /* Different asm based on control flow */
        if (mode & 1) {
            __asm__ volatile (
                "imull %1, %0\n\t"
                : "+r" (temp)
                : "r" (i)
                : "cc"
            );
        }
        
        if (mode & 2) {
            __asm__ volatile (
                "addsd %1, %0\n\t"
                : "+x" (dtemp)
                : "x" (dtemp),
                  "m" (global_accumulator)
                : "cc"
            );
        }
        
        if (mode & 4) {
            /* Complex asm in conditional path */
            __asm__ volatile (
                "mov %1, %%eax\n\t"
                "shrl $3, %%eax\n\t"
                "andl $0xFF, %%eax\n\t"
                "mov %%eax, %0\n\t"
                : "=r" (temp)
                : "r" (temp),
                  "i" (global_counter)
                : "eax", "cc"
            );
        }
        
        /* Loop-carried dependency */
        __asm__ volatile (
            "addl $1, %0\n\t"
            : "+r" (global_counter)
            :
            : "cc"
        );
        
        result[i] = temp + (int)dtemp;
    }
}

/* Main function with command-line control */
int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 2;
    
    /* Parse command line */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    if (iterations > ARRAY_SIZE / 8) {
        iterations = ARRAY_SIZE / 8;
    }
    
    /* Allocate and initialize arrays */
    int *int_in = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_out = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    long *long_in = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    long *long_out = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    float *float_in = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *float_out = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double *double_in = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *double_out = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    char *char_in = (char*)aligned_alloc(64, ARRAY_SIZE * sizeof(char));
    char *char_out = (char*)aligned_alloc(64, ARRAY_SIZE * sizeof(char));
    __m128i *vec_in = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    __m128i *vec_out = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    int *control_data = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *control_result = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_in[i] = i * 3 + 1;
        long_in[i] = i * 5 + 2;
        float_in[i] = i * 1.5f;
        double_in[i] = i * 2.7;
        char_in[i] = (i % 26) + 'a';
        control_data[i] = i * 7 + 3;
        
        if (i % 2 == 0) {
            vec_in[i] = _mm_set_epi32(i, i+1, i+2, i+3);
        } else {
            vec_in[i] = _mm_set_epi32(i*2, i*3, i*4, i*5);
        }
    }
    
    /* Run tests to trigger reloads */
    test_primary_reloads(iterations, int_in, int_out, double_in, double_out);
    test_secondary_reloads(iterations, long_in, long_out, float_in, float_out);
    test_optional_reloads(iterations, char_in, char_out, vec_in, vec_out);
    test_control_flow_reloads(mode, iterations, control_data, control_result);
    
    /* Compute checksum to ensure all asm executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < iterations * 8 && i < ARRAY_SIZE; i++) {
        checksum += int_out[i];
        checksum += long_out[i % (ARRAY_SIZE / 2)];
        if (i < ARRAY_SIZE / 2) {
            checksum += (int)float_out[i];
            checksum += (int)double_out[i];
        }
        checksum += char_out[i];
        checksum += control_result[i];
    }
    
    /* Add vector checksum */
    for (int i = 0; i < iterations && i < ARRAY_SIZE; i++) {
        int *v = (int*)&vec_out[i];
        checksum += v[0] + v[1] + v[2] + v[3];
    }
    
    printf("Checksum: %llu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Cleanup */
    free(int_in);
    free(int_out);
    free(long_in);
    free(long_out);
    free(float_in);
    free(float_out);
    free(double_in);
    free(double_out);
    free(char_in);
    free(char_out);
    free(vec_in);
    free(vec_out);
    free(control_data);
    free(control_result);
    
    return 0;
}
