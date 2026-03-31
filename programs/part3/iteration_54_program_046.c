/* test_mcf.c - Complex CFG generator for MCF pass testing */
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

/* Pattern A: Function targeting ENTRY/EXIT blocks with irreducible region */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    
    /* Create complex initial block that should become ENTRY_BLOCK */
    r0 = seed * 3;
    r1 = seed + 17;
    r2 = seed ^ 0x55AA;
    r3 = seed << 3;
    r4 = seed >> 2;
    
    /* Deeply nested if-else chain */
    if (r0 < 100) {
        if (r1 > 50) {
            r5 = r0 * r1;
            if (r2 & 1) {
                r6 = r5 + r2;
                goto label_irreducible_1;
            } else {
                r6 = r5 - r2;
            }
        } else {
            r5 = r0 / (r1 + 1);
            if (r3 != 0) {
                r6 = r5 % r3;
            } else {
                r6 = r5;
            }
        }
    } else {
        if (r4 < 30) {
            r5 = r0 + r4 * 7;
            if (r2 > 1000) {
                r6 = r5 ^ r2;
            } else {
                r6 = r5 | r2;
            }
        } else {
            r5 = r0 - r4;
            r6 = r5 & r3;
        }
    }
    
    /* Irreducible region created with goto */
    label_irreducible_1:
    for (i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            r7 = r6 * i;
            if (r7 > 1000) goto label_irreducible_2;
        } else if (i % 3 == 1) {
            r8 = r6 + i * 7;
            if (r8 < 500) goto label_irreducible_3;
        } else {
            r9 = r6 - i * 3;
            if (r9 == 0) goto label_irreducible_1;
        }
        
        r10 = r7 + r8 + r9;
        FORCE_USE(r10);
        
        label_irreducible_2:
        for (j = 0; j < 5; j++) {
            if (j == 2) goto label_irreducible_3;
            r0 += j * r10;
        }
        
        label_irreducible_3:
        for (k = 0; k < 3; k++) {
            if (k == 1 && i > iterations/2) goto label_irreducible_1;
            r1 -= k * r0;
        }
        
        result += r0 + r1 + r6;
    }
    
    /* Complex exit region */
    if (result > 1000000) {
        for (i = 0; i < 10; i++) {
            result >>= 1;
            if (result < 1000) break;
        }
    } else {
        while (result < 1000000) {
            result *= 3;
            if (result > 500000) continue;
            result += 17;
        }
    }
    
    FORCE_USE(result);
    return result & 0xFFFF;
}

/* Pattern B: Function using setjmp/longjmp for exceptional edges */
NOINLINE
int pattern_b_new_indices(int depth, int max_depth) {
    static jmp_buf env;
    volatile int counter = 0;
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    
    a = depth * 3;
    b = depth + 17;
    c = depth ^ 0x1234;
    d = depth << 2;
    e = depth >> 1;
    f = a + b;
    g = c - d;
    h = e * f;
    i = g / (depth + 1);
    j = h % (depth + 2);
    k = i ^ j;
    l = k | depth;
    m = l & 0xFF;
    n = m << 4;
    o = n >> 2;
    p = o + depth;
    
    if (setjmp(env) == 0) {
        /* Normal path with many live variables */
        for (int iter = 0; iter < 100; iter++) {
            a += iter;
            b -= iter * 2;
            c ^= iter;
            d <<= (iter & 3);
            e >>= (iter & 1);
            
            f = a + b + c;
            g = d - e + f;
            h = g * iter;
            i = h / (iter + 1);
            j = i % (iter + 2);
            
            k = j ^ p;
            l = k | m;
            m = l & n;
            n = m ^ o;
            o = n + p;
            p = o - a;
            
            FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
            FORCE_USE(e); FORCE_USE(f); FORCE_USE(g); FORCE_USE(h);
            FORCE_USE(i); FORCE_USE(j); FORCE_USE(k); FORCE_USE(l);
            FORCE_USE(m); FORCE_USE(n); FORCE_USE(o); FORCE_USE(p);
            
            counter++;
            
            /* Trigger longjmp at certain depths */
            if (depth < max_depth && iter == depth * 7) {
                pattern_b_new_indices(depth + 1, max_depth);
            }
            
            if (iter == 50 && depth > 0) {
                longjmp(env, 1);
            }
        }
    } else {
        /* longjmp target - different live ranges */
        a = b + c;
        d = e * f;
        g = h ^ i;
        j = k | l;
        m = n & o;
        p = a + d + g + j + m;
    }
    
    FORCE_USE(counter);
    return (a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + counter) & 0xFFFF;
}

