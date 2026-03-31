/* test_mcf.c - Complex CFG generator to trigger MCF special node printing */
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
    /* Complex irreducible CFG with goto to force ENTRY_BLOCK + 1 and 2*EXIT_BLOCK */
    volatile int result = seed;
    int i, j, k;
    
    /* Create many local variables to increase register pressure */
    int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    int r12 = seed + 12, r13 = seed + 13, r14 = seed + 14, r15 = seed + 15;
    
    /* Label to create irreducible region */
    if (iterations > 100) {
        goto irreducible_region;
    }
    
    for (i = 0; i < iterations; i++) {
        /* Deep if-else chain creating many basic blocks */
        if (r0 % 3 == 0) {
            r1 = r0 * r2 + r3;
            if (r1 % 5 == 0) {
                r4 = r1 ^ r2;
                for (j = 0; j < 10; j++) {
                    r5 = r4 + j * r1;
                    if (r5 % 7 == 0) break;
                }
            } else if (r1 % 7 == 0) {
                r6 = r1 | r3;
                while (r6 < 1000) {
                    r6 = r6 * 2 + 1;
                }
            } else {
                r7 = r1 & r2;
                do {
                    r7 = (r7 << 1) ^ 0x5A5A;
                } while (r7 < 10000);
            }
        } else if (r0 % 5 == 0) {
            r8 = r0 / r2;
            switch (r8 % 4) {
                case 0: r9 = r8 + r1; break;
                case 1: r9 = r8 - r1; break;
                case 2: r9 = r8 * r1; break;
                case 3: r9 = r8 ^ r1; break;
            }
        } else {
            r10 = r0 % r2;
            for (k = 0; k < 5; k++) {
                r11 = r10 << k;
                if (k % 2 == 0) continue;
                r12 = r11 >> 1;
            }
        }
        
        r0 = (r0 * 1103515245 + 12345) & 0x7fffffff;
        
irreducible_region:
        /* This goto creates irreducible flow */
        if (i % 17 == 0) {
            r13 = r0 ^ r1;
            goto after_switch;
        }
        
        /* Small switch to add more edges */
        switch (r0 % 8) {
            case 0: r14 = r0 + r1; break;
            case 1: r14 = r0 - r1; break;
            case 2: r14 = r0 * r1; break;
            case 3: r14 = r0 / (r1 ? r1 : 1); break;
            case 4: r14 = r0 & r1; break;
            case 5: r14 = r0 | r1; break;
            case 6: r14 = r0 ^ r1; break;
            case 7: r14 = ~r0; break;
        }
        
after_switch:
        r15 = r14 + r13;
        result += r15;
    }
    
    /* Force all variables to be considered live */
    FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
    FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
    FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
    FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    
    return result;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
NOINLINE
int pattern_b_new_nodes(int iterations, int seed) {
    jmp_buf env;
    volatile int result = seed;
    int i;
    
    /* Many local variables for register pressure */
    int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    
    if (setjmp(env) == 0) {
        /* First call to setjmp */
        for (i = 0; i < iterations; i++) {
            /* Complex arithmetic that might overflow */
            v0 = v0 * 1664525 + 1013904223;
            v1 = v1 * 1103515245 + 12345;
            v2 = v2 * 214013 + 2531011;
            v3 = v3 * 134775813 + 1;
            
            v4 = (v0 ^ v1) + (v2 & v3);
            v5 = (v1 | v2) * (v3 ^ v0);
            v6 = v4 - v5;
            v7 = v5 / (v6 ? v6 : 1);
            
            v8 = v6 << (v7 % 16);
            v9 = v7 >> (v8 % 16);
            v10 = v8 & v9;
            v11 = v9 | v10;
            
            v12 = v10 * v11;
            v13 = v11 + v12;
            v14 = v12 - v13;
            v15 = v13 ^ v14;
            
            result += v15;
            
            /* Occasionally longjmp to create exceptional edge */
            if (i % 37 == 0 && i > 0) {
                longjmp(env, 1);
            }
        }
    } else {
        /* longjmp target - different execution path */
        for (i = 0; i < iterations / 2; i++) {
            v0 = v0 ^ v15;
            v1 = v1 + v14;
            v2 = v2 * v13;
            v3 = v3 - v12;
            
            result = result * 3 + v0 + v1 + v2 + v3;
        }
    }
    
    FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
    FORCE_USE(v4); FORCE_USE(v5); FORCE_USE(v6); FORCE_USE(v7);
    FORCE_USE(v8); FORCE_USE(v9); FORCE_USE(v10); FORCE_USE(v11);
    FORCE_USE(v12); FORCE_USE(v13); FORCE_USE(v14); FORCE_USE(v15);
    
    return result;
}

