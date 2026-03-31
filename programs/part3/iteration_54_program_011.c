/* test_mcf.c - Program to stress GCC's Min-Cost Flow solver */
/* Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-bbro -c test_mcf.c */

#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define USED __attribute__((used))
#else
#define NOINLINE
#define HOT
#define USED
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

static volatile int global_sink = 0;
static int verbose = 0;

/* ========== PATTERN A: ENTRY/EXIT blocks with irreducible region ========== */
HOT NOINLINE USED
int pattern_a_entry_exit(int iterations, int seed) {
    /* Complex function with irreducible control flow */
    int r0 = seed, r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    int result = 0;
    
    /* Label the first block to encourage ENTRY_BLOCK identification */
    volatile int *entry_marker = &r0;
    
    /* Create irreducible region using goto */
    if (iterations > 100) {
        goto middle;
    }
    
    for (int i = 0; i < iterations; i++) {
        /* Deep if-else chain */
        if (r0 % 2 == 0) {
            r1 = r0 * 3 + 1;
            if (r1 % 3 == 0) {
                r2 = r1 / 3;
                goto middle;
            } else {
                r2 = r1 * 2;
            }
        } else if (r0 % 3 == 0) {
            r3 = r0 / 3;
            if (r3 > 100) {
                r4 = r3 % 17;
                goto end_loop;
            }
        } else if (r0 % 5 == 0) {
            r5 = r0 * 5;
            for (int j = 0; j < 5; j++) {
                r6 += r5 * j;
                if (r6 > 1000) break;
            }
        } else {
            r7 = r0 * 7;
        }
        
    middle:
        /* More complex arithmetic */
        r8 = r1 * r2 + r3 * r4 - r5 * r6;
        r9 = (r7 << 3) | (r8 & 0xFF);
        r10 = r9 ^ r0;
        
        if (r10 == 0) {
            /* Create back-edge to different point */
            continue;
        }
        
        result += r10;
        
        /* Update r0 for next iteration */
        r0 = (r0 * 1103515245 + 12345) & 0x7FFFFFFF;
        
        if (i % 7 == 0) {
            /* Another goto creating irreducible flow */
            goto middle;
        }
    }
    
end_loop:
    /* Force all variables to be considered live */
    asm volatile("" : : "g"(r0), "g"(r1), "g"(r2), "g"(r3), "g"(r4),
                   "g"(r5), "g"(r6), "g"(r7), "g"(r8), "g"(r9), "g"(r10));
    
    return result + *entry_marker;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp/longjmp ========== */
NOINLINE USED
int pattern_b_new_indices(int iterations) {
    jmp_buf env;
    int r0 = 0, r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    int r11 = 11, r12 = 12, r13 = 13, r14 = 14, r15 = 15;
    int result = 0;
    
    if (setjmp(env) == 0) {
        /* First call to setjmp */
        for (int i = 0; i < iterations; i++) {
            /* Complex arithmetic using all variables */
            r0 = i * 3;
            r1 = r0 * r0 - r0 + 1;
            r2 = r1 % 17;
            r3 = r2 * r2 + r1;
            r4 = r3 ^ r0;
            r5 = r4 | r1;
            r6 = r5 & 0xFFFF;
            r7 = r6 * 7;
            r8 = r7 / 3;
            r9 = r8 << 2;
            r10 = r9 >> 1;
            r11 = r10 + r0;
            r12 = r11 - r1;
            r13 = r12 * r2;
            r14 = r13 % 19;
            r15 = r14 ^ 0xAA;
            
            result += r15;
            
            /* Occasionally longjmp out */
            if (i > 0 && (i % 13 == 0)) {
                longjmp(env, 1);
            }
            
            /* Force register pressure */
            asm volatile("" : : "g"(r0), "g"(r1), "g"(r2), "g"(r3), "g"(r4),
                       "g"(r5), "g"(r6), "g"(r7), "g"(r8), "g"(r9), "g"(r10),
                       "g"(r11), "g"(r12), "g"(r13), "g"(r14), "g"(r15));
        }
    } else {
        /* longjmp target */
        result = -result;
    }
    
    return result;
}

/* ========== PATTERN C: Mixed pressure with vector operations ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE USED
int pattern_c_mixed_pressure(int iterations, int selector) {
    int r0 = 0, r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    int r11 = 11, r12 = 12, r13 = 13, r14 = 14, r15 = 15;
    int result = 0;
    
#ifdef __GNUC__
    v4si vec0 = {0, 1, 2, 3};
    v4si vec1 = {4, 5, 6, 7};
    v4si vec2 = {8, 9, 10, 11};
    v4si vec3 = {12, 13, 14, 15};
#endif
    
    for (int i = 0; i < iterations; i++) {
        /* Large switch with 30+ cases */
        switch ((selector + i) % 35) {
            case 0:
                r0 = i * 2;
                r1 = r0 + 1;
#ifdef __GNUC__
                vec0 += vec1;
#endif
                break;
            case 1:
                r2 = i * 3;
                r3 = r2 - 1;
#ifdef __GNUC__
                vec1 *= vec0;
#endif
                break;
            case 2:
                r4 = i * 5;
                r5 = r4 % 17;
#ifdef __GNUC__
                vec2 = vec0 + vec1;
#endif
                break;
            case 3:
                r6 = i * 7;
                r7 = r6 & 0xFF;
#ifdef __GNUC__
                vec3 = vec1 - vec2;
#endif
                break;
            case 4:
                r8 = i * 11;
                r9 = r8 | 0xAA;
#ifdef __GNUC__
                vec0 = vec2 * vec3;
#endif
                break;
            case 5:
                r10 = i * 13;
                r11 = r10 ^ 0x55;
#ifdef __GNUC__
                vec1 = vec3 + vec0;
#endif
                break;
            case 6:
                r12 = i * 17;
                r13 = r12 << 2;
#ifdef __GNUC__
                vec2 = vec0 - vec1;
#endif
                break;
            case 7:
                r14 = i * 19;
                r15 = r14 >> 1;
#ifdef __GNUC__
                vec3 = vec1 * vec2;
#endif
                break;
            /* 27 more cases... */
            case 8: r0 = r1 + r2; r3 = r4 - r5; break;
            case 9: r6 = r7 * r8; r9 = r10 % 11; break;
            case 10: r11 = r12 ^ r13; r14 = r15 | 0xFF; break;
            case 11: r0 = r1 << r2; r3 = r4 >> r5; break;
            case 12: r6 = r7 & r8; r9 = r10 * 3; break;
            case 13: r11 = r12 + 17; r13 = r14 - 19; break;
            case 14: r15 = r0 * r1; r2 = r3 / 7; break;
            case 15: r4 = r5 % 13; r6 = r7 ^ 0xAA; break;
            case 16: r8 = r9 | r10; r11 = r12 & 0xF0; break;
            case 17: r13 = r14 << 3; r15 = r0 >> 2; break;
            case 18: r1 = r2 * 5; r3 = r4 + 11; break;
            case 19: r5 = r6 - 7; r7 = r8 % 19; break;
            case 20: r9 = r10 ^ r11; r12 = r13 | r14; break;
            case 21: r15 = r0 & r1; r2 = r3 * 2; break;
            case 22: r4 = r5 + r6; r7 = r8 - r9; break;
            case 23: r10 = r11 * r12; r13 = r14 % 23; break;
            case 24: r15 = r0 ^ 0x33; r1 = r2 | 0xCC; break;
            case 25: r3 = r4 << 1; r5 = r6 >> 3; break;
            case 26: r7 = r8 & r9; r10 = r11 + 29; break;
            case 27: r12 = r13 - 31; r14 = r15 % 37; break;
            case 28: r0 = r1 * r2; r3 = r4 ^ r5; break;
            case 29: r6 = r7 | r8; r9 = r10 & 0x0F; break;
            case 30: r11 = r12 << 4; r13 = r14 >> 2; break;
            case 31: r15 = r0 + 41; r1 = r2 - 43; break;
            case 32: r3 = r4 * 47; r5 = r6 % 53; break;
            case 33: r7 = r8 ^ 0x99; r9 = r10 | 0x66; break;
            case 34:
                r11 = r12 & r13;
                r14 = r15 * 59;
                /* Use continue to create complex back-edge */
                if (i % 3 == 0) continue;
                break;
        }
        
        /* Mix vector and scalar results */
#ifdef __GNUC__
        int sum = vec0[0] + vec0[1] + vec0[2] + vec0[3];
        result += sum + r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
#else
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
#endif
        
        /* Force all variables live */
        asm volatile("" : : "g"(r0), "g"(r1), "g"(r2), "g"(r3), "g"(r4),
                   "g"(r5), "g"(r6), "g"(r7), "g"(r8), "g"(r9), "g"(r10),
                   "g"(r11), "g"(r12), "g"(r13), "g"(r14), "g"(r15));
    }
    
    return result;
}

