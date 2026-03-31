/* test_mcf_coverage.c - Complex CFG generator for MCF pass coverage */
#ifdef __GNUC__
#define FORCE_USE(var) asm volatile("" : : "g"(var))
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))
#else
#define FORCE_USE(var) (void)(var)
#define NOINLINE
#define HOT
#define COLD
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

static int verbose = 0;

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    /* Complex function with irreducible region to force ENTRY/EXIT block creation */
    volatile int result = seed;
    int i, j, k;
    
    /* Create many local variables to increase register pressure */
    int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    int r12 = seed + 12, r13 = seed + 13, r14 = seed + 14, r15 = seed + 15;
    
    /* Irreducible region using goto */
    if (iterations > 0) {
        goto middle_of_function;
    }
    
start_label:
    for (i = 0; i < iterations; i++) {
        /* Deeply nested if-else chain */
        if (r0 % 3 == 0) {
            if (r1 % 5 == 0) {
                r2 = r3 * r4;
                if (r2 > 1000) {
                    r5 = r6 / (r7 + 1);
                    goto middle_of_function;
                } else {
                    r8 = r9 - r10;
                }
            } else if (r1 % 7 == 0) {
                for (j = 0; j < 5; j++) {
                    r11 += r12 * j;
                    if (r11 > 500) break;
                }
            }
        } else if (r0 % 4 == 0) {
            r13 = r14 | r15;
            continue;
        }
        
        /* Another level of nesting */
        switch (r0 % 8) {
            case 0: r0 = r1 + r2; break;
            case 1: r0 = r3 - r4; break;
            case 2: r0 = r5 * r6; break;
            case 3: r0 = r7 / (r8 + 1); break;
            case 4: r0 = r9 ^ r10; break;
            case 5: r0 = r11 & r12; break;
            case 6: r0 = r13 | r14; break;
            case 7: r0 = r15 << 2; break;
        }
        
middle_of_function:
        /* Complex arithmetic to prevent optimization */
        r1 = (r0 * 1103515245 + 12345) & 0x7fffffff;
        r2 = (r1 * 1103515245 + 12345) & 0x7fffffff;
        r3 = (r2 * 1103515245 + 12345) & 0x7fffffff;
        
        if (i % 100 == 0) {
            goto start_label;  /* Creates irreducible region */
        }
        
        /* Force all variables to appear live */
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
        FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    }
    
    result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
             r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    return result;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
NOINLINE
int pattern_b_new_indices(int depth, int value) {
    /* Function using setjmp/longjmp to create exceptional edges */
    volatile int result = 0;
    int i, j, k, l, m, n, o, p;
    int a = value, b = value + 1, c = value + 2, d = value + 3;
    int e = value + 4, f = value + 5, g = value + 6, h = value + 7;
    int w = value + 8, x = value + 9, y = value + 10, z = value + 11;
    
    if (setjmp(jump_buffer) == 0) {
        /* Normal path with complex loop */
        for (i = 0; i < depth; i++) {
            /* Large switch with 32 cases */
            switch (i % 32) {
                case 0: a = b + c; d = e * f; break;
                case 1: b = c - d; e = f / (g + 1); break;
                case 2: c = d * e; f = g ^ h; break;
                case 3: d = e / (f + 1); g = h & w; break;
                case 4: e = f + g; h = w | x; break;
                case 5: f = g - h; w = x << 2; break;
                case 6: g = h * w; x = y >> 1; break;
                case 7: h = w / (x + 1); y = z + a; break;
                case 8: w = x + y; z = a - b; break;
                case 9: x = y - z; a = b * c; break;
                case 10: y = z * a; b = c / (d + 1); break;
                case 11: z = a / (b + 1); c = d ^ e; break;
                case 12: a = b + c; d = e & f; break;
                case 13: b = c - d; e = f | g; break;
                case 14: c = d * e; f = g << 3; break;
                case 15: d = e / (f + 1); g = h >> 2; break;
                case 16: e = f + g; h = w + x; break;
                case 17: f = g - h; w = x * y; break;
                case 18: g = h * w; x = y / (z + 1); break;
                case 19: h = w / (x + 1); y = z ^ a; break;
                case 20: w = x + y; z = a & b; break;
                case 21: x = y - z; a = b | c; break;
                case 22: y = z * a; b = c << 1; break;
                case 23: z = a / (b + 1); c = d >> 3; break;
                case 24: a = b + c; d = e + f; break;
                case 25: b = c - d; e = f - g; break;
                case 26: c = d * e; f = g * h; break;
                case 27: d = e / (f + 1); g = h / (w + 1); break;
                case 28: e = f + g; h = w ^ x; break;
                case 29: f = g - h; w = x & y; break;
                case 30: g = h * w; x = y | z; break;
                case 31: h = w / (x + 1); y = z << 4; break;
            }
            
            /* Nested loops with continue/break to different cases */
            for (j = 0; j < 3; j++) {
                if ((i + j) % 7 == 0) {
                    continue;
                }
                for (k = 0; k < 2; k++) {
                    if ((i + j + k) % 11 == 0) {
                        if (i > depth / 2) {
                            longjmp(jump_buffer, 1);  /* Exceptional edge */
                        }
                        break;
                    }
                }
            }
            
            FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
            FORCE_USE(e); FORCE_USE(f); FORCE_USE(g); FORCE_USE(h);
            FORCE_USE(w); FORCE_USE(x); FORCE_USE(y); FORCE_USE(z);
        }
    } else {
        /* longjmp target */
        a = b = c = d = 1;
    }
    
    result = a + b + c + d + e + f + g + h + w + x + y + z;
    return result;
}

