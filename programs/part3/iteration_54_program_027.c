/* test_mcf.c - Comprehensive test for GCC's Min-Cost Flow pass special nodes */
/* Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-bbro -c test_mcf.c */
/* Then run: ./test_mcf_gen (for PGO) or ./test_mcf_executable (for coverage) */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define USED __attribute__((used))
#define REG_VAR(name, reg) register int name asm(reg)
#else
#define NOINLINE
#define HOT
#define USED
#define REG_VAR(name, reg) int name
#endif

/* Global verbose flag - keep off to avoid I/O affecting CFG */
static volatile int verbose = 0;

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE USED
int pattern_a_entry_exit(int iterations, int seed) {
    /* Complex function with irreducible region to force ENTRY_BLOCK+1 and 2*EXIT_BLOCK */
    volatile int result = seed;
    int i, j, k;
    
    /* Create an irreducible region using goto */
    if (iterations > 100) {
        goto irreducible_label_1;
    }
    
    for (i = 0; i < iterations; i++) {
        /* Deep if-else chain */
        if (i % 3 == 0) {
            if (i % 5 == 0) {
                for (j = 0; j < 10; j++) {
                    result += j * i;
                    if (j == 5) goto irreducible_label_2;
                }
            } else {
                result -= i * 2;
            }
        } else if (i % 3 == 1) {
            irreducible_label_1:
            for (k = 0; k < 5; k++) {
                result += (k << 2);
                if (k == 3) break;
            }
        } else {
            irreducible_label_2:
            result ^= (i << 1);
            if (i % 7 == 0) continue;
            result |= 0x1FF;
        }
        
        /* Nested switch to add complexity */
        switch (i % 8) {
            case 0: result += 0x100; break;
            case 1: result -= 0x200; break;
            case 2: result ^= 0x300; break;
            case 3: result |= 0x400; break;
            case 4: result &= 0x500; break;
            case 5: result <<= 2; break;
            case 6: result >>= 1; break;
            case 7: result = ~result; break;
        }
    }
    
    /* One more irreducible jump to stress CFG normalization */
    if (result > 1000) {
        goto irreducible_label_1;
    }
    
    return result;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
static volatile int longjmp_counter = 0;

NOINLINE USED
int pattern_b_new_nodes(int depth, int max_depth) {
    /* Function using setjmp/longjmp to create exceptional edges */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    volatile int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    if (depth >= max_depth) {
        return a + b + c + d;
    }
    
    /* Force all variables live across setjmp */
    int val = setjmp(jump_buffer);
    if (val == 0) {
        /* First call path */
        for (int idx = 0; idx < 20; idx++) {
            a += b * c - d / (e + 1);
            b ^= c | d & e;
            c = (c << f) | (c >> (32 - f));
            d = d * e + f - g;
            e = e ^ f ^ g ^ h;
            f = f + g - h * i;
            g = g | h & i | j;
            h = h * i / (j + 1);
            i = i - j + k - l;
            j = j ^ k ^ l ^ m;
            k = k * l + m - n;
            l = l | m & n | o;
            m = m + n - o * p;
            n = n ^ o ^ p ^ a;
            o = o * p + a - b;
            p = p | a & b | c;
            
            /* Prevent optimization */
            asm volatile("" : : "g"(a), "g"(b), "g"(c), "g"(d), "g"(e), 
                         "g"(f), "g"(g), "g"(h), "g"(i), "g"(j),
                         "g"(k), "g"(l), "g"(m), "g"(n), "g"(o), "g"(p));
            
            if (idx == 10) {
                longjmp_counter++;
                longjmp(jump_buffer, 1);
            }
        }
    } else {
        /* Longjmp return path */
        for (int idx = 0; idx < 5; idx++) {
            a = b + c;
            b = c + d;
            c = d + e;
            d = e + f;
            /* Recursive call to increase complexity */
            pattern_b_new_nodes(depth + 1, max_depth);
        }
    }
    
    /* Complex return calculation using all variables */
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 +
           i * 9 + j * 10 + k * 11 + l * 12 + m * 13 + n * 14 + o * 15 + p * 16;
}

/* ========== PATTERN C: MIXED PRESSURE with vector ops ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE USED
int pattern_c_mixed_pressure(int selector, int iterations) {
    /* Large switch with vector operations */
    volatile int r0 = 0, r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5, r6 = 6, r7 = 7;
    volatile int r8 = 8, r9 = 9, r10 = 10, r11 = 11, r12 = 12, r13 = 13, r14 = 14, r15 = 15;
    
#ifdef __GNUC__
    v4si v0 = {0, 1, 2, 3};
    v4si v1 = {4, 5, 6, 7};
    v4si v2 = {8, 9, 10, 11};
    v4si v3 = {12, 13, 14, 15};
#endif
    
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Large switch with 30+ cases */
        switch ((selector + i) % 35) {
            case 0:
                r0 = r1 + r2; r1 = r2 * r3; r2 = r3 ^ r4; r3 = r4 | r5;
                r4 = r5 & r6; r5 = r6 << 1; r6 = r7 >> 2; r7 = r8 + r9;
                r8 = r9 - r10; r9 = r10 * r11; r10 = r11 / (r12 + 1); r11 = r12 % (r13 + 1);
                r12 = r13 ^ r14; r13 = r14 | r15; r14 = r15 & r0; r15 = r0 << 3;
#ifdef __GNUC__
                v0 = v1 + v2; v1 = v2 * v3; v2 = v3 ^ v0; v3 = v0 | v1;
#endif
                asm volatile("" : : "g"(r0), "g"(r1), "g"(r2), "g"(r3), "g"(r4), "g"(r5),
                           "g"(r6), "g"(r7), "g"(r8), "g"(r9), "g"(r10), "g"(r11),
                           "g"(r12), "g"(r13), "g"(r14), "g"(r15));
                continue;
            
            case 1:
                r0 = r15 - r14; r15 = r14 * r13; r14 = r13 ^ r12; r13 = r12 | r11;
                r12 = r11 & r10; r11 = r10 << 2; r10 = r9 >> 1; r9 = r8 + r7;
                r8 = r7 - r6; r7 = r6 * r5; r6 = r5 / (r4 + 1); r5 = r4 % (r3 + 1);
                r4 = r3 ^ r2; r3 = r2 | r1; r2 = r1 & r0; r1 = r0 << 1;
#ifdef __GNUC__
                v3 = v2 - v1; v2 = v1 * v0; v1 = v0 ^ v3; v0 = v3 | v2;
#endif
                asm volatile("" : : "g"(r0), "g"(r1), "g"(r2), "g"(r3), "g"(r4), "g"(r5),
                           "g"(r6), "g"(r7), "g"(r8), "g"(r9), "g"(r10), "g"(r11),
                           "g"(r12), "g"(r13), "g"(r14), "g"(r15));
                break;
            
            case 2:
                r0 = (r0 << r1) | (r0 >> (32 - r1));
                r1 = (r1 << r2) | (r1 >> (32 - r2));
                r2 = (r2 << r3) | (r2 >> (32 - r3));
                r3 = (r3 << r4) | (r3 >> (32 - r4));
                r4 = (r4 << r5) | (r4 >> (32 - r5));
                r5 = (r5 << r6) | (r5 >> (32 - r6));
                r6 = (r6 << r7) | (r6 >> (32 - r7));
                r7 = (r7 << r8) | (r7 >> (32 - r8));
                r8 = (r8 << r9) | (r8 >> (32 - r9));
                r9 = (r9 << r10) | (r9 >> (32 - r10));
                r10 = (r10 << r11) | (r10 >> (32 - r11));
                r11 = (r11 << r12) | (r11 >> (32 - r12));
                r12 = (r12 << r13) | (r12 >> (32 - r13));
                r13 = (r13 << r14) | (r13 >> (32 - r14));
                r14 = (r14 << r15) | (r14 >> (32 - r15));
                r15 = (r15 << r0) | (r15 >> (32 - r0));
                asm volatile("" : : "g"(r0), "g"(r1), "g"(r2), "g"(r3), "g"(r4), "g"(r5),
                           "g"(r6), "g"(r7), "g"(r8), "g"(r9), "g"(r10), "g"(r11),
                           "g"(r12), "g"(r13), "g"(r14), "g"(r15));
                continue;
            
            /* 32 more cases following similar pattern with unique operations */
            case 3: r0 = r1 * r2 + r3; r1 = r4 - r5 * r6; break;
            case 4: r2 = r3 ^ r4 | r5; r3 = r6 & r7 ^ r8; break;
            case 5: r4 = r5 << r6; r5 = r7 >> r8; break;
            case 6: r6 = r7 + r8 * r9; r7 = r10 - r11; break;
            case 7: r8 = r9 ^ r10 & r11; r9 = r12 | r13; break;
            case 8: r10 = r11 * r12 / (r13 + 1); r11 = r14 % (r15 + 1); break;
            case 9: r12 = ~r13; r13 = ~r14; r14 = ~r15; break;
            case 10: r15 = r0 + r1 * 2; r0 = r1 + r2 * 3; break;
            case 11: r1 = r2 | r3 & r4; r2 = r5 ^ r6 | r7; break;
            case 12: r3 = r4 << 3; r4 = r5 >> 2; r5 = r6 << 1; break;
            case 13: r6 = r7 * r8 + r9; r7 = r10 * r11 - r12; break;
            case 14: r8 = r9 ^ r10 ^ r11; r9 = r12 ^ r13 ^ r14; break;
            case 15: r10 = r11 & r12 & r13; r11 = r14 & r15 & r0; break;
            case 16: r12 = r13 | r14 | r15; r13 = r0 | r1 | r2; break;
            case 17: r14 = r15 * 3 + r0 * 2; r15 = r0 * 5 - r1 * 3; break;
            case 18: r0 = r1 / (r2 + 1) + r3; r1 = r4 / (r5 + 1) - r6; break;
            case 19: r2 = r3 % (r4 + 1) ^ r5; r3 = r6 % (r7 + 1) | r8; break;
            case 20: r4 = (r5 << r6) | (r5 >> (32 - r6)); break;
            case 21: r5 = (r6 << r7) ^ (r6 >> (32 - r7)); break;
            case 22: r6 = (r7 << r8) & (r7 >> (32 - r8)); break;
            case 23: r7 = (r8 << r9) + (r8 >> (32 - r9)); break;
            case 24: r8 = (r9 << r10) - (r9 >> (32 - r10)); break;
            case 25: r9 = (r10 << r11) * (r10 >> (32 - r11)); break;
            case 26: r10 = (r11 << r12) | ~(r11 >> (32 - r12)); break;
            case 27: r11 = ~(r12 << r13) | (r12 >> (32 - r13)); break;
            case 28: r12 = (r13 << r14) ^ ~(r13 >> (32 - r14)); break;
            case 29: r13 = ~(r14 << r15) & (r14 >> (32 - r15)); break;
            case 30: r14 = (r15 << r0) + ~(r15 >> (32 - r0)); break;
            case 31: r15 = ~(r0 << r1) - (r0 >> (32 - r1)); break;
            case 32: r0 = r1 * r2 - r3 * r4 + r5 * r6; break;
            case 33: r1 = r2 ^ r3 ^ r4 ^ r5 ^ r6 ^ r7; break;
            case 34:
                /* Final case with loop control */
                if (i % 2 == 0) continue;
                else break;
        }
        
        /* Prevent dead code elimination */
        asm volatile("" : : "g"(r0), "g"(r1), "g"(r2), "g"(r3), "g"(r4), "g"(r5),
                   "g"(r6), "g"(r7), "g"(r8), "g"(r9), "g"(r10), "g"(r11),
                   "g"(r12), "g"(r13), "g"(r14), "g"(r15));
        
        /* Vector operations in some iterations */
        if (i % 7 == 0) {
#ifdef __GNUC__
            v0 = v0 + v1;
            v1 = v1 * v2;
            v2 = v2 - v3;
            v3 = v3 ^ v0;
            asm volatile("" : : "g"(v0), "g"(v1), "g"(v2), "g"(v3));
#endif
        }
    }
    
    /* Combine all results */
    result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
             r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
