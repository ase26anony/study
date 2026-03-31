/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm
   with fixup graph creation for coverage of special node printing */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Function 1: Irreducible loop with goto jumping across loop boundaries */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    VOLATILE_VAR int a0 = seed + 1, a1 = seed + 2, a2 = seed + 3, a3 = seed + 4;
    VOLATILE_VAR int b0 = seed + 5, b1 = seed + 6, b2 = seed + 7, b3 = seed + 8;
    VOLATILE_VAR int c0 = seed + 9, c1 = seed + 10, c2 = seed + 11, c3 = seed + 12;
    VOLATILE_VAR float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f, f3 = seed * 0.4f;
    VOLATILE_VAR double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03, d3 = seed * 0.04;
    VOLATILE_VAR long l0 = seed * 100L, l1 = seed * 200L, l2 = seed * 300L, l3 = seed * 400L;
    VOLATILE_VAR int extra[6] = {seed + 100, seed + 101, seed + 102, 
                                 seed + 103, seed + 104, seed + 105};
    
    unsigned long checksum = 0;
    int i, j;
    
    /* Outer loop */
    for (i = 0; i < iterations; i++) {
        /* Label for goto target inside inner loop */
        inner_loop_start:
        
        /* Inner loop with irreducible control flow */
        for (j = 0; j < 10; j++) {
            /* Complex arithmetic chains keeping variables live */
            a0 = a1 + a2 * a3 - j;
            a1 = a2 + a3 * a0 + i;
            a2 = a3 + a0 * a1 - seed;
            a3 = a0 + a1 * a2 + j * i;
            
            b0 = b1 ^ b2 | b3 & j;
            b1 = b2 ^ b3 | b0 & i;
            b2 = b3 ^ b0 | b1 & seed;
            b3 = b0 ^ b1 | b2 & (j * i);
            
            f0 = f1 * 1.1f + f2 - f3 / 2.0f;
            f1 = f2 * 1.2f + f3 - f0 / 3.0f;
            f2 = f3 * 1.3f + f0 - f1 / 4.0f;
            f3 = f0 * 1.4f + f1 - f2 / 5.0f;
            
            d0 = d1 * 1.01 + d2 - d3 / 2.0;
            d1 = d2 * 1.02 + d3 - d0 / 3.0;
            d2 = d3 * 1.03 + d0 - d1 / 4.0;
            d3 = d0 * 1.04 + d1 - d2 / 5.0;
            
            l0 = l1 + l2 - l3 * 2;
            l1 = l2 + l3 - l0 * 3;
            l2 = l3 + l0 - l1 * 4;
            l3 = l0 + l1 - l2 * 5;
            
            /* Use inline assembly to mark variables as used */
            asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3));
            asm volatile("" : : "r"(b0), "r"(b1), "r"(b2), "r"(b3));
            asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
            asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3));
            asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3));
            
            /* Irreducible control flow: jump to outer label from inner loop */
            if ((i * j + seed) % 7 == 0) {
                goto outer_label;  /* Creates irreducible region */
            }
            
            if ((i * j + seed) % 11 == 0) {
                goto inner_loop_start;  /* Jump back to inner loop start */
            }
            
            /* More arithmetic on extra variables */
            extra[0] = extra[1] + extra[2] - extra[3];
            extra[1] = extra[2] + extra[3] - extra[4];
            extra[2] = extra[3] + extra[4] - extra[5];
            extra[3] = extra[4] + extra[5] - extra[0];
            extra[4] = extra[5] + extra[0] - extra[1];
            extra[5] = extra[0] + extra[1] - extra[2];
        }
        
        outer_label:
        /* Continue outer loop computation */
        checksum += a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3;
        checksum += (unsigned long)(f0 + f1 + f2 + f3);
        checksum += (unsigned long)(d0 + d1 + d2 + d3);
        checksum += l0 + l1 + l2 + l3;
        
        /* Another irreducible jump */
        if (i % 3 == 0) {
            goto skip_point;
        }
        
        skip_point:
        /* More computation to keep variables live */
        c0 = c1 + c2 - c3;
        c1 = c2 + c3 - c0;
        c2 = c3 + c0 - c1;
        c3 = c0 + c1 - c2;
    }
    
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int x0 = seed, x1 = seed * 2, x2 = seed * 3, x3 = seed * 4;
    VOLATILE_VAR int y0 = seed + 10, y1 = seed + 20, y2 = seed + 30, y3 = seed + 40;
    VOLATILE_VAR float fx0 = seed * 0.5f, fx1 = seed * 1.5f, fx2 = seed * 2.5f;
    VOLATILE_VAR double dy0 = seed * 0.15, dy1 = seed * 0.25, dy2 = seed * 0.35;
    VOLATILE_VAR long lx0 = seed * 1000L, lx1 = seed * 2000L, lx2 = seed * 3000L;
    
    unsigned long checksum = 0;
    int i;
    
    /* Labels for goto targets */
    label_a:
    label_b:
    label_c:
    label_d:
    
    for (i = 0; i < iterations; i++) {
        /* Long dependency chains */
        x0 = x1 * x2 - x3 + i;
        x1 = x2 * x3 - x0 + seed;
        x2 = x3 * x0 - x1 + i * seed;
        x3 = x0 * x1 - x2 + i + seed;
        
        y0 = y1 | y2 & y3 ^ i;
        y1 = y2 | y3 & y0 ^ seed;
        y2 = y3 | y0 & y1 ^ (i * seed);
        y3 = y0 | y1 & y2 ^ (i + seed);
        
        fx0 = fx1 * 2.0f - fx2 / 3.0f;
        fx1 = fx2 * 3.0f - fx0 / 4.0f;
        fx2 = fx0 * 4.0f - fx1 / 5.0f;
        
        dy0 = dy1 * 1.5 - dy2 / 2.5;
        dy1 = dy2 * 2.5 - dy0 / 3.5;
        dy2 = dy0 * 3.5 - dy1 / 4.5;
        
        lx0 = lx1 + lx2 * 2;
        lx1 = lx2 + lx0 * 3;
        lx2 = lx0 + lx1 * 4;
        
        /* Complex switch with goto to external labels */
        switch ((i * seed) % 8) {
            case 0:
                /* Arithmetic before goto */
                x0 = x1 + x2;
                goto label_a;
            case 1:
                x1 = x2 + x3;
                goto label_b;
            case 2:
                x2 = x3 + x0;
                goto label_c;
            case 3:
                x3 = x0 + x1;
                goto label_d;
            case 4:
                y0 = y1 ^ y2;
                goto label_a;
            case 5:
                y1 = y2 ^ y3;
                goto label_b;
            case 6:
                y2 = y3 ^ y0;
                goto label_c;
            case 7:
                y3 = y0 ^ y1;
                goto label_d;
        }
        
        /* Use variables to prevent elimination */
        asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3));
        asm volatile("" : : "r"(y0), "r"(y1), "r"(y2), "r"(y3));
        asm volatile("" : : "r"(fx0), "r"(fx1), "r"(fx2));
        asm volatile("" : : "r"(dy0), "r"(dy1), "r"(dy2));
        asm volatile("" : : "r"(lx0), "r"(lx1), "r"(lx2));
        
        checksum += x0 + x1 + x2 + x3 + y0 + y1 + y2 + y3;
        checksum += (unsigned long)(fx0 + fx1 + fx2);
        checksum += (unsigned long)(dy0 + dy1 + dy2);
        checksum += lx0 + lx1 + lx2;
    }
    
    return checksum;
}

