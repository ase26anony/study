/* test_mcf.c - Comprehensive test for GCC's Min-Cost Flow pass special nodes */
#ifdef __GNUC__
#define FORCE_LIVE(var) asm volatile("" : : "g"(var))
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))
#else
#define FORCE_LIVE(var) (void)(var)
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
    
    /* Create irreducible region with goto */
    if (selector > 100) {
        goto irreducible_label;
    }
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Deeply nested if-else chain */
        if (selector & 1) {
            if (selector & 2) {
                if (selector & 4) {
                    a = b + c;
                    b = c + d;
                    c = d + e;
                    d = e + f;
                } else {
                    e = f + g;
                    f = g + h;
                    g = h + i;
                    h = i + j;
                }
            } else {
                if (selector & 8) {
                    i = j + k;
                    j = k + l;
                    k = l + m;
                    l = m + n;
                } else {
                    m = n + o;
                    n = o + p;
                    o = p + a;
                    p = a + b;
                }
            }
        } else {
            if (selector & 16) {
                a = a * b - c;
                b = b * c - d;
                c = c * d - e;
                d = d * e - f;
            } else {
                e = e * f - g;
                f = f * g - h;
                g = g * h - i;
                h = h * i - j;
            }
        }
        
        /* Complex loop with continue to different points */
        if (iter % 3 == 0) {
            selector = (selector * 1103515245 + 12345) & 0x7fffffff;
            continue;
        }
        
        if (iter % 5 == 0) {
            irreducible_label:
            /* This creates irreducible flow */
            result += a + b + c + d;
            if (iter % 7 == 0) {
                goto out_of_loop;
            }
        }
        
        /* More arithmetic to increase register pressure */
        a = (a ^ b) | c;
        b = (b ^ c) | d;
        c = (c ^ d) | e;
        d = (d ^ e) | f;
        e = (e ^ f) | g;
        f = (f ^ g) | h;
        g = (g ^ h) | i;
        h = (h ^ i) | j;
        
        FORCE_LIVE(a); FORCE_LIVE(b); FORCE_LIVE(c); FORCE_LIVE(d);
        FORCE_LIVE(e); FORCE_LIVE(f); FORCE_LIVE(g); FORCE_LIVE(h);
    }
    
    out_of_loop:
    result += a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    FORCE_LIVE(result);
    return result;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
static int jump_counter = 0;

NOINLINE
int pattern_b_new_exit_entry(int depth, int width) {
    int r0 = 1, r1 = 2, r2 = 3, r3 = 4, r4 = 5, r5 = 6, r6 = 7, r7 = 8;
    int r8 = 9, r9 = 10, r10 = 11, r11 = 12, r12 = 13, r13 = 14, r14 = 15, r15 = 16;
    int sum = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        for (int i = 0; i < depth; i++) {
            /* Complex arithmetic on all registers */
            r0 = r0 * r1 + r2;
            r1 = r1 * r2 + r3;
            r2 = r2 * r3 + r4;
            r3 = r3 * r4 + r5;
            r4 = r4 * r5 + r6;
            r5 = r5 * r6 + r7;
            r6 = r6 * r7 + r8;
            r7 = r7 * r8 + r9;
            r8 = r8 * r9 + r10;
            r9 = r9 * r10 + r11;
            r10 = r10 * r11 + r12;
            r11 = r11 * r12 + r13;
            r12 = r12 * r13 + r14;
            r13 = r13 * r14 + r15;
            r14 = r14 * r15 + r0;
            r15 = r15 * r0 + r1;
            
            sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
                   r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
            
            /* Occasionally longjmp out */
            if (i == depth / 2 && jump_counter < 3) {
                jump_counter++;
                longjmp(jump_buffer, 1);
            }
        }
    } else {
        /* After longjmp */
        for (int i = 0; i < width; i++) {
            r0 = r0 ^ r1 ^ r2;
            r1 = r1 ^ r2 ^ r3;
            r2 = r2 ^ r3 ^ r4;
            r3 = r3 ^ r4 ^ r5;
            r4 = r4 ^ r5 ^ r6;
            r5 = r5 ^ r6 ^ r7;
            r6 = r6 ^ r7 ^ r8;
            r7 = r7 ^ r8 ^ r9;
            
            sum += r0 * r1 * r2 * r3;
        }
    }
    
    FORCE_LIVE(r0); FORCE_LIVE(r1); FORCE_LIVE(r2); FORCE_LIVE(r3);
    FORCE_LIVE(r4); FORCE_LIVE(r5); FORCE_LIVE(r6); FORCE_LIVE(r7);
    FORCE_LIVE(r8); FORCE_LIVE(r9); FORCE_LIVE(r10); FORCE_LIVE(r11);
    FORCE_LIVE(r12); FORCE_LIVE(r13); FORCE_LIVE(r14); FORCE_LIVE(r15);
    FORCE_LIVE(sum);
    
    return sum;
}

