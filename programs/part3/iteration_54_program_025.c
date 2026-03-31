/* test_mcf.c - Comprehensive test for GCC's Min-Cost Flow pass special nodes */
/* Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-bbro -c test_mcf.c */
/* For PGO: gcc -O2 -fprofile-generate test_mcf.c -o test_gen && ./test_gen && gcc -O2 -fprofile-use test_mcf.c -o test_use */

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

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>

/* Global to prevent optimization */
volatile int global_result = 0;
static int verbose = 0;

/* Vector type for Pattern C */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
/* Creates irreducible region with goto to force ENTRY_BLOCK+1 and 2*EXIT_BLOCK */
HOT NOINLINE USED
int pattern_a_entry_exit(int iterations, int seed) {
    int result = seed;
    int i, j, k;
    
    /* Label the first block to encourage ENTRY_BLOCK identification */
    volatile int entry_marker = 1;
    
    /* Deeply nested if-else chains */
    if (seed < 100) {
        for (i = 0; i < iterations; i++) {
            if (i % 3 == 0) {
                for (j = 0; j < 10; j++) {
                    if (j % 2 == 0) {
                        result += i * j;
                        /* Create irreducible region with goto */
                        if (result > 1000) goto irreducible_label;
                    } else {
                        result -= i / (j + 1);
                    }
                }
            } else if (i % 3 == 1) {
                int temp = 0;
                for (k = 0; k < 15; k++) {
                    temp += k * k;
                    if (temp > 500) break;
                }
                result ^= temp;
            } else {
                /* Complex arithmetic to prevent simplification */
                result = (result * 1103515245 + 12345) & 0x7fffffff;
            }
        }
    } else {
        /* Alternative path with more loops */
        int counter = 0;
        while (counter++ < iterations * 2) {
            result = (result << 3) | (result >> 29);
            if (result & 1) {
                result ^= 0x5A827999;
            }
        }
    }
    
    /* Target of goto - creates irreducible region */
irreducible_label:
    {
        int x = result;
        int y = x * x;
        int z = y % 7919;
        result = z ^ (x + y);
        
        /* Force all variables live */
        asm volatile("" : : "g"(x), "g"(y), "g"(z));
    }
    
    /* More complex control flow after goto */
    switch (result % 7) {
        case 0: result += 111; break;
        case 1: result -= 222; break;
        case 2: result *= 333; break;
        case 3: result /= 4; break;
        case 4: result ^= 0xDEADBEEF; break;
        case 5: result = ~result; break;
        case 6: result = result << 2; break;
    }
    
    return result;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
static volatile int setjmp_counter = 0;

NOINLINE USED
int pattern_b_new_nodes(int depth, int max_depth) {
    int r0 = depth, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0;
    int r8 = 0, r9 = 0, r10 = 0, r11 = 0, r12 = 0, r13 = 0, r14 = 0, r15 = 0;
    
    /* Many local variables to create register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    if (setjmp(jump_buffer) == 0) {
        /* Normal path - creates exceptional edges */
        for (int iter = 0; iter < depth * 10; iter++) {
            /* Complex arithmetic on all variables */
            r0 = (r0 * 1664525 + 1013904223) & 0x7fffffff;
            r1 = r1 ^ r0;
            r2 = r2 + r1;
            r3 = r3 - r2;
            r4 = r4 * r3;
            r5 = r5 / (r4 + 1);
            r6 = r6 | r5;
            r7 = r7 & r6;
            r8 = r8 ^ r7;
            r9 = r9 + r8;
            r10 = r10 - r9;
            r11 = r11 * r10;
            r12 = r12 ^ r11;
            r13 = r13 + r12;
            r14 = r14 - r13;
            r15 = r15 * r14;
            
            /* Mix in other variables */
            a = b + c;
            b = c - d;
            c = d * e;
            d = e / (f + 1);
            e = f ^ g;
            f = g | h;
            g = h & i;
            h = i + j;
            i = j - k;
            j = k * l;
            k = l ^ m;
            l = m + n;
            m = n - o;
            n = o * p;
            o = p ^ a;
            p = a + b;
            
            /* Force all variables live */
            asm volatile("" : : 
                "g"(r0), "g"(r1), "g"(r2), "g"(r3), "g"(r4), "g"(r5), "g"(r6), "g"(r7),
                "g"(r8), "g"(r9), "g"(r10), "g"(r11), "g"(r12), "g"(r13), "g"(r14), "g"(r15),
                "g"(a), "g"(b), "g"(c), "g"(d), "g"(e), "g"(f), "g"(g), "g"(h),
                "g"(i), "g"(j), "g"(k), "g"(l), "g"(m), "g"(n), "g"(o), "g"(p));
            
            /* Occasionally longjmp to create exceptional edge */
            if (iter % 37 == 0 && depth < max_depth) {
                setjmp_counter++;
                longjmp(jump_buffer, 1);
            }
        }
    } else {
        /* longjmp target - creates new exit/entry requirements */
        r0 = (r0 + r15) ^ 0xABCDEF;
        r1 = (r1 * r14) % 65537;
    }
    
    /* Combine all results */
    int result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 +
                 r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15 +
                 a + b + c + d + e + f + g + h +
                 i + j + k + l + m + n + o + p;
    
    return result & 0xFFFF;
}

