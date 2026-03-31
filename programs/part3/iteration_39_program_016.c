/* Main test file with dense computation to force pseudo-register creation */
#include <stdint.h>

/* External helper functions to increase inter-procedural pressure */
extern struct Vec4 helper1(struct Vec4 a, struct Vec4 b);
extern struct Vec4 helper2(struct Vec4 a, struct Vec4 b, struct Vec4 c);
extern struct Vec4 helper3(struct Vec4 a, int scalar);

/* Vector type for wide register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Struct to force register/memory pressure */
struct Vec4 {
    v4si int_vec;
    v4sf float_vec;
    long extra1;
    double extra2;
};

/* Volatile to prevent optimization */
volatile int loop_counter = 100;

/* Noinline to prevent inlining and maintain pressure */
__attribute__((noinline, optimize("O0")))
struct Vec4 test_function(struct Vec4 input1, struct Vec4 input2) {
    /* Many local variables of different types */
    struct Vec4 temp1, temp2, temp3, temp4, temp5;
    v4si vec_temp1, vec_temp2, vec_temp3, vec_temp4, vec_temp5;
    v4sf fvec_temp1, fvec_temp2, fvec_temp3, fvec_temp4, fvec_temp5;
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    float fa, fb, fc, fd, fe, ff, fg, fh, fi, fj;
    double da, db, dc, dd, de, df, dg, dh, di, dj;
    long la, lb, lc, ld, le, lf, lg, lh, li, lj;
    
    /* Initialize with volatile reads to prevent constant propagation */
    volatile int seed = 42;
    a = seed + 1; b = seed + 2; c = seed + 3; d = seed + 4; e = seed + 5;
    f = seed + 6; g = seed + 7; h = seed + 8; i = seed + 9; j = seed + 10;
    k = seed + 11; l = seed + 12; m = seed + 13; n = seed + 14; o = seed + 15;
    p = seed + 16; q = seed + 17; r = seed + 18; s = seed + 19; t = seed + 20;
    
    fa = seed * 0.1f; fb = fa + 1.0f; fc = fb * 2.0f; fd = fc / 3.0f;
    fe = fd - 4.0f; ff = fe * 5.0f; fg = ff + 6.0f; fh = fg / 7.0f;
    fi = fh - 8.0f; fj = fi * 9.0f;
    
    da = seed * 0.01; db = da + 1.0; dc = db * 2.0; dd = dc / 3.0;
    de = dd - 4.0; df = de * 5.0; dg = df + 6.0; dh = dg / 7.0;
    di = dh - 8.0; dj = di * 9.0;
    
    la = seed * 100L; lb = la + 1L; lc = lb * 2L; ld = lc / 3L;
    le = ld - 4L; lf = le * 5L; lg = lf + 6L; lh = lg / 7L;
    li = lh - 8L; lj = li * 9L;
    
    /* Vector operations - use wide registers */
    vec_temp1 = input1.int_vec + input2.int_vec;
    vec_temp2 = vec_temp1 * (v4si){a, b, c, d};
    vec_temp3 = vec_temp2 - (v4si){e, f, g, h};
    vec_temp4 = vec_temp3 / (v4si){i, j, k, l};
    vec_temp5 = vec_temp4 ^ (v4si){m, n, o, p};
    
    fvec_temp1 = input1.float_vec + input2.float_vec;
    fvec_temp2 = fvec_temp1 * (v4sf){fa, fb, fc, fd};
    fvec_temp3 = fvec_temp2 - (v4sf){fe, ff, fg, fh};
    fvec_temp4 = fvec_temp3 / (v4sf){fi, fj, fa, fb};
    fvec_temp5 = fvec_temp4;
    
