/* test_mcf_coverage.c
 * Designed to trigger GCC's Minimum Cost Flow algorithm fixup graph
 * and debug print statements for special node indices.
 * Compile with: gcc -O2 -fsched-pressure -fdump-rtl-sched2 -fno-if-conversion -o test test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Complex irreducible control flow with goto jumping across loops */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many variables to create register pressure */
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
    
    unsigned long checksum = 0;
    int i, j;
    
    /* Outer loop with label for goto jumps */
    outer_loop:
    for (i = 0; i < iterations; i++) {
        /* Inner loop with multiple entry points */
        inner_loop:
        for (j = 0; j < 10; j++) {
            /* Complex arithmetic chains to keep variables live */
            v0 = v1 + v2 * v3 - v4;
            v1 = v2 - v3 + v4 * v5;
            v2 = v3 * v4 - v5 + v6;
            v3 = v4 + v5 * v6 - v7;
            v4 = v5 - v6 + v7 * v8;
            v5 = v6 * v7 - v8 + v9;
            
            f0 = f1 * 1.1f + f2 - f3;
            f1 = f2 - f3 * 0.9f + f0;
            f2 = f3 + f0 * 1.2f - f1;
            f3 = f0 - f1 + f2 * 0.8f;
            
            d0 = d1 * 1.01 + d2 - seed * 0.001;
            d1 = d2 - seed * 0.002 + d0 * 1.02;
            d2 = d0 + d1 * 0.99 - seed * 0.003;
            
            l0 = l1 + l2 - l3 * l4;
            l1 = l2 - l3 + l4 * l5;
            l2 = l3 * l4 - l5 + l6;
            l3 = l4 + l5 * l6 - l7;
            l4 = l5 - l6 + l7 * l8;
            l5 = l6 * l7 - l8 + l9;
            
            /* Use inline assembly to mark variables as used */
            asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
            asm volatile("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9));
            asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
            asm volatile("" : : "r"(d0), "r"(d1), "r"(d2));
            asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3), "r"(l4));
            asm volatile("" : : "r"(l5), "r"(l6), "r"(l7), "r"(l8), "r"(l9));
            
            /* Irreducible control flow: jump to outer loop from inner loop */
            if ((i * j + seed) % 37 == 0) {
                checksum += v0 + v1 + v2;
                goto outer_loop;  /* Creates irreducible region */
            }
            
            /* Jump to middle of inner loop from different point */
            if ((i * j + seed) % 41 == 0) {
                checksum += v3 + v4 + v5;
                goto inner_middle;
            }
        }
        
        /* This label creates another entry point to the inner loop */
        inner_middle:
        if (i % 3 == 0) {
            /* More arithmetic to increase register pressure */
            l6 = l7 * l8 - l9 + l0;
            l7 = l8 + l9 * l0 - l1;
            l8 = l9 - l0 + l1 * l2;
            l9 = l0 * l1 - l2 + l3;
            
            v6 = v7 + v8 * v9 - v0;
            v7 = v8 - v9 + v0 * v1;
            v8 = v9 * v0 - v1 + v2;
            v9 = v0 + v1 * v2 - v3;
            
            checksum += l6 + l7 + l8 + l9 + v6 + v7 + v8 + v9;
            
            /* Jump back to inner loop start */
            if ((i + seed) % 7 == 0) {
                goto inner_loop;
            }
        }
    }
    
    /* Final checksum computation */
    checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    checksum += (unsigned long)(f0 + f1 + f2 + f3);
    checksum += (unsigned long)(d0 + d1 + d2);
    checksum += l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9;
    
    return checksum;
}

