/* test_mcf.c - Complex CFG to trigger MCF special node creation */
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

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

static int verbose = 0;

/* Pattern A: Deeply nested if-else with irreducible region */
HOT NOINLINE USED
int pattern_a_irreducible(int iterations, int seed) {
    int r0 = seed, r1 = seed * 2, r2 = seed * 3, r3 = seed * 4;
    int r4 = seed * 5, r5 = seed * 6, r6 = seed * 7, r7 = seed * 8;
    int result = 0;
    
    /* Create irreducible region with goto */
    if (iterations > 100) {
        goto middle;
    }
    
    for (int i = 0; i < iterations; i++) {
        /* Complex if-else chain */
        if (r0 % 3 == 0) {
            r1 = r2 * r3 + r4;
            r5 = r6 ^ r7;
            if (r1 > r5) {
                r0 = r1 - r5;
                goto middle;
            } else {
                r0 = r5 - r1;
                if (r0 % 2 == 0) {
                    r2 = r3 * r4;
                    r6 = r7 + 1;
                }
            }
        } else if (r0 % 3 == 1) {
            r2 = r3 * r4 - r5;
            r6 = r7 / 2;
            if (r2 < r6) {
                r1 = r6 - r2;
                goto middle;
            }
        } else {
            r3 = r4 + r5 * r6;
            r7 = r0 ^ r1;
        }
        
    middle:
        /* More arithmetic to create register pressure */
        r4 = r5 * r6 + r7;
        r0 = r1 ^ r2 ^ r3;
        r5 = r4 - r3;
        r6 = r7 * 2;
        
        KEEP(r0); KEEP(r1); KEEP(r2); KEEP(r3);
        KEEP(r4); KEEP(r5); KEEP(r6); KEEP(r7);
        
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    }
    
    return result;
}

