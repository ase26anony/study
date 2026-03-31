/* reload_test.c - Complex inline assembly to trigger reload.cc logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <stdint.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *input, int *output);
void test_secondary_reloads(int iterations, double *input, double *output);
void test_optional_reloads(int iterations, float *input, float *output);
void test_control_flow_reloads(int iterations, int mode, long *results);

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_double = 3.14159;

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 2;
    
    if (argc >= 2) iterations = atoi(argv[1]);
    if (argc >= 3) mode = atoi(argv[2]);
    
    /* Initialize arrays with mixed data */
    int int_array[ARRAY_SIZE];
    double double_array[ARRAY_SIZE];
    float float_array[ARRAY_SIZE];
    long result_array[UNROLL_FACTOR];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        double_array[i] = i * 1.5 + 0.25;
        float_array[i] = i * 0.75f + 0.125f;
    }
    
    for (int i = 0; i < UNROLL_FACTOR; i++) {
        result_array[i] = 0;
    }
    
    printf("Starting reload tests with %d iterations, mode %d\n", iterations, mode);
    
    /* Execute all test functions to trigger various reload patterns */
    test_primary_reloads(iterations, int_array, int_array + ARRAY_SIZE/2);
    test_secondary_reloads(iterations, double_array, double_array + ARRAY_SIZE/2);
    test_optional_reloads(iterations, float_array, float_array + ARRAY_SIZE/2);
    test_control_flow_reloads(iterations, mode, result_array);
    
    /* Compute checksum to ensure all assembly executed */
    long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        checksum += int_array[i + ARRAY_SIZE/2];
        checksum += (long)(double_array[i + ARRAY_SIZE/2] * 1000);
        checksum += (long)(float_array[i + ARRAY_SIZE/2] * 1000);
    }
    
    for (int i = 0; i < UNROLL_FACTOR; i++) {
        checksum += result_array[i];
    }
    
    printf("Final checksum: %ld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}

/* Primary reloads with register pressure */
void test_primary_reloads(int iterations, int *input, int *output) {
    /* Create many live variables to exhaust registers */
    int a = input[0], b = input[1], c = input[2], d = input[3];
    int e = input[4], f = input[5], g = input[6], h = input[7];
    int i = input[8], j = input[9], k = input[10], l = input[11];
    int m = input[12], n = input[13], o = input[14], p = input[15];
    
    /* Use vector types alongside scalars */
    __m128i v1 = _mm_set_epi32(a, b, c, d);
    __m128i v2 = _mm_set_epi32(e, f, g, h);
    __m256d vd1 = _mm256_set_pd(i, j, k, l);
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex inline assembly with multiple operands and constraints */
        __asm__ volatile (
            /* Output operands with different constraints */
            "movl %[in_a], %%eax\n\t"
            "addl %[in_b], %%eax\n\t"
            "imull %[in_c], %%eax\n\t"
            "movl %%eax, %[out_x]\n\t"
            "movl %[in_d], %%ebx\n\t"
            "subl %[in_e], %%ebx\n\t"
            "movl %%ebx, %[out_y]\n\t"
            /* Byte register constraint */
            "movb %[in_f], %%cl\n\t"
            "addb %[in_g], %%cl\n\t"
            "movb %%cl, %[out_z]\n\t"
            /* Memory operand with offset */
            "addl $1, %[mem]\n\t"
            /* Top of stack constraint for x87 */
            "fldl %[double_in]\n\t"
            "fstpl %[double_out]\n\t"
            : 
            /* Outputs with mixed constraints */
            [out_x] "=r" (a),           /* General register */
            [out_y] "=q" (b),           /* Byte register (a,b,c,d) */
            [out_z] "=r" (c),           /* Another general register */
            [mem] "+m" (input[iter % 16]), /* Memory read-write */
            [double_out] "=t" (global_double) /* x87 top of stack */
            :
            /* Inputs with diverse constraints */
            [in_a] "r" (d),             /* Register */
            [in_b] "i" (123),           /* Immediate */
            [in_c] "m" (input[(iter + 1) % 16]), /* Memory */
            [in_d] "a" (e),             /* Accumulator */
            [in_e] "r" (f),             /* Register */
            [in_f] "q" (g),             /* Byte register */
            [in_g] "i" (5),             /* Immediate */
            [double_in] "m" (global_double) /* Memory */
            :
            /* Clobber list - many registers */
            "cc", "memory",
            "rax", "rbx", "rcx", "rdx",
            "r8", "r9", "r10", "r11",
            "xmm0", "xmm1", "xmm2", "xmm3",
            "xmm4", "xmm5", "xmm6", "xmm7"
        );
        
        /* More assembly with earlyclobber and matching constraints */
        int temp1, temp2;
        __asm__ volatile (
            "mov %[src], %[dst]\n\t"
            "add $1, %[dst]\n\t"
            "mov %[dst], %[tmp]\n\t"
            : [dst] "=&r" (temp1),  /* Earlyclobber */
              [tmp] "=r" (temp2)
            : [src] "0" (h)         /* Matching constraint */
            : "cc"
        );
        
        /* Update many live variables to keep them active */
        a = b + c;
        b = c + d;
        c = d + e;
        d = e + f;
        e = f + g;
        f = g + h;
        g = h + i;
        h = i + j;
        i = j + k;
        j = k + l;
        k = l + m;
        l = m + n;
        m = n + o;
        n = o + p;
        o = p + a;
        p = a + b;
        
        /* Use vector operations to increase register pressure */
        v1 = _mm_add_epi32(v1, v2);
        vd1 = _mm256_add_pd(vd1, _mm256_set1_pd(1.0));
        
        /* Store results with memory barrier */
        output[iter % (ARRAY_SIZE/2)] = a + b + c + d;
        __asm__ volatile ("" ::: "memory");
    }
    
    /* Final assembly to ensure all values used */
    __asm__ volatile (
        "addl %%eax, %%ebx\n\t"
        "addl %%ebx, %%ecx\n\t"
        : 
        : "a" (a), "b" (b), "c" (c)
        : "cc"
    );
}

