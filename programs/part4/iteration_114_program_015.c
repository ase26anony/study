/* reload_test.c - Complex inline assembly to trigger reload.cc logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <stdint.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout);
void test_secondary_reloads(int iterations, int *in, int *out, float *fin, float *fout);
void test_optional_reloads(int iterations, int *in, int *out, long long *lin, long long *lout);
void test_control_flow_reloads(int iterations, int mode, int *in, int *out);

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_double = 3.14159;

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 2;
    
    if (argc >= 3) {
        iterations = atoi(argv[1]);
        mode = atoi(argv[2]);
    }
    
    /* Allocate and initialize arrays with mixed data types */
    int *int_array = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_out = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double *double_array = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *double_out = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *float_array = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *float_out = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    long long *ll_array = (long long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long long));
    long long *ll_out = (long long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long long));
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        int_out[i] = 0;
        double_array[i] = i * 1.5 + 0.25;
        double_out[i] = 0.0;
        float_array[i] = i * 0.75f + 0.125f;
        float_out[i] = 0.0f;
        ll_array[i] = (long long)i * 7LL + 3LL;
        ll_out[i] = 0LL;
    }
    
    printf("Starting reload tests with iterations=%d, mode=%d\n", iterations, mode);
    
    /* Execute test functions to trigger reload logic */
    test_primary_reloads(iterations, int_array, int_out, double_array, double_out);
    test_secondary_reloads(iterations, int_array, int_out, float_array, float_out);
    test_optional_reloads(iterations, int_array, int_out, ll_array, ll_out);
    test_control_flow_reloads(iterations, mode, int_array, int_out);
    
    /* Compute checksum to ensure all assembly executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_out[i];
        checksum += (unsigned long long)(double_out[i] * 1000.0);
        checksum += (unsigned long long)(float_out[i] * 1000.0f);
        checksum += ll_out[i];
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(int_array);
    free(int_out);
    free(double_array);
    free(double_out);
    free(float_array);
    free(float_out);
    free(ll_array);
    free(ll_out);
    
    return 0;
}

/* Complex inline assembly with many operands to trigger primary reloads */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout) {
    /* Many live variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    double da, db, dc, dd, de, df, dg, dh;
    __m128i v1, v2, v3, v4;
    __m256d vd1, vd2, vd3, vd4;
    
    /* Initialize variables */
    a = in[0]; b = in[1]; c = in[2]; d = in[3];
    e = in[4]; f = in[5]; g = in[6]; h = in[7];
    i = in[8]; j = in[9]; k = in[10]; l = in[11];
    m = in[12]; n = in[13]; o = in[14]; p = in[15];
    
    da = din[0]; db = din[1]; dc = din[2]; dd = din[3];
    de = din[4]; df = din[5]; dg = din[6]; dh = din[7];
    
    v1 = _mm_set_epi32(a, b, c, d);
    v2 = _mm_set_epi32(e, f, g, h);
    v3 = _mm_set_epi32(i, j, k, l);
    v4 = _mm_set_epi32(m, n, o, p);
    
    vd1 = _mm256_set_pd(da, db, dc, dd);
    vd2 = _mm256_set_pd(de, df, dg, dh);
    
    /* Unrolled loop with complex inline assembly */
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex asm with 8+ operands, mixed constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (a),     /* General register */
            "=&r" (b),    /* Early clobber */
            "=q" (c),     /* Byte register (a, b, c, d) */
            "=r" (d),
            "=a" (e),     /* Accumulator */
            "=d" (f),     /* Data register */
            "=t" (da),    /* Top of FP stack */
            "=u" (db)     /* Second FP stack */
            
            /* Inputs with mixed constraints */
            : "0" (a),    /* Matching constraint */
              "r" (b),
              "i" (12345), /* Immediate */
              "m" (in[iter % ARRAY_SIZE]), /* Memory */
              "r" (c),
              "a" (d),
              "rm" (da),  /* Register or memory */
              "r" (db),
              "r" (global_counter)
            
            /* Many clobbers to force register spilling */
            : "rcx", "r8", "r9", "r10", "r11", "xmm0", "xmm1", 
              "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "st", "st(1)", "st(2)", "st(3)", "cc", "memory"
        );
        
        /* Another asm with vector registers */
        __asm__ volatile (
            "vmovapd %%ymm0, %0\n\t"
            "vmovapd %%ymm1, %1\n\t"
            "vaddpd %2, %3, %%ymm2\n\t"
            "vmulpd %%ymm2, %%ymm0, %%ymm3\n\t"
            : "=v" (vd3), "=v" (vd4)
            : "v" (vd1), "v" (vd2), "0" (vd3), "1" (vd4)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5"
        );
        
        /* Use results to prevent dead code elimination */
        out[iter % ARRAY_SIZE] = a + b + c + d + e + f;
        dout[iter % ARRAY_SIZE] = da + db;
        
        /* Update global to prevent optimization */
        global_counter += iter;
    }
}

