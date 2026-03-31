#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <stdint.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions prototypes */
void test_primary_reloads(int iterations, int *input, int *output, double *dinput, double *doutput);
void test_secondary_reloads(int iterations, float *finput, float *foutput, long long *llinput, long long *lloutput);
void test_optional_reloads(int iterations, char *cinput, char *coutput, short *sinput, short *soutput);
void test_control_flow_reloads(int mode, int *data, int *result, int size);

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
    int *int_input = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_output = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double *double_input = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *double_output = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *float_input = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *float_output = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    long long *ll_input = (long long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long long));
    long long *ll_output = (long long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long long));
    char *char_input = (char*)aligned_alloc(64, ARRAY_SIZE * sizeof(char));
    char *char_output = (char*)aligned_alloc(64, ARRAY_SIZE * sizeof(char));
    short *short_input = (short*)aligned_alloc(64, ARRAY_SIZE * sizeof(short));
    short *short_output = (short*)aligned_alloc(64, ARRAY_SIZE * sizeof(short));
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_input[i] = i * 3 + 1;
        double_input[i] = i * 0.5 + 1.0;
        float_input[i] = i * 0.25f + 2.0f;
        ll_input[i] = (long long)i * 7LL + 3LL;
        char_input[i] = (char)(i % 256);
        short_input[i] = (short)(i * 5);
    }
    
    /* Execute test functions to trigger reloads */
    test_primary_reloads(iterations, int_input, int_output, double_input, double_output);
    test_secondary_reloads(iterations, float_input, float_output, ll_input, ll_output);
    test_optional_reloads(iterations, char_input, char_output, short_input, short_output);
    test_control_flow_reloads(mode, int_input, int_output, ARRAY_SIZE);
    
    /* Compute checksum to ensure all assembly executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_output[i];
        checksum += (unsigned long long)double_output[i];
        checksum += (unsigned int)float_output[i];
        checksum += ll_output[i];
        checksum += char_output[i];
        checksum += short_output[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Free allocated memory */
    free(int_input);
    free(int_output);
    free(double_input);
    free(double_output);
    free(float_input);
    free(float_output);
    free(ll_input);
    free(ll_output);
    free(char_input);
    free(char_output);
    free(short_input);
    free(short_output);
    
    return 0;
}

/* Complex inline assembly with multiple operands to trigger primary reloads */
void test_primary_reloads(int iterations, int *input, int *output, double *dinput, double *doutput) {
    /* Many live variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    double da, db, dc, dd, de, df, dg, dh;
    __m128i v1, v2, v3, v4;
    __m256d vd1, vd2, vd3, vd4;
    
    /* Initialize variables */
    a = input[0];
    b = input[1];
    c = input[2];
    d = input[3];
    e = input[4];
    f = input[5];
    g = input[6];
    h = input[7];
    da = dinput[0];
    db = dinput[1];
    dc = dinput[2];
    dd = dinput[3];
    
    /* Unrolled loop with many asm statements */
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex asm with 7 operands, mixed constraints and modes */
        __asm__ volatile (
            /* Outputs with different constraints and modes */
            "=r" (a),     /* General purpose register */
            "=&r" (b),    /* Early clobber */
            "=q" (c),     /* Byte register (a,b,c,d) */
            "=t" (da),    /* Top of FP stack */
            "=a" (d),     /* Accumulator */
            "=d" (e),     /* Data register */
            "=m" (output[iter % ARRAY_SIZE])  /* Memory output */
            
            /* Inputs with mixed constraints */
            : "0" (a),           /* Matching constraint */
              "r" (b),           /* General register */
              "i" (12345),       /* Immediate */
              "rm" (c),          /* Register or memory */
              "g" (d),           /* General (register, memory, or immediate) */
              "X" (da),          /* Anything */
              "m" (input[iter % ARRAY_SIZE])  /* Memory input */
            
            /* Extensive clobber list */
            : "cc", "memory", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
        );
        
        /* Another asm with vector operations */
        v1 = _mm_set_epi32(a, b, c, d);
        v2 = _mm_set_epi32(e, f, g, h);
        
        __asm__ volatile (
            "vpaddd %0, %1, %2\n\t"
            "vmovdqa %0, %3"
            : "=x" (v3), "=x" (v4)
            : "0" (v1), "x" (v2), "m" (input[(iter + 1) % ARRAY_SIZE])
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* Double precision vector operations */
        vd1 = _mm256_set_pd(da, db, dc, dd);
        vd2 = _mm256_set_pd(de, df, dg, dh);
        
        __asm__ volatile (
            "vaddpd %0, %1, %2\n\t"
            "vmovapd %0, %3"
            : "=x" (vd3), "=&x" (vd4)
            : "0" (vd1), "x" (vd2), "m" (dinput[iter % ARRAY_SIZE])
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7"
        );
        
        /* More live variables to increase pressure */
        i = a + b;
        j = c + d;
        k = e + f;
        l = g + h;
        m = i + j;
        n = k + l;
        o = m + n;
        p = o + iter;
        
        de = da + db;
        df = dc + dd;
        dg = de + df;
        dh = dg + iter;
        
        /* Store results to prevent optimization */
        output[iter % ARRAY_SIZE] = p;
        doutput[iter % ARRAY_SIZE] = dh;
    }
    
    global_counter += a + b + c + d;
}

