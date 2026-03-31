/* mcf_coverage.c - Program to trigger MCF algorithm's fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Complex control flow with irreducible loops using goto */
NOINLINE static uint64_t test_irreducible_goto(int iterations, int seed) {
    VOLATILE_VAR int i = 0;
    uint64_t checksum = seed;
    
    /* Many local variables to create register pressure */
    double var0 = 1.1 * seed;
    float var1 = 2.2f * seed;
    int var2 = seed * 3;
    long var3 = seed * 4L;
    double var4 = 5.5 * seed;
    float var5 = 6.6f * seed;
    int var6 = seed * 7;
    long var7 = seed * 8L;
    double var8 = 9.9 * seed;
    float var9 = 10.10f * seed;
    int var10 = seed * 11;
    long var11 = seed * 12L;
    double var12 = 13.13 * seed;
    float var13 = 14.14f * seed;
    int var14 = seed * 15;
    long var15 = seed * 16L;
    double var16 = 17.17 * seed;
    float var17 = 18.18f * seed;
    int var18 = seed * 19;
    long var19 = seed * 20L;
    double var20 = 21.21 * seed;
    float var21 = 22.22f * seed;
    int var22 = seed * 23;
    long var23 = seed * 24L;
    double var24 = 25.25 * seed;
    float var25 = 26.26f * seed;
    int var26 = seed * 27;
    long var27 = seed * 28L;
    double var28 = 29.29 * seed;
    float var29 = 30.30f * seed;
    
    /* Labels for irreducible control flow */
    loop_start:
    if (i >= iterations) goto loop_end;
    
    block_a:
    /* Complex arithmetic chains keeping variables live */
    var0 = var0 * 1.01 + var1;
    var1 = var1 * 1.02f + var2;
    var2 = var2 * 3 + var3;
    var3 = var3 * 4 + var4;
    var4 = var4 * 1.05 + var5;
    var5 = var5 * 1.06f + var6;
    var6 = var6 * 7 + var7;
    var7 = var7 * 8 + var8;
    var8 = var8 * 1.09 + var9;
    var9 = var9 * 1.10f + var10;
    
    /* Conditional goto creating irreducible region */
    if ((i & 1) == 0) goto block_c;
    
    block_b:
    var10 = var10 * 11 + var11;
    var11 = var11 * 12 + var12;
    var12 = var12 * 1.13 + var13;
    var13 = var13 * 1.14f + var14;
    var14 = var14 * 15 + var15;
    var15 = var15 * 16 + var16;
    var16 = var16 * 1.17 + var17;
    var17 = var17 * 1.18f + var18;
    var18 = var18 * 19 + var19;
    var19 = var19 * 20 + var20;
    
    if ((i & 3) == 0) goto block_a;
    
    block_c:
    var20 = var20 * 1.21 + var21;
    var21 = var21 * 1.22f + var22;
    var22 = var22 * 23 + var23;
    var23 = var23 * 24 + var24;
    var24 = var24 * 1.25 + var25;
    var25 = var25 * 1.26f + var26;
    var26 = var26 * 27 + var27;
    var27 = var27 * 28 + var28;
    var28 = var28 * 1.29 + var29;
    var29 = var29 * 1.30f + var0;
    
    /* Jump back to different blocks creating irreducible loop */
    if ((i & 7) == 0) goto block_b;
    if ((i & 15) == 0) goto block_a;
    
    /* Use inline assembly to keep variables live */
    asm volatile("" : : "r"(var0), "r"(var1), "r"(var2), "r"(var3));
    asm volatile("" : : "r"(var4), "r"(var5), "r"(var6), "r"(var7));
    asm volatile("" : : "r"(var8), "r"(var9), "r"(var10), "r"(var11));
    asm volatile("" : : "r"(var12), "r"(var13), "r"(var14), "r"(var15));
    asm volatile("" : : "r"(var16), "r"(var17), "r"(var18), "r"(var19));
    asm volatile("" : : "r"(var20), "r"(var21), "r"(var22), "r"(var23));
    asm volatile("" : : "r"(var24), "r"(var25), "r"(var26), "r"(var27));
    asm volatile("" : : "r"(var28), "r"(var29));
    
    i++;
    goto loop_start;
    
    loop_end:
    
    /* Aggregate checksum from all variables */
    checksum += (uint64_t)var0 + (uint64_t)var1 + var2 + var3 + 
                (uint64_t)var4 + (uint64_t)var5 + var6 + var7 +
                (uint64_t)var8 + (uint64_t)var9 + var10 + var11 +
                (uint64_t)var12 + (uint64_t)var13 + var14 + var15 +
                (uint64_t)var16 + (uint64_t)var17 + var18 + var19 +
                (uint64_t)var20 + (uint64_t)var21 + var22 + var23 +
                (uint64_t)var24 + (uint64_t)var25 + var26 + var27 +
                (uint64_t)var28 + (uint64_t)var29;
    
    return checksum;
}

