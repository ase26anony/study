/* test_mcf.c - Comprehensive test to trigger MCF special node printing */
#ifdef __GNUC__
#define FORCE_USE(var) asm volatile("" : : "g"(var))
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define VECTOR_TYPE __attribute__((vector_size(16)))
#else
#define FORCE_USE(var) (void)(var)
#define NOINLINE
#define HOT
#define VECTOR_TYPE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

static int verbose = 0;

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create complex irreducible region with goto */
    if (iterations > 0) {
        goto start_loop;
    } else {
        goto early_exit;
    }
    
start_loop:
    for (i = 0; i < iterations; i++) {
        /* Deeply nested if-else chain */
        if (i % 3 == 0) {
            if (i % 5 == 0) {
                for (j = 0; j < 10; j++) {
                    if (j % 2 == 0) {
                        result += i * j;
                        if (result > 1000) goto reduce_result;
                    } else {
                        result -= i + j;
                        if (result < 0) goto make_positive;
                    }
                }
            } else {
                result ^= i;
            }
        } else if (i % 3 == 1) {
            int temp = 0;
            for (k = 0; k < i; k++) {
                temp += k * k;
                if (temp > 100) break;
            }
            result += temp;
        } else {
            /* Another irreducible region */
            if (result % 2 == 0) {
                goto even_handler;
            } else {
                goto odd_handler;
            }
            
        even_handler:
            result >>= 1;
            continue;
            
        odd_handler:
            result = result * 3 + 1;
            continue;
        }
        
        if (i % 7 == 0) {
            continue;
        }
        
        result += 1;
        continue;
        
    reduce_result:
        result >>= 2;
        continue;
        
    make_positive:
        result = -result;
        continue;
    }
    
early_exit:
    FORCE_USE(result);
    return result;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf env;
static int setjmp_counter = 0;

NOINLINE int pattern_b_new_nodes(int depth, int max_depth) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    volatile int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    if (depth >= max_depth) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    }
    
    int ret = setjmp(env);
    if (ret == 0) {
        /* Complex arithmetic to create register pressure */
        a = b * c + d;
        b = e / (f + 1);
        c = g ^ h;
        d = i << 2;
        e = j >> 1;
        f = k | l;
        g = m & n;
        h = o - p;
        
        /* Loop with many variables live */
        for (int x = 0; x < 100; x++) {
            a += x;
            b -= x;
            c *= (x % 10) + 1;
            d ^= x;
            e = (e << 1) | (x & 1);
            f = (f >> 1) + x;
            g = g + h - x;
            h = h * 2 - x;
            
            if (x % 23 == 0) {
                setjmp_counter++;
                longjmp(env, x % 7 + 1);
            }
        }
        
        FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
        FORCE_USE(e); FORCE_USE(f); FORCE_USE(g); FORCE_USE(h);
        return pattern_b_new_nodes(depth + 1, max_depth);
    } else {
        /* Return path after longjmp */
        int sum = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
        FORCE_USE(sum);
        return sum + ret;
    }
}

/* ========== PATTERN C: MIXED PRESSURE with vector ops ========== */
typedef int v4si VECTOR_TYPE;

