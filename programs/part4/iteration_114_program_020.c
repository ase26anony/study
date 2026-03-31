/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <stdint.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int* in_ints, double* in_doubles, 
                         int* out_ints, double* out_doubles);
void test_secondary_reloads(int iterations, int* in_ints, double* in_doubles,
                           int* out_ints, double* out_doubles);
void test_optional_reloads(int iterations, int* in_ints, double* in_doubles,
                          int* out_ints, double* out_doubles);

/* Helper to create register pressure */
static inline void create_register_pressure(int* restrict a, int* restrict b, 
                                           int* restrict c, int* restrict d,
                                           int count) {
    for (int i = 0; i < count; i++) {
        a[i] = b[i] * c[i] + d[i];
        b[i] = a[i] ^ c[i] | d[i];
        c[i] = (a[i] + b[i]) * d[i];
        d[i] = a[i] - b[i] + c[i];
    }
}

/* Primary reloads with diverse constraints */
void test_primary_reloads(int iterations, int* in_ints, double* in_doubles,
                         int* out_ints, double* out_doubles) {
    volatile int temp1, temp2, temp3, temp4, temp5;
    volatile double dtemp1, dtemp2, dtemp3;
    volatile __m128i vtemp1, vtemp2;
    volatile __m256d vdtemp1, vdtemp2;
    
    /* Create many live variables to pressure registers */
    int live1 = in_ints[0];
    int live2 = in_ints[1];
    int live3 = in_ints[2];
    int live4 = in_ints[3];
    int live5 = in_ints[4];
    int live6 = in_ints[5];
    int live7 = in_ints[6];
    int live8 = in_ints[7];
    int live9 = in_ints[8];
    int live10 = in_ints[9];
    
    double dlive1 = in_doubles[0];
    double dlive2 = in_doubles[1];
    double dlive3 = in_doubles[2];
    double dlive4 = in_doubles[3];
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex asm with 7 operands, mixing constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (temp1),      /* General register */
            "=&r" (temp2),     /* Earlyclobber */
            "=q" (temp3),      /* Byte-addressable register (a,b,c,d) */
            "=a" (temp4),      /* Accumulator */
            "=d" (temp5),      /* Data register */
            "=t" (dtemp1),     /* Top of FP stack */
            "=m" (out_ints[iter % ARRAY_SIZE])  /* Memory output */
            
            : /* Inputs with diverse constraints */
            "0" (live1),       /* Matching constraint with output 0 */
            "r" (live2),       /* General register */
            "i" (0xDEADBEEF),  /* Immediate */
            "m" (in_ints[iter % ARRAY_SIZE]),  /* Memory input */
            "r" (live3),
            "g" (live4),       /* General or memory */
            "rm" (live5)       /* Register or memory */
            
            : /* Clobbers - many registers to force spills */
            "rcx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
            "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)",
            "cc", "memory"
        );
        
        /* Update live variables to prevent dead code elimination */
        live1 = temp1 + iter;
        live2 = temp2 ^ live1;
        live3 = temp3 | live2;
        live4 = temp4 - live3;
        live5 = temp5 * live4;
        
        /* Another asm with FP/vector constraints */
        __asm__ volatile (
            "=x" (vtemp1),     /* Any SSE register */
            "=Yz" (dtemp2),    /* First SSE register */
            "=v" (vdtemp1),    /* Any vector register */
            "=r" (temp1),
            "=m" (out_doubles[iter % ARRAY_SIZE])
            
            :
            "x" (vtemp1),      /* Keep value in SSE reg */
            "r" (live6),
            "r" (live7),
            "m" (in_doubles[iter % ARRAY_SIZE]),
            "i" (3),
            "r" (live8)
            
            :
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
            "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
            "memory"
        );
        
        /* Unrolled computation to increase register pressure */
        for (int j = 0; j < UNROLL_FACTOR; j++) {
            int idx = (iter * UNROLL_FACTOR + j) % ARRAY_SIZE;
            
            __asm__ volatile (
                "=r" (out_ints[idx]),
                "=r" (live9),
                "=r" (live10)
                
                :
                "r" (in_ints[idx]),
                "r" (live9),
                "r" (live10),
                "i" (j),
                "m" (in_doubles[idx])
                
                :
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                "r8", "r9", "r10", "r11", "memory"
            );
        }
        
        /* Conditional asm to affect control flow */
        if (iter % 3 == 0) {
            __asm__ volatile (
                "=r" (temp1),
                "=r" (temp2)
                :
                "r" (live1),
                "r" (live2),
                "m" (in_ints[(iter + 1) % ARRAY_SIZE])
                :
                "rax", "rbx", "rcx", "memory"
            );
            live6 = temp1;
            live7 = temp2;
        } else if (iter % 3 == 1) {
            __asm__ volatile (
                "=a" (temp4),
                "=d" (temp5)
                :
                "a" (live3),
                "d" (live4),
                "m" (in_ints[(iter + 2) % ARRAY_SIZE])
                :
                "memory"
            );
            live8 = temp4;
            live9 = temp5;
        }
    }
}

