/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Global arrays to create register pressure */
static int int_array[ARRAY_SIZE];
static double double_array[ARRAY_SIZE];
static float float_array[ARRAY_SIZE];
static long long ll_array[ARRAY_SIZE];
static __m128i vec128_array[ARRAY_SIZE/4];
static __m256d vec256_array[ARRAY_SIZE/8];

/* Test function 1: Primary reloads with diverse constraints */
void test_primary_reloads(int iterations, int mode) {
    volatile int a, b, c, d, e, f, g, h;
    volatile long long la, lb, lc, ld;
    volatile double da, db, dc, dd;
    volatile float fa, fb, fc, fd;
    
    /* Create many live variables to exhaust registers */
    a = int_array[0];
    b = int_array[1];
    c = int_array[2];
    d = int_array[3];
    e = int_array[4];
    f = int_array[5];
    g = int_array[6];
    h = int_array[7];
    
    la = ll_array[0];
    lb = ll_array[1];
    lc = ll_array[2];
    ld = ll_array[3];
    
    da = double_array[0];
    db = double_array[1];
    dc = double_array[2];
    dd = double_array[3];
    
    fa = float_array[0];
    fb = float_array[1];
    fc = float_array[2];
    fd = float_array[3];
    
    for (int i = 0; i < iterations; i++) {
        /* Complex asm with 8 operands mixing constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (a),     /* general register */
            "=&r" (b),    /* earlyclobber */
            "=q" (c),     /* byte register (a,b,c,d) */
            "=a" (d),     /* accumulator */
            "=d" (e),     /* data register */
            "=t" (fa),    /* top of FP stack */
            "=m" (int_array[i % 8]), /* memory */
            "=r" (g)      /* general register */
            
            /* Inputs with mixed constraints */
            : "0" (a),    /* matching constraint with output 0 */
            "i" (12345),  /* immediate */
            "r" (b),
            "m" (int_array[(i + 1) % 8]), /* memory */
            "r" (c),
            "a" (d),
            "d" (e),
            "g" (f)       /* general or memory */
            
            /* Clobbers to force register allocation */
            : "cc", "memory", "r8", "r9", "r10", "r11", "xmm0", "xmm1"
        );
        
        /* Another asm with vector constraints */
        __m128i v1, v2;
        v1 = vec128_array[i % (ARRAY_SIZE/4)];
        
        __asm__ volatile (
            "movdqa %1, %0\n\t"
            "psllw $3, %0\n\t"
            "paddw %0, %0"
            : "=x" (v2)           /* xmm register */
            : "xm" (v1)           /* xmm or memory */
            : "xmm2", "xmm3"
        );
        
        vec128_array[i % (ARRAY_SIZE/4)] = v2;
        
        /* Mix scalar and vector operations to increase pressure */
        if (mode & 1) {
            __asm__ volatile (
                "imull %1, %0\n\t"
                "addl %2, %0"
                : "+r" (h)
                : "r" (g), "i" (256)
                : "cc"
            );
        }
    }
    
    /* Store results back */
    int_array[0] = a + b + c + d;
    float_array[0] = fa;
}

/* Test function 2: Secondary reload patterns */
void test_secondary_reloads(int iterations) {
    volatile int a, b, c;
    volatile long long la, lb;
    
    for (int i = 0; i < iterations; i++) {
        /* Force secondary reload by using 'a' constraint with memory operand */
        a = int_array[i % 16];
        b = int_array[(i + 1) % 16];
        
        __asm__ volatile (
            /* This may need secondary reload if 'a' gets memory */
            "movl %1, %%eax\n\t"
            "addl %%eax, %0"
            : "+a" (a)            /* accumulator constraint */
            : "rm" (b)            /* register or memory - may force secondary reload */
            : "cc"
        );
        
        /* Use legacy register constraints with modern registers */
        la = ll_array[i % 8];
        
        __asm__ volatile (
            "movq %1, %%rax\n\t"
            "shrq $4, %%rax\n\t"
            "movq %%rax, %0"
            : "=R" (lb)           /* legacy register (ax,bx,cx,dx,si,di,bp,sp) */
            : "r" (la)            /* any register - may need move if allocated to r8-r15 */
            : "rax", "cc"
        );
        
        ll_array[i % 8] = lb;
        
        /* Complex constraint chain */
        c = int_array[(i + 2) % 16];
        
        __asm__ volatile (
            "movl %1, %%ebx\n\t"
            "leal (%%ebx,%%ebx,2), %0"
            : "=r" (c)
            : "b" (a)             /* must be in ebx */
            : "cc"
        );
        
        int_array[(i + 2) % 16] = c;
    }
}

