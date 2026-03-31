/* mcf_coverage_test.c
 * Test program to trigger MCF fixup graph debugging output
 * Compile with: gcc -O2 -fsched-pressure -fdump-rtl-sched2 -fno-if-conversion -o mcf_test mcf_coverage_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_VARS 30
#define DEFAULT_ITERATIONS 10000

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Complex state machine using computed goto */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    /* Create many local variables to increase register pressure */
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
    VOLATILE_VAR float f4 = seed * 0.5f;
    
    VOLATILE_VAR double d0 = seed * 0.01;
    VOLATILE_VAR double d1 = seed * 0.02;
    VOLATILE_VAR double d2 = seed * 0.03;
    VOLATILE_VAR double d3 = seed * 0.04;
    VOLATILE_VAR double d4 = seed * 0.05;
    
    VOLATILE_VAR long l0 = seed * 100;
    VOLATILE_VAR long l1 = seed * 200;
    VOLATILE_VAR long l2 = seed * 300;
    VOLATILE_VAR long l3 = seed * 400;
    VOLATILE_VAR long l4 = seed * 500;
    VOLATILE_VAR long l5 = seed * 600;
    VOLATILE_VAR long l6 = seed * 700;
    VOLATILE_VAR long l7 = seed * 800;
    VOLATILE_VAR long l8 = seed * 900;
    VOLATILE_VAR long l9 = seed * 1000;
    
    unsigned long checksum = 0;
    int i;
    
    /* Labels for computed goto - creates complex CFG */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, 
        &&state4, &&state5, &&state6, &&state7
    };
    
    int state = seed % 8;
    
    for (i = 0; i < iterations; i++) {
        /* Jump to different states using computed goto */
        goto *labels[state];
        
    state0:
        /* Complex arithmetic chains keeping variables live */
        v0 = v1 + v2 * v3 - v4;
        v1 = v2 + v3 * v4 - v5;
        v2 = v3 + v4 * v5 - v6;
        f0 = f1 * 1.1f + f2 - f3;
        d0 = d1 * 1.01 + d2 - d3;
        l0 = l1 + l2 * l3 - l4;
        state = (state + 1) % 8;
        continue;
        
    state1:
        v3 = v4 + v5 * v6 - v7;
        v4 = v5 + v6 * v7 - v8;
        v5 = v6 + v7 * v8 - v9;
        f1 = f2 * 1.2f + f3 - f4;
        d1 = d2 * 1.02 + d3 - d4;
        l1 = l2 + l3 * l4 - l5;
        state = (state + 3) % 8;
        continue;
        
    state2:
        v6 = v7 + v8 * v9 - v0;
        v7 = v8 + v9 * v0 - v1;
        v8 = v9 + v0 * v1 - v2;
        f2 = f3 * 1.3f + f4 - f0;
        d2 = d3 * 1.03 + d4 - d0;
        l2 = l3 + l4 * l5 - l6;
        state = (state + 5) % 8;
        continue;
        
    state3:
        v9 = v0 + v1 * v2 - v3;
        v0 = v1 + v2 * v3 - v4;
        v1 = v2 + v3 * v4 - v5;
        f3 = f4 * 1.4f + f0 - f1;
        d3 = d4 * 1.04 + d0 - d1;
        l3 = l4 + l5 * l6 - l7;
        state = (state + 7) % 8;
        continue;
        
    state4:
        v2 = v3 + v4 * v5 - v6;
        v3 = v4 + v5 * v6 - v7;
        v4 = v5 + v6 * v7 - v8;
        f4 = f0 * 1.5f + f1 - f2;
        d4 = d0 * 1.05 + d1 - d2;
        l4 = l5 + l6 * l7 - l8;
        state = (state + 2) % 8;
        continue;
        
    state5:
        v5 = v6 + v7 * v8 - v9;
        v6 = v7 + v8 * v9 - v0;
        v7 = v8 + v9 * v0 - v1;
        f0 = f1 * 1.6f + f2 - f3;
        d0 = d1 * 1.06 + d2 - d3;
        l5 = l6 + l7 * l8 - l9;
        state = (state + 4) % 8;
        continue;
        
    state6:
        v8 = v9 + v0 * v1 - v2;
        v9 = v0 + v1 * v2 - v3;
        v0 = v1 + v2 * v3 - v4;
        f1 = f2 * 1.7f + f3 - f4;
        d1 = d2 * 1.07 + d3 - d4;
        l6 = l7 + l8 * l9 - l0;
        state = (state + 6) % 8;
        continue;
        
    state7:
        v1 = v2 + v3 * v4 - v5;
        v2 = v3 + v4 * v5 - v6;
        v3 = v4 + v5 * v6 - v7;
        f2 = f3 * 1.8f + f4 - f0;
        d2 = d3 * 1.08 + d4 - d0;
        l7 = l8 + l9 * l0 - l1;
        state = (state + 1) % 8;
        continue;
    }
    
    /* Aggregate checksum to prevent dead code elimination */
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               (unsigned long)f0 + (unsigned long)f1 + (unsigned long)f2 +
               (unsigned long)f3 + (unsigned long)f4 +
               (unsigned long)d0 + (unsigned long)d1 + (unsigned long)d2 +
               (unsigned long)d3 + (unsigned long)d4 +
               l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9;
    
    return checksum;
}