/* Function 3: Computed goto simulating state machine */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int s0 = seed, s1 = seed + 1, s2 = seed + 2, s3 = seed + 3;
    VOLATILE_VAR int t0 = seed * 5, t1 = seed * 6, t2 = seed * 7, t3 = seed * 8;
    VOLATILE_VAR float fs0 = seed * 0.11f, fs1 = seed * 0.22f, fs2 = seed * 0.33f;
    VOLATILE_VAR double dt0 = seed * 0.055, dt1 = seed * 0.066, dt2 = seed * 0.077;
    VOLATILE_VAR long ls0 = seed * 500L, ls1 = seed * 600L, ls2 = seed * 700L;
    
    /* Labels for computed goto */
    static void* labels[] = { &&state0, &&state1, &&state2, &&state3, &&state4 };
    
    unsigned long checksum = 0;
    int i, state = 0;
    
    for (i = 0; i < iterations; i++) {
        /* Complex arithmetic dependency chains */
        s0 = s1 * s2 - s3 + state;
        s1 = s2 * s3 - s0 + i;
        s2 = s3 * s0 - s1 + seed;
        s3 = s0 * s1 - s2 + state * i;
        
        t0 = t1 & t2 | t3 ^ state;
        t1 = t2 & t3 | t0 ^ i;
        t2 = t3 & t0 | t1 ^ seed;
        t3 = t0 & t1 | t2 ^ (state * i);
        
        fs0 = fs1 * 1.11f - fs2 / 2.22f;
        fs1 = fs2 * 2.22f - fs0 / 3.33f;
        fs2 = fs0 * 3.33f - fs1 / 4.44f;
        
        dt0 = dt1 * 1.055 - dt2 / 2.066;
        dt1 = dt2 * 2.066 - dt0 / 3.077;
        dt2 = dt0 * 3.077 - dt1 / 4.088;
        
        ls0 = ls1 + ls2 * state;
        ls1 = ls2 + ls0 * i;
        ls2 = ls0 + ls1 * seed;
        
        /* Use variables */
        asm volatile("" : : "r"(s0), "r"(s1), "r"(s2), "r"(s3));
        asm volatile("" : : "r"(t0), "r"(t1), "r"(t2), "r"(t3));
        asm volatile("" : : "r"(fs0), "r"(fs1), "r"(fs2));
        asm volatile("" : : "r"(dt0), "r"(dt1), "r"(dt2));
        asm volatile("" : : "r"(ls0), "r"(ls1), "r"(ls2));
        
        /* Computed goto - creates complex CFG */
        goto *labels[state % 5];
        
        state0:
            checksum += s0 + s1;
            state = (state + 1) % 5;
            continue;
        state1:
            checksum += s2 + s3;
            state = (state + 2) % 5;
            continue;
        state2:
            checksum += t0 + t1;
            state = (state + 3) % 5;
            continue;
        state3:
            checksum += t2 + t3;
            state = (state + 4) % 5;
            continue;
        state4:
            checksum += (unsigned long)(fs0 + fs1 + fs2);
            checksum += (unsigned long)(dt0 + dt1 + dt2);
            checksum += ls0 + ls1 + ls2;
            state = (state + 5) % 5;
            continue;
    }
    
    return checksum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = time(NULL);
    unsigned long total_checksum = 0;
    
    /* Parse command line for iteration count */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with iterations=%d, seed=%d\n", 
           iterations, seed);
    
    /* Run all test functions to increase coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    printf("  test_irreducible_goto checksum: %lu\n", total_checksum);
    
    total_checksum += test_switch_goto(iterations, seed + 1);
    printf("  + test_switch_goto checksum: %lu\n", total_checksum);
    
    total_checksum += test_computed_goto(iterations, seed + 2);
    printf("  + test_computed_goto checksum: %lu\n", total_checksum);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}
