/* Main test driver with volatile loop control */
#include <stdint.h>
#include <stdio.h>

/* Forward declarations for helper functions */
extern int helper1(int a, int b, int c, int d, int e);
extern float helper2(float a, float b, float c);
extern double helper3(double a, double b, double c, double d);
extern long helper4(long a, long b, long c, long d, long e, long f);

/* Volatile to prevent optimization */
volatile int loop_counter = 1000;

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct for inter-procedural pressure */
struct ComplexData {
    v4si vec_int;
    v4sf vec_float;
    v2df vec_double;
    int scalar_int;
    float scalar_float;
    double scalar_double;
};

/* Noinline to prevent optimization */
__attribute__((noinline)) 
struct ComplexData process_data(struct ComplexData input) {
    struct ComplexData result;
    
    /* Vector operations */
    result.vec_int = input.vec_int + (v4si){1, 2, 3, 4};
    result.vec_float = input.vec_float * (v4sf){1.5f, 2.5f, 3.5f, 4.5f};
    result.vec_double = input.vec_double - (v2df){0.5, 1.5};
    
    /* Scalar operations with dependencies */
    result.scalar_int = input.scalar_int * 2 + 1;
    result.scalar_float = input.scalar_float / 2.0f + 1.0f;
    result.scalar_double = input.scalar_double * 3.0 - 2.0;
    
    return result;
}

/* Main test function with high register pressure */
__attribute__((noinline, optimize("O3")))
int test_function(int seed) {
    /* Declare many local variables of different types */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    float fa, fb, fc, fd, fe, ff, fg, fh, fi, fj;
    double da, db, dc, dd, de, df, dg, dh, di, dj;
    long la, lb, lc, ld, le, lf, lg, lh, li, lj;
    
    /* Vector variables */
    v4si v1, v2, v3, v4, v5;
    v4sf vf1, vf2, vf3, vf4;
    v2df vd1, vd2, vd3;
    
    /* Initialize with seed */
    a = seed;
    b = a + 1;
    c = b * 2;
    d = c - a;
    e = d / 2;
    f = e % 3;
    g = f << 2;
    h = g >> 1;
    i = h | 0xFF;
    j = i & 0xF0;
    k = j ^ 0xAA;
    l = k + b;
    m = l - c;
    n = m * d;
    o = n / e;
    p = o % f;
    q = p << g;
    r = q >> h;
    s = r | i;
    t = s & j;
    
    /* Float operations with dependencies */
    fa = (float)a / 3.0f;
    fb = fa * 2.0f;
    fc = fb + fa;
    fd = fc - fb;
    fe = fd * fc;
    ff = fe / fd;
    fg = ff + fe;
    fh = fg - ff;
    fi = fh * fg;
    fj = fi / fh;
    
    /* Double operations with more precision */
    da = (double)t / 7.0;
    db = da * 3.0;
    dc = db + da;
    dd = dc - db;
    de = dd * dc;
    df = de / dd;
    dg = df + de;
    dh = dg - df;
    di = dh * dg;
    dj = di / dh;
    
    /* Long operations */
    la = (long)a * 1000L;
    lb = la + 500L;
    lc = lb - 250L;
    ld = lc * 2L;
    le = ld / 3L;
    lf = le % 100L;
    lg = lf << 2;
    lh = lg >> 1;
    li = lh | 0xFFFFL;
    lj = li & 0xFF00L;
    
    /* Vector operations - creates many pseudo-registers */
    v1 = (v4si){a, b, c, d};
    v2 = (v4si){e, f, g, h};
    v3 = v1 + v2;
    v4 = v1 * v2;
    v5 = v3 - v4;
    
    vf1 = (v4sf){fa, fb, fc, fd};
    vf2 = (v4sf){fe, ff, fg, fh};
    vf3 = vf1 * vf2;
    vf4 = vf1 + vf3;
    
    vd1 = (v2df){da, db};
    vd2 = (v2df){dc, dd};
    vd3 = vd1 * vd2;
    
    /* Critical section: operations where variables are used as both
       source and destination in adjacent statements - this creates
       the pattern that might trigger the replacement logic */
    a = b + c;      /* a gets pseudo-register R1 */
    d = a * e;      /* a used here, might trigger rematerialization */
    
    b = c + d;
    e = b * f;
    
    c = d + e;
    f = c * g;
    
    /* More complex dependency chains */
    g = h + i;
    j = g * k;
    h = i + j;
    k = h * l;
    
    /* Inline assembly to clobber physical registers and increase pressure */
    asm volatile(
        "/* Clobber many registers to force pseudo-register usage */\n\t"
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "add r2, r0, r1\n\t"
        : 
        : "r" (a), "r" (b)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* Call helper functions to increase inter-procedural pressure */
    int h1 = helper1(a, b, c, d, e);
    float h2 = helper2(fa, fb, fc);
    double h3 = helper3(da, db, dc, dd);
    long h4 = helper4(la, lb, lc, ld, le, lf);
    
    /* Use all variables in final computation to keep them live */
    int result = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + q + r + s + t;
    result += (int)fa + (int)fb + (int)fc + (int)fd + (int)fe + (int)ff;
    result += (int)da + (int)db + (int)dc + (int)dd;
    result += (int)la + (int)lb + (int)lc + (int)ld;
    
    /* Use vector results */
    result += v1[0] + v2[1] + v3[2] + v4[3] + v5[0];
    result += (int)vf1[0] + (int)vf2[1] + (int)vf3[2] + (int)vf4[3];
    result += (int)vd1[0] + (int)vd2[1] + (int)vd3[0];
    
    /* Add helper results */
    result += h1 + (int)h2 + (int)h3 + (int)h4;
    
    return result;
}

int main() {
    int total = 0;
    
    /* Loop to increase execution time and register pressure */
    for (int i = 0; i < loop_counter; i++) {
        total += test_function(i);
        
        /* Create struct for inter-procedural test */
        struct ComplexData data = {
            .vec_int = (v4si){i, i+1, i+2, i+3},
            .vec_float = (v4sf){i*1.0f, i*2.0f, i*3.0f, i*4.0f},
            .vec_double = (v2df){i*1.0, i*2.0},
            .scalar_int = i,
            .scalar_float = i*5.0f,
            .scalar_double = i*6.0
        };
        
        struct ComplexData processed = process_data(data);
        total += processed.scalar_int;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
