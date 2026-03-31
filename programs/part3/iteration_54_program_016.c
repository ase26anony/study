/* test_mcf_coverage.c - Complex CFG generator for MCF pass coverage testing */
#ifdef __GNUC__
#define FORCE_USE(var) asm volatile("" : : "g"(var))
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define REG_VAR(name, reg) register int name asm(reg)
#else
#define FORCE_USE(var) (void)(var)
#define NOINLINE
#define HOT
#define REG_VAR(name, reg) int name
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

static int verbose = 0;

/* Pattern A: Function targeting ENTRY/EXIT blocks with irreducible region */
HOT NOINLINE static int pattern_a_entry_exit(int iterations, int seed) {
    int result = seed;
    int i, j, k;
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    
    /* Create complex initial block that should become ENTRY_BLOCK */
    r0 = seed * 1103515245 + 12345;
    r1 = r0 ^ (r0 >> 16);
    r2 = r1 * 16807 % 2147483647;
    r3 = r2 ^ seed;
    r4 = r3 + r2;
    r5 = r4 - r3;
    
    /* Irreducible region using goto */
    if (iterations > 100) {
        goto middle_of_loop;
    }
    
    for (i = 0; i < iterations; i++) {
        /* Complex arithmetic to create register pressure */
        r6 = i * i;
        r7 = r6 + r0;
        r8 = r7 * r1;
        r9 = r8 % 997;
        r10 = r9 ^ r2;
        
        if (r10 < 500) {
            /* Forward goto creating irreducible flow */
            if (i % 3 == 0) goto loop_tail;
        } else {
            r0 = r10 * r3;
            r1 = r0 + r4;
        }
        
        middle_of_loop:
        r2 = r1 - r5;
        r3 = r2 * r6;
        
        if (r3 % 7 == 0) {
            continue;
        }
        
        loop_tail:
        r4 = r3 / (r7 + 1);
        r5 = r4 | r8;
        result += r5;
        
        /* Nested loop for additional complexity */
        for (j = 0; j < 5; j++) {
            for (k = 0; k < 3; k++) {
                r6 = (r5 << j) + k;
                r7 = r6 ^ result;
                result = r7 - (j * k);
            }
            if (j == 2) break;
        }
    }
    
    FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
    FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
    FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10);
    
    return result & 0x7FFFFFFF;
}

/* Pattern B: Function using setjmp/longjmp for exceptional edges */
NOINLINE static int pattern_b_new_indices(int depth, int max_depth) {
    static jmp_buf env;
    int result = 0;
    int vars[20];
    
    /* Many local variables to increase register pressure */
    int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
    int b0, b1, b2, b3, b4, b5, b6, b7, b8, b9;
    
    a0 = depth * 3;
    a1 = a0 + 1;
    a2 = a1 * 2;
    a3 = a2 - depth;
    a4 = a3 ^ 0x55AA55AA;
    a5 = a4 >> 4;
    a6 = a5 | 0xFF;
    a7 = a6 & 0xF0F0F0F0;
    a8 = a7 + a0;
    a9 = a8 * a1;
    
    if (setjmp(env) == 0) {
        /* Normal path with complex arithmetic */
        for (int i = 0; i < 10; i++) {
            b0 = i * i;
            b1 = b0 + a0;
            b2 = b1 * a1;
            b3 = b2 % 257;
            b4 = b3 ^ a2;
            b5 = b4 << (i % 4);
            b6 = b5 - a3;
            b7 = b6 | a4;
            b8 = b7 & a5;
            b9 = b8 + a6;
            
            vars[i] = b9;
            vars[i + 10] = b9 ^ a7;
            
            /* Conditionally longjmp to create exceptional edge */
            if (depth < max_depth && (b9 % 13 == 0)) {
                pattern_b_new_indices(depth + 1, max_depth);
                longjmp(env, 1);
            }
        }
        
        /* Complex switch inside loop to create many basic blocks */
        int switch_var = a9 % 20;
        for (int i = 0; i < 5; i++) {
            switch ((switch_var + i) % 20) {
                case 0: result += vars[0] * 2; break;
                case 1: result += vars[1] | 0xFF; break;
                case 2: result += vars[2] ^ vars[3]; break;
                case 3: result += vars[4] - vars[5]; break;
                case 4: result += vars[6] << 2; break;
                case 5: result += vars[7] >> 1; break;
                case 6: result += vars[8] % 17; break;
                case 7: result += vars[9] & 0xF0; break;
                case 8: result += vars[10] + 1; break;
                case 9: result += vars[11] * 3; break;
                case 10: result += vars[12] | 0xAA; break;
                case 11: result += vars[13] ^ 0x55; break;
                case 12: result += vars[14] - 100; break;
                case 13: result += vars[15] << 3; break;
                case 14: result += vars[16] >> 2; break;
                case 15: result += vars[17] % 23; break;
                case 16: result += vars[18] & 0x0F; break;
                case 17: result += vars[19] + 255; break;
                case 18: result += a0 * a1; break;
                case 19: result += a2 ^ a3; break;
            }
            FORCE_USE(a0); FORCE_USE(a1); FORCE_USE(a2); FORCE_USE(a3);
            FORCE_USE(a4); FORCE_USE(a5); FORCE_USE(a6); FORCE_USE(a7);
            FORCE_USE(a8); FORCE_USE(a9);
        }
    } else {
        /* longjmp target - different execution path */
        result = a9 ^ 0x12345678;
    }
    
    FORCE_USE(b0); FORCE_USE(b1); FORCE_USE(b2); FORCE_USE(b3);
    FORCE_USE(b4); FORCE_USE(b5); FORCE_USE(b6); FORCE_USE(b7);
    FORCE_USE(b8); FORCE_USE(b9);
    
    return result;
}