/* ========== PATTERN C: MIXED PRESSURE with VECTOR OPS ========== */
NOINLINE USED
int pattern_c_mixed_pressure(int selector) {
    /* Scalar variables */
    int s0 = selector, s1 = 0, s2 = 0, s3 = 0, s4 = 0, s5 = 0, s6 = 0, s7 = 0;
    int s8 = 0, s9 = 0, s10 = 0, s11 = 0, s12 = 0, s13 = 0, s14 = 0, s15 = 0;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0 = {selector, selector+1, selector+2, selector+3};
    v4si v1 = {4, 5, 6, 7};
    v4si v2 = {8, 9, 10, 11};
    v4si v3 = {12, 13, 14, 15};
#endif
    
    /* Large switch with 30+ cases */
    switch (selector % 37) {
        case 0:
            s0 = s0 * 3; s1 = s0 + 1; s2 = s1 * 2;
#ifdef __GNUC__
            v0 = v0 + v1;
#endif
            asm volatile("" : : "g"(s0), "g"(s1), "g"(s2));
            break;
        case 1:
            s3 = s0 - s1; s4 = s3 * s2; s5 = s4 % 17;
#ifdef __GNUC__
            v1 = v1 - v0;
#endif
            asm volatile("" : : "g"(s3), "g"(s4), "g"(s5));
            break;
        case 2:
            s6 = s5 ^ s4; s7 = s6 | s3; s8 = s7 & 0xFF;
#ifdef __GNUC__
            v2 = v2 * v1;
#endif
            asm volatile("" : : "g"(s6), "g"(s7), "g"(s8));
            break;
        case 3:
            s9 = s8 << 3; s10 = s9 >> 1; s11 = s10 ^ 0x55;
#ifdef __GNUC__
            v3 = v3 + v2;
#endif
            asm volatile("" : : "g"(s9), "g"(s10), "g"(s11));
            break;
        case 4:
            s12 = s11 + 999; s13 = s12 - 888; s14 = s13 * 777;
            asm volatile("" : : "g"(s12), "g"(s13), "g"(s14));
            break;
        case 5:
            s15 = s14 % 31; s0 = s15 ^ s14; s1 = s0 | s13;
            asm volatile("" : : "g"(s15), "g"(s0), "g"(s1));
            break;
        case 6:
            s2 = s1 & 0xF0F0; s3 = s2 + 0x1010; s4 = s3 - 0x2020;
            asm volatile("" : : "g"(s2), "g"(s3), "g"(s4));
            break;
        case 7:
            s5 = s4 * 123; s6 = s5 / 3; s7 = s6 + 456;
            asm volatile("" : : "g"(s5), "g"(s6), "g"(s7));
            break;
        case 8:
            s8 = s7 ^ 0xAAAA; s9 = s8 | 0x5555; s10 = s9 & 0x3333;
            asm volatile("" : : "g"(s8), "g"(s9), "g"(s10));
            break;
        case 9:
            s11 = s10 << 4; s12 = s11 >> 2; s13 = s12 ^ s11;
            asm volatile("" : : "g"(s11), "g"(s12), "g"(s13));
            break;
        case 10:
            s14 = s13 + 0x1111; s15 = s14 - 0x2222; s0 = s15 * 0x3333;
            asm volatile("" : : "g"(s14), "g"(s15), "g"(s0));
            break;
        case 11:
            s1 = s0 % 19; s2 = s1 ^ 0xDEAD; s3 = s2 | 0xBEEF;
            asm volatile("" : : "g"(s1), "g"(s2), "g"(s3));
            break;
        case 12:
            s4 = s3 & 0xCCCC; s5 = s4 + 0x1111; s6 = s5 - 0x2222;
            asm volatile("" : : "g"(s4), "g"(s5), "g"(s6));
            break;
        case 13:
            s7 = s6 * 7; s8 = s7 / 3; s9 = s8 + 11;
            asm volatile("" : : "g"(s7), "g"(s8), "g"(s9));
            break;
        case 14:
            s10 = s9 ^ 0x1234; s11 = s10 | 0x5678; s12 = s11 & 0x9ABC;
            asm volatile("" : : "g"(s10), "g"(s11), "g"(s12));
            break;
        case 15:
            s13 = s12 << 1; s14 = s13 >> 3; s15 = s14 ^ s13;
            asm volatile("" : : "g"(s13), "g"(s14), "g"(s15));
            break;
        case 16:
            s0 = s15 + 1000; s1 = s0 - 500; s2 = s1 * 250;
            asm volatile("" : : "g"(s0), "g"(s1), "g"(s2));
            break;
        case 17:
            s3 = s2 % 23; s4 = s3 ^ 0xCAFE; s5 = s4 | 0xBABE;
            asm volatile("" : : "g"(s3), "g"(s4), "g"(s5));
            break;
        case 18:
            s6 = s5 & 0xF00D; s7 = s6 + 0xBAAD; s8 = s7 - 0xF00D;
            asm volatile("" : : "g"(s6), "g"(s7), "g"(s8));
            break;
        case 19:
            s9 = s8 * 13; s10 = s9 / 5; s11 = s10 + 17;
            asm volatile("" : : "g"(s9), "g"(s10), "g"(s11));
            break;
        case 20:
            s12 = s11 ^ 0xFEED; s13 = s12 | 0xFACE; s14 = s13 & 0xC0DE;
            asm volatile("" : : "g"(s12), "g"(s13), "g"(s14));
            break;
        case 21:
            s15 = s14 << 2; s0 = s15 >> 4; s1 = s0 ^ s15;
            asm volatile("" : : "g"(s15), "g"(s0), "g"(s1));
            break;
        case 22:
            s2 = s1 + 7777; s3 = s2 - 6666; s4 = s3 * 5555;
            asm volatile("" : : "g"(s2), "g"(s3), "g"(s4));
            break;
        case 23:
            s5 = s4 % 29; s6 = s5 ^ 0x1CE; s7 = s6 | 0xC1A;
            asm volatile("" : : "g"(s5), "g"(s6), "g"(s7));
            break;
        case 24:
            s8 = s7 & 0x7E7E; s9 = s8 + 0x8181; s10 = s9 - 0x4242;
            asm volatile("" : : "g"(s8), "g"(s9), "g"(s10));
            break;
        case 25:
            s11 = s10 * 21; s12 = s11 / 7; s13 = s12 + 33;
            asm volatile("" : : "g"(s11), "g"(s12), "g"(s13));
            break;
        case 26:
            s14 = s13 ^ 0x5A5A; s15 = s14 | 0xA5A5; s0 = s15 & 0x3C3C;
            asm volatile("" : : "g"(s14), "g"(s15), "g"(s0));
            break;
        case 27:
            s1 = s0 << 5; s2 = s1 >> 6; s3 = s2 ^ s1;
            asm volatile("" : : "g"(s1), "g"(s2), "g"(s3));
            break;
        case 28:
            s4 = s3 + 2468; s5 = s4 - 1357; s6 = s5 * 8024;
            asm volatile("" : : "g"(s4), "g"(s5), "g"(s6));
            break;
        case 29:
            s7 = s6 % 31; s8 = s7 ^ 0x8888; s9 = s8 | 0x4444;
            asm volatile("" : : "g"(s7), "g"(s8), "g"(s9));
            break;
        case 30:
            s10 = s9 & 0x0FF0; s11 = s10 + 0xF00F; s12 = s11 - 0x0FF0;
            asm volatile("" : : "g"(s10), "g"(s11), "g"(s12));
            break;
        case 31:
            s13 = s12 * 47; s14 = s13 / 11; s15 = s14 + 59;
            asm volatile("" : : "g"(s13), "g"(s14), "g"(s15));
            break;
        case 32:
            s0 = s15 ^ 0x6666; s1 = s0 | 0x9999; s2 = s1 & 0xBBBB;
            asm volatile("" : : "g"(s0), "g"(s1), "g"(s2));
            break;
        case 33:
            s3 = s2 << 7; s4 = s3 >> 8; s5 = s4 ^ s3;
            asm volatile("" : : "g"(s3), "g"(s4), "g"(s5));
            break;
        case 34:
            s6 = s5 + 9876; s7 = s6 - 5432; s8 = s7 * 1098;
            asm volatile("" : : "g"(s6), "g"(s7), "g"(s8));
            break;
        case 35:
            s9 = s8 % 37; s10 = s9 ^ 0x2222; s11 = s10 | 0x1111;
            asm volatile("" : : "g"(s9), "g"(s10), "g"(s11));
            break;
        case 36:
            s12 = s11 & 0xDDDD; s13 = s12 + 0x2222; s14 = s13 - 0x1111;
            asm volatile("" : : "g"(s12), "g"(s13), "g"(s14));
            break;
    }
    
    /* Loop around the switch based on selector */
    for (int i = 0; i < (selector % 10); i++) {
        switch ((selector + i) % 5) {
            case 0: continue;
            case 1: s0++; break;
            case 2: s1--; break;
            case 3: s2 ^= s0; break;
            case 4: s3 |= s1; break;
        }
    }
    
    /* Combine results */
    int result = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 +
                 s8 + s9 + s10 + s11 + s12 + s13 + s14 + s15;
    
#ifdef __GNUC__
    /* Add vector results */
    int vsum = v0[0] + v0[1] + v0[2] + v0[3] +
               v1[0] + v1[1] + v1[2] + v1[3] +
               v2[0] + v2[1] + v2[2] + v2[3] +
               v3[0] + v3[1] + v3[2] + v3[3];
    result += vsum;
#endif
    
    return result & 0x7FFF;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
NOINLINE static void dummy1(int x) { asm volatile("" : : "g"(x)); }
NOINLINE static void dummy2(int x, int y) { asm volatile("" : : "g"(x), "g"(y)); }
NOINLINE static void dummy3(int x, int y, int z) { asm volatile("" : : "g"(x), "g"(y), "g"(z)); }

NOINLINE USED
int pattern_d_register_conflict(int param) {
    /* Explicit register variables that conflict */
    REG_VAR(r10_var1, "r10") = param;
    REG_VAR(r10_var2, "r10") = param * 2;  /* Same register! */
    REG_VAR(r11_var1, "r11") = param + 1;
    REG_VAR(r11_var2, "r11") = param + 2;  /* Same register! */
    
    int local1 = r10_var1 * 3;
    int local2 = r10_var2 / 2;
    int local3 = r11_var1 + 100;
    int local4 = r11_var2 - 50;
    
    /* Call dummy functions to split live ranges */
    dummy1(local1);
    dummy2(local1, local2);
    
    /* More conflicting uses */
    REG_VAR(r10_var3, "r10") = local1 + local2;  /* Reuse r10 */
    REG_VAR(r11_var3, "r11") = local3 - local4;  /* Reuse r11 */
    
    dummy3(local1, local2, local3);
    
    /* Complex arithmetic that uses all variables */
    for (int i = 0; i < 20; i++) {
        local1 = (local1 * 1103515245 + 12345) & 0x7fffffff;
        local2 = local2 ^ local1;
        local3 = local3 + local2;
        local4 = local4 - local3;
        
        /* Force register pressure */
        asm volatile("" : : 
            "g"(r10_var1), "g"(r10_var2), "g"(r10_var3),
            "g"(r11_var1), "g"(r11_var2), "g"(r11_var3),
            "g"(local1), "g"(local2), "g"(local3), "g"(local4));
        
        /* More dummy calls */
        if (i % 3 == 0) dummy1(local1);
        if (i % 5 == 0) dummy2(local2, local3);
        if (i % 7 == 0) dummy3(local1, local3, local4);
    }
    
    /* Final combination */
    int result = (r10_var1 + r10_var2 + r10_var3 +
                  r11_var1 + r11_var2 + r11_var3 +
                  local1 + local2 + local3 + local4) & 0xFFFF;
    
    return result;
}

/* ========== MAIN FUNCTION WITH PGO SUPPORT ========== */
int main(int argc, char **argv) {
    if (argc > 1) verbose = 1;
    
    int total = 0;
    int iterations = 1000;
    
    /* Use CPU features to engage target-specific register allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %d\n", use_avx2);
    }
#endif
    
    /* Run all patterns multiple times to generate profile data */
    for (int i = 0; i < iterations; i++) {
        /* Pattern A - ENTRY/EXIT blocks */
        int r1 = pattern_a_entry_exit(i % 100, i);
        global_result ^= r1;
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        int r2 = pattern_b_new_nodes(i % 10, 5);
        global_result += r2;
        
        /* Pattern C - Mixed pressure with vectors */
        int r3 = pattern_c_mixed_pressure(i);
        global_result ^= r3;
        
        /* Pattern D - Register conflict */
        int r4 = pattern_d_register_conflict(i);
        global_result += r4;
        
        total += r1 + r2 + r3 + r4;
        
        /* Occasionally print progress if verbose */
        if (verbose && (i % 100 == 0)) {
            printf("Iteration %d: total=%d, global=%d\n", i, total, global_result);
        }
    }
    
    if (verbose) {
        printf("Final total: %d\n", total);
        printf("Global result: %d\n", global_result);
        printf("Setjmp calls: %d\n", setjmp_counter);
    }
    
    return total != 0 ? 0 : 1;
}
