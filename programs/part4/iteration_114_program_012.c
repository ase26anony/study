#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions prototypes */
void test_primary_reloads(int iterations, int *input, int *output, double *dinput, double *doutput);
void test_secondary_reloads(int iterations, long long *input, long long *output, float *finput, float *foutput);
void test_optional_reloads(int iterations, unsigned *input, unsigned *output, __m128i *vinput, __m128i *voutput);
void test_control_flow_reloads(int mode, int iterations, int *data, int *results);

/* Global variables to increase register pressure */
volatile int global_counter = 0;
volatile double global_double = 3.14159;
volatile __m128i global_vector;

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 2;
    
    if (argc >= 2) iterations = atoi(argv[1]);
    if (argc >= 3) mode = atoi(argv[2]);
    
    /* Allocate and initialize arrays with mixed data types */
    int *int_array = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_output = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    long long *ll_array = (long long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long long));
    long long *ll_output = (long long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long long));
    unsigned *uint_array = (unsigned*)aligned_alloc(64, ARRAY_SIZE * sizeof(unsigned));
    unsigned *uint_output = (unsigned*)aligned_alloc(64, ARRAY_SIZE * sizeof(unsigned));
    double *double_array = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *double_output = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *float_array = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *float_output = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    __m128i *vector_array = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    __m128i *vector_output = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    
    /* Initialize with pattern to prevent optimization */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 37) & 0xFFF;
        ll_array[i] = (long long)i * 7919;
        uint_array[i] = (unsigned)(i * 0xDEADBEEF);
        double_array[i] = (double)i * 1.41421356;
        float_array[i] = (float)i * 2.71828182;
        vector_array[i] = _mm_set_epi32(i, i+1, i+2, i+3);
    }
    
    /* Execute test functions to trigger reloads */
    test_primary_reloads(iterations, int_array, int_output, double_array, double_output);
    test_secondary_reloads(iterations, ll_array, ll_output, float_array, float_output);
    test_optional_reloads(iterations, uint_array, uint_output, vector_array, vector_output);
    test_control_flow_reloads(mode, iterations, int_array, int_output);
    
    /* Compute checksum to ensure all assembly executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_output[i];
        checksum += ll_output[i];
        checksum += uint_output[i];
        checksum += (unsigned long long)double_output[i];
        checksum += (unsigned long long)float_output[i];
        checksum += _mm_extract_epi32(vector_output[i], 0);
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Free memory */
    free(int_array); free(int_output);
    free(ll_array); free(ll_output);
    free(uint_array); free(uint_output);
    free(double_array); free(double_output);
    free(float_array); free(float_output);
    free(vector_array); free(vector_output);
    
    return 0;
}

/* Complex inline assembly with multiple operands to trigger primary reloads */
void test_primary_reloads(int iterations, int *input, int *output, double *dinput, double *doutput) {
    /* Many live variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    double da, db, dc, dd, de, df, dg, dh;
    long long la, lb, lc, ld;
    
    /* Unrolled loop with many asm statements */
    for (int idx = 0; idx < iterations; idx++) {
        /* Initialize many live variables */
        a = input[idx * 16 + 0];
        b = input[idx * 16 + 1];
        c = input[idx * 16 + 2];
        d = input[idx * 16 + 3];
        e = input[idx * 16 + 4];
        f = input[idx * 16 + 5];
        g = input[idx * 16 + 6];
        h = input[idx * 16 + 7];
        i = input[idx * 16 + 8];
        j = input[idx * 16 + 9];
        k = input[idx * 16 + 10];
        l = input[idx * 16 + 11];
        m = input[idx * 16 + 12];
        n = input[idx * 16 + 13];
        o = input[idx * 16 + 14];
        p = input[idx * 16 + 15];
        
        da = dinput[idx * 8 + 0];
        db = dinput[idx * 8 + 1];
        dc = dinput[idx * 8 + 2];
        dd = dinput[idx * 8 + 3];
        de = dinput[idx * 8 + 4];
        df = dinput[idx * 8 + 5];
        dg = dinput[idx * 8 + 6];
        dh = dinput[idx * 8 + 7];
        
        /* Complex asm with 5+ operands, mixed constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (a),     /* General register */
            "=q" (b),     /* Byte-addressable register (eax, ebx, ecx, edx) */
            "=a" (c),     /* Accumulator */
            "=d" (d),     /* Data register */
            "=&r" (e)     /* Earlyclobber general register */
            :
            /* Inputs with mixed constraints */
            "r" (f),      /* General register */
            "i" (12345),  /* Immediate */
            "m" (input[idx * 16]),  /* Memory */
            "r" (g),
            "0" (a)       /* Matching constraint - same as first output */
            :
            /* Clobbers */
            "cc", "memory"
        );
        
        /* Another asm with different mode requirements */
        __asm__ volatile (
            "movq %1, %%mm0 \n\t"
            "movq %2, %%mm1 \n\t"
            "paddb %%mm1, %%mm0 \n\t"
            "movq %%mm0, %0 \n\t"
            : "=y" (la)   /* MMX register */
            : "y" (lb), "y" (lc)
            : "%mm0", "%mm1"
        );
        
        /* Asm with x87 floating point constraints */
        __asm__ volatile (
            "fldl %1 \n\t"
            "fldl %2 \n\t"
            "faddp %%st, %%st(1) \n\t"
            "fstpl %0 \n\t"
            : "=m" (doutput[idx * 8])
            : "m" (da), "m" (db)
            : "st", "st(1)"
        );
        
        /* Store results to prevent optimization */
        output[idx * 16 + 0] = a;
        output[idx * 16 + 1] = b;
        doutput[idx * 8 + 0] = da;
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
    }
}