/* Force secondary reload scenarios */
void test_secondary_reloads(int iterations, float *finput, float *foutput, 
                           long long *llinput, long long *lloutput) {
    float f1, f2, f3, f4, f5, f6, f7, f8;
    long long ll1, ll2, ll3, ll4;
    int tmp1, tmp2, tmp3, tmp4;
    
    /* Initialize */
    f1 = finput[0];
    f2 = finput[1];
    f3 = finput[2];
    f4 = finput[3];
    ll1 = llinput[0];
    ll2 = llinput[1];
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Asm requiring specific register classes that may need secondary reloads */
        
        /* Force accumulator constraint then base register constraint */
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0"
            : "=b" (tmp1)        /* Result must be in %ebx */
            : "a" (iter),        /* Input must be in %eax */
              "r" (tmp2)         /* General register input */
            : "%eax"             /* Clobber eax */
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Another asm using the result with different constraint */
        __asm__ volatile (
            "movl %1, %%ebx\n\t"
            "imull %2, %%ebx\n\t"
            "movl %%ebx, %0"
            : "=a" (tmp2)        /* Result must be in %eax */
            : "b" (tmp1),        /* Input must be in %ebx */
              "r" (tmp3)         /* General register */
            : "%ebx"
        );
        
        /* Float operations with constraints that may require secondary reloads */
        __asm__ volatile (
            "addss %1, %0\n\t"
            "movss %0, %2"
            : "=x" (f1), "=x" (f2)
            : "0" (f1), "x" (f2), "m" (finput[iter % ARRAY_SIZE])
            : "xmm0", "xmm1"
        );
        
        /* Long long operations with "R" constraint (legacy register) */
        __asm__ volatile (
            "addq %1, %0\n\t"
            "movq %0, %2"
            : "=R" (ll1)         /* Legacy register constraint */
            : "0" (ll1), 
              "m" (llinput[iter % ARRAY_SIZE]),
              "r" (ll2)
            : "cc"
        );
        
        /* More operations to increase register pressure */
        f5 = f1 + f2;
        f6 = f3 + f4;
        f7 = f5 + f6;
        f8 = f7 + iter;
        
        ll3 = ll1 + ll2;
        ll4 = ll3 + iter;
        
        tmp3 = tmp1 + tmp2;
        tmp4 = tmp3 + iter;
        
        /* Store results */
        foutput[iter % ARRAY_SIZE] = f8;
        lloutput[iter % ARRAY_SIZE] = ll4;
    }
    
    global_counter += (int)f1 + (int)ll1;
}