/* Pattern B: Large switch with many cases and loops */
NOINLINE USED
int pattern_b_large_switch(int mode, int iterations) {
    int r0 = 0, r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10, r11 = 11;
    int r12 = 12, r13 = 13, r14 = 14, r15 = 15;
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        switch ((mode + i) % 35) {
            case 0:
                r0 = r1 + r2; r1 = r3 * r4; r2 = r5 ^ r6;
                r3 = r7 - r8; r4 = r9 / (r10 + 1); r5 = r11 % (r12 + 1);
                continue;  /* Jump back to loop start */
            case 1:
                r6 = r13 & r14; r7 = r15 | r0; r8 = r1 << 2;
                r9 = r2 >> 1; r10 = r3 + r4 + r5; r11 = r6 * r7;
                break;  /* Continue in loop */
            case 2:
                r12 = r8 - r9; r13 = r10 ^ r11; r14 = r12 & r13;
                r15 = r14 | r0; r0 = r1 + r2 + r3; r1 = r4 * r5;
                continue;
            case 3:
                r2 = r6 / (r7 + 1); r3 = r8 % (r9 + 1); r4 = r10 & r11;
                r5 = r12 | r13; r6 = r14 ^ r15; r7 = r0 << 3;
                break;
            case 4:
                r8 = r1 >> 2; r9 = r2 + r3; r10 = r4 * r5;
                r11 = r6 - r7; r12 = r8 ^ r9; r13 = r10 & r11;
                continue;
            case 5:
                r14 = r12 | r13; r15 = r14 ^ r0; r0 = r1 * r2;
                r1 = r3 + r4; r2 = r5 - r6; r3 = r7 / (r8 + 1);
                break;
            case 6:
                r4 = r9 % (r10 + 1); r5 = r11 & r12; r6 = r13 | r14;
                r7 = r15 ^ r0; r8 = r1 << 1; r9 = r2 >> 2;
                continue;
            case 7:
                r10 = r3 + r4; r11 = r5 * r6; r12 = r7 - r8;
                r13 = r9 ^ r10; r14 = r11 & r12; r15 = r13 | r14;
                break;
            case 8:
                r0 = r15 ^ r0; r1 = r1 + r2; r2 = r3 * r4;
                r3 = r5 - r6; r4 = r7 / (r8 + 1); r5 = r9 % (r10 + 1);
                continue;
            case 9:
                r6 = r11 & r12; r7 = r13 | r14; r8 = r15 ^ r0;
                r9 = r1 << 2; r10 = r2 >> 1; r11 = r3 + r4;
                break;
            case 10:
                r12 = r5 * r6; r13 = r7 - r8; r14 = r9 ^ r10;
                r15 = r11 & r12; r0 = r13 | r14; r1 = r15 ^ r0;
                continue;
            case 11:
                r2 = r1 + r2; r3 = r3 * r4; r4 = r5 - r6;
                r5 = r7 / (r8 + 1); r6 = r9 % (r10 + 1); r7 = r11 & r12;
                break;
            case 12:
                r8 = r13 | r14; r9 = r15 ^ r0; r10 = r1 << 3;
                r11 = r2 >> 2; r12 = r3 + r4; r13 = r5 * r6;
                continue;
            case 13:
                r14 = r7 - r8; r15 = r9 ^ r10; r0 = r11 & r12;
                r1 = r13 | r14; r2 = r15 ^ r0; r3 = r1 + r2;
                break;
            case 14:
                r4 = r3 * r4; r5 = r5 - r6; r6 = r7 / (r8 + 1);
                r7 = r9 % (r10 + 1); r8 = r11 & r12; r9 = r13 | r14;
                continue;
            case 15:
                r10 = r15 ^ r0; r11 = r1 << 1; r12 = r2 >> 3;
                r13 = r3 + r4; r14 = r5 * r6; r15 = r7 - r8;
                break;
            case 16:
                r0 = r9 ^ r10; r1 = r11 & r12; r2 = r13 | r14;
                r3 = r15 ^ r0; r4 = r1 + r2; r5 = r3 * r4;
                continue;
            case 17:
                r6 = r5 - r6; r7 = r7 / (r8 + 1); r8 = r9 % (r10 + 1);
                r9 = r11 & r12; r10 = r13 | r14; r11 = r15 ^ r0;
                break;
            case 18:
                r12 = r1 << 2; r13 = r2 >> 1; r14 = r3 + r4;
                r15 = r5 * r6; r0 = r7 - r8; r1 = r9 ^ r10;
                continue;
            case 19:
                r2 = r11 & r12; r3 = r13 | r14; r4 = r15 ^ r0;
                r5 = r1 + r2; r6 = r3 * r4; r7 = r5 - r6;
                break;
            case 20:
                r8 = r7 / (r8 + 1); r9 = r9 % (r10 + 1); r10 = r11 & r12;
                r11 = r13 | r14; r12 = r15 ^ r0; r13 = r1 << 3;
                continue;
            case 21:
                r14 = r2 >> 2; r15 = r3 + r4; r0 = r5 * r6;
                r1 = r7 - r8; r2 = r9 ^ r10; r3 = r11 & r12;
                break;
            case 22:
                r4 = r13 | r14; r5 = r15 ^ r0; r6 = r1 + r2;
                r7 = r3 * r4; r8 = r5 - r6; r9 = r7 / (r8 + 1);
                continue;
            case 23:
                r10 = r9 % (r10 + 1); r11 = r11 & r12; r12 = r13 | r14;
                r13 = r15 ^ r0; r14 = r1 << 1; r15 = r2 >> 3;
                break;
            case 24:
                r0 = r3 + r4; r1 = r5 * r6; r2 = r7 - r8;
                r3 = r9 ^ r10; r4 = r11 & r12; r5 = r13 | r14;
                continue;
            case 25:
                r6 = r15 ^ r0; r7 = r1 + r2; r8 = r3 * r4;
                r9 = r5 - r6; r10 = r7 / (r8 + 1); r11 = r9 % (r10 + 1);
                break;
            case 26:
                r12 = r11 & r12; r13 = r13 | r14; r14 = r15 ^ r0;
                r15 = r1 << 2; r0 = r2 >> 1; r1 = r3 + r4;
                continue;
            case 27:
                r2 = r5 * r6; r3 = r7 - r8; r4 = r9 ^ r10;
                r5 = r11 & r12; r6 = r13 | r14; r7 = r15 ^ r0;
                break;
            case 28:
                r8 = r1 + r2; r9 = r3 * r4; r10 = r5 - r6;
                r11 = r7 / (r8 + 1); r12 = r9 % (r10 + 1); r13 = r11 & r12;
                continue;
            case 29:
                r14 = r13 | r14; r15 = r15 ^ r0; r0 = r1 << 3;
                r1 = r2 >> 2; r2 = r3 + r4; r3 = r5 * r6;
                break;
            case 30:
                r4 = r7 - r8; r5 = r9 ^ r10; r6 = r11 & r12;
                r7 = r13 | r14; r8 = r15 ^ r0; r9 = r1 + r2;
                continue;
            case 31:
                r10 = r3 * r4; r11 = r5 - r6; r12 = r7 / (r8 + 1);
                r13 = r9 % (r10 + 1); r14 = r11 & r12; r15 = r13 | r14;
                break;
            case 32:
                r0 = r15 ^ r0; r1 = r1 + r2; r2 = r3 * r4;
                r3 = r5 - r6; r4 = r7 / (r8 + 1); r5 = r9 % (r10 + 1);
                continue;
            case 33:
                r6 = r11 & r12; r7 = r13 | r14; r8 = r15 ^ r0;
                r9 = r1 << 2; r10 = r2 >> 1; r11 = r3 + r4;
                break;
            case 34:
                r12 = r5 * r6; r13 = r7 - r8; r14 = r9 ^ r10;
                r15 = r11 & r12; r0 = r13 | r14; r1 = r15 ^ r0;
                continue;
            default:
                r2 = r1 + r2; r3 = r3 * r4; r4 = r5 - r6;
                r5 = r7 / (r8 + 1); r6 = r9 % (r10 + 1); r7 = r11 & r12;
                break;
        }
        
        /* Force all variables live */
        KEEP(r0); KEEP(r1); KEEP(r2); KEEP(r3); KEEP(r4); KEEP(r5);
        KEEP(r6); KEEP(r7); KEEP(r8); KEEP(r9); KEEP(r10); KEEP(r11);
        KEEP(r12); KEEP(r13); KEEP(r14); KEEP(r15);
        
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
                 r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    }
    
    return result;
}

