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
void test_secondary_reloads(int iterations, long long *in, long long *out, float *fin, float *fout);
void test_optional_reloads(int iterations, unsigned char *in, unsigned char *out, __m128i *vin, __m128i *vout);
void test_control_flow_reloads(int mode, int *data, int *result, int size);

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile int global_counter = 0;

/* Complex inline assembly with multiple operands and constraints */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout) {
    int i, j;
    int temp1, temp2, temp3, temp4, temp5;
    double dtemp1, dtemp2, dtemp3;
    long long lltemp;
    
    /* Create many live variables to increase register pressure */
    int live_vars[UNROLL_FACTOR];
    double live_doubles[UNROLL_FACTOR];
    
    for (i = 0; i < UNROLL_FACTOR; i++) {
        live_vars[i] = global_seed + i;
        live_doubles[i] = (double)(global_seed + i) * 1.234;
    }
    
    /* Unrolled loop with complex asm statements */
    for (i = 0; i < iterations; i++) {
        /* Force register pressure by using many variables */
        temp1 = in[i % ARRAY_SIZE];
        temp2 = in[(i + 1) % ARRAY_SIZE];
        temp3 = in[(i + 2) % ARRAY_SIZE];
        temp4 = in[(i + 3) % ARRAY_SIZE];
        temp5 = in[(i + 4) % ARRAY_SIZE];
        
        dtemp1 = din[i % ARRAY_SIZE];
        dtemp2 = din[(i + 1) % ARRAY_SIZE];
        dtemp3 = din[(i + 2) % ARRAY_SIZE];
        
        /* Complex asm with 5+ operands, mixed constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (temp1),      /* General register */
            "=&r" (temp2),     /* Early clobber */
            "=q" (temp3),      /* Byte-addressable register (eax, ebx, ecx, edx) */
            "=a" (temp4),      /* Accumulator */
            "=d" (temp5),      /* Data register */
            "=t" (dtemp1),     /* Top of FP stack */
            "=m" (dout[i])     /* Memory output */
            
            /* Inputs with various constraints */
            : "0" (temp1),     /* Matching constraint - same as output 0 */
            "r" (temp2),       /* General register */
            "i" (12345),       /* Immediate */
            "m" (in[i]),       /* Memory input */
            "r" (temp3),       /* General register */
            "g" (temp4),       /* General register or memory */
            "rm" (dtemp2),     /* Register or memory */
            "X" (dtemp3)       /* Any operand */
            
            /* Clobber list */
            : "cc", "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
              "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        
        /* Another asm with vector constraints */
        __m128i v1, v2, v3;
        v1 = _mm_set_epi32(temp1, temp2, temp3, temp4);
        v2 = _mm_set1_epi32(global_seed);
        
        __asm__ volatile (
            "vmovdqa %1, %0\n\t"
            "vpaddd %0, %2, %0\n\t"
            "vpslld $2, %0, %0"
            : "=x" (v3), "=x" (v1)
            : "x" (v2), "0" (v1), "1" (v1)
            : "xmm0", "xmm1", "xmm2", "xmm3"
        );
        
        /* Store results to prevent optimization */
        out[i % ARRAY_SIZE] = temp1 + temp2 + temp3 + temp4 + temp5;
        dout[i % ARRAY_SIZE] = dtemp1 + dtemp2 + dtemp3;
        
        /* Update live variables to keep them in use */
        for (j = 0; j < UNROLL_FACTOR; j++) {
            live_vars[j] += temp1;
            live_doubles[j] += dtemp1;
        }
    }
    
    /* Use live variables to prevent dead code elimination */
    int sum = 0;
    for (j = 0; j < UNROLL_FACTOR; j++) {
        sum += live_vars[j];
    }
    global_counter += sum;
}