/* Function with irreducible loops using goto */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    VOLATILE_VAR int a0 = seed, a1 = seed + 1, a2 = seed + 2, a3 = seed + 3;
    VOLATILE_VAR int b0 = seed + 4, b1 = seed + 5, b2 = seed + 6, b3 = seed + 7;
    VOLATILE_VAR int c0 = seed + 8, c1 = seed + 9, c2 = seed + 10, c3 = seed + 11;
    VOLATILE_VAR int d0 = seed + 12, d1 = seed + 13, d2 = seed + 14, d3 = seed + 15;
    VOLATILE_VAR int e0 = seed + 16, e1 = seed + 17, e2 = seed + 18, e3 = seed + 19;
    
    VOLATILE_VAR float fa = seed * 0.123f, fb = seed * 0.456f, fc = seed * 0.789f;
    VOLATILE_VAR double da = seed * 0.0123, db = seed * 0.0456, dc = seed * 0.0789;
    VOLATILE_VAR long la = seed * 1234, lb = seed * 5678, lc = seed * 9012;
    
    unsigned long checksum = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Start of irreducible region */
        if (i % 3 == 0) {
            goto middle_of_loop;
        }
        
    loop_start:
        /* Complex arithmetic to keep variables live */
        a0 = a1 * a2 - a3 + i;
        a1 = a2 * a3 - a0 + i;
        a2 = a3 * a0 - a1 + i;
        a3 = a0 * a1 - a2 + i;
        
        b0 = b1 + b2 * b3 - i;
        b1 = b2 + b3 * b0 - i;
        b2 = b3 + b0 * b1 - i;
        b3 = b0 + b1 * b2 - i;
        
        fa = fb * 1.234f - fc + i;
        fb = fc * 2.345f - fa + i;
        fc = fa * 3.456f - fb + i;
        
        da = db * 1.2345 - dc + i;
        db = dc * 2.3456 - da + i;
        dc = da * 3.4567 - db + i;
        
        la = lb + lc * i - la;
        lb = lc + la * i - lb;
        lc = la + lb * i - lc;
        
        if (i % 5 == 0) {
            goto loop_end;
        }
        
    middle_of_loop:
        c0 = c1 * c2 - c3 + i;
        c1 = c2 * c3 - c0 + i;
        c2 = c3 * c0 - c1 + i;
        c3 = c0 * c1 - c2 + i;
        
        d0 = d1 + d2 * d3 - i;
        d1 = d2 + d3 * d0 - i;
        d2 = d3 + d0 * d1 - i;
        d3 = d0 + d1 * d2 - i;
        
        e0 = e1 * e2 - e3 + i;
        e1 = e2 * e3 - e0 + i;
        e2 = e3 * e0 - e1 + i;
        e3 = e0 * e1 - e2 + i;
        
        if (i % 7 == 0) {
            goto loop_start;
        }
        
    loop_end:
        /* More arithmetic */
        a0 = a0 ^ b0 ^ c0;
        a1 = a1 ^ b1 ^ c1;
        a2 = a2 ^ b2 ^ c2;
        a3 = a3 ^ b3 ^ c3;
        
        fa = fa + fb - fc;
        fb = fb + fc - fa;
        fc = fc + fa - fb;
        
        da = da * 1.01 + db;
        db = db * 1.02 + dc;
        dc = dc * 1.03 + da;
        
        la = la << 1 | lb;
        lb = lb << 2 | lc;
        lc = lc << 3 | la;
    }
    
    checksum = a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3 +
               c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3 +
               e0 + e1 + e2 + e3 +
               (unsigned long)fa + (unsigned long)fb + (unsigned long)fc +
               (unsigned long)da + (unsigned long)db + (unsigned long)dc +
               la + lb + lc;
    
    return checksum;
}