/* ========== PATTERN C: MIXED PRESSURE with VECTOR OPS ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int iterations, int base) {
    volatile int result = base;
    int i;
    
    /* Many scalar variables */
    int s0 = base, s1 = base + 1, s2 = base + 2, s3 = base + 3;
    int s4 = base + 4, s5 = base + 5, s6 = base + 6, s7 = base + 7;
    int s8 = base + 8, s9 = base + 9, s10 = base + 10, s11 = base + 11;
    int s12 = base + 12, s13 = base + 13, s14 = base + 14, s15 = base + 15;
    
#ifdef __GNUC__
    /* Vector variables - pressure on vector registers */
    v4si v0 = {base, base+1, base+2, base+3};
    v4si v1 = {base+4, base+5, base+6, base+7};
    v4si v2 = {base+8, base+9, base+10, base+11};
    v4si v3 = {base+12, base+13, base+14, base+15};
#endif
    
    for (i = 0; i < iterations; i++) {
        /* Large switch with 35 cases */
        switch (i % 35) {
            case 0: s0 = s1 + s2; s3 = s4 * s5; break;
            case 1: s1 = s2 - s3; s4 = s5 / (s6 + 1); break;
            case 2: s2 = s3 * s4; s5 = s6 ^ s7; break;
            case 3: s3 = s4 / (s5 + 1); s6 = s7 & s8; break;
            case 4: s4 = s5 + s6; s7 = s8 | s9; break;
            case 5: s5 = s6 - s7; s8 = s9 << 2; break;
            case 6: s6 = s7 * s8; s9 = s10 >> 1; break;
            case 7: s7 = s8 / (s9 + 1); s10 = s11 + s12; break;
            case 8: s8 = s9 + s10; s11 = s12 - s13; break;
            case 9: s9 = s10 - s11; s12 = s13 * s14; break;
            case 10: s10 = s11 * s12; s13 = s14 / (s15 + 1); break;
            case 11: s11 = s12 / (s13 + 1); s14 = s15 ^ s0; break;
            case 12: s12 = s13 + s14; s15 = s0 & s1; break;
            case 13: s13 = s14 - s15; s0 = s1 | s2; break;
            case 14: s14 = s15 * s0; s1 = s2 << 3; break;
            case 15: s15 = s0 / (s1 + 1); s2 = s3 >> 2; break;
            case 16: s0 = s1 + s2; s3 = s4 + s5; break;
            case 17: s1 = s2 - s3; s4 = s5 * s6; break;
            case 18: s2 = s3 * s4; s5 = s6 / (s7 + 1); break;
            case 19: s3 = s4 / (s5 + 1); s6 = s7 ^ s8; break;
            case 20: s4 = s5 + s6; s7 = s8 & s9; break;
            case 21: s5 = s6 - s7; s8 = s9 | s10; break;
            case 22: s6 = s7 * s8; s9 = s10 << 1; break;
            case 23: s7 = s8 / (s9 + 1); s10 = s11 >> 3; break;
            case 24: s8 = s9 + s10; s11 = s12 + s13; break;
            case 25: s9 = s10 - s11; s12 = s13 - s14; break;
            case 26: s10 = s11 * s12; s13 = s14 * s15; break;
            case 27: s11 = s12 / (s13 + 1); s14 = s15 / (s0 + 1); break;
            case 28: s12 = s13 + s14; s15 = s0 ^ s1; break;
            case 29: s13 = s14 - s15; s0 = s1 & s2; break;
            case 30: s14 = s15 * s0; s1 = s2 | s3; break;
            case 31: s15 = s0 / (s1 + 1); s2 = s3 << 4; break;
            case 32: s0 = s1 + s2; s3 = s4 ^ s5; break;
            case 33: s1 = s2 - s3; s4 = s5 & s6; break;
            case 34: s2 = s3 * s4; s5 = s6 | s7; break;
        }
        
#ifdef __GNUC__
        /* Vector operations mixed with scalar */
        switch (i % 4) {
            case 0: v0 = v1 + v2; v3 = v0 * v1; break;
            case 1: v1 = v2 - v3; v0 = v1 / (v2 + (v4si){1,1,1,1}); break;
            case 2: v2 = v3 * v0; v1 = v2 ^ v3; break;
            case 3: v3 = v0 / (v1 + (v4si){1,1,1,1}); v2 = v3 & v0; break;
        }
        
        /* Mix vector and scalar */
        s0 += v0[0]; s1 += v0[1]; s2 += v0[2]; s3 += v0[3];
        s4 += v1[0]; s5 += v1[1]; s6 += v1[2]; s7 += v1[3];
#endif
        
        /* Complex loop control */
        if (i % 13 == 0) {
            continue;
        } else if (i % 17 == 0) {
            if (i > iterations / 3) {
                break;
            }
        }
        
        FORCE_USE(s0); FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3);
        FORCE_USE(s4); FORCE_USE(s5); FORCE_USE(s6); FORCE_USE(s7);
        FORCE_USE(s8); FORCE_USE(s9); FORCE_USE(s10); FORCE_USE(s11);
        FORCE_USE(s12); FORCE_USE(s13); FORCE_USE(s14); FORCE_USE(s15);
    }
    