/* Test secondary reload patterns */
void test_secondary_reloads(int iterations, long long *in, long long *out, float *fin, float *fout) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        long long ll_in = in[i % ARRAY_SIZE];
        long long ll_out;
        float f_in = fin[i % ARRAY_SIZE];
        float f_out;
        
        /* Force secondary reload by using 'a' constraint then 'b' constraint */
        int a_reg, b_reg;
        
        /* First asm uses accumulator */
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "addl $1, %%eax"
            : "=a" (a_reg)
            : "r" (i)
            : "eax"
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Second asm requires base register, forcing move from eax to ebx */
        __asm__ volatile (
            "movl %%ebx, %0\n\t"
            "addl %%eax, %0"
            : "=r" (b_reg)
            : "a" (a_reg), "b" (a_reg)  /* 'b' constraint forces ebx allocation */
            : "eax", "ebx"
        );
        
        /* Complex asm with 'R' constraint (legacy register) */
        long long legacy_val;
        __asm__ volatile (
            "movq %1, %%rax\n\t"
            "addq $0x12345678, %%rax\n\t"
            "movq %%rax, %0"
            : "=R" (legacy_val)  /* May require secondary reload if allocated to R8-R15 */
            : "rm" (ll_in)
            : "rax", "cc"
        );
        
        /* Asm with mismatched constraints requiring secondary reload */
        __asm__ volatile (
            "flds %1\n\t"
            "fstps %0\n\t"
            "fwait"
            : "=m" (f_out)
            : "f" (f_in)  /* Floating point register constraint */
            : "st", "st(1)"
        );
        
        /* Store results */
        out[i % ARRAY_SIZE] = ll_out + legacy_val + a_reg + b_reg;
        fout[i % ARRAY_SIZE] = f_out;
    }
}

/* Test optional reloads and nocombine patterns */
void test_optional_reloads(int iterations, unsigned char *in, unsigned char *out, __m128i *vin, __m128i *vout) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        unsigned char byte1 = in[i % ARRAY_SIZE];
        unsigned char byte2 = in[(i + 1) % ARRAY_SIZE];
        unsigned char byte3, byte4;
        
        /* Asm with optional constraints */
        __asm__ volatile (
            "movb %2, %%al\n\t"
            "addb %3, %%al\n\t"
            "movb %%al, %0\n\t"
            "testb %%al, %%al\n\t"
            "jnz 1f\n\t"
            "movb $0, %1\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "movb %%al, %1\n\t"
            "2:\n\t"
            : "=?r" (byte3),   /* Optional output */
              "=r" (byte4)     /* Required output */
            : "r" (byte1),
              "i" (0x10)       /* Immediate */
            : "al", "cc"
        );
        
        /* Memory barrier to prevent combination with next asm */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm but with different clobbers to prevent combining */
        __asm__ volatile (
            "movb %1, %%al\n\t"
            "subb $1, %%al"
            : "=r" (byte3)
            : "r" (byte2)
            : "al", "cc", "memory"  /* Extra clobbers prevent combine */
        );
        
        /* Vector asm that could combine but volatile prevents it */
        __m128i v1 = vin[i % ARRAY_SIZE];
        __m128i v2;
        
        __asm__ volatile (
            "vmovdqa %1, %0\n\t"
            "vpsllw $3, %0, %0"
            : "=x" (v2)
            : "x" (v1)
            : "xmm0"
        );
        
        /* Another similar vector operation */
        __asm__ volatile (
            "vmovdqa %1, %0\n\t"
            "vpsrlw $1, %0, %0"
            : "=x" (v1)
            : "x" (v2)
            : "xmm0", "xmm1"  /* Different clobber list */
        );
        
        /* Store results */
        out[i % ARRAY_SIZE] = byte3 + byte4;
        vout[i % ARRAY_SIZE] = _mm_add_epi8(v1, v2);
    }
}