/* Test function 3: Optional and non-combine reloads */
void test_optional_reloads(int iterations) {
    volatile int a, b, c, d;
    volatile int results[4];
    
    for (int i = 0; i < iterations; i++) {
        a = i * 3;
        b = i * 5;
        c = i * 7;
        d = i * 11;
        
        /* Optional output constraint */
        __asm__ volatile (
            "movl %2, %0\n\t"
            "testl %3, %3\n\t"
            "cmovnel %4, %0"
            : "=?r" (results[0])   /* optional output */
            : "0" (a),             /* matching constraint */
              "r" (b),
              "r" (c),
              "rm" (d)
            : "cc"
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that won't combine due to barrier */
        __asm__ volatile (
            "movl %1, %0\n\t"
            "addl $1, %0"
            : "=r" (results[1])
            : "r" (results[0])
            : "cc"
        );
        
        /* Another barrier */
        __asm__ volatile ("" ::: "memory");
        
        /* Different clobber list prevents combination */
        __asm__ volatile (
            "movl %1, %0\n\t"
            "imull $3, %0"
            : "=r" (results[2])
            : "r" (results[1])
            : "cc", "rax"  /* Different clobber than similar asm below */
        );
        
        /* Similar operation but with different clobber */
        __asm__ volatile (
            "movl %1, %0\n\t"
            "imull $3, %0"
            : "=r" (results[3])
            : "r" (results[2])
            : "cc", "rbx"  /* Different register clobber */
        );
        
        /* Store with complex addressing mode */
        int_array[i % 16] = results[0] + results[1] + results[2] + results[3];
    }
}

/* Test function 4: Control flow dependent reloads */
void test_control_flow_reloads(int iterations, int threshold) {
    volatile int a = 0, b = 0, c = 0, d = 0;
    volatile int x = 0, y = 0, z = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Different asm blocks in different branches */
        if (i < threshold) {
            __asm__ volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                "movl %2, %%ebx\n\t"
                "subl %%ebx, %0"
                : "+r" (a)
                : "r" (b), "r" (c)
                : "rax", "rbx", "cc"
            );
            
            x = int_array[i % 8];
            __asm__ volatile (
                "imull %1, %0"
                : "+r" (x)
                : "r" (a)
                : "cc"
            );
            int_array[i % 8] = x;
        } else {
            __asm__ volatile (
                "movl %1, %%ecx\n\t"
                "xorl %%ecx, %0\n\t"
                "movl %2, %%edx\n\t"
                "orl %%edx, %0"
                : "+r" (d)
                : "r" (b), "r" (c)
                : "rcx", "rdx", "cc"
            );
            
            y = int_array[(i + 4) % 8];
            __asm__ volatile (
                "shll $2, %0"
                : "+r" (y)
                : 
                : "cc"
            );
            int_array[(i + 4) % 8] = y;
        }
        
        /* Loop-carried dependency */
        z = (i & 1) ? a : d;
        __asm__ volatile (
            "leal (%0,%0,4), %0"
            : "+r" (z)
            : 
            : "cc"
        );
        
        b = c;
        c = z;
    }
}