/* Test optional reloads and nocombine scenarios */
void test_optional_reloads(int iterations, char *cinput, char *coutput, 
                          short *sinput, short *soutput) {
    char c1, c2, c3, c4;
    short s1, s2, s3, s4;
    int opt1, opt2, opt3;
    
    c1 = cinput[0];
    c2 = cinput[1];
    s1 = sinput[0];
    s2 = sinput[1];
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Asm with optional constraints (?) */
        __asm__ volatile (
            "movb %1, %%al\n\t"
            "addb %2, %%al\n\t"
            "movb %%al, %0"
            : "=?r" (c3)         /* Optional output */
            : "q" (c1),          /* Byte register */
              "i" (5)            /* Immediate */
            : "%al"
        );
        
        /* Memory barrier to prevent combination with next asm */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm but with different clobbers to prevent combining */
        __asm__ volatile (
            "movb %1, %%al\n\t"
            "subb %2, %%al\n\t"
            "movb %%al, %0"
            : "=?r" (c4)         /* Optional output */
            : "q" (c2),
              "i" (3)
            : "%al", "cc"        /* Different clobber list */
        );
        
        /* Short operations with mixed constraints */
        __asm__ volatile (
            "movw %1, %%ax\n\t"
            "addw %2, %%ax\n\t"
            "movw %%ax, %0"
            : "=r" (s3)
            : "0" (s1),
              "rm" (s2)          /* Register or memory - may need secondary reload */
            : "%ax"
        );
        
        /* Another barrier */
        __asm__ volatile ("" ::: "memory");
        
        /* More complex asm that could be combined but won't due to volatile */
        __asm__ volatile (
            "imulw %1, %0\n\t"
            "addw %2, %0"
            : "+r" (s4)
            : "rm" (s3),
              "i" (iter & 0xFF)
            : "cc"
        );
        
        /* Optional output that may not be used */
        __asm__ volatile (
            "movl %1, %0\n\t"
            "addl $1, %0"
            : "=?r" (opt1)       /* Optional */
            : "r" (iter)
            : "cc"
        );
        
        /* Use the optional output conditionally */
        if (opt1 & 1) {
            __asm__ volatile (
                "movl %1, %0\n\t"
                "subl $1, %0"
                : "=r" (opt2)
                : "0" (opt1)
                : "cc"
            );
        }
        
        /* Update variables */
        c1 = c3 + c4;
        c2 = c1 + iter;
        s1 = s3 + s4;
        s2 = s1 + iter;
        
        /* Store results */
        coutput[iter % ARRAY_SIZE] = c2;
        soutput[iter % ARRAY_SIZE] = s2;
    }
    
    global_counter += c1 + s1;
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int mode, int *data, int *result, int size) {
    int a, b, c, d, e, f;
    double x, y, z;
    
    a = data[0];
    b = data[1];
    x = global_double;
    
    /* Complex control flow with asm statements */
    for (int i = 0; i < size; i++) {
        if (mode == 0) {
            __asm__ volatile (
                "movl %1, %%eax\n\t"
                "addl %2, %%eax\n\t"
                "movl %%eax, %0"
                : "=r" (c)
                : "a" (a),
                  "r" (b)
                : "%eax"
            );
        } else if (mode == 1) {
            __asm__ volatile (
                "movl %1, %%ebx\n\t"
                "subl %2, %%ebx\n\t"
                "movl %%ebx, %0"
                : "=r" (d)
                : "b" (a),
                  "r" (b)
                : "%ebx"
            );
            c = d;
        } else {
            __asm__ volatile (
                "imull %1, %0\n\t"
                "addl $1, %0"
                : "+r" (e)
                : "r" (a)
                : "cc"
            );
            c = e;
        }
        
        /* Nested loop with asm */
        for (int j = 0; j < 4; j++) {
            if (j & 1) {
                __asm__ volatile (
                    "addsd %1, %0\n\t"
                    "movsd %0, %2"
                    : "=x" (x)
                    : "0" (x),
                      "m" (global_double)
                    : "xmm0"
                );
            } else {
                __asm__ volatile (
                    "mulsd %1, %0\n\t"
                    "movsd %0, %2"
                    : "=x" (y)
                    : "0" (x),
                      "m" (global_double)
                    : "xmm0"
                );
                x = y;
            }
            
            /* More variables live in the loop */
            f = c + j;
            z = x * 2.0;
        }
        
        /* Store result with asm */
        __asm__ volatile (
            "movl %1, %0"
            : "=m" (result[i])
            : "r" (f)
        );
        
        /* Update for next iteration */
        a = b;
        b = data[(i + 2) % size];
    }
    
    global_counter += c;
}
