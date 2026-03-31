/* mcf_coverage.c - Program to trigger MCF fixup graph debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Complex irreducible CFG with goto jumps */
NOINLINE static uint64_t test_irreducible_goto(int iterations, int seed) {
    /* Declare many variables to increase register pressure */
    VOLATILE_VAR int v0 = seed + 1;
    VOLATILE_VAR int v1 = seed + 2;
    VOLATILE_VAR int v2 = seed + 3;
    VOLATILE_VAR int v3 = seed + 4;
    VOLATILE_VAR int v4 = seed + 5;
    VOLATILE_VAR int v5 = seed + 6;
    VOLATILE_VAR int v6 = seed + 7;
    VOLATILE_VAR int v7 = seed + 8;
    VOLATILE_VAR int v8 = seed + 9;
    VOLATILE_VAR int v9 = seed + 10;
    VOLATILE_VAR float f0 = seed * 0.1f;
    VOLATILE_VAR float f1 = seed * 0.2f;
    VOLATILE_VAR float f2 = seed * 0.3f;
    VOLATILE_VAR float f3 = seed * 0.4f;
    VOLATILE_VAR double d0 = seed * 0.01;
    VOLATILE_VAR double d1 = seed * 0.02;
    VOLATILE_VAR double d2 = seed * 0.03;
    VOLATILE_VAR long l0 = seed * 100L;
    VOLATILE_VAR long l1 = seed * 200L;
    VOLATILE_VAR long l2 = seed * 300L;
    VOLATILE_VAR long l3 = seed * 400L;
    VOLATILE_VAR long l4 = seed * 500L;
    VOLATILE_VAR long l5 = seed * 600L;
    VOLATILE_VAR long l6 = seed * 700L;
    VOLATILE_VAR long l7 = seed * 800L;
    VOLATILE_VAR long l8 = seed * 900L;
    VOLATILE_VAR long l9 = seed * 1000L;
    VOLATILE_VAR uint64_t checksum = 0;
    
    /* Labels for irreducible loop */
    loop_start:
    inner_loop:
    middle_block:
    exit_path:
    
    for (int i = 0; i < iterations; i++) {
        /* Complex arithmetic chains keeping variables live */
        v0 = v1 + v2;
        v1 = v3 * v4;
        v2 = v5 - v6;
        v3 = v7 ^ v8;
        v4 = v9 & v0;
        v5 = v1 | v2;
        v6 = v3 % (v4 + 1);
        v7 = v5 << 2;
        v8 = v6 >> 1;
        v9 = v7 + v8;
        
        f0 = f1 * 1.1f;
        f1 = f2 + f3;
        f2 = f0 - f1;
        f3 = f2 * 0.5f;
        
        d0 = d1 * 1.01;
        d1 = d2 + d0;
        d2 = d1 * 0.99;
        
        l0 = l1 + l2;
        l1 = l3 - l4;
        l2 = l5 * l6;
        l3 = l7 / (l8 + 1);
        l4 = l9 ^ l0;
        l5 = l1 & l2;
        l6 = l3 | l4;
        l7 = l5 << 3;
        l8 = l6 >> 2;
        l9 = l7 + l8;
        
        /* Irreducible control flow with goto */
        if ((i % 17) == 0) {
            goto inner_loop;
        }
        if ((i % 23) == 0) {
            goto middle_block;
        }
        if ((i % 29) == 0) {
            goto exit_path;
        }
        
        /* More arithmetic to increase register pressure */
        asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
        asm volatile("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9));
        asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2));
        asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3), "r"(l4));
        asm volatile("" : : "r"(l5), "r"(l6), "r"(l7), "r"(l8), "r"(l9));
        
        continue;
        
        middle_block:
        v0 = v9 * 2;
        v9 = v0 + 1;
        goto loop_start;
        
        exit_path:
        if (i < iterations - 10) {
            goto loop_start;
        }
    }
    
    inner_loop:
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    checksum += (uint64_t)(f0 + f1 + f2 + f3);
    checksum += (uint64_t)(d0 + d1 + d2);
    checksum += l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9;
    
    return checksum;
}