/* ========== PATTERN C: MIXED PRESSURE with vector ops ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int base, int mod) {
    int result = 0;
    
    /* Scalar variables */
    int s0 = base + 0, s1 = base + 1, s2 = base + 2, s3 = base + 3;
    int s4 = base + 4, s5 = base + 5, s6 = base + 6, s7 = base + 7;
    int s8 = base + 8, s9 = base + 9, s10 = base + 10, s11 = base + 11;
    int s12 = base + 12, s13 = base + 13, s14 = base + 14, s15 = base + 15;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0 = {s0, s1, s2, s3};
    v4si v1 = {s4, s5, s6, s7};
    v4si v2 = {s8, s9, s10, s11};
    v4si v3 = {s12, s13, s14, s15};
#endif
    
    /* Large switch statement with 30+ cases */
    for (int i = 0; i < 100; i++) {
        int selector = (i * 1103515245 + 12345) % mod;
        
        switch (selector) {
            case 0:
                s0 = s1 + s2; s1 = s2 + s3; s2 = s3 + s4; s3 = s4 + s5;
#ifdef __GNUC__
                v0 = v0 + v1;
#endif
                break;
            case 1:
                s4 = s5 + s6; s5 = s6 + s7; s6 = s7 + s8; s7 = s8 + s9;
#ifdef __GNUC__
                v1 = v1 + v2;
#endif
                break;
            case 2:
                s8 = s9 + s10; s9 = s10 + s11; s10 = s11 + s12; s11 = s12 + s13;
#ifdef __GNUC__
                v2 = v2 + v3;
#endif
                break;
            case 3:
                s12 = s13 + s14; s13 = s14 + s15; s14 = s15 + s0; s15 = s0 + s1;
#ifdef __GNUC__
                v3 = v3 + v0;
#endif
                break;
            case 4:
                s0 = s0 * s1 - s2; s1 = s1 * s2 - s3; s2 = s2 * s3 - s4; s3 = s3 * s4 - s5;
#ifdef __GNUC__
                v0 = v0 * v1;
#endif
                break;
            case 5:
                s4 = s4 * s5 - s6; s5 = s5 * s6 - s7; s6 = s6 * s7 - s8; s7 = s7 * s8 - s9;
#ifdef __GNUC__
                v1 = v1 * v2;
#endif
                break;
            case 6:
                s8 = s8 * s9 - s10; s9 = s9 * s10 - s11; s10 = s10 * s11 - s12; s11 = s11 * s12 - s13;
#ifdef __GNUC__
                v2 = v2 * v3;
#endif
                break;
            case 7:
                s12 = s12 * s13 - s14; s13 = s13 * s14 - s15; s14 = s14 * s15 - s0; s15 = s15 * s0 - s1;
#ifdef __GNUC__
                v3 = v3 * v0;
#endif
                break;
            case 8:
                s0 = s0 ^ s1; s1 = s1 ^ s2; s2 = s2 ^ s3; s3 = s3 ^ s4;
#ifdef __GNUC__
                v0 = v0 ^ v1;
#endif
                break;
            case 9:
                s4 = s4 ^ s5; s5 = s5 ^ s6; s6 = s6 ^ s7; s7 = s7 ^ s8;
#ifdef __GNUC__
                v1 = v1 ^ v2;
#endif
                break;
            case 10:
                s8 = s8 ^ s9; s9 = s9 ^ s10; s10 = s10 ^ s11; s11 = s11 ^ s12;
#ifdef __GNUC__
                v2 = v2 ^ v3;
#endif
                break;
            case 11:
                s12 = s12 ^ s13; s13 = s13 ^ s14; s14 = s14 ^ s15; s15 = s15 ^ s0;
#ifdef __GNUC__
                v3 = v3 ^ v0;
#endif
                break;
            case 12:
                s0 = (s0 << 1) | (s1 >> 31); s1 = (s1 << 1) | (s2 >> 31);
                s2 = (s2 << 1) | (s3 >> 31); s3 = (s3 << 1) | (s4 >> 31);
                break;
            case 13:
                s4 = (s4 << 2) | (s5 >> 30); s5 = (s5 << 2) | (s6 >> 30);
                s6 = (s6 << 2) | (s7 >> 30); s7 = (s7 << 2) | (s8 >> 30);
                break;
            case 14:
                s8 = (s8 << 3) | (s9 >> 29); s9 = (s9 << 3) | (s10 >> 29);
                s10 = (s10 << 3) | (s11 >> 29); s11 = (s11 << 3) | (s12 >> 29);
                break;
            case 15:
                s12 = (s12 << 4) | (s13 >> 28); s13 = (s13 << 4) | (s14 >> 28);
                s14 = (s14 << 4) | (s15 >> 28); s15 = (s15 << 4) | (s0 >> 28);
                break;
            case 16:
                s0 = s0 + s15; s1 = s1 + s14; s2 = s2 + s13; s3 = s3 + s12;
                break;
            case 17:
                s4 = s4 + s11; s5 = s5 + s10; s6 = s6 + s9; s7 = s7 + s8;
                break;
            case 18:
                s8 = s8 + s7; s9 = s9 + s6; s10 = s10 + s5; s11 = s11 + s4;
                break;
            case 19:
                s12 = s12 + s3; s13 = s13 + s2; s14 = s14 + s1; s15 = s15 + s0;
                break;
            case 20:
                s0 = s0 - s1; s1 = s1 - s2; s2 = s2 - s3; s3 = s3 - s4;
                break;
            case 21:
                s4 = s4 - s5; s5 = s5 - s6; s6 = s6 - s7; s7 = s7 - s8;
                break;
            case 22:
                s8 = s8 - s9; s9 = s9 - s10; s10 = s10 - s11; s11 = s11 - s12;
                break;
            case 23:
                s12 = s12 - s13; s13 = s13 - s14; s14 = s14 - s15; s15 = s15 - s0;
                break;
            case 24:
                s0 = s0 & s1; s1 = s1 & s2; s2 = s2 & s3; s3 = s3 & s4;
                break;
            case 25:
                s4 = s4 & s5; s5 = s5 & s6; s6 = s6 & s7; s7 = s7 & s8;
                break;
            case 26:
                s8 = s8 & s9; s9 = s9 & s10; s10 = s10 & s11; s11 = s11 & s12;
                break;
            case 27:
                s12 = s12 & s13; s13 = s13 & s14; s14 = s14 & s15; s15 = s15 & s0;
                break;
            case 28:
                s0 = s0 | s1; s1 = s1 | s2; s2 = s2 | s3; s3 = s3 | s4;
                break;
            case 29:
                s4 = s4 | s5; s5 = s5 | s6; s6 = s6 | s7; s7 = s7 | s8;
                break;
            case 30:
                s8 = s8 | s9; s9 = s9 | s10; s10 = s10 | s11; s11 = s11 | s12;
                break;
            case 31:
                s12 = s12 | s13; s13 = s13 | s14; s14 = s14 | s15; s15 = s15 | s0;
                break;
            case 32:
                s0 = ~s0; s1 = ~s1; s2 = ~s2; s3 = ~s3;
                break;
            case 33:
                s4 = ~s4; s5 = ~s5; s6 = ~s6; s7 = ~s7;
                break;
            case 34:
                s8 = ~s8; s9 = ~s9; s10 = ~s10; s11 = ~s11;
                break;
            case 35:
                s12 = ~s12; s13 = ~s13; s14 = ~s14; s15 = ~s15;
                break;
            default:
                /* Complex default case */
                s0 = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7;
                s1 = s8 + s9 + s10 + s11 + s12 + s13 + s14 + s15;
                s2 = s0 * s1;
                s3 = s2 / (mod + 1);
                break;
        }
        
        /* Loop control with complex conditions */
        if (i % 7 == 0) {
            continue;
        }
        if (i % 11 == 0) {
            break;
        }
        if (i % 13 == 0) {
            i += 2;
            continue;
        }
        
        FORCE_LIVE(s0); FORCE_LIVE(s1); FORCE_LIVE(s2); FORCE_LIVE(s3);
        FORCE_LIVE(s4); FORCE_LIVE(s5); FORCE_LIVE(s6); FORCE_LIVE(s7);
        FORCE_LIVE(s8); FORCE_LIVE(s9); FORCE_LIVE(s10); FORCE_LIVE(s11);
        FORCE_LIVE(s12); FORCE_LIVE(s13); FORCE_LIVE(s14); FORCE_LIVE(s15);
    }
    
    result = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + 
             s8 + s9 + s10 + s11 + s12 + s13 + s14 + s15;
    