/* Test secondary reload patterns */
void test_secondary_reloads(int iterations, int *in, int *out, float *fin, float *fout) {
    int x, y, z, w;
    float fx, fy, fz, fw;
    
    /* Initialize */
    x = in[0]; y = in[1]; z = in[2]; w = in[3];
    fx = fin[0]; fy = fin[1]; fz = fin[2]; fw = fin[3];
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Force secondary reload by using "R" constraint (legacy register)
           which may need a secondary move if allocated to R8-R15 */
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=R" (x)      /* Legacy register constraint */
            : "r" (y), "m" (in[iter % ARRAY_SIZE])
            : "eax", "cc"
        );
        
        /* Mix register classes: "a" constraint then "b" constraint */
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "imull %2, %%eax\n\t"
            "movl %%eax, %%ebx\n\t"
            "addl %3, %%ebx\n\t"
            : "=b" (y), "=a" (z)  /* Output in ebx, eax */
            : "a" (x), "r" (w), "m" (global_counter)
            : "cc"
        );
        
        /* FP operation with memory constraint that may need secondary reload */
        __asm__ volatile (
            "flds %1\n\t"
            "fadds %2\n\t"
            "fstps %0\n\t"
            : "=m" (fout[iter % ARRAY_SIZE])
            : "m" (fx), "m" (fy)
            : "st", "st(1)"
        );
        
        /* Update variables to keep them live */
        w = x + y + z;
        fx = fy + 0.5f;
        fy = fz * 1.1f;
        fz = fw - 0.25f;
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Another similar asm that won't combine due to barrier */
        __asm__ volatile (
            "addl $1, %0\n\t"
            : "+r" (w)
            :
            : "cc"
        );
    }
}

/* Test optional reloads and nocombine behavior */
void test_optional_reloads(int iterations, int *in, int *out, long long *lin, long long *lout) {
    long long la, lb, lc, ld;
    int opt1, opt2, opt3;
    
    la = lin[0]; lb = lin[1]; lc = lin[2]; ld = lin[3];
    opt1 = in[0]; opt2 = in[1]; opt3 = in[2];
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Use optional constraints with '?' modifier */
        __asm__ volatile (
            "movq %1, %%rax\n\t"
            "addq %2, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=?r" (la), "=?r" (lb)  /* Optional outputs */
            : "r" (lb), "m" (lin[iter % ARRAY_SIZE]),
              "i" (1000)
            : "rax", "cc"
        );
        
        /* Volatile asm with different clobbers to prevent combination */
        __asm__ volatile (
            "movq %1, %%rcx\n\t"
            "subq %2, %%rcx\n\t"
            : "=r" (lc)
            : "r" (la), "r" (lb)
            : "rcx", "cc"
        );
        
        /* Memory barrier between similar asm statements */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that won't combine due to different clobber list */
        __asm__ volatile (
            "movq %1, %%rdx\n\t"
            "imulq %2, %%rdx\n\t"
            : "=r" (ld)
            : "r" (lc), "r" (la)
            : "rdx", "cc", "memory"
        );
        
        /* Use optional input with immediate alternative */
        __asm__ volatile (
            "addl %1, %0\n\t"
            : "+r" (opt1)
            : "ri" (opt2)  /* Register or immediate */
            : "cc"
        );
        
        /* Store results */
        lout[iter % ARRAY_SIZE] = la + lb + lc + ld;
        out[iter % ARRAY_SIZE] = opt1 + opt2 + opt3;
        
        /* Rotate values to create dependencies */
        long long temp = la;
        la = lb;
        lb = lc;
        lc = ld;
        ld = temp;
    }
}

/* Test control-flow dependent reloads */
void test_control_flow_reloads(int iterations, int mode, int *in, int *out) {
    int x = in[0];
    int y = in[1];
    int z = in[2];
    int w = in[3];
    
    for (int i = 0; i < iterations; i++) {
        /* Conditional execution paths with different asm statements */
        if (mode & 1) {
            __asm__ volatile (
                "movl %1, %%eax\n\t"
                "xorl %2, %%eax\n\t"
                "movl %%eax, %0\n\t"
                : "=r" (x)
                : "r" (y), "r" (z)
                : "eax", "cc"
            );
        } else {
            __asm__ volatile (
                "movl %1, %%ebx\n\t"
                "andl %2, %%ebx\n\t"
                "movl %%ebx, %0\n\t"
                : "=r" (x)
                : "r" (y), "r" (w)
                : "ebx", "cc"
            );
        }
        
        /* Loop with switch to different asm based on iteration */
        switch (i % 4) {
            case 0:
                __asm__ volatile (
                    "addl $1, %0\n\t"
                    : "+r" (y)
                    :
                    : "cc"
                );
                break;
            case 1:
                __asm__ volatile (
                    "subl $1, %0\n\t"
                    : "+r" (z)
                    :
                    : "cc"
                );
                break;
            case 2:
                __asm__ volatile (
                    "imull $2, %0\n\t"
                    : "+r" (w)
                    :
                    : "cc"
                );
                break;
            case 3:
                __asm__ volatile (
                    "shrl $1, %0\n\t"
                    : "+r" (x)
                    :
                    : "cc"
                );
                break;
        }
        
        /* Nested loop to increase register pressure */
        for (int j = 0; j < 8; j++) {
            int temp = x + j;
            __asm__ volatile (
                "leal (%1, %2), %0\n\t"
                : "=r" (temp)
                : "r" (x), "r" (j)
                : "cc"
            );
            out[(i * 8 + j) % ARRAY_SIZE] = temp;
        }
        
        /* Update mode based on runtime values */
        mode = (mode * 1103515245 + 12345) & 0x7fffffff;
    }
}