NOINLINE int pattern_c_mixed_pressure(int selector, int iterations) {
    volatile int r0 = 0, r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5, r6 = 6, r7 = 7;
    volatile int r8 = 8, r9 = 9, r10 = 10, r11 = 11, r12 = 12, r13 = 13, r14 = 14, r15 = 15;
    
    v4si v0 = {0, 1, 2, 3};
    v4si v1 = {4, 5, 6, 7};
    v4si v2 = {8, 9, 10, 11};
    v4si v3 = {12, 13, 14, 15};
    
    int result = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Large switch with vector operations */
        switch ((selector + iter) % 35) {
            case 0:
                v0 = v0 + v1;
                r0 = r1 + r2;
                r3 = r4 * r5;
                result += r0 + r3;
                break;
            case 1:
                v1 = v1 - v2;
                r1 = r2 - r3;
                r4 = r5 / (r6 + 1);
                result += r1 + r4;
                break;
            case 2:
                v2 = v2 * v3;
                r2 = r3 * r4;
                r5 = r6 ^ r7;
                result += r2 + r5;
                break;
            case 3:
                v3 = v0 & v1;
                r3 = r4 & r5;
                r6 = r7 | r8;
                result += r3 + r6;
                break;
            case 4:
                v0 = v1 | v2;
                r4 = r5 | r6;
                r7 = r8 << r9;
                result += r4 + r7;
                break;
            case 5:
                v1 = v2 ^ v3;
                r5 = r6 ^ r7;
                r8 = r9 >> r10;
                result += r5 + r8;
                break;
            case 6:
                v2 = v0 + v3;
                r6 = r7 + r8;
                r9 = r10 * r11;
                result += r6 + r9;
                break;
            case 7:
                v3 = v1 - v0;
                r7 = r8 - r9;
                r10 = r11 / (r12 + 1);
                result += r7 + r10;
                break;
            case 8:
                v0 = v2 * v1;
                r8 = r9 * r10;
                r11 = r12 ^ r13;
                result += r8 + r11;
                break;
            case 9:
                v1 = v3 & v0;
                r9 = r10 & r11;
                r12 = r13 | r14;
                result += r9 + r12;
                break;
            case 10:
                v2 = v0 | v1;
                r10 = r11 | r12;
                r13 = r14 << 1;
                result += r10 + r13;
                break;
            case 11:
                v3 = v1 ^ v2;
                r11 = r12 ^ r13;
                r14 = r15 >> 1;
                result += r11 + r14;
                break;
            case 12:
                v0 = v2 + v3;
                r12 = r13 + r14;
                r15 = r0 * r1;
                result += r12 + r15;
                break;
            case 13:
                v1 = v3 - v2;
                r13 = r14 - r15;
                r0 = r1 / (r2 + 1);
                result += r13 + r0;
                break;
            case 14:
                v2 = v0 * v3;
                r14 = r15 * r0;
                r1 = r2 ^ r3;
                result += r14 + r1;
                break;
            case 15:
                v3 = v1 & v2;
                r15 = r0 & r1;
                r2 = r3 | r4;
                result += r15 + r2;
                break;
            case 16:
                v0 = v2 | v3;
                r0 = r1 | r2;
                r3 = r4 << 2;
                result += r0 + r3;
                break;
            case 17:
                v1 = v3 ^ v0;
                r1 = r2 ^ r3;
                r4 = r5 >> 2;
                result += r1 + r4;
                break;
            case 18:
                v2 = v0 + v1;
                r2 = r3 + r4;
                r5 = r6 * r7;
                result += r2 + r5;
                break;
            case 19:
                v3 = v1 - v2;
                r3 = r4 - r5;
                r6 = r7 / (r8 + 1);
                result += r3 + r6;
                break;
            case 20:
                v0 = v2 * v3;
                r4 = r5 * r6;
                r7 = r8 ^ r9;
                result += r4 + r7;
                break;
            case 21:
                v1 = v3 & v0;
                r5 = r6 & r7;
                r8 = r9 | r10;
                result += r5 + r8;
                break;
            case 22:
                v2 = v0 | v1;
                r6 = r7 | r8;
                r9 = r10 << 3;
                result += r6 + r9;
                break;
            case 23:
                v3 = v1 ^ v2;
                r7 = r8 ^ r9;
                r10 = r11 >> 3;
                result += r7 + r10;
                break;
            case 24:
                v0 = v2 + v3;
                r8 = r9 + r10;
                r11 = r12 * r13;
                result += r8 + r11;
                break;
            case 25:
                v1 = v3 - v2;
                r9 = r10 - r11;
                r12 = r13 / (r14 + 1);
                result += r9 + r12;
                break;
            case 26:
                v2 = v0 * v1;
                r10 = r11 * r12;
                r13 = r14 ^ r15;
                result += r10 + r13;
                break;
            case 27:
                v3 = v1 & v2;
                r11 = r12 & r13;
                r14 = r15 | r0;
                result += r11 + r14;
                break;
            case 28:
                v0 = v2 | v3;
                r12 = r13 | r14;
                r15 = r0 << 4;
                result += r12 + r15;
                break;
            case 29:
                v1 = v3 ^ v0;
                r13 = r14 ^ r15;
                r0 = r1 >> 4;
                result += r13 + r0;
                break;
            case 30:
                v2 = v0 + v3;
                r14 = r15 + r0;
                r1 = r2 * r3;
                result += r14 + r1;
                break;
            case 31:
                v3 = v1 - v0;
                r15 = r0 - r1;
                r2 = r3 / (r4 + 1);
                result += r15 + r2;
                break;
            case 32:
                v0 = v1 * v2;
                r0 = r1 * r2;
                r3 = r4 ^ r5;
                result += r0 + r3;
                break;
            case 33:
                v1 = v2 & v3;
                r1 = r2 & r3;
                r4 = r5 | r6;
                result += r1 + r4;
                break;
            case 34:
                v2 = v3 | v0;
                r2 = r3 | r4;
                r5 = r6 << 5;
                result += r2 + r5;
                break;
        }
        
        /* Force all variables to be considered live */
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
        FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
        FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
    }
    
    return result;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