#ifdef __GNUC__
    /* Use vector results */
    int vsum[4];
    memcpy(vsum, &v0, sizeof(vsum));
    result += vsum[0] + vsum[1] + vsum[2] + vsum[3];
    memcpy(vsum, &v1, sizeof(vsum));
    result += vsum[0] + vsum[1] + vsum[2] + vsum[3];
    memcpy(vsum, &v2, sizeof(vsum));
    result += vsum[0] + vsum[1] + vsum[2] + vsum[3];
    memcpy(vsum, &v3, sizeof(vsum));
    result += vsum[0] + vsum[1] + vsum[2] + vsum[3];
#endif
    
    FORCE_LIVE(result);
    return result;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
#ifdef __GNUC__
register int reg_var1 asm ("r10");
register int reg_var2 asm ("r11");
#else
int reg_var1, reg_var2;
#endif

NOINLINE void dummy1(int x) { FORCE_LIVE(x); }
NOINLINE void dummy2(int x, int y) { FORCE_LIVE(x); FORCE_LIVE(y); }
NOINLINE void dummy3(int x, int y, int z) { FORCE_LIVE(x); FORCE_LIVE(y); FORCE_LIVE(z); }

NOINLINE
int pattern_d_artificial_conflict(int n) {
    int result = 0;
    
    /* Force use of register variables in conflicting ways */
#ifdef __GNUC__
    reg_var1 = n;
    reg_var2 = n * 2;
#endif
    
    for (int i = 0; i < n; i++) {
        /* Call dummy functions to split live ranges */
        dummy1(reg_var1);
        
        /* Complex arithmetic using register variables */
#ifdef __GNUC__
        int temp1 = reg_var1 * reg_var2;
        int temp2 = reg_var1 + reg_var2;
        int temp3 = reg_var1 - reg_var2;
        int temp4 = reg_var1 ^ reg_var2;
        
        reg_var1 = temp1 + i;
        reg_var2 = temp2 - i;
        
        dummy2(temp3, temp4);
        
        /* More complex operations */
        temp1 = reg_var1 * reg_var1;
        temp2 = reg_var2 * reg_var2;
        temp3 = temp1 + temp2;
        temp4 = temp1 - temp2;
        
        dummy3(temp1, temp2, temp3);
        
        result += temp4;
#endif
    }
    
    /* Final conflict */
#ifdef __GNUC__
    dummy2(reg_var1, reg_var2);
    result += reg_var1 + reg_var2;
#endif
    
    FORCE_LIVE(result);
    return result;
}

/* ========== MAIN FUNCTION ========== */
int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose = 1;
    }
    
    int total = 0;
    
    /* Use CPU features to engage target-specific register allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 supported: %d\n", use_avx2);
    }
#endif
    
    /* Call each pattern multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        total += pattern_a_entry_exit(i % 50 + 10, i * 7 + 1);
        total += pattern_b_new_exit_entry(i % 20 + 5, i % 15 + 3);
        total += pattern_c_mixed_pressure(i, 36);  /* 36 > 30 for switch cases */
        total += pattern_d_artificial_conflict(i % 40 + 1);
        
        /* Prevent optimization */
        FORCE_LIVE(total);
    }
    
    /* Store in volatile to prevent dead code elimination */
    global_sink = total;
    
    if (verbose) {
        printf("Total result: %d\n", total);
    }
    
    return 0;
}
