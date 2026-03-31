/* main.c - Primary test file with hot loop and register pressure */
#include <stdint.h>
#include <stdio.h>

/* Force no optimization of helper functions */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile int

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* External helper functions from second compilation unit */
NOINLINE struct LargeStruct helper_func1(int a, int b, float c, double d);
NOINLINE struct LargeStruct helper_func2(v4si vec, v4sf fvec, v2df dvec);
NOINLINE int helper_func3(struct LargeStruct s1, struct LargeStruct s2);

/* Struct to increase register/stack pressure */
struct LargeStruct {
    int a, b, c, d;
    float e, f;
    double g, h;
    v4si vec1;
    v4sf vec2;
};

/* Volatile to prevent dead code elimination */
VOLATILE_VAR loop_counter = 1000;

/* Main test function with extreme register pressure */
NOINLINE long test_function(int seed) {
    /* Declare many variables of different types */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    float fa, fb, fc, fd, fe, ff, fg, fh, fi, fj;
    double da, db, dc, dd, de, df, dg, dh, di, dj;
    long la, lb, lc, ld, le, lf, lg, lh, li, lj;
    v4si va, vb, vc, vd, ve;
    v4sf vfa, vfb, vfc, vfd, vfe;
    v2df vda, vdb, vdc, vdd, vde;
    
    /* Initialize with seed to prevent constant propagation */
    a = seed;
    b = a + 123;
    c = b * 456;
    d = c - 789;
    e = d / (seed + 1);
    f = e ^ 0xABCDEF;
    g = f << 3;
    h = g >> 2;
    i = h | 0xFF;
    j = i & 0xAA;
    
    /* Create complex dependency chain */
    k = a + b - c * d / (e + 1);
    l = k * k - j * j;
    m = (l << 4) | (k >> 4);
    n = m ^ j ^ i ^ h;
    o = n * 3 + m * 5 + l * 7;
    p = o % 997;
    q = p * p - o * o + n * n;
    r = q / (p + 1) + r;
    s = (signed char)r * (unsigned short)q;
    t = s * t - r * q + p * o;
    
    /* Float operations */
    fa = a * 1.5f;
    fb = b * 2.5f;
    fc = fa + fb - c * 0.5f;
    fd = fc * fc - fb * fb;
    fe = fd / (fc + 0.001f);
    ff = fe * 3.14159f;
    fg = ff - fe + fd - fc;
    fh = fg * fg;
    fi = sqrtf(fh + 1.0f);  /* Will be optimized to builtin */
    fj = fi * 2.0f - fg;
    
    /* Double operations */
    da = a * 1.5;
    db = b * 2.5;
    dc = da + db - c * 0.5;
    dd = dc * dc - db * db;
    de = dd / (dc + 0.001);
    df = de * 3.14159265358979;
    dg = df - de + dd - dc;
    dh = dg * dg;
    di = sqrt(dh + 1.0);  /* Will be optimized to builtin */
    dj = di * 2.0 - dg;
    
    /* Long operations */
    la = (long)a * b;
    lb = (long)c * d;
    lc = la + lb - e * f;
    ld = lc * lc - lb * lb;
    le = ld / (lc + 1);
    lf = le * 1000000007L;
    lg = lf - le + ld - lc;
    lh = lg * lg;
    li = lh / (lg + 1);
    lj = li * 2 - lg;
    
    /* Vector operations - these use wide registers */
    va = (v4si){a, b, c, d};
    vb = (v4si){e, f, g, h};
    vc = va + vb;
    vd = va * vb;
    ve = vc - vd;
    
    vfa = (v4sf){fa, fb, fc, fd};
    vfb = (v4sf){fe, ff, fg, fh};
    vfc = vfa + vfb;
    vfd = vfa * vfb;
    vfe = vfc - vfd;
    
    vda = (v2df){da, db};
    vdb = (v2df){dc, dd};
    vdc = vda + vdb;
    vdd = vda * vdb;
    vde = vdc - vdd;
    
    /* Artificial register pressure with inline asm */
    /* Clobber many registers to force spilling */
    asm volatile (
        "# Artificial register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        "add r2, r0, r1\n"
        :
        : "r" (a), "r" (b)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More complex dependency chain with pseudo-register reuse */
    /* This pattern may trigger the specific replacement logic */
    int temp1 = a + b;
    int temp2 = temp1 * c;      /* temp1 used here */
    int temp3 = temp2 - d;      /* temp2 used here */
    int temp4 = temp3 / e;      /* temp3 used here */
    int temp5 = temp4 ^ f;      /* temp4 used here */
    int temp6 = temp5 << g;     /* temp5 used here */
    int temp7 = temp6 >> h;     /* temp6 used here */
    int temp8 = temp7 | i;      /* temp7 used here */
    int temp9 = temp8 & j;      /* temp8 used here */
    int temp10 = temp9 * k;     /* temp9 used here */
    
    /* Even more dependencies */
    temp1 = temp10 + l;         /* temp10 used, temp1 reused */
    temp2 = temp1 * m;          /* temp1 used again */
    temp3 = temp2 - n;          /* temp2 used again */
    temp4 = temp3 / o;          /* temp3 used again */
    temp5 = temp4 ^ p;          /* temp4 used again */
    temp6 = temp5 << q;         /* temp5 used again */
    temp7 = temp6 >> r;         /* temp6 used again */
    temp8 = temp7 | s;          /* temp7 used again */
    temp9 = temp8 & t;          /* temp8 used again */
    temp10 = temp9 * a;         /* temp9 used again */
    
    /* Call helper functions for inter-procedural pressure */
    struct LargeStruct ls1 = helper_func1(a, b, fa, da);
    struct LargeStruct ls2 = helper_func2(va, vfa, vda);
    
    int helper_result = helper_func3(ls1, ls2);
    
    /* Final computation using all variables to keep them live */
    long final_result = 
        (long)temp10 + helper_result +
        (long)va[0] + va[1] + va[2] + va[3] +
        (long)vb[0] + vb[1] + vb[2] + vb[3] +
        (long)vc[0] + vc[1] + vc[2] + vc[3] +
        (long)vd[0] + vd[1] + vd[2] + vd[3] +
        (long)ve[0] + ve[1] + ve[2] + ve[3] +
        (long)(vfa[0] * 1000) + (long)(vfa[1] * 1000) +
        (long)(vfa[2] * 1000) + (long)(vfa[3] * 1000) +
        (long)(vfb[0] * 1000) + (long)(vfb[1] * 1000) +
        (long)(vfb[2] * 1000) + (long)(vfb[3] * 1000) +
        (long)(vda[0] * 1000) + (long)(vda[1] * 1000) +
        (long)(vdb[0] * 1000) + (long)(vdb[1] * 1000) +
        la + lb + lc + ld + le + lf + lg + lh + li + lj;
    
    return final_result;
}

int main() {
    long total = 0;
    VOLATILE_VAR iterations = loop_counter;
    
    /* Hot loop to trigger optimization passes */
    for (int i = 0; i < iterations; i++) {
        total += test_function(i);
        
        /* Prevent loop unrolling from reducing register pressure */
        if (i % 7 == 0) {
            asm volatile ("# Loop barrier" : : : "memory");
        }
    }
    
    printf("Result: %ld\n", total);
    return (int)(total % 1000);
}
