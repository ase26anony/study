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
    
    /* Label to create irreducible region */
    irreducible_region:
    
    for (i = 0; i < iterations; i++) {
        /* Deeply nested if-else chain */
        if (r0 % 3 == 0) {
            if (r1 % 5 == 0) {
                for (j = 0; j < 5; j++) {
                    r2 = r2 * 1103515245 + 12345;
                    if (r2 % 7 == 0) goto irreducible_region;
                }
            } else if (r1 % 7 == 0) {
                r3 = r3 ^ r4;
                r4 = r4 ^ r3;
                r3 = r3 ^ r4;
            } else {
                /* Another level of nesting */
                if (r5 > r6) {
                    r5 = r5 - r6;
                    goto reduce_values;
                } else {
                    r6 = r6 - r5;
                }
            }
        } else if (r0 % 5 == 0) {
            /* Complex loop with continue to different points */
            for (k = 0; k < 10; k++) {
                if (k % 2 == 0) continue;
                r7 = r7 * 1664525 + 1013904223;
                if (r7 % 11 == 0) break;
            }
        }
        
        reduce_values:
        /* Arithmetic on all variables to keep them live */
        r8 = r0 + r1 + r2;
        r9 = r3 * r4 - r5;
        r10 = r6 ^ r7 ^ r8;
        r11 = r9 % (r10 + 1);
        r12 = r11 << (r0 & 3);
        r13 = r12 >> (r1 & 3);
        r14 = r13 | r14;
        r15 = r15 * 6364136223846793005ULL + 1442695040888963407ULL;
        
        /* Force all variables to be considered live */
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
        FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    }
    
    result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
             r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    return result & 0x7FFFFFFF;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
static volatile int jump_counter = 0;

NOINLINE
int pattern_b_new_indices(int depth, int max_depth) {
    /* Function using setjmp/longjmp to create exceptional edges */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        for (int iter = 0; iter < 100; iter++) {
            /* Complex arithmetic to create register pressure */
            a = a * 1103515245 + 12345;
            b = b ^ a;
            c = c + b * 3;
            d = d - c / 2;
            e = e * 1664525 + 1013904223;
            f = f ^ e;
            g = g + f * 7;
            h = h - g / 3;
            i = i * 6364136223846793005ULL + 1442695040888963407ULL;
            j = j ^ i;
            k = k + j * 11;
            l = l - k / 5;
            m = m * 1103515245 + 12345;
            n = n ^ m;
            o = o + n * 13;
            p = p - o / 7;
            
            /* Force all variables live */
            FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
            FORCE_USE(e); FORCE_USE(f); FORCE_USE(g); FORCE_USE(h);
            FORCE_USE(i); FORCE_USE(j); FORCE_USE(k); FORCE_USE(l);
            FORCE_USE(m); FORCE_USE(n); FORCE_USE(o); FORCE_USE(p);
            
            /* Occasionally longjmp to create exceptional edge */
            if (iter % 23 == 0 && depth < max_depth) {
                jump_counter++;
                longjmp(jump_buffer, 1);
            }
        }
    } else {
        /* After longjmp */
        if (depth < max_depth) {
            return pattern_b_new_indices(depth + 1, max_depth);
        }
    }
    
    return (a + b + c + d + e + f + g + h + 
            i + j + k + l + m + n + o + p) & 0x7FFFFFFF;
}

/* ========== PATTERN C: MIXED PRESSURE with vector ops ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int selector, int iterations) {
    /* Combine large switch with vector operations */
    int result = 0;
    
    /* Scalar variables */
    int s0 = 1, s1 = 2, s2 = 3, s3 = 4, s4 = 5, s5 = 6, s6 = 7, s7 = 8;
    int s8 = 9, s9 = 10, s10 = 11, s11 = 12, s12 = 13, s13 = 14, s14 = 15, s15 = 16;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0 = {1, 2, 3, 4};
    v4si v1 = {5, 6, 7, 8};
    v4si v2 = {9, 10, 11, 12};
    v4si v3 = {13, 14, 15, 16};
