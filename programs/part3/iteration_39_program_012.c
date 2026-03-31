/* Primary test file with dense computation to create register pressure */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* External helper functions from second compilation unit */
extern struct Vec4 add_vec4(struct Vec4 a, struct Vec4 b);
extern struct Vec4 mul_vec4(struct Vec4 a, struct Vec4 b);
extern struct Vec4 cross_vec4(struct Vec4 a, struct Vec4 b);
extern double dot_product(struct Vec4 a, struct Vec4 b);
extern struct Matrix4x4 create_matrix(double a, double b, double c, double d,
                                      double e, double f, double g, double h,
                                      double i, double j, double k, double l,
                                      double m, double n, double o, double p);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

struct Vec4 {
    double x, y, z, w;
};

struct Matrix4x4 {
    double m[4][4];
};

/* Volatile variables to prevent optimization */
volatile int g_iterations = 100;
volatile double g_seed = 3.14159;

/* Noinline function to prevent optimization and create register pressure */
__attribute__((noinline, noipa))
double test_function(double seed) {
    /* Declare many local variables of different types */
    double a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    float fa, fb, fc, fd, fe, ff, fg, fh, fi, fj;
    long la, lb, lc, ld, le, lf, lg, lh;
    v4sf va, vb, vc, vd, ve;
    v4si via, vib, vic, vid;
    
    /* Initialize with seed to create dependencies */
    a = seed * 1.1;
    b = seed * 2.2;
    c = seed * 3.3;
    d = seed * 4.4;
    e = seed * 5.5;
    f = seed * 6.6;
    
    /* Chain of interdependent computations - forces serial evaluation */
    /* Each computation uses previous results, creating many pseudo-registers */
    g = a + b + c;          /* Use a, b, c; produce g */
    h = d * e - f;          /* Use d, e, f; produce h */
    i = g * h / a;          /* Use g, h, a; produce i */
    j = b + c + d + e;      /* Use b, c, d, e; produce j */
    k = i * j - g;          /* Use i, j, g; produce k */
    l = h / i * j;          /* Use h, i, j; produce l */
    m = k + l - a;          /* Use k, l, a; produce m */
    n = b * c * d * e;      /* Use b, c, d, e; produce n */
    o = m / n * f;          /* Use m, n, f; produce o */
    p = g + h + i + j;      /* Use g, h, i, j; produce p */
    q = k * l * m;          /* Use k, l, m; produce q */
    r = n / o * p;          /* Use n, o, p; produce r */
    s = q - r + a;          /* Use q, r, a; produce s */
    t = b * c - d * e + f;  /* Use b, c, d, e, f; produce t */
    
    /* Float computations */
    fa = (float)a * 1.5f;
    fb = (float)b * 2.5f;
    fc = fa + fb;
    fd = (float)c * 3.5f;
    fe = fc * fd;
    ff = (float)d * 4.5f;
    fg = fe / ff;
    fh = (float)e * 5.5f;
    fi = fg + fh;
    fj = (float)f * 6.5f;
    
    /* Long integer computations */
    la = (long)(a * 1000);
    lb = (long)(b * 2000);
    lc = la + lb;
    ld = (long)(c * 3000);
    le = lc * ld;
    lf = (long)(d * 4000);
    lg = le / lf;
    lh = (long)(e * 5000);
    
    /* Vector operations - use wide registers to increase pressure */
    va = (v4sf){fa, fb, fc, fd};
    vb = (v4sf){fe, ff, fg, fh};
    vc = va + vb;
    vd = va * vb;
    ve = vc - vd;
    
    via = (v4si){la, lb, lc, ld};
    vib = (v4si){le, lf, lg, lh};
    vic = via + vib;
    vid = via * vib;
    
    /* Inline assembly to clobber physical registers and increase pressure */
    /* Clobber multiple registers to force pseudo-register usage */
    asm volatile (
        "# Clobber registers to increase pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        "add r0, r1\n"
        : 
        : "r" ((int)a), "r" ((int)b)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More complex interdependent computations */
    double u = t * s / r;
    double v = q * p / o;
    double w = u + v - m;
    double x = n * l / k;
    double y = j * i / h;
    double z = g * f / e;
    
    /* Create a pattern where variables are used as both source and dest */
    /* This creates multiple references to the same pseudo-register */
    double temp1 = a + b;
    double temp2 = temp1 * c;      /* temp1 used as source, temp2 as dest */
    double temp3 = temp2 - d;      /* temp2 used as source, temp3 as dest */
    double temp4 = temp3 / e;      /* temp3 used as source, temp4 as dest */
    double temp5 = temp4 + f;      /* temp4 used as source, temp5 as dest */
    double temp6 = temp5 * g;      /* temp5 used as source, temp6 as dest */
    double temp7 = temp6 - h;      /* temp6 used as source, temp7 as dest */
    double temp8 = temp7 / i;      /* temp7 used as source, temp8 as dest */
    double temp9 = temp8 + j;      /* temp8 used as source, temp9 as dest */
    double temp10 = temp9 * k;     /* temp9 used as source, temp10 as dest */
    
    /* Use all variables in final computation to ensure they're live */
    double result = (a + b + c + d + e + f + g + h + i + j + 
                    k + l + m + n + o + p + q + r + s + t +
                    u + v + w + x + y + z +
                    temp1 + temp2 + temp3 + temp4 + temp5 +
                    temp6 + temp7 + temp8 + temp9 + temp10 +
                    fa + fb + fc + fd + fe + ff + fg + fh + fi + fj +
                    la + lb + lc + ld + le + lf + lg + lh +
                    vc[0] + vc[1] + vc[2] + vc[3] +
                    vd[0] + vd[1] + vd[2] + vd[3] +
                    ve[0] + ve[1] + ve[2] + ve[3] +
                    vic[0] + vic[1] + vic[2] + vic[3] +
                    vid[0] + vid[1] + vid[2] + vid[3]);
    
    return result;
}

/* Hot loop to repeatedly call test_function */
int main() {
    double total = 0.0;
    int iterations = g_iterations;
    
    for (int i = 0; i < iterations; i++) {
        /* Use volatile seed to prevent dead code elimination */
        double seed = g_seed + i * 0.01;
        total += test_function(seed);
        
        /* Additional computation to create more pressure */
        if (i % 10 == 0) {
            /* Create structs to use helper functions */
            struct Vec4 v1 = {seed, seed*2, seed*3, seed*4};
            struct Vec4 v2 = {seed*5, seed*6, seed*7, seed*8};
            struct Vec4 v3 = add_vec4(v1, v2);
            struct Vec4 v4 = mul_vec4(v3, v1);
            total += dot_product(v3, v4);
        }
    }
    
    printf("Result: %f\n", total);
    return (int)total % 256;
}