    /* Critical section: Chain of dependent operations to force 
       pseudo-register creation and potential replacement */
    /* Pattern that should trigger the uncovered code:
       Multiple uses of same pseudo-register in adjacent statements */
    int temp_reg = a + b;          /* Creates pseudo-register for temp_reg */
    c = temp_reg * d;              /* Use as operand */
    e = temp_reg + f;              /* Another use - multiple references */
    g = c * temp_reg;              /* Use both operand and temp_reg */
    
    /* More chains with different data types */
    float f_temp = fa * fb;
    fc = f_temp + fd;
    fe = f_temp / fg;
    fh = fc * f_temp;
    
    double d_temp = da * db;
    dc = d_temp + dd;
    de = d_temp / df;
    dg = dc * d_temp;
    
    /* Inline assembly to clobber physical registers and increase pressure */
    asm volatile (
        "/* Clobber many registers to force pseudo-register usage */\n\t"
        "nop"
        : /* no outputs */
        : /* no inputs */
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory"
    );
    
    /* Long chain of interdependent computations */
    /* This creates many pseudo-registers that might need rematerialization */
    int chain1 = a + b;
    int chain2 = chain1 * c;
    int chain3 = chain2 - d;
    int chain4 = chain3 / e;
    int chain5 = chain4 ^ f;
    int chain6 = chain5 | g;
    int chain7 = chain6 & h;
    int chain8 = chain7 + i;
    int chain9 = chain8 * j;
    int chain10 = chain9 - k;
    int chain11 = chain10 / l;
    int chain12 = chain11 ^ m;
    int chain13 = chain12 | n;
    int chain14 = chain13 & o;
    int chain15 = chain14 + p;
    int chain16 = chain15 * q;
    int chain17 = chain16 - r;
    int chain18 = chain17 / s;
    int chain19 = chain18 ^ t;
    int chain20 = chain19 | chain1;  /* Circular dependency */
    
    /* Mix all computations for final result */
    temp1.int_vec = vec_temp5 + (v4si){chain1, chain2, chain3, chain4};
    temp1.float_vec = fvec_temp5 * (v4sf){f_temp, fc, fe, fh};
    temp1.extra1 = la + lb + lc + ld + le + lf + lg + lh + li + lj;
    temp1.extra2 = d_temp + dc + de + dg;
    
    /* Use helper functions to increase cross-function pressure */
    temp2 = helper1(temp1, input1);
    temp3 = helper2(temp2, input2, temp1);
    temp4 = helper3(temp3, chain20);
    
    /* Final computation using all temporaries */
    temp5.int_vec = temp4.int_vec + temp3.int_vec - temp2.int_vec + temp1.int_vec;
    temp5.float_vec = temp4.float_vec * temp3.float_vec / temp2.float_vec + temp1.float_vec;
    temp5.extra1 = temp4.extra1 ^ temp3.extra1 | temp2.extra1 & temp1.extra1;
    temp5.extra2 = temp4.extra2 + temp3.extra2 - temp2.extra2 * temp1.extra2;
    
    return temp5;
}

int main() {
    struct Vec4 result;
    struct Vec4 input1 = {
        .int_vec = {1, 2, 3, 4},
        .float_vec = {1.0f, 2.0f, 3.0f, 4.0f},
        .extra1 = 100,
        .extra2 = 200.0
    };
    
    struct Vec4 input2 = {
        .int_vec = {5, 6, 7, 8},
        .float_vec = {5.0f, 6.0f, 7.0f, 8.0f},
        .extra1 = 200,
        .extra2 = 400.0
    };
    
    /* Loop to increase optimization opportunities */
    int count = loop_counter;  /* Volatile prevents dead code elimination */
    for (int i = 0; i < count; i++) {
        /* Vary inputs slightly each iteration */
        input1.int_vec[0] += i;
        input2.int_vec[1] += i;
        result = test_function(input1, input2);
    }
    
    /* Use result to prevent elimination */
    volatile struct Vec4 sink = result;
    return sink.int_vec[0] + sink.int_vec[1] + sink.int_vec[2] + sink.int_vec[3];
}
