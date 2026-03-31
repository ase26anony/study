/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper functions to split basic blocks */
__attribute__((noinline, noclone))
static int helper1(int a, int b, int c) {
    if (a > b) return a * c - b;
    return b * c + a;
}

__attribute__((noinline, noclone))
static float helper2(float a, float b, float c) {
    if (a < b) return a * b + c;
    return b * c - a;
}

__attribute__((noinline, noclone))
static v4si helper3(v4si a, v4si b) {
    v4si mask = {1, 2, 3, 4};
    return a * b + mask;
}

/* Main test function implementing all requirements */
__attribute__((noinline, noclone))
static volatile int test_remat(volatile int input1, volatile long input2,
                               volatile float input3, volatile double input4) {
    /* Requirement 1: Many local variables of mixed types */
    int a = input1 + 1;
    int b = input1 * 2;
    int c = input1 - 3;
    int d = input1 / 4;
    int e = input1 % 5;
    int f = input1 << 2;
    int g = input1 >> 1;
    int h = ~input1;
    int i = input1 | 0xFF;
    int j = input1 & 0x0F;
    
    long la = input2 + 1000;
    long lb = input2 * 2;
    long lc = input2 - 500;
    long ld = input2 / 3;
    long le = input2 % 7;
    
    float fa = input3 + 1.5f;
    float fb = input3 * 2.0f;
    float fc = input3 - 0.5f;
    float fd = input3 / 3.0f;
    
    double da = input4 + 2.5;
    double db = input4 * 1.5;
    double dc = input4 - 1.0;
    double dd = input4 / 2.0;
    
    /* Additional variables for more pressure */
    int m, n, o, p, q, r, s, t, u, v, w, x, y, z;
    
    /* Vector variables (Requirement 4) */
    v4si vec1 = {a, b, c, d};
    v4si vec2 = {e, f, g, h};
    v4sf vecf1 = {fa, fb, fc, fd};
    v4sf vecf2 = {fc, fd, fa, fb};
    v2df vecd1 = {da, db};
    v2df vecd2 = {dc, dd, da, db}; /* Last two elements ignored for v2df */
    
    /* Requirement 2: Long serial chain of interdependent operations */
    /* First computation chain */
    int t1 = a + b;
    int t2 = t1 * c;
    int t3 = t2 - d;
    int t4 = t3 ^ e;
    int t5 = t4 | f;
    int t6 = t5 & g;
    int t7 = t6 << 2;
    int t8 = t7 >> 1;
    
    /* Mix with long operations */
    long lt1 = la + lb;
    long lt2 = lt1 * lc;
    long lt3 = lt2 - ld;
    long lt4 = lt3 ^ le;
    
    /* Mix with float operations */
    float ft1 = fa + fb;
    float ft2 = ft1 * fc;
    float ft3 = ft2 - fd;
    float ft4 = ft3 / fa;
    
    /* Mix with double operations */
    double dt1 = da + db;
    double dt2 = dt1 * dc;
    double dt3 = dt2 - dd;
    double dt4 = dt3 / da;
    
    /* Vector operations */
    v4si vt1 = vec1 + vec2;
    v4si vt2 = vt1 * vec1;
    v4si vt3 = vt2 - vec2;
    
    v4sf vft1 = vecf1 + vecf2;
    v4sf vft2 = vft1 * vecf1;
    v4sf vft3 = vft2 - vecf2;
    
    /* Requirement 5: Control flow splitting */
    if (t1 > 100) {
        m = helper1(t1, t2, t3);
        n = helper1(t4, t5, t6);
        
        /* More computations in this branch */
        o = m * n + t7;
        p = (m ^ n) | t8;
        
        /* Vector operations in branch */
        vt1 = helper3(vt1, vt2);
    } else {
        m = helper1(t2, t3, t4);
        n = helper1(t5, t6, t7);
        
        o = m / n + t8;
        p = (m & n) ^ t1;
        
        vt2 = helper3(vt2, vt3);
    }
    
    /* Switch statement for more control flow */
    switch (t1 & 0x3) {
        case 0:
            q = t2 + t3 * t4;
            ft1 = helper2(ft1, ft2, ft3);
            break;
        case 1:
            q = t3 - t4 / t5;
            ft2 = helper2(ft2, ft3, ft4);
            break;
        case 2:
            q = t4 ^ t5 | t6;
            ft3 = helper2(ft3, ft4, ft1);
            break;
        default:
            q = t5 & t6 << t7;
            ft4 = helper2(ft4, ft1, ft2);
            break;
    }
    
    /* Requirement 3: Inline assembly to clobber registers */
    /* For x86_64 */
    asm volatile (
        "# Clobber many registers\n\t"
        "nop"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* For ARM/AArch64 (commented out, choose based on target)
    asm volatile (
        "# Clobber many registers\n\t"
        "nop"
        : 
        : 
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
          "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
          "x16", "x17", "x18", "x19", "x20", "x21", "x22",
          "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30",
          "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
          "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
          "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
          "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",
          "memory"
    );
    */
    
    /* Requirement 2 continued: Recomputation in slightly different form */
    /* Recomputation of earlier values with variations */
    int t1_prime = a - b;  /* Different from t1 = a + b */
    int t2_prime = t1_prime / c;  /* Different from t2 = t1 * c */
    int t3_prime = t2_prime + d;  /* Different from t3 = t2 - d */
    
    long lt1_prime = la - lb;
    long lt2_prime = lt1_prime / lc;
    
    float ft1_prime = fa - fb;
    float ft2_prime = ft1_prime / fc;
    
    /* Use recomputed values */
    r = t1_prime * t2_prime + t3_prime;
    s = lt1_prime ^ lt2_prime;
    
    /* More vector operations after assembly clobber */
    v4si vt4 = vt1 * vt2 + vt3;
    v4sf vft4 = vft1 * vft2 - vft3;
    
    /* Final complex expression using many variables */
    int result = (t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 +
                  m + n + o + p + q + r + s +
                  (int)lt1 + (int)lt2 + (int)lt3 + (int)lt4 +
                  (int)ft1 + (int)ft2 + (int)ft3 + (int)ft4 +
                  (int)dt1 + (int)dt2 + (int)dt3 + (int)dt4 +
                  vt1[0] + vt1[1] + vt1[2] + vt1[3] +
                  vt2[0] + vt2[1] + vt2[2] + vt2[3] +
                  vt3[0] + vt3[1] + vt3[2] + vt3[3] +
                  vt4[0] + vt4[1] + vt4[2] + vt4[3] +
                  (int)vft1[0] + (int)vft1[1] + (int)vft1[2] + (int)vft1[3] +
                  (int)vft2[0] + (int)vft2[1] + (int)vft2[2] + (int)vft2[3] +
                  (int)vft3[0] + (int)vft3[1] + (int)vft3[2] + (int)vft3[3] +
                  (int)vft4[0] + (int)vft4[1] + (int)vft4[2] + (int)vft4[3]);
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
    }
    
    volatile long total = 0;
    
    /* Requirement 2: Loop with volatile trip count */
    for (volatile int i = 0; i < loop_count; i++) {
        /* Use volatile inputs to prevent constant propagation */
        volatile int input1 = i * 3 + 1;
        volatile long input2 = i * 5L + 2;
        volatile float input3 = i * 1.5f + 3.0f;
        volatile double input4 = i * 2.5 + 4.0;
        
        /* Call test function in loop */
        volatile int result = test_remat(input1, input2, input3, input4);
        
        /* Accumulate to prevent optimization */
        total += result;
        
        /* Occasionally add more register pressure */
        if (i % 100 == 0) {
            /* Extra computation to vary patterns */
            total += test_remat(input1 + 1, input2 - 1, 
                               input3 * 1.1f, input4 / 1.1);
        }
    }
    
    printf("Final result: %ld\n", (long)total);
    return (int)(total % 1000);
}