/* Secondary reload patterns */
void test_secondary_reloads(int iterations, double *input, double *output) {
    double x = input[0], y = input[1], z = input[2];
    double accum = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Assembly requiring specific register classes */
        double result1, result2;
        
        /* Constraint requiring legacy register (eax, ebx, ecx, edx) */
        __asm__ volatile (
            "movq %[in1], %%rax\n\t"
            "addq %[in2], %%rax\n\t"
            "movq %%rax, %[out1]\n\t"
            "movq %[in3], %%rbx\n\t"
            "subq %%rax, %%rbx\n\t"
            "movq %%rbx, %[out2]\n\t"
            : [out1] "=R" (result1),  /* Legacy register constraint */
              [out2] "=r" (result2)
            : [in1] "r" (x),
              [in2] "m" (input[(i + 3) % ARRAY_SIZE]),
              [in3] "r" (y)
            : "rax", "rbx", "cc"
        );
        
        /* Mismatched constraints forcing secondary reload */
        long temp_long;
        __asm__ volatile (
            "mov %[in], %%rax\n\t"
            "imul $7, %%rax\n\t"
            "mov %%rax, %[out]\n\t"
            : [out] "=rm" (temp_long)  /* Register or memory - may need secondary */
            : [in] "r" ((long)(x * 1000))
            : "rax", "cc"
        );
        
        /* x87 floating point with specific register constraints */
        double fp_result;
        __asm__ volatile (
            "fldl %[in1]\n\t"
            "faddl %[in2]\n\t"
            "fstpl %[out]\n\t"
            : [out] "=m" (fp_result)
            : [in1] "m" (z),
              [in2] "m" (input[(i + 5) % ARRAY_SIZE])
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        
        /* Update variables in complex pattern */
        x = y + result1;
        y = z + result2;
        z = input[i % ARRAY_SIZE] + fp_result;
        accum += temp_long;
        
        /* Memory operand that might need secondary reload */
        __asm__ volatile (
            "addsd %[src], %[dst]\n\t"
            : [dst] "+x" (accum)      /* SSE register */
            : [src] "xm" (input[(i + 7) % ARRAY_SIZE])  /* SSE register or memory */
            : "cc"
        );
        
        output[i % (ARRAY_SIZE/2)] = accum;
    }
}

