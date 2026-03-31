/* test_mcf.c - Complex CFG generator for GCC MCF pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define USED __attribute__((used))
#define KEEP(var) asm volatile("" : : "g"(var))
#else
#define NOINLINE
#define HOT
#define USED
#define KEEP(var) (void)(var)
#endif

/* Global to prevent optimization */
volatile int global_sink = 0;
static int verbose = 0;

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE USED
int pattern_a_entry_exit(int iterations, int seed) {
    /* Complex irreducible CFG with goto to create ENTRY_BLOCK+1 and 2*EXIT_BLOCK */
    int r0 = seed, r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10, r11 = 11;
    int r12 = 12, r13 = 13, r14 = 14, r15 = 15;
    int result = 0;
    
    /* Create irreducible region with goto */
    if (iterations > 1000) {
        goto irreducible_label;
    }
    
    for (int i = 0; i < iterations; i++) {
        /* Deep if-else chain creating many basic blocks */
        if (r0 & 1) {
            r1 = r0 * 3 + 1;
            if (r1 & 1) {
                r2 = r1 ^ r0;
                if (r2 > 100) {
                    r3 = r2 % 17;
                } else {
                    r3 = r2 * 11;
                }
            } else {
                r2 = r1 + r0;
                if (r2 < 50) {
                    r3 = r2 << 2;
                } else {
                    r3 = r2 >> 1;
                }
            }
        } else if (r0 & 2) {
            r1 = r0 / 2;
            for (int j = 0; j < 5; j++) {
                r2 += j * r1;
                if (r2 & 4) break;
            }
            r3 = r2 ^ r1;
        } else if (r0 & 4) {
            r1 = r0 | 0xFF;
            while (r1 > 0) {
                r2 += r1 & 1;
                r1 >>= 1;
                if (r2 > 100) continue;
                r3 += r2;
            }
        } else {
            r1 = r0 & 0x0F;
            do {
                r2 = r1 * r1;
                r1--;
                if (r1 == 0) break;
                r3 += r2;
            } while (r1 > 0);
        }
        
        /* More disjoint paths */
        switch (r3 % 8) {
            case 0: r4 = r0 + r1; r5 = r2 * r3; break;
            case 1: r4 = r0 - r1; r5 = r2 / (r3 ? r3 : 1); break;
            case 2: r4 = r0 ^ r1; r5 = r2 | r3; break;
            case 3: r4 = r0 & r1; r5 = r2 & r3; break;
            case 4: r4 = r0 | r1; r5 = r2 ^ r3; break;
            case 5: r4 = r0 * r1; r5 = r2 + r3; break;
            case 6: r4 = r0 / (r1 ? r1 : 1); r5 = r2 - r3; break;
            case 7: r4 = r0 << (r1 & 3); r5 = r2 >> (r3 & 3); break;
        }
        
        /* Force all variables live */
        KEEP(r0); KEEP(r1); KEEP(r2); KEEP(r3);
        KEEP(r4); KEEP(r5); KEEP(r6); KEEP(r7);
        KEEP(r8); KEEP(r9); KEEP(r10); KEEP(r11);
        KEEP(r12); KEEP(r13); KEEP(r14); KEEP(r15);
        
        result += r4 + r5;
        r0 = (r0 * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Create back-edge to different case */
        if (i % 7 == 0) {
            continue;
        } else if (i % 13 == 0) {
            break;
        }
    }
    
irreducible_label:
    /* This creates an irreducible region */
    if (r0 & 8) {
        goto back_to_loop;
    }
    
    result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
             r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    
back_to_loop:
    return result;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
NOINLINE USED
int pattern_b_new_indices(int depth, int value) {
    int vars[32];
    for (int i = 0; i < 32; i++) {
        vars[i] = value + i;
    }
    
    /* Many scalar variables to pressure registers */
    int a0 = vars[0], a1 = vars[1], a2 = vars[2], a3 = vars[3];
    int a4 = vars[4], a5 = vars[5], a6 = vars[6], a7 = vars[7];
    int b0 = vars[8], b1 = vars[9], b2 = vars[10], b3 = vars[11];
    int b4 = vars[12], b5 = vars[13], b6 = vars[14], b7 = vars[15];
    int c0 = vars[16], c1 = vars[17], c2 = vars[18], c3 = vars[19];
    int c4 = vars[20], c5 = vars[21], c6 = vars[22], c7 = vars[23];
    int d0 = vars[24], d1 = vars[25], d2 = vars[26], d3 = vars[27];
    int d4 = vars[28], d5 = vars[29], d6 = vars[30], d7 = vars[31];
    
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* Complex loop with exceptional edge potential */
        for (int i = 0; i < depth; i++) {
            /* Heavy arithmetic on all variables */
            a0 = a0 * 3 + a1; a1 = a1 ^ a2; a2 = a2 + a3; a3 = a3 | a4;
            a4 = a4 & a5; a5 = a5 - a6; a6 = a6 * a7; a7 = a7 / (a0 ? a0 : 1);
            
            b0 = b0 << (b1 & 3); b1 = b1 >> (b2 & 3); b2 = b2 + b3;
            b3 = b3 - b4; b4 = b4 * b5; b5 = b5 ^ b6; b6 = b6 | b7;
            b7 = b7 & b0;
            
            c0 = c0 + c1 * c2; c1 = c1 - c3 / (c4 ? c4 : 1);
            c2 = c2 ^ c5; c3 = c3 | c6; c4 = c4 & c7;
            c5 = c5 * c0; c6 = c6 + c1; c7 = c7 - c2;
            
            d0 = d0 * d1 + d2; d1 = d1 ^ d3 | d4;
            d2 = d2 & d5 + d6; d3 = d3 - d7 * d0;
            d4 = d4 | d1 & d2; d5 = d5 ^ d3 | d4;
            d6 = d6 + d5 * d0; d7 = d7 - d1 & d2;
            
            /* Force all variables live across loop */
            KEEP(a0); KEEP(a1); KEEP(a2); KEEP(a3); KEEP(a4); KEEP(a5); KEEP(a6); KEEP(a7);
            KEEP(b0); KEEP(b1); KEEP(b2); KEEP(b3); KEEP(b4); KEEP(b5); KEEP(b6); KEEP(b7);
            KEEP(c0); KEEP(c1); KEEP(c2); KEEP(c3); KEEP(c4); KEEP(c5); KEEP(c6); KEEP(c7);
            KEEP(d0); KEEP(d1); KEEP(d2); KEEP(d3); KEEP(d4); KEEP(d5); KEEP(d6); KEEP(d7);
            
            result += a0 + b0 + c0 + d0;
            
            /* Potential longjmp creates exceptional edge */
            if (i == depth / 2 && value > 1000) {
                longjmp(jump_buffer, 1);
            }
        }
    } else {
        /* Handler path - different register pressure */
        result = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 +
                 b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 +
                 c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7 +
                 d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7;
    }
    
    return result;
}

/* ========== PATTERN C: MIXED PRESSURE with vector ops ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE USED
int pattern_c_mixed_pressure(int selector, int count) {
    int r0 = selector, r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10, r11 = 11;
    int r12 = 12, r13 = 13, r14 = 14, r15 = 15;
    int result = 0;
    
#ifdef __GNUC__
    /* Vector variables for additional register pressure */
    v4si v0 = {r0, r1, r2, r3};
    v4si v1 = {r4, r5, r6, r7};
    v4si v2 = {r8, r9, r10, r11};
    v4si v3 = {r12, r13, r14, r15};
#endif
    
    /* Large switch with 30+ cases */
    for (int i = 0; i < count; i++) {
        switch ((r0 + i) % 35) {
            case 0:
                r1 = r0 * 3 + 1; r2 = r1 ^ r0; r3 = r2 % 17;
                r4 = r3 << 2; r5 = r4 | r0; r6 = r5 & 0xFF;
#ifdef __GNUC__
                v0 = v0 + v1; v1 = v1 * v2;
#endif
                break;
            case 1:
                r1 = r0 / 2; r2 = r1 + r0; r3 = r2 * 11;
                r4 = r3 >> 1; r5 = r4 ^ r1; r6 = r5 | r2;
#ifdef __GNUC__
                v0 = v0 - v1; v2 = v2 | v3;
#endif
                break;
            case 2:
                r1 = r0 | 0xFF; r2 = r1 & r0; r3 = r2 * 3;
                r4 = r3 + r1; r5 = r4 ^ r2; r6 = r5 % 19;
#ifdef __GNUC__
                v1 = v1 ^ v2; v3 = v3 + v0;
#endif
                break;
            case 3:
                r1 = r0 & 0x0F; r2 = r1 * r1; r3 = r2 - r0;
                r4 = r3 / (r1 ? r1 : 1); r5 = r4 << 3; r6 = r5 | r2;
                break;
            case 4:
                r1 = r0 ^ 0x55; r2 = r1 + r0; r3 = r2 & 0xAA;
                r4 = r3 * 5; r5 = r4 - r1; r6 = r5 ^ r3;
                break;
            case 5:
                r1 = r0 * 7; r2 = r1 % 23; r3 = r2 | r0;
                r4 = r3 << 1; r5 = r4 & 0xF0; r6 = r5 + r2;
                break;
            case 6:
                r1 = r0 + 100; r2 = r1 / 3; r3 = r2 ^ r0;
                r4 = r3 * 9; r5 = r4 % 29; r6 = r5 | r1;
                break;
            case 7:
                r1 = r0 - 50; r2 = r1 * r0; r3 = r2 >> 2;
                r4 = r3 & 0x1F; r5 = r4 + r1; r6 = r5 ^ r3;
                break;
            case 8:
                r1 = r0 | 0x33; r2 = r1 & 0xCC; r3 = r2 * 13;
                r4 = r3 - r0; r5 = r4 % 31; r6 = r5 << 2;
                break;
            case 9:
                r1 = r0 ^ 0x99; r2 = r1 + 77; r3 = r2 / 7;
                r4 = r3 | r0; r5 = r4 & 0x88; r6 = r5 * 3;
                break;
            /* 25 more cases to reach 35 total */
            case 10: r1 = r0 * 2; r2 = r1 + 1; r3 = r2 ^ r0; r4 = r3; r5 = r4; r6 = r5; break;
            case 11: r1 = r0 / 3; r2 = r1 * 4; r3 = r2 - r0; r4 = r3; r5 = r4; r6 = r5; break;
            case 12: r1 = r0 | 1; r2 = r1 & 2; r3 = r2 ^ 3; r4 = r3; r5 = r4; r6 = r5; break;
            case 13: r1 = r0 << 1; r2 = r1 >> 2; r3 = r2 | r0; r4 = r3; r5 = r4; r6 = r5; break;
            case 14: r1 = r0 + 2; r2 = r1 * 3; r3 = r2 - 4; r4 = r3; r5 = r4; r6 = r5; break;
            case 15: r1 = r0 ^ 5; r2 = r1 & 6; r3 = r2 | 7; r4 = r3; r5 = r4; r6 = r5; break;
            case 16: r1 = r0 * 6; r2 = r1 / 7; r3 = r2 + 8; r4 = r3; r5 = r4; r6 = r5; break;
            case 17: r1 = r0 | 9; r2 = r1 ^ 10; r3 = r2 & 11; r4 = r3; r5 = r4; r6 = r5; break;
            case 18: r1 = r0 + 12; r2 = r1 - 13; r3 = r2 * 14; r4 = r3; r5 = r4; r6 = r5; break;
            case 19: r1 = r0 & 15; r2 = r1 | 16; r3 = r2 ^ 17; r4 = r3; r5 = r4; r6 = r5; break;
            case 20: r1 = r0 << 3; r2 = r1 >> 4; r3 = r2 + 18; r4 = r3; r5 = r4; r6 = r5; break;
            case 21: r1 = r0 * 19; r2 = r1 % 20; r3 = r2 | 21; r4 = r3; r5 = r4; r6 = r5; break;
            case 22: r1 = r0 ^ 22; r2 = r1 & 23; r3 = r2 + 24; r4 = r3; r5 = r4; r6 = r5; break;
            case 23: r1 = r0 | 25; r2 = r1 - 26; r3 = r2 * 27; r4 = r3; r5 = r4; r6 = r5; break;
            case 24: r1 = r0 + 28; r2 = r1 / 29; r3 = r2 ^ 30; r4 = r3; r5 = r4; r6 = r5; break;
            case 25: r1 = r0 & 31; r2 = r1 | 32; r3 = r2 - 33; r4 = r3; r5 = r4; r6 = r5; break;
            case 26: r1 = r0 << 5; r2 = r1 >> 6; r3 = r2 & 34; r4 = r3; r5 = r4; r6 = r5; break;
            case 27: r1 = r0 * 35; r2 = r1 + 36; r3 = r2 % 37; r4 = r3; r5 = r4; r6 = r5; break;
            case 28: r1 = r0 ^ 38; r2 = r1 | 39; r3 = r2 & 40; r4 = r3; r5 = r4; r6 = r5; break;
            case 29: r1 = r0 + 41; r2 = r1 * 42; r3 = r2 - 43; r4 = r3; r5 = r4; r6 = r5; break;
            case 30: r1 = r0 & 44; r2 = r1 ^ 45; r3 = r2 | 46; r4 = r3; r5 = r4; r6 = r5; break;
            case 31: r1 = r0 << 7; r2 = r1 >> 8; r3 = r2 + 47; r4 = r3; r5 = r4; r6 = r5; break;
            case 32: r1 = r0 * 48; r2 = r1 / 49; r3 = r2 % 50; r4 = r3; r5 = r4; r6 = r5; break;
            case 33: r1 = r0 ^ 51; r2 = r1 & 52; r3 = r2 | 53; r4 = r3; r5 = r4; r6 = r5; break;
            case 34:
                r1 = r0 + 54; r2 = r1 - 55; r3 = r2 * 56;
                r4 = r3 ^ 57; r5 = r4 & 58; r6 = r5 | 59;
#ifdef __GNUC__
                v0 = v0 * v1 + v2 - v3;
                v1 = v1 ^ v2 | v3 & v0;
#endif
                break;
        }
        
        /* Force all variables live */
        KEEP(r0); KEEP(r1); KEEP(r2); KEEP(r3); KEEP(r4); KEEP(r5); KEEP(r6); KEEP(r7);
        KEEP(r8); KEEP(r9); KEEP(r10); KEEP(r11); KEEP(r12); KEEP(r13); KEEP(r14); KEEP(r15);
#ifdef __GNUC__
        KEEP(v0); KEEP(v1); KEEP(v2); KEEP(v3);
#endif
        
        result += r1 + r2 + r3 + r4 + r5 + r6;
        
        /* Complex loop control */
        if (i % 11 == 0) {
            continue;
        } else if (i % 17 == 0) {
            break;
        }
        
        r0 = (r0 * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
#ifdef __GNUC__
    /* Use vector results */
    int vsum = v0[0] + v0[1] + v0[2] + v0[3] +
               v1[0] + v1[1] + v1[2] + v1[3] +
               v2[0] + v2[1] + v2[2] + v2[3] +
               v3[0] + v3[1] + v3[2] + v3[3];
    result += vsum;
#endif
    
    return result;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
#ifdef __GNUC__
register int conflict_var1 asm ("r10");
register int conflict_var2 asm ("r10");
#else
int conflict_var1, conflict_var2;
#endif

NOINLINE static void dummy1(int x) { KEEP(x); }
NOINLINE static void dummy2(int x) { KEEP(x); }
NOINLINE static void dummy3(int x) { KEEP(x); }

NOINLINE USED
int pattern_d_artificial_conflict(int n) {
    /* Force register conflict */
#ifdef __GNUC__
    conflict_var1 = n;
    conflict_var2 = n * 2;
    
    /* Use conflicting register in different ways */
    int temp = conflict_var1;
    conflict_var1 = conflict_var2;
    conflict_var2 = temp;
#endif
    
    int array[64];
    for (int i = 0; i < 64; i++) {
        array[i] = i * n;
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Many live ranges that need splitting */
        int a = array[i];
        int b = array[i + 1];
        int c = array[i + 2];
        int d = array[i + 3];
        int e = array[i + 4];
        int f = array[i + 5];
        int g = array[i + 6];
        int h = array[i + 7];
        
        /* Call dummy functions to create call boundaries */
        dummy1(a);
        dummy2(b);
        dummy3(c);
        
        /* Complex computation with many intermediates */
        int t1 = a * b + c;
        int t2 = d - e * f;
        int t3 = g ^ h | a;
        int t4 = (b + c) & (d ^ e);
        int t5 = f * g - h;
        int t6 = (a | b) ^ (c & d);
        int t7 = e + f * g;
        int t8 = h - a ^ b;
        
        /* Force all temporaries live */
        KEEP(t1); KEEP(t2); KEEP(t3); KEEP(t4);
        KEEP(t5); KEEP(t6); KEEP(t7); KEEP(t8);
        
        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
        
#ifdef __GNUC__
        /* More register conflict */
        conflict_var1 = t1;
        conflict_var2 = t2;
        temp = conflict_var1 + conflict_var2;
        conflict_var1 = temp;
        conflict_var2 = temp * 2;
#endif
    }
    
#ifdef __GNUC__
    sum += conflict_var1 + conflict_var2;
#endif
    
    return sum;
}

/* ========== MAIN DRIVER ========== */
int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose = 1;
    }
    
    int result = 0;
    int iterations = 1000;
    
    /* Use profile to guide optimization */
    for (int i = 0; i < iterations; i++) {
        /* Pattern A - ENTRY/EXIT blocks */
        result ^= pattern_a_entry_exit(i % 100 + 50, i * 3);
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        if (i % 7 == 0) {
            result += pattern_b_new_indices(i % 20 + 5, i * 7);
        }
        
        /* Pattern C - Mixed pressure */
        result |= pattern_c_mixed_pressure(i, i % 30 + 10);
        
        /* Pattern D - Artificial conflict */
        if (i % 3 == 0) {
            result += pattern_d_artificial_conflict(i % 40 + 8);
        }
        
        /* Use CPU features to engage target-specific allocation */
#ifdef __GNUC__
        if (__builtin_cpu_supports("avx2")) {
            result += 1;
        }
#endif
    }
    
    /* Prevent optimization */
    global_sink = result;
    
    if (verbose) {
        printf("Final result: %d\n", result);
    }
    
    return 0;
}
