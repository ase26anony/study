#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test function for primary reloads with register pressure */
void test_primary_reloads(int iterations, int* input, int* output, double* dinput, double* doutput) {
    volatile int i, j, k;
    volatile int a, b, c, d, e, f, g, h;
    volatile int w, x, y, z;
    volatile double da, db, dc, dd, de, df;
    volatile __m128i v1, v2, v3;
    volatile __m128d vd1, vd2;
    
    /* Create register pressure with many live variables */
    a = input[0]; b = input[1]; c = input[2]; d = input[3];
    e = input[4]; f = input[5]; g = input[6]; h = input[7];
    w = input[8]; x = input[9]; y = input[10]; z = input[11];
    
    da = dinput[0]; db = dinput[1]; dc = dinput[2]; dd = dinput[3];
    de = dinput[4]; df = dinput[5];
    
    v1 = _mm_set_epi32(a, b, c, d);
    v2 = _mm_set_epi32(e, f, g, h);
    
    /* Complex inline assembly with multiple operands and constraints */
    for (i = 0; i < iterations; i++) {
        /* Unrolled loop to increase register pressure */
        for (j = 0; j < UNROLL_FACTOR; j++) {
            int idx = (i * UNROLL_FACTOR + j) % ARRAY_SIZE;
            
            /* Extended asm with 5+ operands, mixed constraints */
            __asm__ volatile (
                /* Outputs with different constraints and modes */
                "movl %[in1], %%eax\n\t"
                "addl %[in2], %%eax\n\t"
                "imull %[in3], %%eax\n\t"
                "movl %%eax, %[out1]\n\t"
                "movb %%al, %[out2]\n\t"
                "movl %%eax, %[out3]\n\t"
                "fldl %[din1]\n\t"
                "faddl %[din2]\n\t"
                "fstpl %[dout1]\n\t"
                : [out1] "=r" (output[idx]),      /* word register */
                  [out2] "=q" (*(char*)&output[idx+1]), /* byte register */
                  [out3] "=m" (output[idx+2]),    /* memory */
                  [dout1] "=m" (doutput[idx])     /* memory for double */
                : [in1] "r" (input[idx]),         /* register */
                  [in2] "rm" (input[idx+1]),      /* register or memory */
                  [in3] "i" (37),                 /* immediate */
                  [din1] "m" (dinput[idx]),       /* memory */
                  [din2] "m" (dinput[idx+1])      /* memory */
                : "eax", "cc", "memory", "st"
            );
            
            /* Another asm with earlyclobber and matching constraints */
            int temp1, temp2;
            __asm__ volatile (
                "mov %[src], %[dest]\n\t"
                "add $1, %[dest]\n\t"
                : [dest] "=&r" (temp1), "=r" (temp2)
                : [src] "0" (a), "1" (b)  /* Matching constraints */
                : "cc"
            );
            
            /* Update live variables to keep them in use */
            a = (a + temp1) & 0xFFF;
            b = (b + temp2) & 0xFFF;
            
            /* Use vector intrinsics alongside scalar operations */
            v3 = _mm_add_epi32(v1, v2);
            v1 = _mm_add_epi32(v1, v3);
            
            /* More asm with different register classes */
            __asm__ volatile (
                "mov %[val], %%ebx\n\t"
                "shl $2, %%ebx\n\t"
                "mov %%ebx, %[res]\n\t"
                : [res] "=r" (c)
                : [val] "r" (c)
                : "ebx", "cc"
            );
        }
        
        /* Conditional asm to create control flow dependent reloads */
        if (i % 3 == 0) {
            __asm__ volatile (
                "movl %[in], %%ecx\n\t"
                "rorl $4, %%ecx\n\t"
                "movl %%ecx, %[out]\n\t"
                : [out] "=rm" (d)
                : [in] "r" (d)
                : "ecx", "cc"
            );
        } else if (i % 3 == 1) {
            __asm__ volatile (
                "movl %[in], %%edx\n\t"
                "bswap %%edx\n\t"
                "movl %%edx, %[out]\n\t"
                : [out] "=r" (e)
                : [in] "r" (e)
                : "edx", "cc"
            );
        }
    }
}

