/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <stdint.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout);
void test_secondary_reloads(int iterations, long long *in, long long *out, float *fin, float *fout);
void test_optional_reloads(int iterations, char *cin, char *cout, __m128i *vin, __m128i *vout);
void test_control_flow_reloads(int iterations, int mode, int *data, int *result);

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_mode = 0;

/* Complex inline assembly with many operands and constraints */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout) {
    int i, j;
    long long temp1, temp2, temp3, temp4;
    double dtemp1, dtemp2;
    int r0, r1, r2, r3, r4, r5, r6, r7;
    int s0, s1, s2, s3, s4, s5;
    
    /* Unrolled loop with many live variables */
    for (i = 0; i < iterations; i++) {
        /* Load many values into registers */
        r0 = in[i * 8 + 0];
        r1 = in[i * 8 + 1];
        r2 = in[i * 8 + 2];
        r3 = in[i * 8 + 3];
        r4 = in[i * 8 + 4];
        r5 = in[i * 8 + 5];
        r6 = in[i * 8 + 6];
        r7 = in[i * 8 + 7];
        
        s0 = r0 + r1;
        s1 = r2 + r3;
        s2 = r4 + r5;
        s3 = r6 + r7;
        s4 = s0 + s1;
        s5 = s2 + s3;
        
        /* Complex asm with 7 operands, mixed constraints */
        __asm__ volatile (
            "/* Primary reload test */\n\t"
            "mov %[imm], %%eax\n\t"
            "add %[in1], %%eax\n\t"
            "imul %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "lea (%[in3], %[in4], 2), %%ebx\n\t"
            "sub %%ebx, %[out2]\n\t"
            "mov %[din1], %%xmm0\n\t"
            "addpd %[din2], %%xmm0\n\t"
            "movq %%xmm0, %[dout1]"
            : [out1] "=r" (temp1), 
              [out2] "=r" (temp2),
              [dout1] "=m" (dout[i]),
              "=&a" (temp3),      /* Early clobber */
              "=&d" (temp4)       /* Early clobber */
            : [in1] "r" (r0),
              [in2] "rm" (r1),    /* May need secondary reload */
              [in3] "r" (s0),
              [in4] "r" (s1),
              [imm] "i" (0x1234),
              [din1] "xm" (din[i * 2]),
              [din2] "xm" (din[i * 2 + 1]),
              "0" (s2),           /* Matching constraint */
              "1" (s3)
            : "memory", "cc", "xmm0", "xmm1", "ebx"
        );
        
        /* Another asm with different register classes */
        __asm__ volatile (
            "/* Mixed register classes */\n\t"
            "mov %[in_a], %%al\n\t"
            "mov %[in_b], %%bl\n\t"
            "add %%bl, %%al\n\t"
            "mov %%al, %[out_q]\n\t"
            "mov %[in_r], %%ecx\n\t"
            "shl $3, %%ecx\n\t"
            "mov %%ecx, %[out_r]"
            : [out_q] "=q" (cout[i]),  /* Byte register constraint */
              [out_r] "=r" (out[i * 2])
            : [in_a] "q" ((char)r0),   /* Must be in byte register */
              [in_b] "q" ((char)r1),
              [in_r] "r" (r2)
            : "cc", "eax", "ebx", "ecx"
        );
        
        /* Keep variables live */
        global_counter += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    }
}

/* Force secondary reloads with mismatched constraints */
void test_secondary_reloads(int iterations, long long *in, long long *out, float *fin, float *fout) {
    int i;
    long long acc = 0;
    float facc = 0.0f;
    
    for (i = 0; i < iterations; i++) {
        /* Force register pressure with many live values */
        register long long r8 asm("r8") = in[i * 4];
        register long long r9 asm("r9") = in[i * 4 + 1];
        register long long r10 asm("r10") = in[i * 4 + 2];
        register long long r11 asm("r11") = in[i * 4 + 3];
        
        /* asm requiring specific legacy register with 'R' constraint */
        __asm__ volatile (
            "/* Secondary reload test with R constraint */\n\t"
            "mov %[in1], %%rax\n\t"
            "add %[in2], %%rax\n\t"
            "mov %%rax, %[out1]"
            : [out1] "=rm" (out[i])  /* May need secondary reload if memory */
            : [in1] "R" (r8),        /* Legacy register constraint */
              [in2] "r" (r9)
            : "rax", "cc"
        );
        
        /* asm with accumulator constraint then base register constraint */
        int temp;
        __asm__ volatile (
            "/* Accumulator to base register move */\n\t"
            "mov %[val], %%eax\n\t"
            "add $1, %%eax\n\t"
            "mov %%eax, %[temp]"
            : [temp] "=r" (temp)
            : [val] "a" (i)          /* Must be in accumulator */
            : "cc"
        );
        
        /* Use result in another asm with different constraint */
        __asm__ volatile (
            "mov %[in], %%ebx\n\t"
            "imul $7, %%ebx\n\t"
            "mov %%ebx, %[out]"
            : [out] "=r" (out[i + iterations])
            : [in] "b" (temp)        /* Must be in base register */
            : "cc", "ebx"
        );
        
        /* Vector operations to increase register pressure */
        __m128 v1 = _mm_load_ps(&fin[i * 4]);
        __m128 v2 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
        __m128 v3 = _mm_add_ps(v1, v2);
        _mm_store_ps(&fout[i * 4], v3);
        
        /* Complex asm with memory operand that may need secondary reload */
        __asm__ volatile (
            "/* Memory operand needing register */\n\t"
            "mov %[mem], %%rcx\n\t"
            "add %%rcx, %[acc]\n\t"
            "mov %[acc], %[mem]"
            : [acc] "+r" (acc),
              [mem] "=m" (out[i * 2])
            : 
            : "rcx", "cc", "memory"
        );
        
        r8 = r9 + r10;
        r9 = r11 + acc;
        acc += r8 + r9;
    }
}