/* Switch with goto creating complex CFG */
NOINLINE static uint64_t test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int a0 = seed, a1 = seed+1, a2 = seed+2, a3 = seed+3, a4 = seed+4;
    VOLATILE_VAR int b0 = seed+5, b1 = seed+6, b2 = seed+7, b3 = seed+8, b4 = seed+9;
    VOLATILE_VAR int c0 = seed+10, c1 = seed+11, c2 = seed+12, c3 = seed+13, c4 = seed+14;
    VOLATILE_VAR float fa = seed * 0.123f, fb = seed * 0.456f, fc = seed * 0.789f;
    VOLATILE_VAR double da = seed * 0.0123, db = seed * 0.0456, dc = seed * 0.0789;
    VOLATILE_VAR long la = seed * 111L, lb = seed * 222L, lc = seed * 333L;
    VOLATILE_VAR uint64_t checksum = 0;
    
    /* Labels for goto targets */
    label_alpha:
    label_beta:
    label_gamma:
    label_delta:
    label_epsilon:
    
    for (int i = 0; i < iterations; i++) {
        /* Long dependency chains */
        a0 = a1 * a2 + a3;
        a1 = a4 ^ a0;
        a2 = a1 - a3;
        a3 = a2 | a4;
        a4 = a0 & a3;
        
        b0 = b1 + b2 * b3;
        b1 = b4 - b0;
        b2 = b1 / (b3 + 1);
        b3 = b2 << 1;
        b4 = b3 >> 2;
        
        c0 = c1 * c2 - c3;
        c1 = c4 + c0;
        c2 = c1 % (c3 + 2);
        c3 = c2 ^ c4;
        c4 = c3 & c0;
        
        fa = fb * 1.234f - fc;
        fb = fc + fa * 0.5f;
        fc = fa - fb * 2.0f;
        
        da = db * 1.5 + dc;
        db = dc - da * 0.3;
        dc = da + db * 0.7;
        
        la = lb << (lc & 3);
        lb = lc >> (la & 3);
        lc = la + lb * 2;
        
        /* Switch with goto to different labels */
        switch (i % 7) {
            case 0:
                a0 += b0;
                goto label_alpha;
            case 1:
                b0 += c0;
                goto label_beta;
            case 2:
                c0 += a0;
                goto label_gamma;
            case 3:
                a1 = b1 * c1;
                goto label_delta;
            case 4:
                b1 = c1 - a1;
                goto label_epsilon;
            case 5:
                c1 = a1 ^ b1;
                /* Fall through */
            default:
                a0 = a0 * 3 + 1;
                break;
        }
        
        /* Force variables to stay live */
        asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4));
        asm volatile("" : : "r"(b0), "r"(b1), "r"(b2), "r"(b3), "r"(b4));
        asm volatile("" : : "r"(c0), "r"(c1), "r"(c2), "r"(c3), "r"(c4));
        asm volatile("" : : "r"(fa), "r"(fb), "r"(fc));
        asm volatile("" : : "r"(da), "r"(db), "r"(dc));
        asm volatile("" : : "r"(la), "r"(lb), "r"(lc));
        
        continue;
        
        label_alpha:
        a0 = a0 * 2;
        continue;
        
        label_beta:
        b0 = b0 + i;
        continue;
        
        label_gamma:
        c0 = c0 - i;
        continue;
        
        label_delta:
        a1 = a1 | 0xFF;
        continue;
        
        label_epsilon:
        b1 = b1 & 0x0F;
        continue;
    }
    
    checksum = a0 + a1 + a2 + a3 + a4 + b0 + b1 + b2 + b3 + b4 + c0 + c1 + c2 + c3 + c4;
    checksum += (uint64_t)(fa + fb + fc);
    checksum += (uint64_t)(da + db + dc);
    checksum += la + lb + lc;
    
    return checksum;
}