#ifdef __GNUC__
    result += v0[0] + v0[1] + v0[2] + v0[3];
    result += v1[0] + v1[1] + v1[2] + v1[3];
#endif
    
    return result;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
NOINLINE static void dummy1(REG_VAR(x, "r10"), REG_VAR(y, "r11")) {
    asm volatile("" : : "r"(x), "r"(y));
}

NOINLINE static void dummy2(REG_VAR(a, "r10"), REG_VAR(b, "r12")) {
    asm volatile("" : : "r"(a), "r"(b));
}

NOINLINE static void dummy3(REG_VAR(c, "r11"), REG_VAR(d, "r12")) {
    asm volatile("" : : "r"(c), "r"(d));
}

NOINLINE USED
int pattern_d_register_conflict(int param) {
    /* Function with explicit register conflicts */
    REG_VAR(reg1, "r10") = param;
    REG_VAR(reg2, "r11") = param * 2;
    REG_VAR(reg3, "r12") = param * 3;
    REG_VAR(reg4, "r10") = param * 4;  /* Conflict with reg1 */
    REG_VAR(reg5, "r11") = param * 5;  /* Conflict with reg2 */
    REG_VAR(reg6, "r12") = param * 6;  /* Conflict with reg3 */
    
    volatile int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * param;
        
        /* Force register conflicts through dummy calls */
        if (i % 3 == 0) {
            dummy1(reg1, reg2);
            reg1 = reg4 ^ reg5;
            reg2 = reg6 & reg3;
        } else if (i % 3 == 1) {
            dummy2(reg4, reg3);
            reg4 = reg1 | reg2;
            reg3 = reg5 ^ reg6;
        } else {
            dummy3(reg2, reg6);
            reg2 = reg3 + reg4;
            reg6 = reg1 - reg5;
        }
        
        /* Complex arithmetic using all registers */
        reg1 = reg1 * reg2 + reg3;
        reg2 = reg2 - reg3 * reg4;
        reg3 = reg3 ^ reg4 | reg5;
        reg4 = reg4 & reg5 ^ reg6;
        reg5 = reg5 << (reg6 % 8);
        reg6 = reg6 >> (reg1 % 8);
        
        /* Use array to prevent optimization */
        arr[i] += reg1 + reg2 + reg3 + reg4 + reg5 + reg6;
    }
    
    /* Final computation with all conflicting registers */
    return reg1 + reg2 * 2 + reg3 * 3 + reg4 * 4 + reg5 * 5 + reg6 * 6 + arr[param % 100];
}

/* ========== MAIN FUNCTION WITH PGO SUPPORT ========== */
int main(int argc, char **argv) {
    volatile int total_result = 0;
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
    
    /* Check CPU features to engage target-specific register allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", use_avx2 ? "yes" : "no");
    }
#endif
    
    /* Call all pattern functions multiple times with different arguments */
    for (int i = 0; i < iterations; i++) {
        /* Pattern A - ENTRY/EXIT blocks */
        total_result ^= pattern_a_entry_exit(i % 100 + 50, i);
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        if (i % 10 == 0) {
            total_result += pattern_b_new_nodes(0, 3);
        }
        
        /* Pattern C - Mixed pressure with vectors */
        total_result += pattern_c_mixed_pressure(i, 50 + (i % 30));
        
        /* Pattern D - Register conflicts */
        total_result ^= pattern_d_register_conflict(i);
        
        /* Prevent loop optimization */
        asm volatile("" : "+g"(total_result));
    }
    
    if (verbose) {
        printf("Final result: %d\n", total_result);
        printf("Longjmp count: %d\n", longjmp_counter);
    }
    
    /* Use result to prevent dead code elimination */
    return total_result % 256;
}
