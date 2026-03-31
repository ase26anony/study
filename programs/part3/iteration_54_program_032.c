/* test_mcf.c - Complex CFG generator to trigger MCF special node printing */
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define USED __attribute__((used))
#else
#define NOINLINE
#define HOT
#define USED
#endif

/* Global volatile to prevent optimization */
volatile int global_counter = 0;
static int verbose = 0;

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE USED
int pattern_a_entry_exit(int iterations, int seed) {
    int result = seed;
    int i, j, k;
    
    /* Many local variables to create register pressure */
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
    
    /* Label to create irreducible region */
    if (seed & 1) {
        goto irreducible_start;
    }
    
    /* Deep nesting to create complex CFG */
    for (i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            for (j = 0; j < 5; j++) {
                if (j % 2 == 0) {
                    r0 = i * j + seed;
                    r1 = r0 ^ 0x12345678;
                    r2 = r1 * 3;
                    r3 = r2 + r0;
                    asm volatile("" : : "g"(r0), "g"(r1), "g"(r2), "g"(r3));
                } else {
                    r4 = i ^ j;
                    r5 = r4 << 3;
                    r6 = r5 - seed;
                    r7 = r6 / 2;
                    asm volatile("" : : "g"(r4), "g"(r5), "g"(r6), "g"(r7));
                }
                
                /* Complex condition with goto */
                if (r3 > r7 && i > 2) {
                    goto irreducible_start;
                }
            }
        } else if (i % 3 == 1) {
            k = 0;
            while (k < 10) {
                r8 = k * i;
                r9 = r8 + seed;
                r10 = r9 ^ k;
                asm volatile("" : : "g"(r8), "g"(r9), "g"(r10));
                
                if (k == 5) {
                    break;
                }
                k++;
            }
        } else {
            /* Another nested loop */
            for (k = 0; k < 3; k++) {
                r11 = (i << k) + seed;
                r12 = r11 * 7;
                r13 = r12 & 0xFF;
                asm volatile("" : : "g"(r11), "g"(r12), "g"(r13));
            }
        }
        
        result += r0 + r4 + r8 + r11;
    }
    
    return result;

irreducible_start:
    /* This creates an irreducible region */
    r14 = seed * 2;
    r15 = r14 + 1;
    
    /* Jump back to different parts of the function */
    if (r15 % 2 == 0) {
        goto irreducible_start;
    } else {
        /* Force creation of EXIT block variants */
        r16 = 0;
        for (i = 0; i < 100; i++) {
            r16 += i * seed;
            if (r16 > 1000) {
                goto early_exit;
            }
        }
    }
    
    return r15;

early_exit:
    r17 = seed ^ 0xDEADBEEF;
    asm volatile("" : : "g"(r17));
    return r17;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
NOINLINE USED
int pattern_b_new_nodes(int depth, int value) {
    static jmp_buf env;
    int a, b, c, d, e, f, g, h, i, j;
    int k, l, m, n, o, p, q, r, s, t;
    
    /* Initialize many variables */
    a = value;
    b = a * 2;
    c = b + 1;
    d = c ^ a;
    e = d << 3;
    f = e / 2;
    g = f - b;
    h = g & 0xFF;
    i = h | 0x80;
    j = i * 3;
    k = j + a;
    l = k ^ b;
    m = l * 5;
    n = m - c;
    o = n / 7;
    p = o << 2;
    q = p | 0xF0;
    r = q + d;
    s = r * 11;
    t = s % 13;
    
    /* setjmp creates exceptional control flow */
    if (setjmp(env) == 0) {
        /* First call path */
        if (depth > 0) {
            /* Recursive call to create stack depth */
            int ret = pattern_b_new_nodes(depth - 1, value + 1);
            t += ret;
        }
        
        /* Complex arithmetic that can't be optimized away */
        for (int idx = 0; idx < 50; idx++) {
            a = (a * 1103515245 + 12345) & 0x7FFFFFFF;
            b = (b * 1664525 + 1013904223) & 0x7FFFFFFF;
            c = c ^ (a >> 16);
            d = d + (b & 0xFFFF);
            
            /* Force all variables live */
            asm volatile("" : : "g"(a), "g"(b), "g"(c), "g"(d));
            asm volatile("" : : "g"(e), "g"(f), "g"(g), "g"(h));
            asm volatile("" : : "g"(i), "g"(j), "g"(k), "g"(l));
            asm volatile("" : : "g"(m), "g"(n), "g"(o), "g"(p));
            asm volatile("" : : "g"(q), "g"(r), "g"(s), "g"(t));
            
            /* Occasionally longjmp to create abnormal edges */
            if ((idx % 17) == 0 && depth > 0) {
                longjmp(env, 1);
            }
        }
    } else {
        /* longjmp return path */
        t = (t * 3) & 0xFFF;
    }
    
    /* Mix all results */
    int result = a + b + c + d + e + f + g + h + i + j +
                 k + l + m + n + o + p + q + r + s + t;
    asm volatile("" : : "g"(result));
    return result;
}