/* ========== PATTERN D: Artificial conflict with register variables ========== */
NOINLINE USED
int dummy1(int x) { return x * 2; }
NOINLINE USED
int dummy2(int x, int y) { return x + y * 3; }
NOINLINE USED
int dummy3(int x, int y, int z) { return x * y - z; }

NOINLINE USED
int pattern_d_register_conflict(int iterations) {
    /* Use explicit register variables to create conflicts */
#ifdef __GNUC__
    register int x asm ("r10");
    register int y asm ("r11");
    register int z asm ("r12");
    register int w asm ("r13");
#else
    int x, y, z, w;
#endif
    
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    int result = 0;
    
    x = iterations;
    y = x * 2;
    z = y + 1;
    w = z % 17;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Call dummy functions to split live ranges */
        a = dummy1(x);
        b = dummy2(y, a);
        c = dummy3(z, b, c);
        d = dummy1(w);
        e = dummy2(a, d);
        f = dummy3(b, e, f);
        g = dummy1(c);
        h = dummy2(d, g);
        i = dummy3(e, h, i);
        j = dummy1(f);
        k = dummy2(g, j);
        l = dummy3(h, k, l);
        m = dummy1(i);
        n = dummy2(j, m);
        o = dummy3(k, n, o);
        p = dummy1(l);
        
        /* Update register variables */
        x = a + b;
        y = c + d;
        z = e + f;
        w = g + h;
        
        /* More arithmetic */
        a = x * y - z;
        b = y * z + w;
        c = z * w ^ x;
        d = w * x | y;
        e = a + b + c + d;
        f = e * 2;
        g = f / 3;
        h = g % 5;
        i = h << 2;
        j = i >> 1;
        k = j & 0xFF;
        l = k | 0xAA;
        m = l ^ 0x55;
        n = m + 1;
        o = n - 2;
        p = o * 3;
        
        result += p;
        
        /* Force all variables live */
        asm volatile("" : : "g"(x), "g"(y), "g"(z), "g"(w),
                   "g"(a), "g"(b), "g"(c), "g"(d), "g"(e), "g"(f),
                   "g"(g), "g"(h), "g"(i), "g"(j), "g"(k), "g"(l),
                   "g"(m), "g"(n), "g"(o), "g"(p));
    }
    
    return result;
}

/* ========== MAIN FUNCTION with PGO support ========== */
int main(int argc, char **argv) {
    int total = 0;
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
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
    
    /* Call each pattern multiple times with different arguments */
    for (int i = 0; i < 10; i++) {
        total += pattern_a_entry_exit(iterations / 10 + i * 10, i * 12345);
        total += pattern_b_new_indices(iterations / 20 + i * 5);
        total += pattern_c_mixed_pressure(iterations / 30 + i * 3, i);
        total += pattern_d_register_conflict(iterations / 40 + i * 2);
        
        /* Prevent optimization */
        global_sink = total;
    }
    
    if (verbose) {
        printf("Total result: %d\n", total);
    }
    
    return total == 0 ? 0 : 1;
}