/* Pattern C: Mixed pressure with vector operations */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE static int pattern_c_mixed_pressure(int base, int iterations) {
    int result = base;
    
    /* Many scalar variables */
    int s0, s1, s2, s3, s4, s5, s6, s7, s8, s9;
    int s10, s11, s12, s13, s14, s15, s16, s17, s18, s19;
    
    s0 = base;
    s1 = s0 + 1;
    s2 = s1 * 2;
    s3 = s2 - base;
    s4 = s3 ^ 0xDEADBEEF;
    s5 = s4 >> 3;
    s6 = s5 | 0x7F;
    s7 = s6 & 0x3C3C3C3C;
    s8 = s7 + s0;
    s9 = s8 * s1;
    s10 = s9 % 1009;
    s11 = s10 ^ s2;
    s12 = s11 << 1;
    s13 = s12 - s3;
    s14 = s13 | s4;
    s15 = s14 & s5;
    s16 = s15 + s6;
    s17 = s16 * s7;
    s18 = s17 % 997;
    s19 = s18 ^ s8;
    
#ifdef __GNUC__
    /* Vector operations */
    v4si v0 = {s0, s1, s2, s3};
    v4si v1 = {s4, s5, s6, s7};
    v4si v2 = {s8, s9, s10, s11};
    v4si v3 = {s12, s13, s14, s15};
    v4si v4 = {s16, s17, s18, s19};
    
    v4si vsum = v0 + v1;
    v4si vprod = v2 * v3;
    v4si vxor = vsum ^ vprod;
    v4si vand = vxor & v4;
    
    /* Extract results from vectors */
    int varr[4];
    memcpy(varr, &vand, sizeof(vand));
    result += varr[0] + varr[1] + varr[2] + varr[3];
#endif
    
    /* Large switch statement with 30+ cases inside loop */
    for (int i = 0; i < iterations; i++) {
        int switch_val = (s19 + i * 9973) % 35;
        
        switch (switch_val) {
            case 0: result += s0 * 2; break;
            case 1: result += s1 | 0xFF; break;
            case 2: result += s2 ^ s3; break;
            case 3: result += s4 - s5; break;
            case 4: result += s6 << 2; break;
            case 5: result += s7 >> 1; break;
            case 6: result += s8 % 17; break;
            case 7: result += s9 & 0xF0; break;
            case 8: result += s10 + 1; break;
            case 9: result += s11 * 3; break;
            case 10: result += s12 | 0xAA; break;
            case 11: result += s13 ^ 0x55; break;
            case 12: result += s14 - 100; break;
            case 13: result += s15 << 3; break;
            case 14: result += s16 >> 2; break;
            case 15: result += s17 % 23; break;
            case 16: result += s18 & 0x0F; break;
            case 17: result += s19 + 255; break;
            case 18: result += s0 * s1; break;
            case 19: result += s2 ^ s3; break;
            case 20: result += s4 | s5; break;
            case 21: result += s6 - s7; break;
            case 22: result += s8 << 1; break;
            case 23: result += s9 >> 3; break;
            case 24: result += s10 % 29; break;
            case 25: result += s11 & 0xCC; break;
            case 26: result += s12 + 512; break;
            case 27: result += s13 * 5; break;
            case 28: result += s14 | 0x33; break;
            case 29: result += s15 ^ 0x99; break;
            case 30: result += s16 - 200; break;
            case 31: result += s17 << 4; break;
            case 32: result += s18 >> 2; break;
            case 33: result += s19 % 31; break;
            case 34: result += s0 & s1; break;
            default: result += i; break;
        }
        
        /* Use continue/break to different cases */
        if (result % 7 == 0) {
            continue;
        } else if (result % 13 == 0) {
            switch_val = (switch_val + 1) % 35;
            /* Simulate goto to different case */
            if (switch_val < 10) continue;
        }
        
        FORCE_USE(s0); FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3);
        FORCE_USE(s4); FORCE_USE(s5); FORCE_USE(s6); FORCE_USE(s7);
        FORCE_USE(s8); FORCE_USE(s9); FORCE_USE(s10); FORCE_USE(s11);
        FORCE_USE(s12); FORCE_USE(s13); FORCE_USE(s14); FORCE_USE(s15);
        FORCE_USE(s16); FORCE_USE(s17); FORCE_USE(s18); FORCE_USE(s19);
    }
    
    return result;
}