/* ========== PATTERN C: MIXED PRESSURE WITH VECTORS ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE USED
int pattern_c_mixed_pressure(int selector, int iterations) {
    int result = 0;
    
    /* Scalar variables */
    int s0, s1, s2, s3, s4, s5, s6, s7, s8, s9;
    int s10, s11, s12, s13, s14, s15, s16, s17, s18, s19;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0, v1, v2, v3, v4, v5;
#endif
    
    /* Large switch with 30+ cases */
    for (int iter = 0; iter < iterations; iter++) {
        int case_selector = (selector + iter) % 35;
        
        switch (case_selector) {
            case 0:
                s0 = iter * 3;
                s1 = s0 + 1;
                s2 = s1 * 5;
                s3 = s2 - iter;
#ifdef __GNUC__
                v0 = (v4si){s0, s1, s2, s3};
                v1 = v0 * 2;
#endif
                asm volatile("" : : "g"(s0), "g"(s1), "g"(s2), "g"(s3));
                break;
                
            case 1:
                s4 = iter ^ 0x55;
                s5 = s4 << 2;
                s6 = s5 + 17;
                s7 = s6 / 3;
#ifdef __GNUC__
                v2 = (v4si){s4, s5, s6, s7};
                v3 = v2 + (v4si){1, 2, 3, 4};
#endif
                asm volatile("" : : "g"(s4), "g"(s5), "g"(s6), "g"(s7));
                break;
                
            case 2:
                s8 = (iter * 7) & 0xFF;
                s9 = s8 | 0x80;
                s10 = s9 ^ iter;
                s11 = s10 * 11;
#ifdef __GNUC__
                v4 = (v4si){s8, s9, s10, s11};
                v5 = v4 - v0;
#endif
                asm volatile("" : : "g"(s8), "g"(s9), "g"(s10), "g"(s11));
                break;
                
            /* Cases 3-32 with similar complex patterns */
            case 3: s12 = iter + 100; s13 = s12 * 2; s14 = s13 ^ 0xFF; s15 = s14 & 0x7F; break;
            case 4: s16 = iter - 50; s17 = s16 * 3; s18 = s17 + 255; s19 = s18 % 13; break;
            case 5: s0 = s19 * s12; s1 = s0 >> 3; s2 = s1 | 0x40; s3 = s2 + iter; break;
            case 6: s4 = s3 ^ s16; s5 = s4 * 7; s6 = s5 - 1000; s7 = s6 / 5; break;
            case 7: s8 = s7 & 0x3F; s9 = s8 << 1; s10 = s9 + 33; s11 = s10 ^ 0x99; break;
            case 8: s12 = s11 * 13; s13 = s12 % 17; s14 = s13 + 66; s15 = s14 & 0xF0; break;
            case 9: s16 = s15 | 0x0F; s17 = s16 * 2; s18 = s17 - 128; s19 = s18 ^ iter; break;
            case 10: s0 = s19 + s11; s1 = s0 * 3; s2 = s1 >> 2; s3 = s2 | 0xC0; break;
            case 11: s4 = s3 & 0x1F; s5 = s4 << 3; s6 = s5 + 77; s7 = s6 ^ 0xAA; break;
            case 12: s8 = s7 * 5; s9 = s8 % 19; s10 = s9 + 88; s11 = s10 & 0xE0; break;
            case 13: s12 = s11 | 0x07; s13 = s12 * 11; s14 = s13 - 256; s15 = s14 ^ iter; break;
            case 14: s16 = s15 + s7; s17 = s16 * 2; s18 = s17 >> 1; s19 = s18 | 0x30; break;
            case 15: s0 = s19 & 0x0F; s1 = s0 << 4; s2 = s1 + 99; s3 = s2 ^ 0xBB; break;
            case 16: s4 = s3 * 7; s5 = s4 % 23; s6 = s5 + 111; s7 = s6 & 0xC0; break;
            case 17: s8 = s7 | 0x03; s9 = s8 * 13; s10 = s9 - 512; s11 = s10 ^ iter; break;
            case 18: s12 = s11 + s3; s13 = s12 * 3; s14 = s13 >> 3; s15 = s14 | 0x60; break;
            case 19: s16 = s15 & 0x07; s17 = s16 << 5; s18 = s17 + 122; s19 = s18 ^ 0xCC; break;
            case 20: s0 = s19 * 11; s1 = s0 % 29; s2 = s1 + 133; s3 = s2 & 0xA0; break;
            case 21: s4 = s3 | 0x01; s5 = s4 * 17; s6 = s5 - 1024; s7 = s6 ^ iter; break;
            case 22: s8 = s7 + s19; s9 = s8 * 2; s10 = s9 >> 4; s11 = s10 | 0x90; break;
            case 23: s12 = s11 & 0x03; s13 = s12 << 6; s14 = s13 + 144; s15 = s14 ^ 0xDD; break;
            case 24: s16 = s15 * 19; s17 = s16 % 31; s18 = s17 + 155; s19 = s18 & 0x50; break;
            case 25: s0 = s19 | 0x0A; s1 = s0 * 23; s2 = s1 - 2048; s3 = s2 ^ iter; break;
            case 26: s4 = s3 + s15; s5 = s4 * 5; s6 = s5 >> 5; s7 = s6 | 0xB0; break;
            case 27: s8 = s7 & 0x01; s9 = s8 << 7; s10 = s9 + 166; s11 = s10 ^ 0xEE; break;
            case 28: s12 = s11 * 29; s13 = s12 % 37; s14 = s13 + 177; s15 = s14 & 0x30; break;
            case 29: s16 = s15 | 0x0C; s17 = s16 * 31; s18 = s17 - 4096; s19 = s18 ^ iter; break;
            case 30: s0 = s19 + s11; s1 = s0 * 7; s2 = s1 >> 6; s3 = s2 | 0xD0; break;
            case 31: s4 = s3 & 0x80; s5 = s4 >> 1; s6 = s5 + 188; s7 = s6 ^ 0xFF; break;
            case 32: s8 = s7 * 37; s9 = s8 % 41; s10 = s9 + 199; s11 = s10 & 0x20; break;
            case 33: s12 = s11 | 0x0E; s13 = s12 * 41; s14 = s13 - 8192; s15 = s14 ^ iter; break;
            case 34: s16 = s15 + s7; s17 = s16 * 11; s18 = s17 >> 7; s19 = s18 | 0xF0; break;
        }
        
        /* Mix results and continue/break based on conditions */
        result += s0 + s4 + s8 + s12 + s16;
        
        if (iter % 7 == 0) {
            continue;
        } else if (iter % 13 == 0) {
            /* Jump to different case using computed goto */
            switch ((result + iter) % 5) {
                case 0: continue;
                case 1: break;
                case 2: result += s1; continue;
                case 3: result += s5; break;
                case 4: result += s9; continue;
            }
        }
        
#ifdef __GNUC__
        /* Vector operations mixing */
        if (iter % 3 == 0) {
            v0 = v0 + v1;
            v2 = v2 * v3;
            v4 = v4 - v5;
            /* Force vector variables live */
            asm volatile("" : : "g"(v0), "g"(v1), "g"(v2), "g"(v3), "g"(v4), "g"(v5));
        }
#endif
    }
    
    return result;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
