/* test_mcf.c - Comprehensive test for GCC's Min-Cost Flow pass special nodes */
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

static volatile int global_sink = 0;
static int verbose = 0;

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int selector) {
    /* Complex function with irreducible region to force ENTRY_BLOCK+1 and 2*EXIT_BLOCK */
    int result = 0;
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    /* Create irreducible region using goto */
    if (selector < 0) goto irreducible_label_1;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Deep if-else chain creating many basic blocks */
        if (selector % 2 == 0) {
            a = b + c;
            b = c * d;
            if (selector % 3 == 0) goto irreducible_label_2;
        } else if (selector % 3 == 0) {
            c = d - e;
            d = e / (f ? f : 1);
            goto irreducible_label_1;
        } else if (selector % 5 == 0) {
            e = f ^ g;
            f = g | h;
        } else if (selector % 7 == 0) {
            g = h & i;
            h = i << 2;
        } else {
            i = j >> 1;
            j = k + l;
        }
        
        /* More disjoint paths */
        switch (selector % 11) {
            case 0: k = l + m; break;
            case 1: l = m - n; break;
            case 2: m = n * o; break;
            case 3: n = o / (p ? p : 1); break;
            case 4: o = p ^ a; break;
            case 5: p = a | b; break;
            case 6: a = b & c; break;
            case 7: b = c << 3; break;
            case 8: c = d >> 2; break;
            case 9: d = e + f; break;
            default: e = f - g; break;
        }
        
        /* Loop with continue to different points */
        if (iter % 2 == 0) continue;
        
        irreducible_label_2:
        result += a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
        
        if (iter % 3 == 0) break;
    }
    
    irreducible_label_1:
    result += selector;
    FORCE_USE(result);
    return result;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
NOINLINE
int pattern_b_new_nodes(int depth, int *data) {
    static jmp_buf env;
    int r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0;
    int r8 = 0, r9 = 0, r10 = 0, r11 = 0, r12 = 0, r13 = 0, r14 = 0, r15 = 0;
    int result = 0;
    
    if (setjmp(env) == 0) {
        /* Complex arithmetic creating many temporary values */
        for (int i = 0; i < depth; i++) {
            r0 = data[i % 16] + 1;
            r1 = r0 * 2 - data[(i + 1) % 16];
            r2 = r1 / (data[(i + 2) % 16] ? data[(i + 2) % 16] : 1);
            r3 = r2 ^ data[(i + 3) % 16];
            r4 = r3 | data[(i + 4) % 16];
            r5 = r4 & data[(i + 5) % 16];
            r6 = r5 << (i % 8);
            r7 = r6 >> ((i + 1) % 8);
            r8 = r7 + data[(i + 6) % 16];
            r9 = r8 - data[(i + 7) % 16];
            r10 = r9 * data[(i + 8) % 16];
            r11 = r10 / (data[(i + 9) % 16] ? data[(i + 9) % 16] : 1);
            r12 = r11 ^ data[(i + 10) % 16];
            r13 = r12 | data[(i + 11) % 16];
            r14 = r13 & data[(i + 12) % 16];
            r15 = r14 << ((i + 2) % 8);
            
            result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + 
                     r9 + r10 + r11 + r12 + r13 + r14 + r15;
            
            /* Force longjmp on certain conditions */
            if (i == depth / 2) {
                longjmp(env, 1);
            }
        }
    } else {
        /* Handler after longjmp */
        result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    }
    
    FORCE_USE(result);
    return result;
}

/* ========== PATTERN C: MIXED PRESSURE with vector ops ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int selector) {
    int result = 0;
    
    /* Many scalar variables */
    int s0 = 0, s1 = 0, s2 = 0, s3 = 0, s4 = 0, s5 = 0, s6 = 0, s7 = 0;
    int s8 = 0, s9 = 0, s10 = 0, s11 = 0, s12 = 0, s13 = 0, s14 = 0, s15 = 0;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0 = {0}, v1 = {0}, v2 = {0}, v3 = {0};
