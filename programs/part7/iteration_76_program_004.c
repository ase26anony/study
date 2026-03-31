/* mcf_coverage.c - Program to trigger MCF fixup graph debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Complex function 1: Irreducible loops with goto jumps */
NOINLINE static uint64_t test_irreducible_goto(int iterations, int seed) {
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
    VOLATILE_VAR uint64_t checksum = 0;
    
    int i;
    for (i = 0; i < iterations; i++) {
        /* Create irreducible loop structure with gotos */
        if (i % 3 == 0) {
            goto outer_label1;
        } else if (i % 3 == 1) {
            goto inner_label1;
        } else {
            goto inner_label2;
        }
        
    outer_label1:
        /* Complex arithmetic to keep variables live */
        v0 = v1 + v2 * v3 - v4;
        v1 = v5 ^ v6 | v7;
        v2 = v8 * v9 / (v0 + 1);
        f0 = f1 * f2 + f3;
        f1 = f2 - f0 * 0.5f;
        d0 = d1 + d2 * 0.5;
        d1 = d0 - d2;
        l0 = l1 + l2 - l3;
        l1 = l4 * l5 / (l6 + 1);
        if (i % 7 == 0) goto inner_label2;
        
    inner_label1:
        /* More arithmetic operations */
        v3 = v4 + v5 - v6;
        v4 = v7 * v8 % (v9 + 1);
        v5 = v0 ^ v1 & v2;
        f2 = f3 * 2.0f - f0;
        f3 = f1 + f2 * 0.25f;
        d2 = d0 * 1.5 - d1;
        l2 = l3 + l4 - l5;
        l3 = l6 * l7 % (l8 + 1);
        l4 = l9 + l0 - l1;
        if (i % 5 == 0) goto outer_label2;
        else goto inner_label2;
        
    inner_label2:
        /* Even more operations */
        v6 = v7 + v8 - v9;
        v7 = v0 * v1 / (v2 + 1);
        v8 = v3 ^ v4 | v5;
        v9 = v6 + v7 - v8;
        l5 = l6 + l7 - l8;
        l6 = l9 * l0 % (l1 + 1);
        l7 = l2 + l3 - l4;
        l8 = l5 * l6 / (l7 + 1);
        l9 = l8 + l0 - l1;
        if (i % 11 == 0) goto outer_label1;
        
    outer_label2:
        /* Final block with dependency chain */
        v0 = v1 + v2;
        v1 = v3 + v4;
        v2 = v5 + v6;
        v3 = v7 + v8;
        v4 = v9 + v0;
        f0 = f1 + f2;
        f1 = f3 + f0;
        d0 = d1 + d2;
        l0 = l1 + l2;
        l1 = l3 + l4;
        
        /* Use inline assembly to mark variables as used */
        asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
        asm volatile("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9));
        asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2));
        asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3), "r"(l4));
        asm volatile("" : : "r"(l5), "r"(l6), "r"(l7), "r"(l8), "r"(l9));
        
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        checksum += (uint64_t)(f0 * 100) + (uint64_t)(f1 * 100) + 
                   (uint64_t)(f2 * 100) + (uint64_t)(f3 * 100);
        checksum += (uint64_t)(d0 * 1000) + (uint64_t)(d1 * 1000) + 
                   (uint64_t)(d2 * 1000);
        checksum += l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9;
    }
    
    return checksum;
}