#ifdef __GNUC__
    result = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + 
             s8 + s9 + s10 + s11 + s12 + s13 + s14 + s15 +
             v0[0] + v0[1] + v0[2] + v0[3] +
             v1[0] + v1[1] + v1[2] + v1[3];
#else
    result = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + 
             s8 + s9 + s10 + s11 + s12 + s13 + s14 + s15;
#endif
    return result;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
NOINLINE static int dummy_helper1(int x) { return x * 3; }
NOINLINE static int dummy_helper2(int x) { return x / 2; }
NOINLINE static int dummy_helper3(int x) { return x + 7; }
NOINLINE static int dummy_helper4(int x) { return x - 5; }

NOINLINE
int pattern_d_artificial_conflict(int loops, int init) {
    volatile int result = init;
    
    /* Explicit register variables creating artificial conflicts */
#ifdef __GNUC__
    register int r10_var asm ("r10") = init;
    register int r11_var asm ("r11") = init + 1;
    /* Note: We can't bind multiple variables to same register in one function,
       but we can create conflicts through calls */
#else
    int r10_var = init;
    int r11_var = init + 1;
#endif
    
    int a = init, b = init + 2, c = init + 3, d = init + 4;
    int e = init + 5, f = init + 6, g = init + 7, h = init + 8;
    int i, j;
    
    for (i = 0; i < loops; i++) {
        /* Call dummy helpers that might clobber registers */
        a = dummy_helper1(a);
        b = dummy_helper2(b);
        c = dummy_helper3(c);
        d = dummy_helper4(d);
        
        /* Large switch creating many basic blocks */
        switch (i % 40) {
            case 0: r10_var = a + b; e = c * d; break;
            case 1: r11_var = b - c; f = d / (a + 1); break;
            case 2: a = c * d; g = e ^ f; break;
            case 3: b = d / (e + 1); h = f & g; break;
            case 4: c = e + f; a = g | h; break;
            case 5: d = f - g; b = h << 2; break;
            case 6: e = g * h; c = a >> 1; break;
            case 7: f = h / (a + 1); d = b + c; break;
            case 8: g = a + b; e = c - d; break;
            case 9: h = b - c; f = d * e; break;
            case 10: a = c * d; g = e / (f + 1); break;
            case 11: b = d / (e + 1); h = f ^ g; break;
            case 12: c = e + f; a = g & h; break;
            case 13: d = f - g; b = h | a; break;
            case 14: e = g * h; c = a << 3; break;
            case 15: f = h / (a + 1); d = b >> 2; break;
            case 16: g = a + b; e = c + d; break;
            case 17: h = b - c; f = d * e; break;
            case 18: a = c * d; g = e / (f + 1); break;
            case 19: b = d / (e + 1); h = f ^ g; break;
            case 20: c = e + f; a = g & h; break;
            case 21: d = f - g; b = h | a; break;
            case 22: e = g * h; c = a << 1; break;
            case 23: f = h / (a + 1); d = b >> 3; break;
            case 24: g = a + b; e = c ^ d; break;
            case 25: h = b - c; f = d & e; break;
            case 26: a = c * d; g = e | f; break;
            case 27: b = d / (e + 1); h = f << 4; break;
            case 28: c = e + f; a = g >> 2; break;
            case 29: d = f - g; b = h + a; break;
            case 30: e = g * h; c = a - b; break;
            case 31: f = h / (a + 1); d = b * c; break;
            case 32: g = a + b; e = c / (d + 1); break;
            case 33: h = b - c; f = d ^ e; break;
            case 34: a = c * d; g = e & f; break;
            case 35: b = d / (e + 1); h = f | g; break;
            case 36: c = e + f; a = g << 3; break;
            case 37: d = f - g; b = h >> 1; break;
            case 38: e = g * h; c = a + b; break;
            case 39: f = h / (a + 1); d = b - c; break;
        }
        
        /* More helper calls to split live ranges */
        if (i % 5 == 0) {
            r10_var = dummy_helper1(r10_var);
            r11_var = dummy_helper2(r11_var);
        }
        
        /* Nested loop with complex control flow */
        for (j = 0; j < 3; j++) {
            if ((i + j) % 7 == 0) {
                a = dummy_helper3(a);
                continue;
            }
            if ((i + j) % 11 == 0) {
                b = dummy_helper4(b);
                break;
            }
        }
        
        FORCE_USE(r10_var); FORCE_USE(r11_var);
        FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
        FORCE_USE(e); FORCE_USE(f); FORCE_USE(g); FORCE_USE(h);
    }
    
    result = r10_var + r11_var + a + b + c + d + e + f + g + h;
    return result;
}