/* Switch statement with goto creating complex CFG */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int a0 = seed, a1 = seed+1, a2 = seed+2, a3 = seed+3, a4 = seed+4;
    VOLATILE_VAR int b0 = seed+5, b1 = seed+6, b2 = seed+7, b3 = seed+8, b4 = seed+9;
    VOLATILE_VAR float fa = seed*0.5f, fb = seed*0.6f, fc = seed*0.7f;
    VOLATILE_VAR double da = seed*0.05, db = seed*0.06, dc = seed*0.07;
    VOLATILE_VAR long la = seed*50, lb = seed*60, lc = seed*70, ld = seed*80;
    
    unsigned long checksum = 0;
    int i, state = 0;
    
    /* Labels for goto targets */
    state_a:
    state_b:
    state_c:
    state_d:
    
    for (i = 0; i < iterations; i++) {
        /* Complex switch with goto to different labels */
        switch (state) {
            case 0:
                a0 = a1 * a2 - a3 + a4;
                a1 = a2 + a3 * a4 - a0;
                a2 = a3 - a4 + a0 * a1;
                checksum += a0 + a1 + a2;
                if ((i + seed) % 11 == 0) goto state_c;
                else if ((i + seed) % 13 == 0) goto state_d;
                state = 1;
                break;
                
            case 1:
                b0 = b1 + b2 * b3 - b4;
                b1 = b2 - b3 + b4 * b0;
                b2 = b3 * b4 - b0 + b1;
                checksum += b0 + b1 + b2;
                if ((i + seed) % 17 == 0) goto state_a;
                state = 2;
                break;
                
            case 2:
                fa = fb * 1.3f - fc + i * 0.01f;
                fb = fc + fa * 0.7f - i * 0.02f;
                fc = fa - fb * 1.1f + i * 0.03f;
                checksum += (unsigned long)(fa + fb + fc);
                if ((i + seed) % 19 == 0) goto state_b;
                state = 3;
                break;
                
            case 3:
                da = db * 1.03 - dc + i * 0.001;
                db = dc + da * 0.97 - i * 0.002;
                dc = da - db * 1.01 + i * 0.003;
                checksum += (unsigned long)(da + db + dc);
                if ((i + seed) % 23 == 0) goto state_d;
                state = 0;
                break;
        }
        
        /* More arithmetic operations to increase register pressure */
        la = lb + lc - ld * i;
        lb = lc - ld + la * i;
        lc = ld * la - lb + i;
        ld = la + lb * lc - i;
        
        a3 = a4 * a0 - a1 + a2;
        a4 = a0 + a1 * a2 - a3;
        b3 = b4 + b0 * b1 - b2;
        b4 = b0 - b1 + b2 * b3;
        
        /* Use inline assembly to keep variables live */
        asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4));
        asm volatile("" : : "r"(b0), "r"(b1), "r"(b2), "r"(b3), "r"(b4));
        asm volatile("" : : "r"(fa), "r"(fb), "r"(fc));
        asm volatile("" : : "r"(da), "r"(db), "r"(dc));
        asm volatile("" : : "r"(la), "r"(lb), "r"(lc), "r"(ld));
        
        /* Update state based on complex condition */
        state = (state + (i * seed) % 5) % 4;
    }
    
    checksum += a0 + a1 + a2 + a3 + a4 + b0 + b1 + b2 + b3 + b4;
    checksum += (unsigned long)(fa + fb + fc + da + db + dc);
    checksum += la + lb + lc + ld;
    
    return checksum;
}

/* Computed goto (labels as values) for state machine */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int x0 = seed, x1 = seed+10, x2 = seed+20, x3 = seed+30, x4 = seed+40;
    VOLATILE_VAR int y0 = seed+50, y1 = seed+60, y2 = seed+70, y3 = seed+80, y4 = seed+90;
    VOLATILE_VAR float fx = seed*0.15f, fy = seed*0.25f, fz = seed*0.35f;
    VOLATILE_VAR double dx = seed*0.015, dy = seed*0.025, dz = seed*0.035;
    VOLATILE_VAR long lx = seed*150, ly = seed*250, lz = seed*350, lw = seed*450;
    
    /* Labels for computed goto */
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    unsigned long checksum = 0;
    int i, state = 0;
    
    for (i = 0; i < iterations; i++) {
        /* Computed goto based on state */
        goto *labels[state % 5];
        
        label0:
            x0 = x1 * x2 - x3 + x4;
            x1 = x2 + x3 * x4 - x0;
            x2 = x3 - x4 + x0 * x1;
            checksum += x0 + x1 + x2;
            state = (state + 1) % 5;
            continue;
            
        label1:
            y0 = y1 + y2 * y3 - y4;
            y1 = y2 - y3 + y4 * y0;
            y2 = y3 * y4 - y0 + y1;
            checksum += y0 + y1 + y2;
            state = (state + 2) % 5;
            continue;
            
        label2:
            fx = fy * 1.5f - fz + i * 0.005f;
            fy = fz + fx * 0.5f - i * 0.006f;
            fz = fx - fy * 1.2f + i * 0.007f;
            checksum += (unsigned long)(fx + fy + fz);
            state = (state + 3) % 5;
            continue;
            
        label3:
            dx = dy * 1.015 - dz + i * 0.0005;
            dy = dz + dx * 0.985 - i * 0.0006;
            dz = dx - dy * 1.012 + i * 0.0007;
            checksum += (unsigned long)(dx + dy + dz);
            state = (state + 4) % 5;
            continue;
            
        label4:
            lx = ly + lz - lw * i;
            ly = lz - lw + lx * i;
            lz = lw * lx - ly + i;
            lw = lx + ly * lz - i;
            checksum += lx + ly + lz + lw;
            state = (state + (i % 3)) % 5;
            continue;
    }
    
    /* Additional arithmetic to ensure all variables are used */
    x3 = x4 * x0 - x1 + x2;
    x4 = x0 + x1 * x2 - x3;
    y3 = y4 + y0 * y1 - y2;
    y4 = y0 - y1 + y2 * y3;
    
    asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4));
    asm volatile("" : : "r"(y0), "r"(y1), "r"(y2), "r"(y3), "r"(y4));
    asm volatile("" : : "r"(fx), "r"(fy), "r"(fz));
    asm volatile("" : : "r"(dx), "r"(dy), "r"(dz));
    asm volatile("" : : "r"(lx), "r"(ly), "r"(lz), "r"(lw));
    
    checksum += x0 + x1 + x2 + x3 + x4 + y0 + y1 + y2 + y3 + y4;
    checksum += (unsigned long)(fx + fy + fz + dx + dy + dz);
    checksum += lx + ly + lz + lw;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = time(NULL);
    unsigned long total_checksum = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with iterations=%d, seed=%d\n", 
           iterations, seed);
    
    /* Run all test functions to increase chance of triggering MCF */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}