/* Pattern D: Artificial register conflicts */
NOINLINE static int helper1(int x) { return x * 3 + 1; }
NOINLINE static int helper2(int x) { return x ^ 0x55AA55AA; }
NOINLINE static int helper3(int x) { return (x << 3) | (x >> 29); }
NOINLINE static int helper4(int x) { return x % 997; }

NOINLINE static int pattern_d_register_conflict(int n) {
    /* Explicit register variables creating conflicts */
    REG_VAR(r0, "r10");
    REG_VAR(r1, "r11");
    REG_VAR(r2, "r12");
    REG_VAR(r3, "r13");
    REG_VAR(r4, "r14");
    REG_VAR(r5, "r15");
    
    int result = n;
    
    /* Force register usage conflicts */
    r0 = n;
    r1 = helper1(r0);
    r2 = helper2(r1);
    r0 = helper3(r2);  /* Reuse r0 */
    r3 = helper4(r0);
    r1 = helper1(r3);  /* Reuse r1 */
    r4 = helper2(r1);
    r2 = helper3(r4);  /* Reuse r2 */
    r5 = helper4(r2);
    
    /* Complex loop with register pressure */
    for (int i = 0; i < n % 100; i++) {
        /* More register conflicts */
        int t0 = r0 + i;
        int t1 = r1 - i;
        int t2 = r2 * i;
        int t3 = r3 ^ i;
        int t4 = r4 | i;
        int t5 = r5 & i;
        
        /* Call helpers with conflicting registers */
        r0 = helper1(t0);
        r1 = helper2(t1);
        r2 = helper3(t2);
        r3 = helper4(t3);
        r4 = helper1(t4);
        r5 = helper2(t5);
        
        result += r0 + r1 + r2 + r3 + r4 + r5;
        
        /* Conditional with many live variables */
        if (result % 2 == 0) {
            r0 = r1 ^ r2;
            r1 = r3 | r4;
            r2 = r5 & r0;
        } else {
            r3 = r0 << 1;
            r4 = r1 >> 2;
            r5 = r2 * 3;
        }
        
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2);
        FORCE_USE(r3); FORCE_USE(r4); FORCE_USE(r5);
    }
    
    result += r0 + r1 + r2 + r3 + r4 + r5;
    return result;
}

/* Main driver with PGO support */
int main(int argc, char **argv) {
    volatile int total = 0;  /* volatile to prevent optimization */
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 100000) iterations = 100000;
    }
    
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
    }
    
    if (verbose) {
        printf("Starting MCF coverage test with %d iterations\n", iterations);
    }
    
    /* Check CPU features to engage target-specific allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", use_avx2 ? "yes" : "no");
    }
#endif
    
    /* Call all pattern functions multiple times to build profile */
    for (int i = 0; i < iterations; i++) {
        int seed = i * 1103515245 + 12345;
        
        /* Pattern A - targeting ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(50 + (i % 50), seed);
        
        /* Pattern B - targeting NEW_EXIT/NEW_ENTRY with setjmp */
        if (i % 10 == 0) {
            total += pattern_b_new_indices(0, 3);
        }
        
        /* Pattern C - mixed pressure with vectors and large switch */
        total += pattern_c_mixed_pressure(seed % 1000, 10 + (i % 20));
        
        /* Pattern D - artificial register conflicts */
        total += pattern_d_register_conflict(seed % 200);
        
        /* Prevent loop unrolling from simplifying CFG */
        if (total > 0x7FFFFFFF) {
            total = total & 0x7FFFFFFF;
        }
    }
    
    if (verbose) {
        printf("Final total: %d\n", total);
    }
    
    return total == 0 ? 1 : 0;
}