/* ========== PATTERN C: MIXED PRESSURE WITH VECTORS ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int iterations, int seed) {
    volatile int result = seed;
    int i, j;
    
    /* Scalar variables */
    int s0 = seed, s1 = seed + 1, s2 = seed + 2, s3 = seed + 3;
    int s4 = seed + 4, s5 = seed + 5, s6 = seed + 6, s7 = seed + 7;
    int s8 = seed + 8, s9 = seed + 9, s10 = seed + 10, s11 = seed + 11;
    int s12 = seed + 12, s13 = seed + 13, s14 = seed + 14, s15 = seed + 15;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0 = {seed, seed+1, seed+2, seed+3};
    v4si v1 = {seed+4, seed+5, seed+6, seed+7};
    v4si v2 = {seed+8, seed+9, seed+10, seed+11};
    v4si v3 = {seed+12, seed+13, seed+14, seed+15};
#endif
    
    for (i = 0; i < iterations; i++) {
        /* Large switch with 30+ cases */
        switch (s0 % 35) {
            case 0:
                s1 = s0 * s2 + s3;
                s4 = s1 ^ s2;
#ifdef __GNUC__
                v0 = v0 + v1;
#endif
                break;
            case 1:
                s5 = s0 - s2 * s3;
                s6 = s5 | s3;
#ifdef __GNUC__
                v1 = v1 - v2;
#endif
                break;
            case 2:
                s7 = s0 / (s2 ? s2 : 1);
                s8 = s7 & s3;
#ifdef __GNUC__
                v2 = v2 * v3;
#endif
                break;
            case 3:
                s9 = s0 << (s2 % 16);
                s10 = s9 >> (s3 % 8);
#ifdef __GNUC__
                v3 = v3 ^ v0;
#endif
                break;
            case 4:
                s11 = s0 + s2 - s3;
                s12 = s11 * s2;
#ifdef __GNUC__
                v0 = v0 | v1;
#endif
                break;
            case 5:
                s13 = s0 ^ s2 ^ s3;
                s14 = s13 + s2;
#ifdef __GNUC__
                v1 = v1 & v2;
#endif
                break;
            case 6:
                s15 = s0 % (s2 ? s2 : 1);
                s1 = s15 * s3;
#ifdef __GNUC__
                v2 = v2 << 1;
#endif
                break;
            case 7:
                s2 = s0 + s1 + s3;
                s3 = s2 - s1;
#ifdef __GNUC__
                v3 = v3 >> 1;
#endif
                break;
            case 8:
                s4 = s0 * 3 + s1 * 5;
                s5 = s4 / 7;
#ifdef __GNUC__
                v0 = v0 + (v4si){1,2,3,4};
#endif
                break;
            case 9:
                s6 = s0 & 0xFF;
                s7 = s6 | 0xAA;
#ifdef __GNUC__
                v1 = v1 * (v4si){2,2,2,2};
#endif
                break;
            case 10:
                s8 = ~s0;
                s9 = s8 ^ s1;
#ifdef __GNUC__
                v2 = v2 - v0;
#endif
                break;
            case 11:
                s10 = s0 + s1 * s2;
                s11 = s10 - s3;
#ifdef __GNUC__
                v3 = v3 | v1;
#endif
                break;
            case 12:
                s12 = s0 << 3;
                s13 = s12 >> 1;
#ifdef __GNUC__
                v0 = v0 & v2;
#endif
                break;
            case 13:
                s14 = s0 * s0;
                s15 = s14 % 997;
#ifdef __GNUC__
                v1 = v1 ^ v3;
#endif
                break;
            case 14:
                s1 = s0 + 12345;
                s2 = s1 * 1103515245;
#ifdef __GNUC__
                v2 = v2 + v1;
#endif
                break;
            case 15:
                s3 = s0 | 0x5555;
                s4 = s3 & 0xAAAA;
#ifdef __GNUC__
                v3 = v3 * v0;
#endif
                break;
            case 16:
                s5 = s0 ^ 0x3333;
                s6 = s5 + 0x6666;
#ifdef __GNUC__
                v0 = v0 - v3;
#endif
                break;
            case 17:
                s7 = s0 * 13;
                s8 = s7 / 17;
#ifdef __GNUC__
                v1 = v1 << 2;
#endif
                break;
            case 18:
                s9 = s0 % 19;
                s10 = s9 * 23;
#ifdef __GNUC__
                v2 = v2 >> 2;
#endif
                break;
            case 19:
                s11 = s0 + s1 + s2 + s3;
                s12 = s11 / 4;
#ifdef __GNUC__
                v3 = v3 + (v4si){4,3,2,1};
#endif
                break;
            case 20:
                s13 = s0 * s1 * s2;
                s14 = s13 % 10007;
#ifdef __GNUC__
                v0 = v0 * (v4si){3,3,3,3};
#endif
                break;
            case 21:
                s15 = s0 & s1 & s2;
                s1 = s15 | s3;
#ifdef __GNUC__
                v1 = v1 - v0;
#endif
                break;
            case 22:
                s2 = s0 ^ s1 ^ s2 ^ s3;
                s3 = ~s2;
#ifdef __GNUC__
                v2 = v2 | v3;
#endif
                break;
            case 23:
                s4 = (s0 << 4) | (s1 >> 4);
                s5 = s4 ^ 0xCCCC;
#ifdef __GNUC__
                v3 = v3 & v1;
#endif
                break;
            case 24:
                s6 = s0 + s1 * 2 + s2 * 3;
                s7 = s6 / 6;
#ifdef __GNUC__
                v0 = v0 ^ v2;
#endif
                break;
            case 25:
                s8 = s0 % 256;
                s9 = s8 * 257;
#ifdef __GNUC__
                v1 = v1 + (v4si){5,6,7,8};
#endif
                break;
            case 26:
                s10 = s0 * 0x9E3779B9;
                s11 = s10 + 0x7F4A7C15;
#ifdef __GNUC__
                v2 = v2 * (v4si){4,4,4,4};
#endif
                break;
            case 27:
                s12 = s0 | s1 | s2;
                s13 = s12 & s3;
#ifdef __GNUC__
                v3 = v3 - v2;
#endif
                break;
            case 28:
                s14 = s0 * 16807;
                s15 = s14 % 2147483647;
#ifdef __GNUC__
                v0 = v0 << 3;
#endif
                break;
            case 29:
                s1 = s0 + 0x12345678;
                s2 = s1 ^ 0x87654321;
#ifdef __GNUC__
                v1 = v1 >> 3;
#endif
                break;
            case 30:
                s3 = s0 * s0 / (s0 ? s0 : 1);
                s4 = s3 + s0;
#ifdef __GNUC__
                v2 = v2 + v0;
#endif
                break;
            case 31:
                s5 = s0 & 0xF0F0F0F0;
                s6 = s5 | 0x0F0F0F0F;
#ifdef __GNUC__
                v3 = v3 * v1;
#endif
                break;
            case 32:
                s7 = s0 ^ s1 ^ 0xAAAAAAAA;
                s8 = s7 + 0x55555555;
#ifdef __GNUC__
                v0 = v0 | v3;
#endif
                break;
            case 33:
                s9 = s0 * 65537;
                s10 = s9 % 65521;
#ifdef __GNUC__
                v1 = v1 & v2;
#endif
                break;
            case 34:
                s11 = s0 << 8;
                s12 = s11 >> 4;
#ifdef __GNUC__
                v2 = v2 ^ v3;
#endif
                break;
        }
        
        /* Loop with continue/break to different cases */
        for (j = 0; j < 5; j++) {
            if (s0 % 11 == 0) {
                s0 = (s0 * 1664525 + 1013904223) & 0x7fffffff;
                continue;
            }
            if (s0 % 13 == 0) {
                s0 = (s0 * 1103515245 + 12345) & 0x7fffffff;
                break;
            }
            s0 = (s0 + 1) & 0x7fffffff;
        }
        
        result += s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + 
                  s8 + s9 + s10 + s11 + s12 + s13 + s14 + s15;
    }
    
    FORCE_USE(s0); FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3);
    FORCE_USE(s4); FORCE_USE(s5); FORCE_USE(s6); FORCE_USE(s7);
    FORCE_USE(s8); FORCE_USE(s9); FORCE_USE(s10); FORCE_USE(s11);
    FORCE_USE(s12); FORCE_USE(s13); FORCE_USE(s14); FORCE_USE(s15);
    