/* Test function 5: Maximum register pressure with unrolling */
void test_max_pressure(int iterations) {
    /* Declare many variables to exhaust registers */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7;
    volatile int v8, v9, v10, v11, v12, v13, v14, v15;
    volatile double d0, d1, d2, d3, d4, d5, d6, d7;
    volatile __m128i xmm0, xmm1, xmm2, xmm3;
    volatile __m256d ymm0, ymm1;
    
    /* Initialize from arrays */
    v0 = int_array[0]; v1 = int_array[1]; v2 = int_array[2]; v3 = int_array[3];
    v4 = int_array[4]; v5 = int_array[5]; v6 = int_array[6]; v7 = int_array[7];
    v8 = int_array[8]; v9 = int_array[9]; v10 = int_array[10]; v11 = int_array[11];
    v12 = int_array[12]; v13 = int_array[13]; v14 = int_array[14]; v15 = int_array[15];
    
    d0 = double_array[0]; d1 = double_array[1]; d2 = double_array[2]; d3 = double_array[3];
    d4 = double_array[4]; d5 = double_array[5]; d6 = double_array[6]; d7 = double_array[7];
    
    xmm0 = vec128_array[0]; xmm1 = vec128_array[1];
    xmm2 = vec128_array[2]; xmm3 = vec128_array[3];
    
    ymm0 = vec256_array[0]; ymm1 = vec256_array[1];
    
    for (int i = 0; i < iterations; i++) {
        /* Unrolled computation with many asm statements */
        for (int j = 0; j < UNROLL_FACTOR; j++) {
            /* Mix scalar and vector operations */
            __asm__ volatile (
                "paddd %1, %0\n\t"
                "pslld $1, %0"
                : "+x" (xmm0)
                : "xm" (xmm1)
                : "xmm4", "xmm5"
            );
            
            __asm__ volatile (
                "vaddpd %1, %0, %0\n\t"
                "vmulpd %2, %0, %0"
                : "+x" (ymm0)
                : "xm" (ymm1), "xm" (vec256_array[j % 2])
                : "ymm2", "ymm3"
            );
            
            /* Scalar operations keeping many values live */
            __asm__ volatile (
                "movl %1, %%eax\n\t"
                "imull %%eax, %0\n\t"
                "addl %2, %0"
                : "+r" (v0)
                : "r" (v1), "r" (v2)
                : "rax", "cc"
            );
            
            __asm__ volatile (
                "movl %1, %%ebx\n\t"
                "xorl %%ebx, %0"
                : "+r" (v3)
                : "r" (v4)
                : "rbx", "cc"
            );
            
            /* Chain dependencies */
            v5 = v0 + v3;
            v6 = v1 + v4;
            v7 = v2 + v5;
            v8 = v3 + v6;
            
            /* Floating point with stack constraints */
            __asm__ volatile (
                "fldl %1\n\t"
                "fadd %%st(0), %%st(0)\n\t"
                "fstpl %0"
                : "=m" (d0)
                : "m" (d1)
                : "st", "st(1)", "st(2)"
            );
        }
        
        /* Rotate values to create varying patterns */
        int temp = v0;
        v0 = v1; v1 = v2; v2 = v3; v3 = v4;
        v4 = v5; v5 = v6; v6 = v7; v7 = v8;
        v8 = v9; v9 = v10; v10 = v11; v11 = v12;
        v12 = v13; v13 = v14; v14 = v15; v15 = temp;
    }
    
    /* Store back some results */
    int_array[0] = v0; int_array[1] = v1;
    double_array[0] = d0;
    vec128_array[0] = xmm0;
    vec256_array[0] = ymm0;
}

/* Main function with command line arguments */
int main(int argc, char *argv[]) {
    int iterations = 100;
    int mode = 2;
    
    /* Parse command line */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    if (iterations <= 0) iterations = 100;
    if (iterations > 10000) iterations = 10000;
    
    printf("Running reload tests with %d iterations, mode %d\n", iterations, mode);
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        double_array[i] = i * 0.5;
        float_array[i] = i * 0.25f;
        ll_array[i] = i * 7LL;
    }
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        vec128_array[i] = _mm_set_epi32(i*4+3, i*4+2, i*4+1, i*4);
    }
    
    for (int i = 0; i < ARRAY_SIZE/8; i++) {
        vec256_array[i] = _mm256_set_pd(i*8+3, i*8+2, i*8+1, i*8);
    }
    
    /* Run all test functions */
    test_primary_reloads(iterations, mode);
    test_secondary_reloads(iterations / 2);
    test_optional_reloads(iterations / 4);
    test_control_flow_reloads(iterations, iterations / 3);
    test_max_pressure(iterations / 10);
    
    /* Compute checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_array[i];
        checksum += (unsigned long long)(double_array[i] * 1000);
        checksum += (unsigned long long)(float_array[i] * 1000);
        checksum += ll_array[i];
    }
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        int vals[4];
        _mm_storeu_si128((__m128i*)vals, vec128_array[i]);
        checksum += vals[0] + vals[1] + vals[2] + vals[3];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