/* Secondary reload patterns */
void test_secondary_reloads(int iterations, int* in_ints, double* in_doubles,
                           int* out_ints, double* out_doubles) {
    volatile int temp;
    volatile double dtemp;
    
    /* Force secondary reloads by using specific register constraints
       that may not be directly allocatable */
    for (int iter = 0; iter < iterations; iter++) {
        int idx = iter % ARRAY_SIZE;
        
        /* Constraint requiring accumulator for input, different for output */
        __asm__ volatile (
            "=b" (temp)        /* Must be in base register */
            :
            "a" (in_ints[idx]), /* Must be in accumulator */
            "m" (in_doubles[idx]),
            "i" (0x12345678)
            :
            "rcx", "rdx", "memory"
        );
        out_ints[idx] = temp;
        
        /* Force move between register classes */
        __asm__ volatile (
            "=R" (temp)        /* Legacy register (ax,bx,cx,dx,si,di,bp,sp) */
            :
            "r" (in_ints[(idx + 1) % ARRAY_SIZE]), /* Could be R8-R15 */
            "m" (in_ints[(idx + 2) % ARRAY_SIZE])
            :
            "memory"
        );
        
        /* "rm" constraint that may need secondary reload if in memory */
        __asm__ volatile (
            "=rm" (out_ints[(idx + 3) % ARRAY_SIZE])
            :
            "rm" (in_ints[idx]),  /* May need secondary reload */
            "rm" (in_ints[(idx + 1) % ARRAY_SIZE]),
            "rm" (in_ints[(idx + 2) % ARRAY_SIZE])
            :
            "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* FP stack manipulation requiring specific stack registers */
        __asm__ volatile (
            "=t" (dtemp)       /* Top of FP stack */
            :
            "u" (in_doubles[idx]), /* Second FP stack register */
            "m" (in_doubles[(idx + 1) % ARRAY_SIZE])
            :
            "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)",
            "memory"
        );
        out_doubles[idx] = dtemp;
    }
}

/* Optional and non-combine reloads */
void test_optional_reloads(int iterations, int* in_ints, double* in_doubles,
                          int* out_ints, double* out_doubles) {
    volatile int temp1, temp2, temp3;
    volatile double dtemp1, dtemp2;
    
    for (int iter = 0; iter < iterations; iter++) {
        int idx = iter % ARRAY_SIZE;
        
        /* Optional output constraint */
        __asm__ volatile (
            "=r" (temp1),
            "=?r" (temp2),     /* Optional output */
            "=r" (temp3)
            :
            "r" (in_ints[idx]),
            "r" (in_ints[(idx + 1) % ARRAY_SIZE]),
            "m" (in_doubles[idx])
            :
            "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Memory barrier to prevent combination with next asm */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that could be combined but won't due to barrier */
        __asm__ volatile (
            "=r" (out_ints[idx]),
            "=?r" (temp2),     /* Optional again */
            "=r" (out_ints[(idx + 1) % ARRAY_SIZE])
            :
            "r" (temp1),
            "r" (temp3),
            "m" (in_doubles[(idx + 1) % ARRAY_SIZE])
            :
            "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Different clobber list prevents combination */
        __asm__ volatile (
            "=r" (temp1),
            "=r" (temp2)
            :
            "r" (in_ints[(idx + 2) % ARRAY_SIZE]),
            "m" (in_doubles[(idx + 2) % ARRAY_SIZE])
            :
            "r8", "r9", "r10", "r11", "memory"  /* Different clobbers */
        );
        
        /* Volatile asm with optional inputs */
        __asm__ volatile (
            "=r" (out_ints[(idx + 3) % ARRAY_SIZE])
            :
            "?r" (temp1),      /* Optional input */
            "r" (temp2),
            "?m" (in_doubles[(idx + 3) % ARRAY_SIZE]) /* Optional memory */
            :
            "memory"
        );
        
        /* Conditional optional outputs */
        if (iter % 2 == 0) {
            __asm__ volatile (
                "=r" (temp1),
                "=?r" (temp2)  /* Optional based on control flow */
                :
                "r" (in_ints[idx]),
                "m" (in_doubles[idx])
                :
                "memory"
            );
        } else {
            __asm__ volatile (
                "=r" (temp1)
                :
                "r" (in_ints[idx]),
                "m" (in_doubles[idx])
                :
                "memory"
            );
            temp2 = 0;
        }
    }
}

int main(int argc, char** argv) {
    int iterations = 100;
    int mode = 0;
    
    /* Parse command line */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        mode = atoi(argv[2]);
    }
    
    /* Allocate and initialize arrays */
    int* in_ints = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* out_ints = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double* in_doubles = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double* out_doubles = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    if (!in_ints || !out_ints || !in_doubles || !out_doubles) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with mixed patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        in_ints[i] = i * 3 + 7;
        out_ints[i] = 0;
        in_doubles[i] = i * 0.5 + 1.25;
        out_doubles[i] = 0.0;
    }
    
    /* Additional arrays for register pressure */
    int* pressure_a = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* pressure_b = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* pressure_c = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* pressure_d = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    if (pressure_a && pressure_b && pressure_c && pressure_d) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            pressure_a[i] = i;
            pressure_b[i] = i * 2;
            pressure_c[i] = i * 3;
            pressure_d[i] = i * 4;
        }
    }
    
    printf("Running reload tests with %d iterations, mode %d\n", iterations, mode);
    
    /* Run tests based on mode */
    switch (mode % 3) {
        case 0:
            test_primary_reloads(iterations, in_ints, in_doubles, out_ints, out_doubles);
            if (pressure_a) create_register_pressure(pressure_a, pressure_b, pressure_c, pressure_d, ARRAY_SIZE/4);
            test_secondary_reloads(iterations/2, in_ints, in_doubles, out_ints, out_doubles);
            if (pressure_a) create_register_pressure(pressure_b, pressure_c, pressure_d, pressure_a, ARRAY_SIZE/4);
            test_optional_reloads(iterations/3, in_ints, in_doubles, out_ints, out_doubles);
            break;
            
        case 1:
            test_secondary_reloads(iterations, in_ints, in_doubles, out_ints, out_doubles);
            if (pressure_a) create_register_pressure(pressure_c, pressure_d, pressure_a, pressure_b, ARRAY_SIZE/4);
            test_optional_reloads(iterations, in_ints, in_doubles, out_ints, out_doubles);
            break;
            
        case 2:
            test_primary_reloads(iterations, in_ints, in_doubles, out_ints, out_doubles);
            if (pressure_a) create_register_pressure(pressure_d, pressure_a, pressure_b, pressure_c, ARRAY_SIZE/4);
            test_optional_reloads(iterations, in_ints, in_doubles, out_ints, out_doubles);
            break;
    }
    
    /* Compute checksum to ensure all asm executed */
    uint64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += out_ints[i];
        checksum += *(uint64_t*)&out_doubles[i];
    }
    
    if (pressure_a) {
        for (int i = 0; i < ARRAY_SIZE; i += 4) {
            checksum += pressure_a[i] + pressure_b[i] + pressure_c[i] + pressure_d[i];
        }
    }
    
    printf("Checksum: 0x%016lx\n", checksum);
    
    /* Cleanup */
    free(in_ints);
    free(out_ints);
    free(in_doubles);
    free(out_doubles);
    if (pressure_a) free(pressure_a);
    if (pressure_b) free(pressure_b);
    if (pressure_c) free(pressure_c);
    if (pressure_d) free(pressure_d);
    
    return 0;
}