#ifdef __GNUC__
    FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
#endif
    
    return result;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
#ifdef __GNUC__
NOINLINE void dummy1(register int a asm ("r10"), register int b asm ("r11")) {
    FORCE_USE(a); FORCE_USE(b);
}

NOINLINE void dummy2(register int c asm ("r10"), register int d asm ("r12")) {
    FORCE_USE(c); FORCE_USE(d);
}

NOINLINE void dummy3(register int e asm ("r11"), register int f asm ("r12")) {
    FORCE_USE(e); FORCE_USE(f);
}
#endif

NOINLINE
int pattern_d_artificial_conflict(int iterations, int seed) {
    volatile int result = seed;
    int i;
    
#ifdef __GNUC__
    /* Explicit register variables creating conflicts */
    register int x asm ("r10") = seed;
    register int y asm ("r11") = seed + 1;
    register int z asm ("r12") = seed + 2;
#endif
    
    int a = seed + 3, b = seed + 4, c = seed + 5, d = seed + 6;
    int e = seed + 7, f = seed + 8, g = seed + 9, h = seed + 10;
    
    for (i = 0; i < iterations; i++) {
        /* Complex arithmetic using all variables */
        a = x * y + z;
        b = y * z + x;
        c = z * x + y;
        d = a ^ b ^ c;
        
        e = (a << 3) | (b >> 2);
        f = (b << 2) | (c >> 3);
        g = (c << 1) | (a >> 4);
        h = d * e * f;
        
        /* Call dummy functions to force register conflicts */
#ifdef __GNUC__
        dummy1(x, y);
        dummy2(y, z);
        dummy3(z, x);
#endif
        
        /* More arithmetic */
        x = a + b + c;
        y = d + e + f;
        z = g + h + result;
        
        result = (x ^ y ^ z) + result;
        
        /* Switch to create control flow merges */
        switch (result % 7) {
            case 0: x = x * 2; break;
            case 1: y = y * 3; break;
            case 2: z = z * 5; break;
            case 3: x = x + y; break;
            case 4: y = y + z; break;
            case 5: z = z + x; break;
            case 6: x = x ^ y ^ z; break;
        }
    }
    
    FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
    FORCE_USE(e); FORCE_USE(f); FORCE_USE(g); FORCE_USE(h);
    
#ifdef __GNUC__
    FORCE_USE(x); FORCE_USE(y); FORCE_USE(z);
#endif
    
    return result;
}

/* ========== MAIN FUNCTION ========== */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int i, iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 100000) iterations = 100000;
    }
    
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
    }
    
    if (verbose) {
        printf("Starting MCF stress test with %d iterations\n", iterations);
    }
    
    /* Check CPU features to engage target-specific allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", use_avx2 ? "yes" : "no");
    }
#endif
    
    /* Run all patterns multiple times */
    for (i = 0; i < 10; i++) {
        int seed = i * 1234567 + 89101112;
        
        if (verbose && i % 3 == 0) {
            printf("Iteration %d, seed = %d\n", i, seed);
        }
        
        /* Pattern A - ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(iterations / 10, seed);
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        total += pattern_b_new_nodes(iterations / 20, seed + 1);
        
        /* Pattern C - Mixed pressure with vectors */
        total += pattern_c_mixed_pressure(iterations / 5, seed + 2);
        
        /* Pattern D - Artificial register conflict */
        total += pattern_d_artificial_conflict(iterations / 8, seed + 3);
        
        /* Prevent optimization */
        FORCE_USE(total);
    }
    
    if (verbose) {
        printf("Final total: %d\n", total);
    }
    
    return total != 0 ? 0 : 1;
}