#ifdef __GNUC__
/* Dummy helpers to prevent inlining */
NOINLINE int helper1(int x) { return x * 3 + 1; }
NOINLINE int helper2(int x) { return x ^ 0x1234; }
NOINLINE int helper3(int x) { return x << 2; }
NOINLINE int helper4(int x) { return x >> 1; }
NOINLINE int helper5(int x) { return x & 0xFF; }
#endif

NOINLINE USED
int pattern_d_register_conflict(int param) {
    /* Explicit register variables to create conflicts */
#ifdef __GNUC__
    register int r10_var asm("r10") = param;
    register int r11_var asm("r11") = param * 2;
    register int r12_var asm("r12") = param + 1;
    register int r13_var asm("r13") = param ^ 0x55;
    register int r14_var asm("r14") = param << 3;
#else
    int r10_var = param;
    int r11_var = param * 2;
    int r12_var = param + 1;
    int r13_var = param ^ 0x55;
    int r14_var = param << 3;
#endif
    
    int temp;
    
    /* Force many calls that use the same registers */
    for (int i = 0; i < 100; i++) {
#ifdef __GNUC__
        temp = helper1(r10_var);
        r11_var = helper2(temp);
        r12_var = helper3(r11_var);
        r13_var = helper4(r12_var);
        r14_var = helper5(r13_var);
        r10_var = helper1(r14_var);
#endif
        
        /* Complex conditional to split live ranges */
        if (i % 7 == 0) {
            temp = r10_var + r11_var;
            asm volatile("" : : "g"(temp));
        } else if (i % 13 == 0) {
            temp = r12_var * r13_var;
            asm volatile("" : : "g"(temp));
        } else {
            temp = r14_var ^ r10_var;
            asm volatile("" : : "g"(temp));
        }
        
        /* Force all register variables live across calls */
        asm volatile("" : : "g"(r10_var), "g"(r11_var), "g"(r12_var), 
                     "g"(r13_var), "g"(r14_var));
    }
    
    return r10_var + r11_var + r12_var + r13_var + r14_var + temp;
}

/* ========== MAIN FUNCTION WITH PGO SUPPORT ========== */
int main(int argc, char *argv[]) {
    int total = 0;
    
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose = 1;
    }
    
    /* Use CPU features to engage target-specific register allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %d\n", use_avx2);
    }
#endif
    
    /* Run each pattern multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        /* Pattern A - ENTRY/EXIT blocks */
        int a = pattern_a_entry_exit(i % 10 + 5, i * 3);
        global_counter += a;
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        int b = pattern_b_new_nodes(i % 4, i * 7 + 1);
        global_counter += b;
        
        /* Pattern C - Mixed pressure with vectors */
        int c = pattern_c_mixed_pressure(i, 50);
        global_counter += c;
        
        /* Pattern D - Artificial register conflict */
        int d = pattern_d_register_conflict(i * 11 + 3);
        global_counter += d;
        
        total += a + b + c + d;
        
        if (verbose && i % 20 == 0) {
            printf("Iteration %d: a=%d, b=%d, c=%d, d=%d, total=%d\n", 
                   i, a, b, c, d, total);
        }
    }
    
    if (verbose) {
        printf("Final total: %d, global_counter: %d\n", total, (int)global_counter);
    }
    
    return total > 0 ? 0 : 1;
}