/* Complex function 2: Switch with goto creating irreducible regions */
NOINLINE static uint64_t test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int a0 = seed, a1 = seed+1, a2 = seed+2, a3 = seed+3, a4 = seed+4;
    VOLATILE_VAR int b0 = seed+5, b1 = seed+6, b2 = seed+7, b3 = seed+8, b4 = seed+9;
    VOLATILE_VAR int c0 = seed+10, c1 = seed+11, c2 = seed+12, c3 = seed+13, c4 = seed+14;
    VOLATILE_VAR float fa = seed * 0.123f, fb = seed * 0.456f, fc = seed * 0.789f;
    VOLATILE_VAR double da = seed * 0.0123, db = seed * 0.0456, dc = seed * 0.0789;
    VOLATILE_VAR long la = seed * 111L, lb = seed * 222L, lc = seed * 333L;
    VOLATILE_VAR long ld = seed * 444L, le = seed * 555L, lf = seed * 666L;
    VOLATILE_VAR uint64_t checksum = 0;
    
    int i;
    for (i = 0; i < iterations; i++) {
        int mod = i % 20;
        
        switch (mod) {
            case 0: case 1: case 2:
                a0 = a1 + a2 * a3 - a4;
                b0 = b1 ^ b2 | b3;
                c0 = c1 * c2 / (c3 + 1);
                fa = fb * fc + 1.0f;
                da = db + dc * 0.5;
                la = lb + lc - ld;
                if (mod == 0) goto label_x;
                else if (mod == 1) goto label_y;
                break;
                
            case 3: case 4: case 5:
            label_x:
                a1 = a2 + a3 - a4;
                b1 = b2 * b3 % (b4 + 1);
                c1 = c2 ^ c3 & c4;
                fb = fc * 2.0f - fa;
                db = dc * 1.5 - da;
                lb = lc + ld - le;
                if (mod == 3) goto label_z;
                else if (mod == 4) goto label_y;
                break;
                
            case 6: case 7: case 8:
            label_y:
                a2 = a3 + a4 - a0;
                b2 = b3 * b4 / (b0 + 1);
                c2 = c3 ^ c4 | c0;
                fc = fa + fb * 0.25f;
                dc = da + db * 0.75;
                lc = ld + le - lf;
                if (mod == 6) goto label_x;
                else if (mod == 7) goto label_z;
                break;
                
            case 9: case 10: case 11:
            label_z:
                a3 = a4 + a0 - a1;
                b3 = b4 * b0 % (b1 + 1);
                c3 = c4 ^ c0 & c1;
                fa = fb + fc * 0.33f;
                da = db + dc * 0.66;
                ld = le + lf - la;
                if (mod == 9) goto label_x;
                else if (mod == 10) goto label_y;
                break;
                
            default:
                a4 = a0 + a1 - a2;
                b4 = b0 * b1 / (b2 + 1);
                c4 = c0 ^ c1 | c2;
                fb = fc + fa * 0.5f;
                db = dc + da * 0.25;
                le = lf + la - lb;
                break;
        }
        
        /* Long dependency chain */
        a0 = a1 + a2; a1 = a3 + a4; a2 = a0 + a1;
        a3 = a2 + a4; a4 = a0 + a3;
        b0 = b1 + b2; b1 = b3 + b4; b2 = b0 + b1;
        b3 = b2 + b4; b4 = b0 + b3;
        c0 = c1 + c2; c1 = c3 + c4; c2 = c0 + c1;
        c3 = c2 + c4; c4 = c0 + c3;
        fa = fb + fc; fb = fa + fc; fc = fa + fb;
        da = db + dc; db = da + dc; dc = da + db;
        la = lb + lc; lb = lc + ld; lc = ld + le;
        ld = le + lf; le = lf + la; lf = la + lb;
        
        /* Force register usage */
        asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4));
        asm volatile("" : : "r"(b0), "r"(b1), "r"(b2), "r"(b3), "r"(b4));
        asm volatile("" : : "r"(c0), "r"(c1), "r"(c2), "r"(c3), "r"(c4));
        asm volatile("" : : "r"(fa), "r"(fb), "r"(fc));
        asm volatile("" : : "r"(da), "r"(db), "r"(dc));
        asm volatile("" : : "r"(la), "r"(lb), "r"(lc), "r"(ld), "r"(le), "r"(lf));
        
        checksum += a0 + a1 + a2 + a3 + a4 + b0 + b1 + b2 + b3 + b4 + 
                   c0 + c1 + c2 + c3 + c4;
        checksum += (uint64_t)(fa * 100) + (uint64_t)(fb * 100) + (uint64_t)(fc * 100);
        checksum += (uint64_t)(da * 1000) + (uint64_t)(db * 1000) + (uint64_t)(dc * 1000);
        checksum += la + lb + lc + ld + le + lf;
    }
    
    return checksum;
}