#endif
    
    /* Large switch with 30+ cases */
    switch (selector % 35) {
        case 0:
            s0 = selector + 1;
            s1 = s0 * 2;
            s2 = s1 - selector;
#ifdef __GNUC__
            v0 = (v4si){s0, s1, s2, selector};
#endif
            break;
        case 1:
            s3 = selector * 3;
            s4 = s3 / (selector ? selector : 1);
            s5 = s4 ^ selector;
#ifdef __GNUC__
            v1 = (v4si){s3, s4, s5, selector};
#endif
            break;
        case 2:
            s6 = selector << 2;
            s7 = s6 >> 1;
            s8 = s7 | selector;
            break;
        case 3:
            s9 = selector & 0xFF;
            s10 = s9 + 256;
            s11 = s10 - 128;
            break;
        case 4:
            s12 = selector * selector;
            s13 = s12 % 17;
            s14 = s13 + 42;
            break;
        case 5:
            s15 = ~selector;
            s0 = s15 + 1;
            s1 = s0 * -1;
            break;
        case 6:
            s2 = selector + 1000;
            s3 = s2 - 500;
            s4 = s3 * 2;
            break;
        case 7:
            s5 = selector / 3;
            s6 = s5 * 3;
            s7 = selector - s6;
            break;
        case 8:
            s8 = selector | 0xAAAA;
            s9 = s8 & 0x5555;
            s10 = s9 ^ 0xFFFF;
            break;
        case 9:
            s11 = selector << 4;
            s12 = s11 >> 2;
            s13 = s12 << 1;
            break;
        case 10:
            s14 = selector + 0x1000;
            s15 = s14 - 0x800;
            s0 = s15 + 0x400;
            break;
        case 11:
            s1 = selector * 5;
            s2 = s1 / 5;
            s3 = s2 + 1;
            break;
        case 12:
            s4 = selector ^ 0x1234;
            s5 = s4 | 0x4321;
            s6 = s5 & 0xF0F0;
            break;
        case 13:
            s7 = selector + 999;
            s8 = s7 - 333;
            s9 = s8 * 3;
            break;
        case 14:
            s10 = selector % 19;
            s11 = s10 + 7;
            s12 = s11 * 2;
            break;
        case 15:
            s13 = ~selector;
            s14 = s13 + selector;
            s15 = s14 * 2;
            break;
        case 16:
            s0 = selector | 0xF0F0;
            s1 = s0 & 0x0F0F;
            s2 = s1 ^ selector;
            break;
        case 17:
            s3 = selector << 8;
            s4 = s3 >> 4;
            s5 = s4 << 2;
            break;
        case 18:
            s6 = selector + 777;
            s7 = s6 - 222;
            s8 = s7 + 111;
            break;
        case 19:
            s9 = selector * 7;
            s10 = s9 / 7;
            s11 = s10 - 1;
            break;
        case 20:
            s12 = selector ^ 0xDEAD;
            s13 = s12 | 0xBEEF;
            s14 = s13 & 0xCAFE;
            break;
        case 21:
            s15 = selector + 1234;
            s0 = s15 - 567;
            s1 = s0 * 2;
            break;
        case 22:
            s2 = selector % 23;
            s3 = s2 + 11;
            s4 = s3 * 3;
            break;
        case 23:
            s5 = ~selector;
            s6 = s5 | selector;
            s7 = s6 & 0xFF;
            break;
        case 24:
            s8 = selector | 0xCCCC;
            s9 = s8 & 0x3333;
            s10 = s9 ^ 0xAAAA;
            break;
        case 25:
            s11 = selector << 12;
            s12 = s11 >> 6;
            s13 = s12 << 3;
            break;
        case 26:
            s14 = selector + 8888;
            s15 = s14 - 4444;
            s0 = s15 + 2222;
            break;
        case 27:
            s1 = selector * 11;
            s2 = s1 / 11;
            s3 = s2 + 5;
            break;
        case 28:
            s4 = selector ^ 0x5555;
            s5 = s4 | 0xAAAA;
            s6 = s5 & 0x0F0F;
            break;
        case 29:
            s7 = selector + 9876;
            s8 = s7 - 5432;
            s9 = s8 * 2;
            break;
        case 30:
            s10 = selector % 29;
            s11 = s10 + 13;
            s12 = s11 * 2;
            break;
        case 31:
            s13 = ~selector;
            s14 = s13 ^ selector;
            s15 = s14 | 0xFF00;
            break;
        case 32:
            s0 = selector | 0xAAAA;
            s1 = s0 & 0x5555;
            s2 = s1 ^ 0xFFFF;
            break;
        case 33:
            s3 = selector << 16;
            s4 = s3 >> 8;
            s5 = s4 << 4;
            break;
        default: /* case 34 */
            s6 = selector + 65535;
            s7 = s6 - 32768;
            s8 = s7 + 16384;
            break;
    }
    
    /* Force all variables live */
    result = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 + s10 + 
             s11 + s12 + s13 + s14 + s15;
    