/* Optional and non-combine reloads */
void test_optional_reloads(int iterations, float *input, float *output) {
    float a = input[0], b = input[1], c = input[2];
    float results[UNROLL_FACTOR];
    
    for (int i = 0; i < UNROLL_FACTOR; i++) {
        results[i] = input[i];
    }
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Optional output constraint */
        float opt_result;
        int optional_success;
        
        __asm__ volatile (
            "test %[flag], %[flag]\n\t"
            "jz 1f\n\t"
            "movss %[in], %[out]\n\t"
            "addss %[const], %[out]\n\t"
            "mov $1, %[success]\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "mov $0, %[success]\n\t"
            "2:\n\t"
            : [out] "=?r" (opt_result),    /* Optional output */
              [success] "=r" (optional_success)
            : [in] "r" (a),
              [const] "r" (b),
              [flag] "r" (iter & 1)
            : "cc"
        );
        
        if (optional_success) {
            c = opt_result;
        }
        
        /* Multiple similar asm blocks with memory barriers to prevent combining */
        for (int j = 0; j < 4; j++) {
            float temp;
            __asm__ volatile (
                "movss %[in], %[out]\n\t"
                "mulss %[scale], %[out]\n\t"
                : [out] "=r" (temp)
                : [in] "r" (results[j]),
                  [scale] "r" (1.1f)
                : "cc"
            );
            results[j] = temp;
            
            /* Memory barrier prevents reload combination */
            __asm__ volatile ("" ::: "memory");
        }
        
        /* Different clobber lists create non-combine scenarios */
        __asm__ volatile (
            "addss %[a], %[b]\n\t"
            : [b] "+r" (b)
            : [a] "r" (a)
            : "cc", "xmm0", "xmm1"
        );
        
        __asm__ volatile (
            "subss %[a], %[b]\n\t"
            : [b] "+r" (b)
            : [a] "r" (c)
            : "cc", "xmm2", "xmm3"  /* Different clobbers */
        );
        
        /* Complex update pattern */
        a = b + input[(iter * 3) % ARRAY_SIZE];
        b = c + input[(iter * 5) % ARRAY_SIZE];
        c = a + input[(iter * 7) % ARRAY_SIZE];
        
        /* Store with volatile to ensure execution */
        output[iter % (ARRAY_SIZE/2)] = a + b + c;
        global_counter++;
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int iterations, int mode, long *results) {
    long vars[UNROLL_FACTOR];
    for (int i = 0; i < UNROLL_FACTOR; i++) {
        vars[i] = i * 1000;
    }
    
    for (int i = 0; i < iterations; i++) {
        /* Assembly inside conditionals */
        if (mode & 1) {
            long temp;
            __asm__ volatile (
                "mov %[a], %%rax\n\t"
                "imul %[b], %%rax\n\t"
                "mov %%rax, %[out]\n\t"
                : [out] "=r" (temp)
                : [a] "r" (vars[0]),
                  [b] "r" (vars[1])
                : "rax", "cc"
            );
            vars[0] = temp;
        } else {
            __asm__ volatile (
                "add %[a], %[b]\n\t"
                : [b] "+r" (vars[1])
                : [a] "r" (vars[2])
                : "cc"
            );
        }
        
        /* Assembly in loops with varying trip counts */
        int limit = (i % 8) + 1;
        for (int j = 0; j < limit; j++) {
            __asm__ volatile (
                "inc %[val]\n\t"
                : [val] "+r" (vars[j % UNROLL_FACTOR])
                :
                : "cc"
            );
        }
        
        /* Switch statement with assembly in different cases */
        switch (i % 4) {
            case 0:
                __asm__ volatile (
                    "mov %[src], %[dst]\n\t"
                    "shl $2, %[dst]\n\t"
                    : [dst] "=r" (vars[2])
                    : [src] "r" (vars[3])
                    : "cc"
                );
                break;
            case 1:
                __asm__ volatile (
                    "mov %[src], %[dst]\n\t"
                    "shr $1, %[dst]\n\t"
                    : [dst] "=r" (vars[3])
                    : [src] "r" (vars[4])
                    : "cc"
                );
                break;
            case 2:
                __asm__ volatile (
                    "xor %[src], %[dst]\n\t"
                    : [dst] "+r" (vars[4])
                    : [src] "r" (vars[5])
                    : "cc"
                );
                break;
            case 3:
                __asm__ volatile (
                    "or %[src], %[dst]\n\t"
                    : [dst] "+r" (vars[5])
                    : [src] "r" (vars[6])
                    : "cc"
                );
                break;
        }
        
        /* Complex expression with inline assembly */
        long computed = 0;
        for (int k = 0; k < UNROLL_FACTOR; k++) {
            computed += vars[k];
            __asm__ volatile (
                "add %[inc], %[total]\n\t"
                : [total] "+r" (computed)
                : [inc] "ri" (k * 10)  /* Register or immediate */
                : "cc"
            );
        }
        
        results[i % UNROLL_FACTOR] = computed;
    }
}