/* ========== MAIN FUNCTION WITH PGO SUPPORT ========== */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int i, iterations;
    
    if (argc > 1) {
        verbose = atoi(argv[1]);
    }
    
    iterations = 1000;
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    if (verbose) {
        printf("Starting MCF stress test with %d iterations\n", iterations);
    }
    
    /* Use CPU features to engage target-specific register allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", use_avx2 ? "yes" : "no");
    }
#endif
    
    /* Run all patterns multiple times to generate profile data */
    for (i = 0; i < iterations; i++) {
        int seed = i * 1103515245 + 12345;
        
        /* Pattern A: ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(i % 100 + 50, seed);
        
        /* Pattern B: NEW_EXIT/NEW_ENTRY with setjmp */
        total += pattern_b_new_indices(i % 80 + 40, seed + 1);
        
        /* Pattern C: Mixed pressure with vector ops */
        total += pattern_c_mixed_pressure(i % 120 + 60, seed + 2);
        
        /* Pattern D: Artificial conflict */
        total += pattern_d_artificial_conflict(i % 90 + 30, seed + 3);
        
        /* Prevent optimization of the loop */
        if (i % 1000 == 0 && verbose) {
            printf("Progress: %d/%d\n", i, iterations);
        }
    }
    
    if (verbose) {
        printf("Total result: %d\n", total);
    }
    
    /* Use the result to prevent dead code elimination */
    FORCE_USE(total);
    
    return total != 0 ? 0 : 1;
}