#ifdef __GNUC__
    /* Use vector variables */
    v0 += v1 + v2 + v3;
    result += v0[0] + v0[1] + v0[2] + v0[3];
#endif
    
    FORCE_USE(result);
    return result;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
#ifdef __GNUC__
register int conflict_var_1 asm ("r10");
register int conflict_var_2 asm ("r11");
#else
int conflict_var_1;
int conflict_var_2;
#endif

NOINLINE void dummy_helper_1(int x) {
    FORCE_USE(x);
}

NOINLINE void dummy_helper_2(int x, int y) {
    FORCE_USE(x);
    FORCE_USE(y);
}

NOINLINE void dummy_helper_3(int x, int y, int z) {
    FORCE_USE(x);
    FORCE_USE(y);
    FORCE_USE(z);
}

NOINLINE
int pattern_d_artificial_conflict(int iterations) {
    int result = 0;
    
#ifdef __GNUC__
    /* Force register variable usage */
    conflict_var_1 = iterations;
    conflict_var_2 = iterations * 2;
#endif
    
    for (int i = 0; i < iterations; i++) {
        /* Artificial live-range splits with calls */
        int temp1 = i * 3;
        dummy_helper_1(temp1);
        
        int temp2 = temp1 + i;
        conflict_var_1 = temp2;
        dummy_helper_2(temp2, conflict_var_1);
        
        int temp3 = temp2 * 2;
        conflict_var_2 = temp3;
        dummy_helper_3(temp3, conflict_var_1, conflict_var_2);
        
        /* More register pressure */
        int r0 = conflict_var_1 + 1;
        int r1 = conflict_var_2 - 1;
        int r2 = r0 * r1;
        int r3 = r2 / (i + 1);
        int r4 = r3 ^ r0;
        int r5 = r4 | r1;
        int r6 = r5 & r2;
        int r7 = r6 << (i % 8);
        int r8 = r7 >> ((i + 1) % 8);
        int r9 = r8 + r3;
        int r10 = r9 - r4;
        
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
        
        /* Force register variable updates */
        conflict_var_1 = r10;
        conflict_var_2 = result;
    }
    
    FORCE_USE(result);
#ifdef __GNUC__
    FORCE_USE(conflict_var_1);
    FORCE_USE(conflict_var_2);
#endif
    return result;
}

/* ========== MAIN FUNCTION WITH PGO SUPPORT ========== */
int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose = 1;
    }
    
    int total_result = 0;
    int test_data[16];
    
    /* Initialize test data */
    for (int i = 0; i < 16; i++) {
        test_data[i] = i * 3 + 7;
    }
    
    /* Check CPU features to engage target-specific heuristics */
    int has_avx2 = 0;
#ifdef __GNUC__
    has_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", has_avx2 ? "yes" : "no");
    }
#endif
    
    /* Run each pattern multiple times with different arguments */
    for (int run = 0; run < 100; run++) {
        /* Pattern A - ENTRY/EXIT blocks */
        total_result += pattern_a_entry_exit(run % 50 + 10, run);
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        total_result += pattern_b_new_nodes(run % 30 + 5, test_data);
        
        /* Pattern C - Mixed pressure */
        total_result += pattern_c_mixed_pressure(run);
        
        /* Pattern D - Artificial conflict */
        total_result += pattern_d_artificial_conflict(run % 20 + 3);
        
        /* Modify test data slightly each iteration */
        test_data[run % 16] += run;
    }
    
    /* Store result in volatile to prevent optimization */
    global_sink = total_result;
    
    if (verbose) {
        printf("Final result: %d\n", total_result);
    }
    
    return total_result == 0 ? 0 : 1;
}