/* Function with switch and goto creating irreducible CFG */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int x0 = seed, x1 = seed + 1, x2 = seed + 2, x3 = seed + 3;
    VOLATILE_VAR int y0 = seed + 4, y1 = seed + 5, y2 = seed + 6, y3 = seed + 7;
    VOLATILE_VAR int z0 = seed + 8, z1 = seed + 9, z2 = seed + 10, z3 = seed + 11;
    
    VOLATILE_VAR float fx = seed * 0.111f, fy = seed * 0.222f, fz = seed * 0.333f;
    VOLATILE_VAR double dx = seed * 0.0111, dy = seed * 0.0222, dz = seed * 0.0333;
    VOLATILE_VAR long lx = seed * 1111, ly = seed * 2222, lz = seed * 3333;
    
    unsigned long checksum = 0;
    int i, state;
    
    for (i = 0; i < iterations; i++) {
        state = (i + seed) % 10;
        
        switch (state) {
            case 0:
                x0 = x1 * x2 - x3 + i;
                x1 = x2 * x3 - x0 + i;
                fx = fy * 1.1f - fz + i;
                dx = dy * 1.01 - dz + i;
                lx = ly + lz * i;
                goto out_of_switch;
                
            case 1:
                x2 = x3 * x0 - x1 + i;
                x3 = x0 * x1 - x2 + i;
                fy = fz * 1.2f - fx + i;
                dy = dz * 1.02 - dx + i;
                ly = lz + lx * i;
                goto middle_block;
                
            case 2:
                y0 = y1 + y2 * y3 - i;
                y1 = y2 + y3 * y0 - i;
                fz = fx * 1.3f - fy + i;
                dz = dx * 1.03 - dy + i;
                lz = lx + ly * i;
                goto out_of_switch;
                
            case 3:
                y2 = y3 + y0 * y1 - i;
                y3 = y0 + y1 * y2 - i;
                fx = fx * 0.9f + 1.0f;
                dx = dx * 0.99 + 1.0;
                lx = lx ^ ly ^ lz;
                goto middle_block;
                
            case 4:
                z0 = z1 * z2 - z3 + i;
                z1 = z2 * z3 - z0 + i;
                fy = fy * 0.8f + 2.0f;
                dy = dy * 0.98 + 2.0;
                ly = ly << 1 | lx;
                goto out_of_switch;
                
            case 5:
                z2 = z3 * z0 - z1 + i;
                z3 = z0 * z1 - z2 + i;
                fz = fz * 0.7f + 3.0f;
                dz = dz * 0.97 + 3.0;
                lz = lz << 2 | ly;
                goto middle_block;
                
            default:
                x0 = x0 ^ y0 ^ z0;
                x1 = x1 ^ y1 ^ z1;
                fx = fx + fy + fz;
                dx = dx + dy + dz;
                lx = lx + ly + lz;
                break;
        }
        
        /* This label is inside the switch but jumped to from outside */
        middle_block:
        y0 = y0 ^ x0 ^ z0;
        y1 = y1 ^ x1 ^ z1;
        fy = fy - fx + fz;
        dy = dy - dx + dz;
        ly = ly - lx + lz;
        
        /* Jump back into the switch */
        if (i % 11 == 0) {
            goto case_6;
        }
        
        continue;
        
    out_of_switch:
        z0 = z0 ^ x0 ^ y0;
        z1 = z1 ^ x1 ^ y1;
        fz = fz - fx - fy;
        dz = dz - dx - dy;
        lz = lz - lx - ly;
        
        /* Jump to a case label inside the switch */
        if (i % 13 == 0) {
            goto case_7;
        }
        continue;
        
    case_6:
        /* This is a case label outside the switch block */
        x2 = x2 ^ y2 ^ z2;
        x3 = x3 ^ y3 ^ z3;
        state = 6;
        continue;
        
    case_7:
        /* Another external case label */
        y2 = y2 ^ x2 ^ z2;
        y3 = y3 ^ x3 ^ z3;
        state = 7;
        continue;
    }
    
    checksum = x0 + x1 + x2 + x3 + y0 + y1 + y2 + y3 + z0 + z1 + z2 + z3 +
               (unsigned long)fx + (unsigned long)fy + (unsigned long)fz +
               (unsigned long)dx + (unsigned long)dy + (unsigned long)dz +
               lx + ly + lz;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = DEFAULT_ITERATIONS;
    int seed = time(NULL);
    unsigned long total_checksum = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = DEFAULT_ITERATIONS;
        }
    }
    
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    /* Call all test functions to trigger different CFG patterns */
    total_checksum += test_computed_goto(iterations, seed);
    total_checksum += test_irreducible_goto(iterations, seed + 1);
    total_checksum += test_switch_goto(iterations, seed + 2);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}