/* Pattern C: Mixed pressure with vector operations and large switch */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int mode, int iterations) {
    volatile int result = 0;
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    
#ifdef __GNUC__
    v4si v0, v1, v2, v3, v4, v5;
    v0 = (v4si){mode, mode+1, mode+2, mode+3};
    v1 = (v4si){mode*2, mode*3, mode*4, mode*5};
#endif
    
    r0 = mode;
    r1 = iterations;
    r2 = r0 ^ r1;
    r3 = r0 + r1 * 3;
    r4 = r1 - r0;
    r5 = r2 * r3;
    r6 = r4 / (r0 + 1);
    r7 = r5 % (r1 + 1);
    r8 = r6 << 2;
    r9 = r7 >> 1;
    r10 = r8 | r9;
    r11 = r10 & 0x7F;
    r12 = r11 + r0;
    r13 = r12 - r1;
    r14 = r13 * 3;
    r15 = r14 / 2;
    
    for (int i = 0; i < iterations; i++) {
        /* Large switch with 30+ cases */
        switch ((r0 + i) % 35) {
            case 0:
                r0 = r1 + r2;
                r3 = r4 * r5;
                r6 = r7 ^ r8;
                r9 = r10 | r11;
                r12 = r13 - r14;
                r15 = r0 + r3 + r6;
#ifdef __GNUC__
                v0 = v0 + v1;
                v2 = v0 * (v4si){2, 2, 2, 2};
#endif
                FORCE_USE(r0); FORCE_USE(r3); FORCE_USE(r6); FORCE_USE(r9);
                FORCE_USE(r12); FORCE_USE(r15);
                break;
            case 1:
                r1 = r2 - r3;
                r4 = r5 / (r6 + 1);
                r7 = r8 % (r9 + 1);
                r10 = r11 << 1;
                r13 = r14 >> 2;
                r15 = r1 + r4 + r7;
#ifdef __GNUC__
                v1 = v1 - v0;
                v3 = v1 & (v4si){0xFF, 0xFF, 0xFF, 0xFF};
#endif
                FORCE_USE(r1); FORCE_USE(r4); FORCE_USE(r7); FORCE_USE(r10);
                FORCE_USE(r13); FORCE_USE(r15);
                break;
            case 2:
                r2 = r3 * r4;
                r5 = r6 ^ r7;
                r8 = r9 | r10;
                r11 = r12 & r13;
                r14 = r15 + r0;
                r1 = r2 + r5 + r8;
#ifdef __GNUC__
                v2 = v0 * v1;
                v4 = v2 | (v4si){1, 2, 3, 4};
#endif
                FORCE_USE(r2); FORCE_USE(r5); FORCE_USE(r8); FORCE_USE(r11);
                FORCE_USE(r14); FORCE_USE(r1);
                break;
            case 3:
                r3 = r4 / (r5 + 1);
                r6 = r7 % (r8 + 1);
                r9 = r10 << 3;
                r12 = r13 >> 1;
                r15 = r0 ^ r1;
                r2 = r3 + r6 + r9;
#ifdef __GNUC__
                v3 = v1 + (v4si){1, 1, 1, 1};
                v5 = v3 - v2;
#endif
                FORCE_USE(r3); FORCE_USE(r6); FORCE_USE(r9); FORCE_USE(r12);
                FORCE_USE(r15); FORCE_USE(r2);
                break;
            case 4:
                r4 = r5 - r6;
                r7 = r8 * r9;
                r10 = r11 ^ r12;
                r13 = r14 | r15;
                r0 = r1 & r2;
                r3 = r4 + r7 + r10;
#ifdef __GNUC__
                v4 = v2 * (v4si){3, 3, 3, 3};
                v0 = v4 + v3;
#endif
                FORCE_USE(r4); FORCE_USE(r7); FORCE_USE(r10); FORCE_USE(r13);
                FORCE_USE(r0); FORCE_USE(r3);
                break;
            /* 30 more similar cases omitted for brevity but should be expanded */
            case 5: r5 = r6 + r7; r8 = r9 * r10; r11 = r12 ^ r13; r14 = r15 | r0; r1 = r2 & r3; r4 = r5 + r8 + r11; break;
            case 6: r6 = r7 - r8; r9 = r10 / (r11+1); r12 = r13 % (r14+1); r15 = r0 << 2; r1 = r2 >> 1; r3 = r6 + r9 + r12; break;
            case 7: r7 = r8 * r9; r10 = r11 ^ r12; r13 = r14 | r15; r0 = r1 & r2; r3 = r4 + r5; r6 = r7 + r10 + r13; break;
            case 8: r8 = r9 / (r10+1); r11 = r12 % (r13+1); r14 = r15 << 1; r0 = r1 >> 3; r2 = r3 ^ r4; r5 = r8 + r11 + r14; break;
            case 9: r9 = r10 - r11; r12 = r13 * r14; r15 = r0 ^ r1; r2 = r3 | r4; r5 = r6 & r7; r8 = r9 + r12 + r15; break;
            case 10: r10 = r11 + r12; r13 = r14 * r15; r0 = r1 ^ r2; r3 = r4 | r5; r6 = r7 & r8; r9 = r10 + r13 + r0; break;
            case 11: r11 = r12 - r13; r14 = r15 / (r0+1); r1 = r2 % (r3+1); r4 = r5 << 2; r6 = r7 >> 1; r8 = r11 + r14 + r1; break;
            case 12: r12 = r13 * r14; r15 = r0 ^ r1; r2 = r3 | r4; r5 = r6 & r7; r8 = r9 + r10; r11 = r12 + r15 + r2; break;
            case 13: r13 = r14 / (r15+1); r0 = r1 % (r2+1); r3 = r4 << 3; r5 = r6 >> 2; r7 = r8 ^ r9; r10 = r13 + r0 + r3; break;
            case 14: r14 = r15 - r0; r1 = r2 * r3; r4 = r5 ^ r6; r7 = r8 | r9; r10 = r11 & r12; r13 = r14 + r1 + r4; break;
            case 15: r15 = r0 + r1; r2 = r3 * r4; r5 = r6 ^ r7; r8 = r9 | r10; r11 = r12 & r13; r14 = r15 + r2 + r5; break;
            case 16: r0 = r1 - r2; r3 = r4 / (r5+1); r6 = r7 % (r8+1); r9 = r10 << 1; r11 = r12 >> 3; r13 = r0 + r3 + r6; break;
            case 17: r1 = r2 * r3; r4 = r5 ^ r6; r7 = r8 | r9; r10 = r11 & r12; r13 = r14 + r15; r0 = r1 + r4 + r7; break;
            case 18: r2 = r3 / (r4+1); r5 = r6 % (r7+1); r8 = r9 << 2; r10 = r11 >> 1; r12 = r13 ^ r14; r15 = r2 + r5 + r8; break;
            case 19: r3 = r4 - r5; r6 = r7 * r8; r9 = r10 ^ r11; r12 = r13 | r14; r15 = r0 & r1; r2 = r3 + r6 + r9; break;
            case 20: r4 = r5 + r6; r7 = r8 * r9; r10 = r11 ^ r12; r13 = r14 | r15; r0 = r1 & r2; r3 = r4 + r7 + r10; break;
            case 21: r5 = r6 - r7; r8 = r9 / (r10+1); r11 = r12 % (r13+1); r14 = r15 << 3; r0 = r1 >> 2; r2 = r5 + r8 + r11; break;
            case 22: r6 = r7 * r8; r9 = r10 ^ r11; r12 = r13 | r14; r15 = r0 & r1; r2 = r3 + r4; r5 = r6 + r9 + r12; break;
            case 23: r7 = r8 / (r9+1); r10 = r11 % (r12+1); r13 = r14 << 1; r15 = r0 >> 3; r1 = r2 ^ r3; r4 = r7 + r10 + r13; break;
            case 24: r8 = r9 - r10; r11 = r12 * r13; r14 = r15 ^ r0; r1 = r2 | r3; r4 = r5 & r6; r7 = r8 + r11 + r14; break;
            case 25: r9 = r10 + r11; r12 = r13 * r14; r15 = r0 ^ r1; r2 = r3 | r4; r5 = r6 & r7; r8 = r9 + r12 + r15; break;
            case 26: r10 = r11 - r12; r13 = r14 / (r15+1); r0 = r1 % (r2+1); r3 = r4 << 2; r5 = r6 >> 1; r7 = r10 + r13 + r0; break;
            case 27: r11 = r12 * r13; r14 = r15 ^ r0; r1 = r2 | r3; r4 = r5 & r6; r7 = r8 + r9; r10 = r11 + r14 + r1; break;
            case 28: r12 = r13 / (r14+1); r15 = r0 % (r1+1); r2 = r3 << 3; r4 = r5 >> 2; r6 = r7 ^ r8; r9 = r12 + r15 + r2; break;
            case 29: r13 = r14 - r15; r0 = r1 * r2; r3 = r4 ^ r5; r6 = r7 | r8; r9 = r10 & r11; r12 = r13 + r0 + r3; break;
            case 30: r14 = r15 + r0; r1 = r2 * r3; r4 = r5 ^ r6; r7 = r8 | r9; r10 = r11 & r12; r13 = r14 + r1 + r4; break;
            case 31: r15 = r0 - r1; r2 = r3 / (r4+1); r5 = r6 % (r7+1); r8 = r9 << 1; r10 = r11 >> 3; r12 = r15 + r2 + r5; break;
            case 32: r0 = r1 * r2; r3 = r4 ^ r5; r6 = r7 | r8; r9 = r10 & r11; r12 = r13 + r14; r15 = r0 + r3 + r6; break;
            case 33: r1 = r2 / (r3+1); r4 = r5 % (r6+1); r7 = r8 << 2; r9 = r10 >> 1; r11 = r12 ^ r13; r14 = r1 + r4 + r7; break;
            case 34:
                r2 = r3 - r4;
                r5 = r6 * r7;
                r8 = r9 ^ r10;
                r11 = r12 | r13;
                r14 = r15 & r0;
                r1 = r2 + r5 + r8;
                if (i == iterations - 1) {
                    continue;  /* Create back-edge */
                }
                break;
        }
        
        /* Loop with continue to different cases */
        if (i % 7 == 0) {
            r0 = (r0 + r15) & 0xFF;
            continue;
        } else if (i % 7 == 3) {
            r1 = (r1 + r14) ^ 0x55;
            if (i > iterations / 2) break;
        }
        
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    }
    
#ifdef __GNUC__
    result += v0[0] + v0[1] + v0[2] + v0[3];
    result += v1[0] + v1[1] + v1[2] + v1[3];
#endif
    
    FORCE_USE(result);
    return result & 0xFFFF;
}