/* Test function for secondary reload patterns */
void test_secondary_reloads(int iterations, int* input, int* output) {
    volatile int i;
    
    for (i = 0; i < iterations; i++) {
        int idx = i % ARRAY_SIZE;
        
        /* Asm requiring accumulator register with memory input */
        int acc_result;
        __asm__ volatile (
            "movl %[mem_in], %%eax\n\t"
            "addl %%eax, %%eax\n\t"
            "movl %%eax, %[acc_out]\n\t"
            : [acc_out] "=a" (acc_result)        /* Must be in eax */
            : [mem_in] "m" (input[idx])          /* Memory operand */
            : "cc"
        );
        
        /* Use the accumulator result in another asm with different constraint */
        int base_result;
        __asm__ volatile (
            "movl %[in], %%ebx\n\t"
            "addl %[acc], %%ebx\n\t"
            "movl %%ebx, %[out]\n\t"
            : [out] "=b" (base_result)          /* Must be in ebx */
            : [in] "r" (output[idx]),
              [acc] "a" (acc_result)            /* From accumulator */
            : "cc"
        );
        
        output[idx] = base_result;
        
        /* Asm with "rm" constraint that may need secondary reload */
        int temp = input[idx] + i;
        __asm__ volatile (
            "lea (%[val], %[val], 2), %[res]\n\t"  /* res = val * 3 */
            : [res] "=rm" (output[idx+1])          /* May need secondary reload */
            : [val] "rm" (temp)                    /* Register or memory */
            : "cc"
        );
        
        /* Force use of legacy registers with R constraint */
        int legacy_val;
        __asm__ volatile (
            "movl %[in], %%edi\n\t"
            "notl %%edi\n\t"
            "movl %%edi, %[out]\n\t"
            : [out] "=R" (legacy_val)            /* Legacy register constraint */
            : [in] "r" (input[idx])
            : "edi", "cc"
        );
        
        output[idx+2] = legacy_val;
    }
}

/* Test function for optional and non-combine reloads */
void test_optional_reloads(int iterations, int* input, int* output) {
    volatile int i;
    
    for (i = 0; i < iterations; i++) {
        int idx = i % ARRAY_SIZE;
        
        /* Asm with optional output constraint */
        int opt_result;
        __asm__ volatile (
            "testl %[in], %[in]\n\t"
            "jz 1f\n\t"
            "movl %[in], %[out]\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "movl $1, %[out]\n\t"
            "2:\n\t"
            : [out] "=?r" (opt_result)          /* Optional output */
            : [in] "r" (input[idx])
            : "cc"
        );
        
        output[idx] = opt_result;
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that could be combined but won't due to barrier */
        __asm__ volatile (
            "addl $1, %[val]\n\t"
            : [val] "+r" (output[idx])
            :: "cc"
        );
        
        /* Another asm with different clobbers to prevent combination */
        __asm__ volatile (
            "movl %[in], %%ecx\n\t"
            "incl %%ecx\n\t"
            "movl %%ecx, %[out]\n\t"
            : [out] "=r" (output[idx+1])
            : [in] "r" (input[idx+1])
            : "ecx", "cc"
        );
        
        /* Volatile asm with nocombine effect */
        __asm__ volatile (
            "nop\n\t"
            ::: "cc"
        );
    }
}

int main(int argc, char** argv) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line arguments */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        mode = atoi(argv[2]);
        if (mode < 1 || mode > 3) mode = 1;
    }
    
    /* Initialize arrays with mixed data */
    int* input = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* output = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double* dinput = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double* doutput = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    if (!input || !output || !dinput || !doutput) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i] = (i * 37) & 0xFFFF;
        output[i] = 0;
        dinput[i] = i * 0.5;
        doutput[i] = 0.0;
    }
    
    /* Run test functions based on mode */
    if (mode == 1 || mode == 0) {
        test_primary_reloads(iterations, input, output, dinput, doutput);
    }
    
    if (mode == 2 || mode == 0) {
        test_secondary_reloads(iterations, input, output);
    }
    
    if (mode == 3 || mode == 0) {
        test_optional_reloads(iterations, input, output);
    }
    
    /* Compute checksum to ensure all asm blocks execute */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
        checksum += (unsigned long long)doutput[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(input);
    free(output);
    free(dinput);
    free(doutput);
    
    return 0;
}