/* Test control flow dependent reloads */
void test_control_flow_reloads(int mode, int *data, int *result, int size) {
    int i;
    
    for (i = 0; i < size; i++) {
        int x = data[i];
        int y;
        
        /* Different asm statements in different control flow paths */
        if (mode == 0) {
            __asm__ volatile (
                "movl %1, %%eax\n\t"
                "imull $7, %%eax"
                : "=a" (y)
                : "r" (x)
                : "eax", "edx"
            );
        } else if (mode == 1) {
            __asm__ volatile (
                "movl %1, %%ecx\n\t"
                "shrl $2, %%ecx"
                : "=c" (y)
                : "r" (x)
                : "ecx", "cc"
            );
        } else {
            __asm__ volatile (
                "leal (%1,%1,2), %0"
                : "=r" (y)
                : "r" (x)
                : "cc"
            );
        }
        
        /* Nested loop with asm to increase complexity */
        int j;
        for (j = 0; j < 3; j++) {
            int temp;
            __asm__ volatile (
                "addl $1, %0"
                : "+r" (y)
                :
                : "cc"
            );
            
            if (j % 2 == 0) {
                __asm__ volatile (
                    "xorl %%eax, %%eax\n\t"
                    "cpuid"
                    : /* no outputs */
                    : "a" (0)
                    : "eax", "ebx", "ecx", "edx", "memory"
                );
            }
        }
        
        result[i] = y;
    }
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line arguments */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        mode = atoi(argv[2]);
    }
    
    /* Allocate and initialize arrays with mixed data types */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *int_out = (int*)malloc(ARRAY_SIZE * sizeof(int));
    long long *ll_data = (long long*)malloc(ARRAY_SIZE * sizeof(long long));
    long long *ll_out = (long long*)malloc(ARRAY_SIZE * sizeof(long long));
    double *double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double *double_out = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *float_out = (float*)malloc(ARRAY_SIZE * sizeof(float));
    unsigned char *byte_data = (unsigned char*)malloc(ARRAY_SIZE);
    unsigned char *byte_out = (unsigned char*)malloc(ARRAY_SIZE);
    __m128i *vec_data = (__m128i*)_mm_malloc(ARRAY_SIZE * sizeof(__m128i), 16);
    __m128i *vec_out = (__m128i*)_mm_malloc(ARRAY_SIZE * sizeof(__m128i), 16);
    
    if (!int_data || !int_out || !ll_data || !ll_out || 
        !double_data || !double_out || !float_data || !float_out ||
        !byte_data || !byte_out || !vec_data || !vec_out) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = global_seed + i;
        ll_data[i] = (long long)global_seed * i;
        double_data[i] = (double)global_seed / (i + 1);
        float_data[i] = (float)global_seed * 0.5f / (i + 1);
        byte_data[i] = (unsigned char)(global_seed + i);
        vec_data[i] = _mm_set_epi32(i, i+1, i+2, i+3);
    }
    
    /* Run test functions */
    printf("Running primary reload tests...\n");
    test_primary_reloads(iterations, int_data, int_out, double_data, double_out);
    
    printf("Running secondary reload tests...\n");
    test_secondary_reloads(iterations / 2, ll_data, ll_out, float_data, float_out);
    
    printf("Running optional reload tests...\n");
    test_optional_reloads(iterations / 4, byte_data, byte_out, vec_data, vec_out);
    
    printf("Running control flow reload tests...\n");
    test_control_flow_reloads(mode, int_data, int_out, ARRAY_SIZE);
    
    /* Compute checksum to ensure all asm executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_out[i];
        checksum += ll_out[i];
        checksum += (unsigned long long)double_out[i];
        checksum += (unsigned long long)float_out[i];
        checksum += byte_out[i];
        
        /* Add vector checksum */
        __m128i v = vec_out[i];
        int v_elems[4];
        _mm_storeu_si128((__m128i*)v_elems, v);
        checksum += v_elems[0] + v_elems[1] + v_elems[2] + v_elems[3];
    }
    
    printf("Final checksum: %llu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Cleanup */
    free(int_data);
    free(int_out);
    free(ll_data);
    free(ll_out);
    free(double_data);
    free(double_out);
    free(float_data);
    free(float_out);
    free(byte_data);
    free(byte_out);
    _mm_free(vec_data);
    _mm_free(vec_out);
    
    return 0;
}