/* Pattern D: Artificial register conflicts */
NOINLINE
static int dummy_helper1(int x, int y) {
    return x * y + 17;
}

NOINLINE
static int dummy_helper2(int x, int y) {
    return (x ^ y) - 3;
}

NOINLINE
static int dummy_helper3(int x, int y, int z) {
    return (x + y) * z;
}

NOINLINE
int pattern_d_register_conflict(int param) {
    /* Explicit register variables creating conflicts */
#ifdef __GNUC__
    register int a asm("r10") = param;
    register int b asm("r11") = param * 2;
    register int c asm("r12") = param + 17;
    register int d asm("r13") = param ^ 0x1234;
    /* Force use of same register class for different purposes */
    register int e asm("r10") = a + b;  /* Conflict with a */
    register int f asm("r11") = c - d;  /* Conflict with b */
#else
    int a = param;
    int b = param * 2;
    int c = param + 17;
    int d = param ^ 0x1234;
    int e = a + b;
    int f = c - d;
#endif
    
    int g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
    
    /* Many calls creating live range splits */
    g = dummy_helper1(a, b);
    h = dummy_helper2(c, d);
    i = dummy_helper3(e, f, param);
    j = dummy_helper1(g, h);
    k = dummy_helper2(i, j);
    l = dummy_helper3(k, l, m);
    m = dummy_helper1(n, o);
    n = dummy_helper2(p, q);
    o = dummy_helper3(r, s, t);
    p = dummy_helper1(u, v);
    q = dummy_helper2(w, x);
    r = dummy_helper3(y, z, a);
    
    /* Complex arithmetic spreading values */
    s = a + b + c + d + e + f;
    t = g - h + i - j;
    u = k * l / (m + 1);
    v = n % (o + 1) ^ p;
    w = q | r << 2;
    x = s >> 1 & t;
    y = u + v * w;
    z = x ^ y | z;
    
    /* Force all variables live */
    FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
    FORCE_USE(e); FORCE_USE(f); FORCE_USE(g); FORCE_USE(h);
    FORCE_USE(i); FORCE_USE(j); FORCE_USE(k); FORCE_USE(l);
    FORCE_USE(m); FORCE_USE(n); FORCE_USE(o); FORCE_USE(p);
    FORCE_USE(q); FORCE_USE(r); FORCE_USE(s); FORCE_USE(t);
    FORCE_USE(u); FORCE_USE(v); FORCE_USE(w); FORCE_USE(x);
    FORCE_USE(y); FORCE_USE(z);
    
    return (a + b + c + d + e + f + g + h + i + j + k + l + 
            m + n + o + p + q + r + s + t + u + v + w + x + y + z) & 0xFFFF;
}