/* Switch with goto creating complex CFG */
NOINLINE static uint64_t test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int i = 0;
    uint64_t checksum = seed;
    
    /* Another set of many variables */
    double a0 = seed * 0.1, a1 = seed * 0.2, a2 = seed * 0.3, a3 = seed * 0.4;
    float b0 = seed * 0.5f, b1 = seed * 0.6f, b2 = seed * 0.7f, b3 = seed * 0.8f;
    int c0 = seed, c1 = seed * 2, c2 = seed * 3, c3 = seed * 4;
    long d0 = seed * 5L, d1 = seed * 6L, d2 = seed * 7L, d3 = seed * 8L;
    double e0 = seed * 0.9, e1 = seed * 1.0, e2 = seed * 1.1, e3 = seed * 1.2;
    float f0 = seed * 1.3f, f1 = seed * 1.4f, f2 = seed * 1.5f, f3 = seed * 1.6f;
    
    /* Labels for switch goto targets */
    switch_start:
    case_0:
    case_1:
    case_2:
    case_3:
    default_case:
    
    while (i < iterations) {
        /* Switch with goto to different labels */
        switch (i % 5) {
            case 0:
                a0 = a0 * 1.1 + a1;
                a1 = a1 * 1.2 + a2;
                a2 = a2 * 1.3 + a3;
                a3 = a3 * 1.4 + b0;
                goto case_1;
            
            case 1:
                b0 = b0 * 1.5f + b1;
                b1 = b1 * 1.6f + b2;
                b2 = b2 * 1.7f + b3;
                b3 = b3 * 1.8f + c0;
                goto case_2;
            
            case 2:
                c0 = c0 * 2 + c1;
                c1 = c1 * 3 + c2;
                c2 = c2 * 4 + c3;
                c3 = c3 * 5 + d0;
                goto case_3;
            
            case 3:
                d0 = d0 * 6 + d1;
                d1 = d1 * 7 + d2;
                d2 = d2 * 8 + d3;
                d3 = d3 * 9 + e0;
                goto default_case;
            
            default:
                e0 = e0 * 1.9 + e1;
                e1 = e1 * 2.0 + e2;
                e2 = e2 * 2.1 + e3;
                e3 = e3 * 2.2 + f0;
                goto switch_start;
        }
        
        /* More arithmetic if we reach here */
        f0 = f0 * 2.3f + f1;
        f1 = f1 * 2.4f + f2;
        f2 = f2 * 2.5f + f3;
        f3 = f3 * 2.6f + a0;
        
        /* Keep variables live */
        asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3));
        asm volatile("" : : "r"(b0), "r"(b1), "r"(b2), "r"(b3));
        asm volatile("" : : "r"(c0), "r"(c1), "r"(c2), "r"(c3));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3));
        asm volatile("" : : "r"(e0), "r"(e1), "r"(e2), "r"(e3));
        asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
        
        i++;
    }
    
    checksum += (uint64_t)a0 + (uint64_t)a1 + (uint64_t)a2 + (uint64_t)a3 +
                (uint64_t)b0 + (uint64_t)b1 + (uint64_t)b2 + (uint64_t)b3 +
                c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3 +
                (uint64_t)e0 + (uint64_t)e1 + (uint64_t)e2 + (uint64_t)e3 +
                (uint64_t)f0 + (uint64_t)f1 + (uint64_t)f2 + (uint64_t)f3;
    
    return checksum;
}