#endif
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Large switch statement with 30+ cases */
        switch ((selector + iter) % 35) {
            case 0:
                s0 = s0 * 3 + s1;
                s1 = s1 * 5 + s2;
#ifdef __GNUC__
                v0 = v0 + v1;
#endif
                break;
            case 1:
                s2 = s2 ^ s3;
                s3 = s3 | s4;
#ifdef __GNUC__
                v1 = v1 * v2;
#endif
                break;
            case 2:
                s4 = s4 + s5 * 7;
                s5 = s5 - s6 / 3;
                break;
            case 3:
                s6 = s6 << (s7 & 3);
                s7 = s7 >> (s8 & 3);
#ifdef __GNUC__
                v2 = v2 + v3;
#endif
                break;
            case 4:
                s8 = s8 * 1103515245 + 12345;
                s9 = s9 ^ s8;
                break;
            case 5:
                s10 = s10 + s11;
                s11 = s11 - s10;
                break;
            case 6:
                s12 = s12 * 1664525 + 1013904223;
                s13 = s13 ^ s12;
#ifdef __GNUC__
                v3 = v3 * v0;
#endif
                break;
            case 7:
                s14 = s14 | s15;
                s15 = s15 & s0;
                break;
            case 8:
                s0 = s1 + s2;
                s1 = s3 - s4;
                break;
            case 9:
                s2 = s5 * s6;
                s3 = s7 / (s8 + 1);
                break;
            case 10:
                s4 = s9 ^ s10;
                s5 = s11 | s12;
                break;
            case 11:
                s6 = s13 << 2;
                s7 = s14 >> 1;
                break;
            case 12:
                s8 = s15 * 3;
                s9 = s0 + 1;
                break;
            case 13:
                s10 = s1 - 2;
                s11 = s2 * 5;
                break;
            case 14:
                s12 = s3 / 7;
                s13 = s4 ^ 0xFF;
                break;
            case 15:
                s14 = s5 | 0xAA;
                s15 = s6 & 0x55;
                break;
            case 16:
                s0 = s7 + s8;
                s1 = s9 - s10;
                break;
            case 17:
                s2 = s11 * s12;
                s3 = s13 % (s14 + 1);
                break;
            case 18:
                s4 = s15 ^ s0;
                s5 = s1 | s2;
                break;
            case 19:
                s6 = s3 << (s4 & 3);
                s7 = s5 >> (s6 & 3);
                break;
            case 20:
                s8 = s7 * 6364136223846793005ULL;
                s9 = s8 + 1442695040888963407ULL;
                break;
            case 21:
                s10 = s9 ^ s10;
                s11 = s11 + s12;
                break;
            case 22:
                s12 = s13 - s14;
                s13 = s15 * s0;
                break;
            case 23:
                s14 = s1 / (s2 + 1);
                s15 = s3 % (s4 + 1);
                break;
            case 24:
                s0 = s5 ^ s6;
                s1 = s7 | s8;
                break;
            case 25:
                s2 = s9 << 1;
                s3 = s10 >> 2;
                break;
            case 26:
                s4 = s11 * 7;
                s5 = s12 + 11;
                break;
            case 27:
                s6 = s13 - 13;
                s7 = s14 * 17;
                break;
            case 28:
                s8 = s15 / 19;
                s9 = s0 ^ 0xCC;
                break;
            case 29:
                s10 = s1 | 0x33;
                s11 = s2 & 0x99;
                break;
            case 30:
                s12 = s3 + s4 + s5;
                s13 = s6 * s7 - s8;
                break;
            case 31:
                s14 = s9 ^ s10 ^ s11;
                s15 = s12 % (s13 + 1);
                break;
            case 32:
                s0 = s14 << (s15 & 3);
                s1 = s0 >> (s1 & 3);
                break;
            case 33:
                s2 = s2 | s3;
                s3 = s4 ^ s5;
                break;
            case 34:
                s4 = s6 * 1103515245;
                s5 = s7 + 12345;
                break;
        }
        
        /* Force all scalars live */
        FORCE_USE(s0); FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3);
        FORCE_USE(s4); FORCE_USE(s5); FORCE_USE(s6); FORCE_USE(s7);
        FORCE_USE(s8); FORCE_USE(s9); FORCE_USE(s10); FORCE_USE(s11);
        FORCE_USE(s12); FORCE_USE(s13); FORCE_USE(s14); FORCE_USE(s15);
        
#ifdef __GNUC__
        /* Force all vectors live */
        FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
#endif
        
        /* Loop control with complex condition */
        if (iter % 7 == 0) continue;
        if (iter % 13 == 0) break;
    }
    
    result = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + 
             s8 + s9 + s10 + s11 + s12 + s13 + s14 + s15;