/* Test secondary reload patterns */
void test_secondary_reloads(int iterations, long long *input, long long *output, float *finput, float *foutput) {
    /* Variables with specific register requirements */
    register long long r8 asm("r8");
    register long long r9 asm("r9");
    register long long r10 asm("r10");
    register long long r11 asm("r11");
    register long long r12 asm("r12");
    register long long r13 asm("r13");
    register long long r14 asm("r14");
    register long long r15 asm("r15");
    
    float fa, fb, fc, fd;
    int ia, ib, ic, id;
    
    for (int idx = 0; idx < iterations; idx++) {
        /* Load values into specific registers */
        r8 = input[idx * 8 + 0];
        r9 = input[idx * 8 + 1];
        r10 = input[idx * 8 + 2];
        r11 = input[idx * 8 + 3];
        r12 = input[idx * 8 + 4];
        r13 = input[idx * 8 + 5];
        r14 = input[idx * 8 + 6];
        r15 = input[idx * 8 + 7];
        
        fa = finput[idx * 8 + 0];
        fb = finput[idx * 8 + 1];
        fc = finput[idx * 8 + 2];
        fd = finput[idx * 8 + 3];
        
        /* Asm with "R" constraint (legacy register) that may need secondary reload
           if allocated to R8-R15 */
        __asm__ volatile (
            "mov %1, %%eax \n\t"
            "add %2, %%eax \n\t"
            "mov %%eax, %0 \n\t"
            : "=R" (ia)      /* Legacy register constraint */
            : "R" (ib), "R" (ic)
            : "%eax"
        );
        
        /* Mismatched constraints requiring secondary reload */
        __asm__ volatile (
            /* Input in "a" constraint, but we need result in "b" */
            : "=b" (ib)
            : "a" (ia), "m" (input[idx])
            : "cc"
        );
        
        /* Asm with "rm" constraint that may need secondary reload if in memory */
        __asm__ volatile (
            "imul %2, %0 \n\t"
            : "=rm" (output[idx])
            : "0" (input[idx]), "rm" (123)
            : "cc"
        );
        
        /* Use AVX alongside scalar operations */
        __m256d vd = _mm256_set_pd(fa, fb, fc, fd);
        __m256d vd2 = _mm256_set1_pd(2.0);
        __m256d vresult = _mm256_add_pd(vd, vd2);
        
        /* Asm that uses the vector result */
        double scalar_result;
        __asm__ volatile (
            "vextractf128 $0, %1, %0 \n\t"
            : "=x" (scalar_result)
            : "x" (vresult)
        );
        
        foutput[idx] = (float)scalar_result;
        
        /* More register pressure */
        __asm__ volatile (
            "mov %1, %0 \n\t"
            "add %2, %0 \n\t"
            "sub %3, %0 \n\t"
            "imul %4, %0 \n\t"
            : "=r" (output[idx * 2])
            : "r" (r8), "r" (r9), "r" (r10), "r" (r11)
            : "cc"
        );
    }
}