/* Test optional reloads and nocombine behavior */
void test_optional_reloads(int iterations, char *cin, char *cout, __m128i *vin, __m128i *vout) {
    int i;
    char opt1, opt2, opt3;
    int forced_spill[UNROLL_FACTOR];
    
    for (i = 0; i < iterations; i++) {
        /* Many live variables to force spills */
        int v0 = cin[i * 16 + 0];
        int v1 = cin[i * 16 + 1];
        int v2 = cin[i * 16 + 2];
        int v3 = cin[i * 16 + 3];
        int v4 = cin[i * 16 + 4];
        int v5 = cin[i * 16 + 5];
        int v6 = cin[i * 16 + 6];
        int v7 = cin[i * 16 + 7];
        int v8 = cin[i * 16 + 8];
        int v9 = cin[i * 16 + 9];
        int v10 = cin[i * 16 + 10];
        int v11 = cin[i * 16 + 11];
        int v12 = cin[i * 16 + 12];
        int v13 = cin[i * 16 + 13];
        int v14 = cin[i * 16 + 14];
        int v15 = cin[i * 16 + 15];
        
        /* asm with optional output constraint */
        __asm__ volatile (
            "/* Optional output test */\n\t"
            "test %[in], %[in]\n\t"
            "jz 1f\n\t"
            "mov %[in], %[out]\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "mov $0, %[out]\n\t"
            "2:\n\t"
            "nop"
            : [out] "=?r" (opt1)     /* Optional output */
            : [in] "r" (v0)
            : "cc"
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that could be combined but won't due to barrier */
        __asm__ volatile (
            "mov %[in], %%eax\n\t"
            "add $1, %%eax"
            : "=a" (opt2)
            : [in] "r" (v1)
            : "cc"
        );
        
        /* Another memory barrier */
        __asm__ volatile ("" ::: "memory");
        
        /* Third asm with different clobbers to prevent combine */
        __asm__ volatile (
            "mov %[in], %%ebx\n\t"
            "sub $1, %%ebx"
            : "=b" (opt3)
            : [in] "r" (v2)
            : "cc"
        );
        
        /* Use all variables to keep them live */
        forced_spill[0] = v0 + v1;
        forced_spill[1] = v2 + v3;
        forced_spill[2] = v4 + v5;
        forced_spill[3] = v6 + v7;
        forced_spill[4] = v8 + v9;
        forced_spill[5] = v10 + v11;
        forced_spill[6] = v12 + v13;
        forced_spill[7] = v14 + v15;
        
        /* Vector operations for additional register pressure */
        __m128i vec1 = vin[i];
        __m128i vec2 = _mm_set_epi32(1, 2, 3, 4);
        __m128i vec3 = _mm_add_epi32(vec1, vec2);
        vout[i] = vec3;
        
        /* Store results */
        cout[i] = opt1 + opt2 + opt3;
        for (int j = 0; j < 8; j++) {
            cout[i + j + 1] = forced_spill[j] & 0xFF;
        }
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int iterations, int mode, int *data, int *result) {
    int i, j;
    int temp[UNROLL_FACTOR];
    
    for (i = 0; i < iterations; i++) {
        /* Different asm blocks in different control flow paths */
        if (mode & 0x1) {
            /* Path 1: Complex asm with many operands */
            int a = data[i * 4];
            int b = data[i * 4 + 1];
            int c = data[i * 4 + 2];
            int d = data[i * 4 + 3];
            
            __asm__ volatile (
                "/* Control flow path 1 */\n\t"
                "mov %[a], %%eax\n\t"
                "imul %[b], %%eax\n\t"
                "add %[c], %%eax\n\t"
                "sub %[d], %%eax\n\t"
                "mov %%eax, %[out]"
                : [out] "=rm" (result[i])
                : [a] "r" (a),
                  [b] "rm" (b),
                  [c] "i" (100),     /* Immediate */
                  [d] "r" (d)
                : "rax", "cc"
            );
        } else {
            /* Path 2: Different asm pattern */
            int x = data[i * 4] ^ 0x55;
            int y = data[i * 4 + 1] << 2;
            
            __asm__ volatile (
                "/* Control flow path 2 */\n\t"
                "mov %[x], %%ecx\n\t"
                "xor %[y], %%ecx\n\t"
                "ror $4, %%ecx\n\t"
                "mov %%ecx, %[out]"
                : [out] "=rm" (result[i])
                : [x] "r" (x),
                  [y] "r" (y)
                : "rcx", "cc"
            );
        }
        
        /* Nested loop with asm to create complex live ranges */
        for (j = 0; j < UNROLL_FACTOR; j++) {
            int idx = i * UNROLL_FACTOR + j;
            if (idx < iterations * UNROLL_FACTOR) {
                /* asm inside nested loop */
                __asm__ volatile (
                    "mov %[in], %%r8d\n\t"
                    "lea (%%r8d, %%r8d, 2), %%r9d\n\t"
                    "mov %%r9d, %[out]"
                    : [out] "=rm" (temp[j])
                    : [in] "r" (data[idx])
                    : "r8", "r9", "cc"
                );
                
                /* Modify mode based on result */
                if (temp[j] & 0x1) {
                    mode ^= 0x2;
                }
            }
        }
        
        /* Use temp results */
        int sum = 0;
        for (j = 0; j < UNROLL_FACTOR; j++) {
            sum += temp[j];
        }
        result[i + iterations] = sum;
    }
}

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 2;
    
    /* Parse command line */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    if (iterations <= 0) iterations = 100;
    if (iterations > ARRAY_SIZE / 16) iterations = ARRAY_SIZE / 16;
    
    /* Allocate and initialize arrays */
    int *int_data = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_out = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    long long *ll_data = (long long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long long));
    long long *ll_out = (long long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long long));
    double *double_data = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *double_out = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *float_data = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *float_out = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    char *char_data = (char*)aligned_alloc(64, ARRAY_SIZE * sizeof(char));
    char *char_out = (char*)aligned_alloc(64, ARRAY_SIZE * sizeof(char));
    __m128i *vec_data = (__m128i*)aligned_alloc(64, (ARRAY_SIZE / 4) * sizeof(__m128i));
    __m128i *vec_out = (__m128i*)aligned_alloc(64, (ARRAY_SIZE / 4) * sizeof(__m128i));
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = i ^ 0xAA55AA55;
        int_out[i] = 0;
        ll_data[i] = (long long)i * 3;
        ll_out[i] = 0;
        double_data[i] = i * 1.5;
        double_out[i] = 0.0;
        float_data[i] = i * 0.75f;
        float_out[i] = 0.0f;
        char_data[i] = (char)(i & 0xFF);
        char_out[i] = 0;
        if (i % 4 == 0) {
            vec_data[i / 4] = _mm_set_epi32(i, i+1, i+2, i+3);
            vec_out[i / 4] = _mm_setzero_si128();
        }
    }
    
    /* Run tests */
    test_primary_reloads(iterations / 4, int_data, int_out, double_data, double_out);
    test_secondary_reloads(iterations / 4, ll_data, ll_out, float_data, float_out);
    test_optional_reloads(iterations / 4, char_data, char_out, vec_data, vec_out);
    test_control_flow_reloads(iterations / 4, mode, int_data, int_out + ARRAY_SIZE/2);
    
    /* Compute checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_out[i];
        checksum += ll_out[i] & 0xFFFFFFFF;
        checksum += (unsigned long long)(double_out[i] * 1000);
        checksum += (unsigned long long)(float_out[i] * 1000);
        checksum += (unsigned char)char_out[i];
        if (i % 4 == 0) {
            __m128i v = vec_out[i / 4];
            checksum += _mm_extract_epi32(v, 0);
            checksum += _mm_extract_epi32(v, 1);
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Free memory */
    free(int_data);
    free(int_out);
    free(ll_data);
    free(ll_out);
    free(double_data);
    free(double_out);
    free(float_data);
    free(float_out);
    free(char_data);
    free(char_out);
    free(vec_data);
    free(vec_out);
    
    return 0;
}