/* Computed goto state machine */
NOINLINE static uint64_t test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int x0 = seed, x1 = seed*2, x2 = seed*3, x3 = seed*4, x4 = seed*5;
    VOLATILE_VAR int y0 = seed*6, y1 = seed*7, y2 = seed*8, y3 = seed*9, y4 = seed*10;
    VOLATILE_VAR float fx = seed * 1.1f, fy = seed * 2.2f, fz = seed * 3.3f;
    VOLATILE_VAR double dx = seed * 0.11, dy = seed * 0.22, dz = seed * 0.33;
    VOLATILE_VAR long lx = seed * 1000L, ly = seed * 2000L, lz = seed * 3000L;
    VOLATILE_VAR uint64_t checksum = 0;
    
    /* Labels for computed goto */
    static void* states[] = {
        &&state_a, &&state_b, &&state_c, 
        &&state_d, &&state_e, &&state_f
    };
    
    void* state = &&state_a;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex arithmetic with many variables */
        x0 = x1 + x2 * x3;
        x1 = x4 - x0;
        x2 = x1 / (x3 + 1);
        x3 = x2 << (i & 3);
        x4 = x3 >> 1;
        
        y0 = y1 * y2 + y3;
        y1 = y4 ^ y0;
        y2 = y1 - y3;
        y3 = y2 | y4;
        y4 = y0 & y3;
        
        fx = fy * 1.5f - fz;
        fy = fz + fx * 0.25f;
        fz = fx - fy;
        
        dx = dy * 2.0 + dz;
        dy = dz - dx * 0.1;
        dz = dx + dy;
        
        lx = ly << (lz & 7);
        ly = lz >> (lx & 7);
        lz = lx + ly;
        
        /* Computed goto based on complex condition */
        int state_idx = (x0 + y0 + i) % 6;
        state = states[state_idx];
        goto *state;
        
        state_a:
        x0 = x0 + 1;
        y0 = y0 - 1;
        fx = fx * 1.1f;
        goto state_continue;
        
        state_b:
        x1 = x1 * 2;
        y1 = y1 / 2;
        fy = fy + 0.5f;
        goto state_continue;
        
        state_c:
        x2 = x2 ^ 0xAA;
        y2 = y2 | 0x55;
        fz = fz - 0.25f;
        goto state_continue;
        
        state_d:
        x3 = x3 << 1;
        y3 = y3 >> 1;
        dx = dx * 1.5;
        goto state_continue;
        
        state_e:
        x4 = x4 + i;
        y4 = y4 - i;
        dy = dy / 2.0;
        goto state_continue;
        
        state_f:
        x0 = x0 ^ x4;
        y0 = y0 | y4;
        dz = dz + 1.0;
        goto state_continue;
        
        state_continue:
        /* Keep variables live */
        asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4));
        asm volatile("" : : "r"(y0), "r"(y1), "r"(y2), "r"(y3), "r"(y4));
        asm volatile("" : : "r"(fx), "r"(fy), "r"(fz));
        asm volatile("" : : "r"(dx), "r"(dy), "r"(dz));
        asm volatile("" : : "r"(lx), "r"(ly), "r"(lz));
    }
    
    checksum = x0 + x1 + x2 + x3 + x4 + y0 + y1 + y2 + y3 + y4;
    checksum += (uint64_t)(fx + fy + fz);
    checksum += (uint64_t)(dx + dy + dz);
    checksum += lx + ly + lz;
    
    return checksum;
}

int main(int argc, char** argv) {
    int iterations = 10000;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    uint64_t total = 0;
    
    /* Call all test functions to increase coverage chances */
    total += test_irreducible_goto(iterations, seed);
    total += test_switch_goto(iterations, seed + 1);
    total += test_computed_goto(iterations, seed + 2);
    
    printf("Checksum: %lu\n", (unsigned long)total);
    
    return 0;
}