/* Pattern C: setjmp/longjmp for exceptional edges */
static jmp_buf env;
NOINLINE USED
int pattern_c_setjmp(int iterations) {
    int r0 = 0, r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    int result = 0;
    
    if (setjmp(env) == 0) {
        for (int i = 0; i < iterations; i++) {
            /* Complex arithmetic that might overflow */
            r0 = r1 * r2 + r3;
            r1 = r4 - r5 * r6;
            r2 = r7 / (r8 + 1);
            r3 = r9 % (r10 + 1);
            r4 = r0 ^ r1 ^ r2;
            r5 = r3 & r4 | r5;
            r6 = r6 << (i % 4);
            r7 = r7 >> (i % 3);
            r8 = r8 + r9 * r10;
            r9 = r0 - r1 + r2;
            r10 = r3 * r4 / (r5 + 1);
            
            KEEP(r0); KEEP(r1); KEEP(r2); KEEP(r3); KEEP(r4);
            KEEP(r5); KEEP(r6); KEEP(r7); KEEP(r8); KEEP(r9); KEEP(r10);
            
            result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
            
            /* Occasionally longjmp out */
            if (i > iterations / 2 && (result % 1000) == 0) {
                longjmp(env, 1);
            }
        }
    } else {
        /* Land here after longjmp */
        result = -result;
    }
    
    return result;
}

