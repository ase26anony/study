/* reload_stress.c - Stress GCC's reload pass to cover rld initialization block */

/* Prevent optimizations that could eliminate reloads */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Opaque function to prevent register reuse */
extern int barrier(int x);
int barrier(int x) { return x ^ 0x55AA55AA; }

/* Complex structure to force non-trivial addressing */
struct Nested {
    int a[3];
    long b[2];
    struct {
        short s1, s2;
        int i;
    } inner;
    volatile int v;
};

/* Multi-dimensional array with volatile indices */
volatile int idx1 = 1, idx2 = 2, idx3 = 3;

/* Register variables to force specific register allocation */
register int reg_var1 asm ("r12");
register int reg_var2 asm ("r13");

/* Test function with high register pressure and complex addressing */
NOINLINE static long test_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    long l1, long l2, long l3, long l4, long l5,
    float f1, float f2, double d1, double d2)
{
    /* Local variables - many to exhaust registers */
    int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    int c1, c2, c3, c4, c5;
    long l6, l7, l8, l9, l10;
    float f3, f4, f5;
    double d3, d4;
    
    /* Complex array with SIB-like addressing potential */
    int big_array[100][10];
    struct Nested nested[5];
    
    /* Initialize with arithmetic to prevent constant propagation */
    b1 = barrier(a1) + 1;
    b2 = barrier(a2) + 2;
    b3 = barrier(a3) + 3;
    b4 = barrier(a4) + 4;
    b5 = barrier(a5) + 5;
    b6 = barrier(a6) + 6;
    b7 = barrier(a7) + 7;
    b8 = barrier(a8) + 8;
    b9 = barrier(a9) + 9;
    b10 = barrier(a10) + 10;
    
    c1 = b1 * b2;
    c2 = b3 * b4;
    c3 = b5 * b6;
    c4 = b7 * b8;
    c5 = b9 * b10;
    
    l6 = l1 * l2 + barrier((int)l1);
    l7 = l3 * l4 + barrier((int)l2);
    l8 = l5 * l6 + barrier((int)l3);
    l9 = l7 * l8 + barrier((int)l4);
    l10 = l9 * l1 + barrier((int)l5);
    
    /* Mixed floating-point operations */
    f3 = f1 * f2 + (float)b1;
    f4 = f3 * 3.14159f + (float)b2;
    f5 = f4 / 2.71828f + (float)b3;
    
    d3 = d1 * d2 + (double)l1;
    d4 = d3 / 1.41421356 + (double)l2;
    
    /* Complex addressing with volatile indices - forces SIB/reloads */
    /* This should trigger secondary reloads on many architectures */
    for (int i = 0; i < 5; i++) {
        /* Multi-dimensional array access with scaling */
        big_array[i * idx1][i + idx2] = 
            barrier(big_array[(i + idx3) * 2][i * 3]) + i;
        
        /* Nested structure access with complex addressing */
        nested[i].a[(i + idx1) % 3] = barrier(nested[i].inner.i);
        nested[i].b[i % 2] = l6 + i * l7;
        nested[i].inner.s1 = (short)(b1 + i);
        nested[i].inner.s2 = (short)(b2 + i);
        nested[i].inner.i = c1 + i * c2;
        nested[i].v = barrier(i);  /* volatile access */
    }
    
    /* Inline assembly that clobbers many registers */
    /* This forces the compiler to spill/reload around it */
    asm volatile (
        "# Complex inline assembly\n"
        "mov %[val1], %[tmp1]\n\t"
        "mov %[val2], %[tmp2]\n\t"
        "add %[tmp1], %[tmp2], %[tmp3]\n\t"
        : [tmp3] "=r" (c1)
        : [val1] "m" (big_array[idx1][idx2]),  /* Memory constraint */
          [val2] "r" (b1),                      /* Register constraint */
          [tmp1] "r" (reg_var1),                /* Specific register */
          [tmp2] "r" (reg_var2)                 /* Specific register */
        : "r0", "r1", "r2", "r3", "r4", "r5",   /* Clobber many regs */
          "r6", "r7", "r8", "r9", "r10", "r11",
          "memory", "cc"
    );
    
    /* More complex addressing with pointer arithmetic */
    int *ptr1 = &big_array[idx1][0];
    int *ptr2 = &big_array[idx2][0];
    int *ptr3 = &big_array[idx3][0];
    
    /* Force register pressure with many live values */
    for (int i = 0; i < 10; i++) {
        /* Complex addressing with scaling */
        ptr1[i * 4] = barrier(ptr2[i * 3] + ptr3[i * 2]);
        
        /* Atomic operations to prevent optimizations */
        __atomic_store_n(&nested[i % 5].v, 
                        barrier(ptr1[i * 4]), 
                        __ATOMIC_RELAXED);
    }
    
    /* Type punning between int and float - forces moves between reg classes */
    union {
        int i;
        float f;
    } pun;
    
    pun.f = f3;
    c2 = barrier(pun.i);  /* Integer view of float */
    
    pun.i = barrier(c1);
    f4 = pun.f;           /* Float view of integer */
    
    /* Vector-like operations using GCC extensions */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec1 = {b1, b2, b3, b4};
    v4si vec2 = {b5, b6, b7, b8};
    v4si vec3 = vec1 + vec2;
    
    /* Extract elements - forces scalar/vector register moves */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += vec3[i];  /* Element extraction */
    }
    
    /* Final computation using all variables to keep them live */
    long result = 
        (long)b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
        c1 + c2 + c3 + c4 + c5 +
        l6 + l7 + l8 + l9 + l10 +
        (long)f3 + (long)f4 + (long)f5 +
        (long)d3 + (long)d4 +
        sum + reg_var1 + reg_var2;
    
    return barrier((int)result) + result;
}