/* Complex function 3: Computed goto for state machine */
NOINLINE static uint64_t test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int x0 = seed, x1 = seed+1, x2 = seed+2, x3 = seed+3, x4 = seed+4;
    VOLATILE_VAR int y0 = seed+5, y1 = seed+6, y2 = seed+7, y3 = seed+8, y4 = seed+9;
    VOLATILE_VAR int z0 = seed+10, z1 = seed+11, z2 = seed+12, z3 = seed+13, z4 = seed+14;
    VOLATILE_VAR float fx = seed * 0.111f, fy = seed * 0.222f, fz = seed * 0.333f;
    VOLATILE_VAR double dx = seed * 0.0111, dy = seed * 0.0222, dz = seed * 0.0333;
    VOLATILE_VAR long lx = seed * 1000L, ly = seed * 2000L, lz = seed * 3000L;
    VOLATILE_VAR long lw = seed * 4000L, lv = seed * 5000L, lu = seed * 6000L;
    VOLATILE_VAR uint64_t checksum = 0;
    
    /* Labels for computed goto */
    static void* labels[] = { &&state0, &&state1, &&state2, &&state3, 
                             &&state4, &&state5, &&state6, &&state7 };
    
    int i;
    for (i = 0; i < iterations; i++) {
        int state = i % 8;
        
        /* Computed goto */
        goto *labels[state];
        
    state0:
        x0 = x1 + x2 * x3 - x4;
        y0 = y1 ^ y2 | y3;
        z0 = z1 * z2 / (z3 + 1);
        fx = fy * fz + 0.1f;
        dx = dy + dz * 0.2;
        lx = ly + lz - lw;
        if (i % 3 == 0) goto state2;
        else if (i % 3 == 1) goto state4;
        continue;
        
    state1:
        x1 = x2 + x3 - x4;
        y1 = y2 * y3 % (y4 + 1);
        z1 = z2 ^ z3 & z4;
        fy = fz * 2.0f - fx;
        dy = dz * 1.5 - dx;
        ly = lz + lw - lv;
        if (i % 4 == 0) goto state3;
        else if (i % 4 == 1) goto state5;
        continue;
        
    state2:
        x2 = x3 + x4 - x0;
        y2 = y3 * y4 / (y0 + 1);
        z2 = z3 ^ z4 | z0;
        fz = fx + fy * 0.25f;
        dz = dx + dy * 0.75;
        lz = lw + lv - lu;
        if (i % 5 == 0) goto state0;
        else if (i % 5 == 1) goto state6;
        continue;
        
    state3:
        x3 = x4 + x0 - x1;
        y3 = y4 * y0 % (y1 + 1);
        z3 = z4 ^ z0 & z1;
        fx = fy + fz * 0.33f;
        dx = dy + dz * 0.66;
        lw = lv + lu - lx;
        if (i % 6 == 0) goto state1;
        else if (i % 6 == 1) goto state7;
        continue;
        
    state4:
        x4 = x0 + x1 - x2;
        y4 = y0 * y1 / (y2 + 1);
        z4 = z0 ^ z1 | z2;
        fy = fz + fx * 0.5f;
        dy = dz + dx * 0.25;
        lv = lu + lx - ly;
        if (i % 7 == 0) goto state2;
        else if (i % 7 == 1) goto state0;
        continue;
        
    state5:
        x0 = x1 + x2; x1 = x3 + x4; x2 = x0 + x1;
        y0 = y1 + y2; y1 = y3 + y4; y2 = y0 + y1;
        z0 = z1 + z2; z1 = z3 + z4; z2 = z0 + z1;
        fx = fy + fz; fy = fx + fz; fz = fx + fy;
        dx = dy + dz; dy = dx + dz; dz = dx + dy;
        lx = ly + lz; ly = lz + lw; lz = lw + lv;
        if (i % 8 == 0) goto state3;
        else if (i % 8 == 1) goto state1;
        continue;
        
    state6:
        x3 = x4 + x0; x4 = x1 + x2; x0 = x3 + x4;
        y3 = y4 + y0; y4 = y1 + y2; y0 = y3 + y4;
        z3 = z4 + z0; z4 = z1 + z2; z0 = z3 + z4;
        fx = fy * 0.7f + fz;
        fy = fz * 0.8f + fx;
        fz = fx * 0.9f + fy;
        dx = dy * 0.07 + dz;
        dy = dz * 0.08 + dx;
        dz = dx * 0.09 + dy;
        lw = lv + lu; lv = lu + lx; lu = lx + ly;
        if (i % 9 == 0) goto state4;
        else if (i % 9 == 1) goto state2;
        continue;
        
    state7:
        x1 = x2 * x3; x2 = x4 / (x0 + 1); x3 = x1 ^ x2;
        y1 = y2 * y3; y2 = y4 / (y0 + 1); y3 = y1 ^ y2;
        z1 = z2 * z3; z2 = z4 / (z0 + 1); z3 = z1 ^ z2;
        fx = fy - fz; fy = fz - fx; fz = fx - fy;
        dx = dy - dz; dy = dz - dx; dz = dx - dy;
        lx = ly * lz; ly = lz / (lw + 1); lz = lx ^ ly;
        if (i % 10 == 0) goto state5;
        else if (i % 10 == 1) goto state3;
        continue;
    }
    
    /* Final computation */
    checksum += x0 + x1 + x2 + x3 + x4 + y0 + y1 + y2 + y3 + y4 + 
               z0 + z1 + z2 + z3 + z4;
    checksum += (uint64_t)(fx * 100) + (uint64_t)(fy * 100) + (uint64_t)(fz * 100);
    checksum += (uint64_t)(dx * 1000) + (uint64_t)(dy * 1000) + (uint64_t)(dz * 1000);
    checksum += lx + ly + lz + lw + lv + lu;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with iterations=%d, seed=%d\n", 
           iterations, seed);
    
    uint64_t total_checksum = 0;
    
    /* Call all test functions to create different CFG patterns */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Final checksum: %lu\n", (unsigned long)total_checksum);
    
    return 0;
}
