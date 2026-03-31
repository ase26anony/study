/* test_mcf.c - Complex CFG generator for GCC MCF pass testing */
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))
#define USED __attribute__((used))
#else
#define NOINLINE
#define HOT
#define COLD
#define USED
#endif

static volatile int verbose = 0;

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE USED
int pattern_a_entry_exit(int iterations, int selector) {
    /* Complex function with irreducible region to force ENTRY_BLOCK+1 and 2*EXIT_BLOCK */
    int result = 0;
    int i, j, k;
    int r0 = 0, r1 = 1, r2 = 2, r3 = 3, r4 = 4;
    int r5 = 5, r6 = 6, r7 = 7, r8 = 8, r9 = 9;
    
    /* Create irreducible region with goto */
    if (selector > 100) {
        goto irreducible_label;
    }
    
    for (i = 0; i < iterations; i++) {
        /* Deeply nested if-else chain */
        if (i & 1) {
            if (i & 2) {
                if (i & 4) {
                    r0 = r1 + r2;
                    r3 = r4 - r5;
                } else {
                    r6 = r7 * r8;
                    r9 = r0 ^ r1;
                }
            } else {
                for (j = 0; j < 5; j++) {
                    r2 = r3 << (j & 3);
                    r4 = r5 >> (j & 3);
                    if (j == 3) break;
                }
            }
        } else {
            switch (i % 7) {
                case 0: r0 += r1; break;
                case 1: r1 -= r2; break;
                case 2: r2 *= r3; break;
                case 3: r3 /= (r4 ? r4 : 1); break;
                case 4: r4 ^= r5; break;
                case 5: r5 |= r6; break;
                case 6: r6 &= r7; break;
            }
        }
        
        /* Irreducible region entry */
        irreducible_label:
        for (k = 0; k < 3; k++) {
            r7 = r8 + r9;
            r8 = r9 - r0;
            r9 = r0 * r1;
            if (k == 1 && (i & 8)) {
                goto back_edge_label;
            }
        }
        
        back_edge_label:
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
        
        /* Prevent dead code elimination */
        asm volatile("" : : "g"(r0), "g"(r1), "g"(r2), "g"(r3), "g"(r4),
                         "g"(r5), "g"(r6), "g"(r7), "g"(r8), "g"(r9));
    }
    
    return result;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
NOINLINE USED
int pattern_b_new_indices(int depth, int trigger) {
    int result = 0;
    int vars[20];
    
    /* Initialize many local variables */
    for (int i = 0; i < 20; i++) {
        vars[i] = i * 3 + 1;
    }
    
    /* Complex loop with potential longjmp */
    for (int iter = 0; iter < depth; iter++) {
        int a = vars[0], b = vars[1], c = vars[2], d = vars[3];
        int e = vars[4], f = vars[5], g = vars[6], h = vars[7];
        
        if (setjmp(jump_buffer) == 0) {
            /* Normal path with heavy computation */
            for (int i = 0; i < 100; i++) {
                a = b * c + d;
                b = c / (d ? d : 1) - e;
                c = d ^ e | f;
                d = e << (g & 3);
                e = f >> (h & 3);
                f = g + h * a;
                g = h - a * b;
                h = a ^ b ^ c;
                
                /* Occasionally trigger longjmp */
                if (iter == trigger && i == 50) {
                    longjmp(jump_buffer, 1);
                }
            }
            
            /* Update array with results */
            vars[0] = a; vars[1] = b; vars[2] = c; vars[3] = d;
            vars[4] = e; vars[5] = f; vars[6] = g; vars[7] = h;
        } else {
            /* longjmp target - different computation */
            for (int i = 0; i < 20; i++) {
                vars[i] = vars[i] * 2 + 1;
            }
        }
        
        /* Complex switch to create many basic blocks */
        switch (iter % 13) {
            case 0: result += vars[0]; break;
            case 1: result += vars[1] * 2; break;
            case 2: result += vars[2] / 2; break;
            case 3: result ^= vars[3]; break;
            case 4: result |= vars[4]; break;
            case 5: result &= vars[5]; break;
            case 6: result += vars[6] << 1; break;
            case 7: result += vars[7] >> 1; break;
            case 8: result -= vars[8]; break;
            case 9: result *= (vars[9] ? vars[9] : 1); break;
            case 10: result = ~result; break;
            case 11: result = result ^ vars[10]; break;
            case 12: result = result | vars[11]; break;
        }
        
        /* Force all variables live */
        for (int i = 0; i < 20; i++) {
            asm volatile("" : : "g"(vars[i]));
        }
    }
    
    return result;
}

/* ========== PATTERN C: MIXED PRESSURE with vector ops ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE USED
int pattern_c_mixed_pressure(int count, int seed) {
    int result = seed;
    
#ifdef __GNUC__
    v4si vec0 = {1, 2, 3, 4};
    v4si vec1 = {5, 6, 7, 8};
    v4si vec2 = {9, 10, 11, 12};
    v4si vec3 = {13, 14, 15, 16};
#endif
    
    int r0 = 0, r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10, r11 = 11;
    int r12 = 12, r13 = 13, r14 = 14, r15 = 15;
    
    /* Large switch with 30+ cases inside loop */
    for (int i = 0; i < count; i++) {
        int mod = (result + i) % 35;
        
        switch (mod) {
            case 0:
                r0 = r1 + r2;
                r3 = r4 - r5;
#ifdef __GNUC__
                vec0 = vec1 + vec2;
#endif
                break;
            case 1:
                r6 = r7 * r8;
                r9 = r10 ^ r11;
#ifdef __GNUC__
                vec1 = vec2 * vec3;
#endif
                break;
            case 2:
                r12 = r13 << (r14 & 3);
                r15 = r0 >> (r1 & 3);
                break;
            case 3:
                r1 = r2 * r3 + r4;
                r5 = r6 - r7 * r8;
                break;
            case 4:
                r9 = r10 / (r11 ? r11 : 1);
                r12 = r13 % (r14 ? r14 : 1);
                break;
            case 5:
                r0 = r1 | r2;
                r3 = r4 & r5;
                break;
            case 6:
                r6 = ~r7;
                r8 = r9 ^ r10;
                break;
            case 7:
                r11 = r12 + r13;
                r14 = r15 - r0;
                break;
            case 8:
                r1 = r2 * 3;
                r3 = r4 / 2;
                break;
            case 9:
                r5 = r6 << 1;
                r7 = r8 >> 2;
                break;
            case 10:
                r9 = (r10 > r11) ? r12 : r13;
                r14 = (r15 < r0) ? r1 : r2;
                break;
            case 11:
                r3 = r4 * r5 - r6;
                r7 = r8 + r9 * r10;
                break;
            case 12:
                r11 = r12 ^ r13 ^ r14;
                r15 = r0 | r1 | r2;
                break;
            case 13:
                r3 = r4 & ~r5;
                r6 = r7 | ~r8;
                break;
            case 14:
                r9 = r10 + 1;
                r11 = r12 - 1;
                break;
            case 15:
                r13 = r14 * 2;
                r15 = r0 / 4;
                break;
            case 16:
                r1 = r2 << 3;
                r3 = r4 >> 1;
                break;
            case 17:
                r5 = r6 + r7 + r8;
                r9 = r10 - r11 - r12;
                break;
            case 18:
                r13 = r14 * r15;
                r0 = r1 * r2;
                break;
            case 19:
                r3 = r4 ^ r5 ^ r6;
                r7 = r8 ^ r9 ^ r10;
                break;
            case 20:
                r11 = r12 | r13;
                r14 = r15 | r0;
                break;
            case 21:
                r1 = r2 & r3;
                r4 = r5 & r6;
                break;
            case 22:
                r7 = ~r8;
                r9 = ~r10;
                break;
            case 23:
                r11 = r12 + 5;
                r13 = r14 - 3;
                break;
            case 24:
                r15 = r0 * 7;
                r1 = r2 / 3;
                break;
            case 25:
                r3 = r4 << 2;
                r5 = r6 >> 3;
                break;
            case 26:
                r7 = r8 > r9 ? r10 : r11;
                r12 = r13 < r14 ? r15 : r0;
                break;
            case 27:
                r1 = r2 * r3 / (r4 ? r4 : 1);
                r5 = r6 + r7 - r8;
                break;
            case 28:
                r9 = r10 ^ r11 | r12;
                r13 = r14 & r15 ^ r0;
                break;
            case 29:
                r1 = ~r2 & r3;
                r4 = r5 | ~r6;
                break;
            case 30:
                r7 = r8 + 11;
                r9 = r10 - 7;
                break;
            case 31:
                r11 = r12 * 13;
                r13 = r14 / 5;
                break;
            case 32:
                r15 = r0 << 4;
                r1 = r2 >> 2;
                break;
            case 33:
                r3 = r4 > r5 ? r6 + r7 : r8 - r9;
                r10 = r11 < r12 ? r13 * r14 : r15 / r0;
                break;
            case 34:
                r1 = r2 * r3 + r4 * r5;
                r6 = r7 - r8 + r9 - r10;
                break;
        }
        
        /* Mix vector operations */
#ifdef __GNUC__
        if (i % 5 == 0) {
            vec0 = vec0 + vec1;
            vec1 = vec1 * vec2;
            vec2 = vec2 - vec3;
            vec3 = vec3 + vec0;
        }
        
        /* Extract from vectors to scalars */
        int v0 = vec0[0], v1 = vec0[1], v2 = vec0[2], v3 = vec0[3];
        r0 += v0; r1 += v1; r2 += v2; r3 += v3;
#endif
        
        /* Use continue/break to create complex control flow */
        if (i % 7 == 0) {
            continue;
        } else if (i % 13 == 0) {
            break;
        } else if (i % 17 == 0) {
            i += 2;
            continue;
        }
        
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                  r10 + r11 + r12 + r13 + r14 + r15;
        
        /* Force all variables live */
        asm volatile("" : : "g"(r0), "g"(r1), "g"(r2), "g"(r3), "g"(r4),
                         "g"(r5), "g"(r6), "g"(r7), "g"(r8), "g"(r9),
                         "g"(r10), "g"(r11), "g"(r12), "g"(r13), "g"(r14), "g"(r15));
#ifdef __GNUC__
        asm volatile("" : : "g"(vec0), "g"(vec1), "g"(vec2), "g"(vec3));
#endif
    }
    
    return result;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
NOINLINE USED
int helper1(int a, int b) { return a + b; }
NOINLINE USED
int helper2(int a, int b) { return a - b; }
NOINLINE USED
int helper3(int a, int b) { return a * b; }
NOINLINE USED
int helper4(int a, int b) { return a ^ b; }

NOINLINE USED
int pattern_d_register_conflict(int n) {
    /* Explicit register variables creating conflicts */
#ifdef __GNUC__
    register int x asm ("r10");
    register int y asm ("r11");
    register int z asm ("r12");
#else
    int x, y, z;
#endif
    
    int result = 0;
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    x = n;
    y = n * 2;
    z = n * 3;
    
    /* Loop with calls that clobber registers */
    for (int iter = 0; iter < n; iter++) {
        /* Use register variables in complex expressions */
        x = helper1(x, y);
        y = helper2(y, z);
        z = helper3(z, x);
        
        /* Force spills with many live variables */
        a = helper1(b, c);
        b = helper2(c, d);
        c = helper3(d, e);
        d = helper4(e, f);
        e = helper1(f, g);
        f = helper2(g, h);
        g = helper3(h, i);
        h = helper4(i, j);
        i = helper1(j, a);
        j = helper2(a, b);
        
        /* Complex conditional with goto */
        if (iter % 3 == 0) {
            goto label1;
        } else if (iter % 5 == 0) {
            goto label2;
        }
        
        label1:
        x = x + a + b;
        label2:
        y = y + c + d;
        
        /* Switch creating many basic blocks */
        switch (iter % 11) {
            case 0: result += x; break;
            case 1: result += y; break;
            case 2: result += z; break;
            case 3: result += a; break;
            case 4: result += b; break;
            case 5: result += c; break;
            case 6: result += d; break;
            case 7: result += e; break;
            case 8: result += f; break;
            case 9: result += g; break;
            case 10: result += h; break;
        }
        
        /* Force all variables live */
        asm volatile("" : : "g"(x), "g"(y), "g"(z),
                         "g"(a), "g"(b), "g"(c), "g"(d), "g"(e),
                         "g"(f), "g"(g), "g"(h), "g"(i), "g"(j));
    }
    
    return result + x + y + z;
}

/* ========== MAIN FUNCTION ========== */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int iterations = 100;
    
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
    
    /* Profile-guided optimization will see different paths taken */
    for (int i = 0; i < iterations; i++) {
        /* Call each pattern with different arguments to exercise different paths */
        total += pattern_a_entry_exit(i % 50 + 10, i);
        total += pattern_b_new_indices(i % 20 + 5, i % 10);
        total += pattern_c_mixed_pressure(i % 30 + 15, i);
        total += pattern_d_register_conflict(i % 40 + 20);
        
        /* Use CPU feature detection to engage target-specific allocation */
        if (i % 100 == 0) {
#ifdef __GNUC__
            if (__builtin_cpu_supports("avx2")) {
                total += 1; /* Engage AVX2 register allocation */
            }
#endif
        }
    }
    
    if (verbose) {
        printf("Final result: %d\n", total);
    }
    
    return total != 0 ? 0 : 1;
}