NOINLINE void dummy1(register int a asm ("r10"), register int b asm ("r11")) {
    FORCE_USE(a); FORCE_USE(b);
}

NOINLINE void dummy2(register int c asm ("r10"), register int d asm ("r12")) {
    FORCE_USE(c); FORCE_USE(d);
}

NOINLINE void dummy3(register int e asm ("r11"), register int f asm ("r13")) {
    FORCE_USE(e); FORCE_USE(f);
}

NOINLINE int pattern_d_register_conflict(int n) {
    register int x asm ("r10") = n;
    register int y asm ("r11") = n * 2;
    register int z asm ("r12") = n * 3;
    register int w asm ("r13") = n * 4;
    
    int result = 0;
    
    for (int i = 0; i < 100; i++) {
        /* Force conflicting register uses */
        dummy1(x, y);
        dummy2(z, w);
        dummy3(y, z);
        
        /* Complex arithmetic with register variables */
        x = x + i;
        y = y - i;
        z = z * ((i % 5) + 1);
        w = w ^ i;
        
        if (i % 7 == 0) {
            result += x;
        } else if (i % 7 == 1) {
            result += y;
        } else if (i % 7 == 2) {
            result += z;
        } else if (i % 7 == 3) {
            result += w;
        } else if (i % 7 == 4) {
            result += x + y;
        } else if (i % 7 == 5) {
            result += z + w;
        } else {
            result += x + y + z + w;
        }
        
        /* Create back-edges to different parts of the loop */
        switch (i % 11) {
            case 0: continue;
            case 1: break;
            case 2: continue;
            case 3: break;
            case 4: continue;
            case 5: break;
            case 6: continue;
            case 7: break;
            case 8: continue;
            case 9: break;
            case 10: continue;
        }
        
        /* More register pressure */
        dummy2(x, z);
        dummy1(y, w);
        dummy3(x, w);
    }
    
    FORCE_USE(x); FORCE_USE(y); FORCE_USE(z); FORCE_USE(w);
    return result;
}

/* ========== MAIN FUNCTION WITH PGO SUPPORT ========== */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int iterations = 1000;
    
    if (argc > 1) {
        verbose = 1;
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
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
    
    /* Call all patterns multiple times to generate profile data */
    for (int i = 0; i < iterations; i++) {
        int seed = i * 123456789;
        
        /* Pattern A - ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(i % 100 + 1, seed);
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        if (i % 10 == 0) {
            total += pattern_b_new_nodes(0, 3);
        }
        
        /* Pattern C - Mixed pressure with vector ops */
        total += pattern_c_mixed_pressure(seed % 100, 10);
        
        /* Pattern D - Artificial register conflict */
        total += pattern_d_register_conflict(seed % 1000);
        
        /* Prevent optimization of the loop */
        if (total > 1000000000) {
            total = total % 1000000;
        }
    }
    
    if (verbose) {
        printf("Final total: %d\n", total);
        printf("Setjmp calls: %d\n", setjmp_counter);
    }
    
    /* Force use of total to prevent dead code elimination */
    FORCE_USE(total);
    
    return total == 0 ? 0 : 1;
}