#ifdef __GNUC__
    /* Extract from vectors */
    int vsum = v0[0] + v0[1] + v0[2] + v0[3] +
               v1[0] + v1[1] + v1[2] + v1[3] +
               v2[0] + v2[1] + v2[2] + v2[3] +
               v3[0] + v3[1] + v3[2] + v3[3];
    result += vsum;
#endif
    
    return result & 0x7FFFFFFF;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
NOINLINE void dummy1(int x) { FORCE_USE(x); }
NOINLINE void dummy2(int x, int y) { FORCE_USE(x); FORCE_USE(y); }
NOINLINE void dummy3(int x, int y, int z) { FORCE_USE(x); FORCE_USE(y); FORCE_USE(z); }

NOINLINE
int pattern_d_artificial_conflict(int param) {
    /* Use explicit register variables to create artificial conflicts */
#ifdef __GNUC__
    register int r10_var asm ("r10") = param;
    register int r11_var asm ("r11") = param + 1;
    /* Note: We can't bind multiple variables to same register in one function,
       but we can create pressure by using many register variables */
#endif
    
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    /* Call dummy functions to split live ranges */
    dummy1(a);
    a = a * 3;
    
    dummy2(b, c);
    b = b + c;
    
    dummy3(d, e, f);
    d = d ^ e ^ f;
    
    /* Complex loop with register pressure */
    for (int iter = 0; iter < 50; iter++) {
        g = g * 1103515245 + 12345;
        h = h ^ g;
        i = i + h * 3;
        j = j - i / 2;
        k = k * 1664525 + 1013904223;
        l = l ^ k;
        m = m + l * 7;
        n = n - m / 3;
        o = o * 6364136223846793005ULL;
        p = p + 1442695040888963407ULL;
        
        /* Mix in register variables */
#ifdef __GNUC__
        r10_var = r10_var * 3 + iter;
        r11_var = r11_var ^ r10_var;
        dummy1(r10_var);
        dummy2(r10_var, r11_var);
#endif
        
        /* Force all variables live */
        FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
        FORCE_USE(e); FORCE_USE(f); FORCE_USE(g); FORCE_USE(h);
        FORCE_USE(i); FORCE_USE(j); FORCE_USE(k); FORCE_USE(l);
        FORCE_USE(m); FORCE_USE(n); FORCE_USE(o); FORCE_USE(p);
        
        /* Complex control flow */
        switch (iter % 8) {
            case 0: continue;
            case 1: a++; break;
            case 2: b--; break;
            case 3: c = c * 2; break;
            case 4: d = d / 2; break;
            case 5: e = e ^ f; break;
            case 6: f = f | g; break;
            case 7: break;
        }
    }
    
    int result = a + b + c + d + e + f + g + h + 
                 i + j + k + l + m + n + o + p;
#ifdef __GNUC__
    result += r10_var + r11_var;
#endif
    
    return result & 0x7FFFFFFF;
}

/* ========== MAIN FUNCTION WITH PGO SUPPORT ========== */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int i, iterations;
    
    if (argc > 1) {
        verbose = 1;
    }
    
    /* Determine iterations based on input or default */
    iterations = (argc > 1) ? atoi(argv[1]) : 1000;
    if (iterations < 10) iterations = 10;
    
    if (verbose) {
        printf("Running MCF coverage test with %d iterations\n", iterations);
        
        /* Check for CPU features to engage target-specific heuristics */
#ifdef __GNUC__
        if (__builtin_cpu_supports("avx2")) {
            printf("AVX2 supported - engaging vector register pressure\n");
        } else if (__builtin_cpu_supports("sse2")) {
            printf("SSE2 supported - engaging vector register pressure\n");
        }
#endif
    }
    
    /* Run all patterns multiple times to generate profile data */
    for (i = 0; i < iterations; i++) {
        /* Pattern A: ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(10 + (i % 5), i);
        
        /* Pattern B: NEW_EXIT/NEW_ENTRY with setjmp */
        if (i % 7 == 0) {
            total += pattern_b_new_indices(0, 3);
        }
        
        /* Pattern C: Mixed pressure with vectors */
        total += pattern_c_mixed_pressure(i, 5 + (i % 3));
        
        /* Pattern D: Artificial conflict */
        if (i % 3 == 0) {
            total += pattern_d_artificial_conflict(i);
        }
        
        /* Prevent optimization of the loop */
        if (total > 1000000000) {
            total = total % 1000000000;
        }
    }
    
    if (verbose) {
        printf("Final result: %d\n", total);
    }
    
    return total & 0xFF;
}