/* Main function that creates register pressure */
int main(int argc, char *argv[]) {
    /* Many live variables to exhaust registers */
    int v1 = barrier(argc) + 1;
    int v2 = barrier(v1) + 2;
    int v3 = barrier(v2) + 3;
    int v4 = barrier(v3) + 4;
    int v5 = barrier(v4) + 5;
    int v6 = barrier(v5) + 6;
    int v7 = barrier(v6) + 7;
    int v8 = barrier(v7) + 8;
    int v9 = barrier(v8) + 9;
    int v10 = barrier(v9) + 10;
    int v11 = barrier(v10) + 11;
    int v12 = barrier(v11) + 12;
    int v13 = barrier(v12) + 13;
    int v14 = barrier(v13) + 14;
    int v15 = barrier(v14) + 15;
    int v16 = barrier(v15) + 16;
    int v17 = barrier(v16) + 17;
    int v18 = barrier(v17) + 18;
    int v19 = barrier(v18) + 19;
    int v20 = barrier(v19) + 20;
    
    long l1 = barrier(v1) * 100L;
    long l2 = barrier(v2) * 200L;
    long l3 = barrier(v3) * 300L;
    long l4 = barrier(v4) * 400L;
    long l5 = barrier(v5) * 500L;
    
    float f1 = (float)barrier(v6) / 100.0f;
    float f2 = (float)barrier(v7) / 200.0f;
    
    double d1 = (double)barrier(v8) / 100.0;
    double d2 = (double)barrier(v9) / 200.0;
    
    /* Initialize register variables */
    reg_var1 = barrier(v10);
    reg_var2 = barrier(v11);
    
    /* Call test function with many arguments */
    long result = test_function(
        v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,
        l1, l2, l3, l4, l5,
        f1, f2, d1, d2
    );
    
    /* Use all variables to prevent dead code elimination */
    result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    /* Print result to ensure side effects are observable */
    printf("Result: %ld\n", result);
    
    return (int)(result & 0x7FFFFFFF);
}