/* Pattern D: Vector operations with register pressure */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE USED
int pattern_d_vector_mixed(int iterations) {
#ifdef __GNUC__
    v4si v0 = {0, 1, 2, 3};
    v4si v1 = {4, 5, 6, 7};
    v4si v2 = {8, 9, 10, 11};
    v4si v3 = {12, 13, 14, 15};
#endif
    int s0 = 0, s1 = 1, s2 = 2, s3 = 3, s4 = 4, s5 = 5;
    int s6 = 6, s7 = 7, s8 = 8, s9 = 9, s10 = 10, s11 = 11;
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix scalar and vector operations */
        s0 = s1 + s2 * s3;
        s1 = s4 - s5 / (s6 + 1);
        s2 = s7 % (s8 + 1) ^ s9;
        s3 = s10 & s11 | s0;
        
#ifdef __GNUC__
        v0 = v0 + v1;
        v1 = v1 * v2;
        v2 = v2 - v3;
        v3 = v3 ^ v0;
        
        /* Extract elements to force spills */
        s4 = v0[0] + v0[1];
        s5 = v0[2] + v0[3];
        s6 = v1[0] * v1[1];
        s7 = v1[2] * v1[3];
#endif
        
        s8 = s2 + s3 * s4;
        s9 = s5 - s6 / (s7 + 1);
        s10 = s8 % (s9 + 1) ^ s0;
        s11 = s1 & s2 | s3;
        
        KEEP(s0); KEEP(s1); KEEP(s2); KEEP(s3); KEEP(s4); KEEP(s5);
        KEEP(s6); KEEP(s7); KEEP(s8); KEEP(s9); KEEP(s10); KEEP(s11);
#ifdef __GNUC__
        KEEP(v0); KEEP(v1); KEEP(v2); KEEP(v3);
#endif
        
        result += s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 + s10 + s11;
#ifdef __GNUC__
        result += v0[0] + v0[1] + v0[2] + v0[3];
#endif
    }
    
    return result;
}

/* Pattern E: Explicit register variables */
#ifdef __GNUC__
NOINLINE USED
int pattern_e_explicit_registers(int iterations) {
    register int x asm ("r10") = 0;
    register int y asm ("r11") = 1;
    register int z asm ("r12") = 2;
    int a = 3, b = 4, c = 5, d = 6, e = 7, f = 8;
    int g = 9, h = 10, i = 11, j = 12, k = 13, l = 14;
    int result = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Force conflicts with explicit registers */
        x = y + z;
        y = a * b - c;
        z = d ^ e | f;
        a = g & h << 2;
        b = i >> 1 + j;
        c = k % (l + 1);
        d = x * y + z;
        e = a - b / (c + 1);
        f = d % (e + 1) ^ g;
        g = h & i | j;
        h = k + l * x;
        i = y - z / (a + 1);
        j = b % (c + 1) ^ d;
        k = e & f | g;
        l = h + i * j;
        
        /* Force all variables live */
        KEEP(x); KEEP(y); KEEP(z);
        KEEP(a); KEEP(b); KEEP(c); KEEP(d); KEEP(e); KEEP(f);
        KEEP(g); KEEP(h); KEEP(i); KEEP(j); KEEP(k); KEEP(l);
        
        result += x + y + z + a + b + c + d + e + f + g + h + i + j + k + l;
    }
    
    return result;
}
#else
int pattern_e_explicit_registers(int iterations) {
    return iterations;
}
#endif

/* Dummy helper functions for calls */
NOINLINE int helper1(int a, int b) { return a + b; }
NOINLINE int helper2(int a, int b) { return a - b; }
NOINLINE int helper3(int a, int b) { return a * b; }
NOINLINE int helper4(int a, int b) { return a ^ b; }

/* Main driver with profile-guided optimization */
int main(int argc, char **argv) {
    volatile int total = 0;  /* Prevent optimization */
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
    
    /* Use CPU features to engage target-specific allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 supported: %d\n", use_avx2);
    }
#endif
    
    /* Call all patterns multiple times with different arguments */
    for (int i = 0; i < 10; i++) {
        total += pattern_a_irreducible(iterations / 10, i * 123);
        total += pattern_b_large_switch(i % 5, iterations / 20);
        total += pattern_c_setjmp(iterations / 50);
        total += pattern_d_vector_mixed(iterations / 30);
        total += pattern_e_explicit_registers(iterations / 40);
        
        /* Mix in some helper calls */
        total += helper1(total, i);
        total += helper2(total, i * 2);
        total += helper3(total, i * 3);
        total += helper4(total, i * 4);
    }
    
    if (verbose) {
        printf("Final total: %d\n", total);
    }
    
    /* Return something non-trivial */
    return (total % 256) == 0 ? 0 : 1;
}