/* Test optional reloads and nocombine behavior */
void test_optional_reloads(int iterations, unsigned *input, unsigned *output, __m128i *vinput, __m128i *voutput) {
    unsigned opt1, opt2, opt3, opt4;
    __m128i vec1, vec2, vec3;
    
    for (int idx = 0; idx < iterations; idx++) {
        vec1 = vinput[idx * 2];
        vec2 = vinput[idx * 2 + 1];
        
        /* Asm with optional constraints (?) */
        __asm__ volatile (
            "movd %2, %%xmm0 \n\t"
            "movd %3, %%xmm1 \n\t"
            "paddd %%xmm1, %%xmm0 \n\t"
            "movd %%xmm0, %0 \n\t"
            "movd %%xmm0, %1 \n\t"
            : "=?r" (opt1),   /* Optional output */
              "=r" (opt2)
            : "r" (input[idx * 4]),
              "r" (input[idx * 4 + 1])
            : "%xmm0", "%xmm1", "cc"
        );
        
        /* Memory barrier to prevent combination with next asm */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm but with different clobbers to force nocombine */
        __asm__ volatile (
            "movd %2, %%xmm2 \n\t"
            "movd %3, %%xmm3 \n\t"
            "paddd %%xmm3, %%xmm2 \n\t"
            "movd %%xmm2, %0 \n\t"
            "movd %%xmm2, %1 \n\t"
            : "=?r" (opt3),
              "=r" (opt4)
            : "r" (input[idx * 4 + 2]),
              "r" (input[idx * 4 + 3])
            : "%xmm2", "%xmm3", "cc", "memory"  /* Extra clobber */
        );
        
        /* Vector operations mixed with scalar */
        vec3 = _mm_add_epi32(vec1, vec2);
        
        /* Asm that could combine but volatile prevents it */
        __asm__ volatile (
            "paddd %1, %0 \n\t"
            : "+x" (vec3)
            : "x" (vec1)
        );
        
        __asm__ volatile (
            "pslld $2, %0 \n\t"
            : "+x" (vec3)
        );
        
        voutput[idx] = vec3;
        output[idx * 4] = opt1 + opt3;
        output[idx * 4 + 1] = opt2 + opt4;
    }
}

/* Test control flow dependent reloads */
void test_control_flow_reloads(int mode, int iterations, int *data, int *results) {
    int temp1, temp2, temp3, temp4;
    double dtemp1, dtemp2;
    
    /* Many live variables in different scopes */
    if (mode & 1) {
        for (int i = 0; i < iterations; i++) {
            temp1 = data[i * 2];
            temp2 = data[i * 2 + 1];
            
            /* Asm inside conditional path */
            __asm__ volatile (
                "lea (%1, %2, 2), %0 \n\t"
                : "=r" (results[i])
                : "r" (temp1), "r" (temp2)
                : "cc"
            );
            
            if (i & 1) {
                /* Different asm in nested conditional */
                __asm__ volatile (
                    "imul %1, %0 \n\t"
                    "add $0x1234, %0 \n\t"
                    : "+r" (results[i])
                    : "r" (global_counter)
                    : "cc"
                );
            }
        }
    }
    
    if (mode & 2) {
        /* Switch statement with asm in different cases */
        for (int i = 0; i < iterations; i++) {
            switch (i % 4) {
                case 0:
                    __asm__ volatile (
                        "mov %1, %0 \n\t"
                        "ror $8, %0 \n\t"
                        : "=r" (temp3)
                        : "r" (data[i])
                        : "cc"
                    );
                    break;
                case 1:
                    __asm__ volatile (
                        "mov %1, %0 \n\t"
                        "rol $16, %0 \n\t"
                        : "=r" (temp3)
                        : "r" (data[i])
                        : "cc"
                    );
                    break;
                case 2:
                    /* Asm with x87 in one path */
                    dtemp1 = (double)data[i];
                    __asm__ volatile (
                        "fldl %1 \n\t"
                        "fsqrt \n\t"
                        "fstpl %0 \n\t"
                        : "=m" (dtemp2)
                        : "m" (dtemp1)
                        : "st"
                    );
                    temp3 = (int)dtemp2;
                    break;
                case 3:
                    __asm__ volatile (
                        "popcnt %1, %0 \n\t"
                        : "=r" (temp3)
                        : "r" (data[i])
                        : "cc"
                    );
                    break;
            }
            results[iterations + i] = temp3;
        }
    }
    
    /* Loop with early exit */
    int j = 0;
    while (j < iterations) {
        temp4 = data[j * 3];
        
        __asm__ volatile (
            "bsf %1, %0 \n\t"
            : "=r" (results[j * 2])
            : "r" (temp4)
            : "cc"
        );
        
        if (temp4 == 0) break;
        j++;
    }
}