/* Computed goto for state machine */
NOINLINE static uint64_t test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int i = 0;
    uint64_t checksum = seed;
    
    /* Yet another set of variables */
    double x0 = seed * 0.01, x1 = seed * 0.02, x2 = seed * 0.03, x3 = seed * 0.04;
    float y0 = seed * 0.05f, y1 = seed * 0.06f, y2 = seed * 0.07f, y3 = seed * 0.08f;
    int z0 = seed * 10, z1 = seed * 11, z2 = seed * 12, z3 = seed * 13;
    long w0 = seed * 14L, w1 = seed * 15L, w2 = seed * 16L, w3 = seed * 17L;
    
    /* Labels for computed goto */
    static void* labels[] = { &&state0, &&state1, &&state2, &&state3, &&state4 };
    
    void* state = &&state0;
    
    while (i < iterations) {
        goto *state;
        
        state0:
            x0 = x0 * 1.01 + x1;
            x1 = x1 * 1.02 + x2;
            x2 = x2 * 1.03 + x3;
            x3 = x3 * 1.04 + y0;
            state = labels[(i + 1) % 5];
            goto next_iter;
        
        state1:
            y0 = y0 * 1.05f + y1;
            y1 = y1 * 1.06f + y2;
            y2 = y2 * 1.07f + y3;
            y3 = y3 * 1.08f + z0;
            state = labels[(i + 2) % 5];
            goto next_iter;
        
        state2:
            z0 = z0 * 2 + z1;
            z1 = z1 * 3 + z2;
            z2 = z2 * 4 + z3;
            z3 = z3 * 5 + w0;
            state = labels[(i + 3) % 5];
            goto next_iter;
        
        state3:
            w0 = w0 * 6 + w1;
            w1 = w1 * 7 + w2;
            w2 = w2 * 8 + w3;
            w3 = w3 * 9 + x0;
            state = labels[(i + 4) % 5];
            goto next_iter;
        
        state4:
            x0 = x0 * 0.99 + w0;
            y0 = y0 * 0.98f + w1;
            z0 = z0 + w2;
            w0 = w0 + w3;
            state = labels[i % 5];
            goto next_iter;
        
        next_iter:
        /* More arithmetic mixing all variables */
        x1 = x1 + y1 + z1 + w1;
        x2 = x2 + y2 + z2 + w2;
        x3 = x3 + y3 + z3 + w3;
        y1 = y1 + x0 + z0 + w0;
        
        /* Keep all variables live */
        asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3));
        asm volatile("" : : "r"(y0), "r"(y1), "r"(y2), "r"(y3));
        asm volatile("" : : "r"(z0), "r"(z1), "r"(z2), "r"(z3));
        asm volatile("" : : "r"(w0), "r"(w1), "r"(w2), "r"(w3));
        
        i++;
    }
    
    checksum += (uint64_t)x0 + (uint64_t)x1 + (uint64_t)x2 + (uint64_t)x3 +
                (uint64_t)y0 + (uint64_t)y1 + (uint64_t)y2 + (uint64_t)y3 +
                z0 + z1 + z2 + z3 + w0 + w1 + w2 + w3;
    
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
    
    printf("Running MCF coverage test with iterations=%d, seed=%d\n", 
           iterations, seed);
    
    uint64_t total_checksum = 0;
    
    /* Call all test functions to increase coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Final checksum: %lu\n", (unsigned long)total_checksum);
    
    return 0;
}