/* Main driver with profile feedback */
int main(int argc, char *argv[]) {
    volatile int total_result = 0;
    int i, iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 10000) iterations = 10000;
    }
    
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
    }
    
    if (verbose) {
        printf("Starting MCF stress test with %d iterations\n", iterations);
    }
    
    /* Check CPU features to engage target-specific allocation */
    int has_avx2 = 0;
#ifdef __GNUC__
    has_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", has_avx2 ? "yes" : "no");
    }
#endif
    
    /* Call each pattern multiple times with different arguments */
    for (i = 0; i < iterations; i++) {
        int r1 = pattern_a_entry_exit(i % 50 + 10, i * 3 + 1);
        int r2 = pattern_b_new_indices(i % 5, 4);
        int r3 = pattern_c_mixed_pressure(i % 7, i % 30 + 20);
        int r4 = pattern_d_register_conflict(i * 7 + 11);
        
        total_result += r1 + r2 + r3 + r4;
        
        if (verbose && i % (iterations / 10) == 0) {
            printf("Iteration %d: results = %d, %d, %d, %d (total = %d)\n", 
                   i, r1, r2, r3, r4, total_result);
        }
        
        /* Create varying control flow in main */
        switch (i % 8) {
            case 0: total_result ^= 0x55AA; break;
            case 1: total_result += i * 3; break;
            case 2: total_result -= i * 2; break;
            case 3: total_result *= (i % 7 + 1); break;
            case 4: total_result /= (i % 5 + 1); break;
            case 5: total_result |= i; break;
            case 6: total_result &= ~i; break;
            case 7: total_result = (total_result << 3) | (total_result >> 29); break;
        }
    }
    
    if (verbose) {
        printf("Final result: %d\n", total_result);
    }
    
    /* Prevent optimization */
    FORCE_USE(total_result);
    
    return total_result & 0xFF;
}
