/* mcf_coverage.c - Program to trigger GCC's MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MANY_VARS 30
#define LOOP_ITERATIONS 10000

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Function 1: Irreducible loops with goto jumping across boundaries */
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
    VOLATILE_VAR float f0 = seed * 1.1f;
    VOLATILE_VAR float f1 = seed * 1.2f;
    VOLATILE_VAR float f2 = seed * 1.3f;
    VOLATILE_VAR float f3 = seed * 1.4f;
    VOLATILE_VAR double d0 = seed * 2.1;
    VOLATILE_VAR double d1 = seed * 2.2;
    VOLATILE_VAR double d2 = seed * 2.3;
    VOLATILE_VAR long l0 = seed * 3L;
    VOLATILE_VAR long l1 = seed * 4L;
    VOLATILE_VAR long l2 = seed * 5L;
    VOLATILE_VAR long l3 = seed * 6L;
    VOLATILE_VAR long l4 = seed * 7L;
    VOLATILE_VAR long l5 = seed * 8L;
    VOLATILE_VAR long l6 = seed * 9L;
    VOLATILE_VAR long l7 = seed * 10L;
    VOLATILE_VAR long l8 = seed * 11L;
    VOLATILE_VAR long l9 = seed * 12L;
    
    uint64_t checksum = 0;
    
    /* Create irreducible loop structure with goto */
    int i = 0;
    outer_loop_start:
    while (i < iterations) {
        /* Complex arithmetic to keep variables live */
        v0 = v1 + v2;
        v1 = v3 * v4;
        v2 = v5 - v6;
        v3 = v7 ^ v8;
        v4 = v9 & v0;
        f0 = f1 * f2 + f3;
        f1 = f2 - f0 * f3;
        d0 = d1 * 1.01 + d2;
        d1 = d0 * 0.99 - d2;
        l0 = l1 + l2;
        l1 = l3 * l4;
        l2 = l5 - l6;
        l3 = l7 ^ l8;
        l4 = l9 & l0;
        
        /* Force register usage with inline asm */
        asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
        asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2));
        asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3), "r"(l4));
        
        /* Create irreducible region: goto jumps into inner loop */
        if ((i & 0x3) == 0) {
            goto inner_loop_middle;
        }
        
        inner_loop_start:
        for (int j = 0; j < 5; j++) {
            v5 = v6 + v7 * j;
            v6 = v8 - v9 / (j + 1);
            f2 = f3 * j + f0;
            d2 = d0 * j - d1;
            l5 = l6 + l7 * j;
            
            if ((j & 1) == 0 && (i & 0x1) == 0) {
                /* Jump out to outer loop label */
                goto outer_loop_label;
            }
            
            inner_loop_middle:
            v7 = v0 ^ v1 * j;
            v8 = v2 | v3;
            f3 = f1 / (j + 2.0f);
            l6 = l8 + l9 * j;
            
            if ((j & 2) == 0 && (i & 0x2) == 0) {
                /* Jump to different part of outer loop */
                goto outer_loop_end;
            }
        }
        
        outer_loop_label:
        v9 = v4 + v5;
        l7 = l0 ^ l1;
        i++;
        continue;
        
        outer_loop_end:
        v0 = v9 - v8;
        l8 = l2 + l3;
        i++;
    }
    
    /* Aggregate checksum */
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               (uint64_t)f0 + (uint64_t)f1 + (uint64_t)f2 + (uint64_t)f3 +
               (uint64_t)d0 + (uint64_t)d1 + (uint64_t)d2 +
               l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9;
    
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static uint64_t test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int a0 = seed, a1 = seed+1, a2 = seed+2, a3 = seed+3, a4 = seed+4;
    VOLATILE_VAR int b0 = seed+5, b1 = seed+6, b2 = seed+7, b3 = seed+8, b4 = seed+9;
    VOLATILE_VAR float fa0 = seed*1.5f, fa1 = seed*1.6f, fa2 = seed*1.7f;
    VOLATILE_VAR double da0 = seed*2.5, da1 = seed*2.6, da2 = seed*2.7;
    VOLATILE_VAR long la0 = seed*100, la1 = seed*101, la2 = seed*102, la3 = seed*103;
    
    uint64_t checksum = 0;
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Long dependency chain */
        a0 = a1 * a2 - a3;
        a1 = a4 ^ a0 + i;
        a2 = b0 & a1 | a3;
        a3 = b1 + a2 * 3;
        a4 = b2 - a3 / 2;
        b0 = b3 * b4 + a0;
        b1 = b0 ^ a1 - i;
        b2 = b1 & a2 | 0xFF;
        b3 = b2 + a3 * 5;
        b4 = b3 - a4 / 3;
        
        fa0 = fa1 * 1.1f + fa2;
        fa1 = fa0 - fa2 * 0.9f;
        fa2 = fa1 * 1.2f + i * 0.01f;
        
        da0 = da1 * 1.01 + da2;
        da1 = da0 * 0.99 - da2;
        da2 = da1 * 1.02 + i * 0.001;
        
        la0 = la1 + la2 * i;
        la1 = la3 ^ la0;
        la2 = la1 & 0xFFFF;
        la3 = la2 + i * 100;
        
        /* Force register usage */
        asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4));
        asm volatile("" : : "r"(b0), "r"(b1), "r"(b2), "r"(b3), "r"(b4));
        asm volatile("" : : "r"(fa0), "r"(fa1), "r"(fa2));
        asm volatile("" : : "r"(da0), "r"(da1), "r"(da2));
        asm volatile("" : : "r"(la0), "r"(la1), "r"(la2), "r"(la3));
        
        /* Complex switch with goto creating irreducible regions */
        switch (state % 7) {
            case 0:
                a0 = a1 + 1;
                if ((i & 0x1) == 0) goto label_case2;
                else goto label_case4;
                break;
            case 1:
                a1 = a2 * 2;
                goto label_case5;
                break;
            label_case2:
            case 2:
                a2 = a3 - 3;
                if ((i & 0x2) == 0) goto label_case6;
                break;
            case 3:
                a3 = a4 ^ 0xAA;
                goto label_case0;
                break;
            label_case4:
            case 4:
                a4 = b0 + 5;
                if ((i & 0x3) == 0) goto label_case1;
                break;
            label_case5:
            case 5:
                b0 = b1 * 6;
                goto label_case3;
                break;
            label_case6:
            case 6:
                b1 = b2 - 7;
                break;
            label_case0:
                b2 = b3 ^ 0x55;
                break;
            label_case1:
                b3 = b4 + 8;
                break;
            label_case3:
                b4 = a0 * 9;
                break;
        }
        
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    checksum = a0 + a1 + a2 + a3 + a4 + b0 + b1 + b2 + b3 + b4 +
               (uint64_t)fa0 + (uint64_t)fa1 + (uint64_t)fa2 +
               (uint64_t)da0 + (uint64_t)da1 + (uint64_t)da2 +
               la0 + la1 + la2 + la3;
    
    return checksum;
}

/* Function 3: Computed goto state machine */
NOINLINE static uint64_t test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int x0 = seed, x1 = seed+10, x2 = seed+20, x3 = seed+30, x4 = seed+40;
    VOLATILE_VAR int y0 = seed+50, y1 = seed+60, y2 = seed+70, y3 = seed+80, y4 = seed+90;
    VOLATILE_VAR float fx0 = seed*3.1f, fx1 = seed*3.2f, fx2 = seed*3.3f, fx3 = seed*3.4f;
    VOLATILE_VAR double dx0 = seed*4.1, dx1 = seed*4.2, dx2 = seed*4.3, dx3 = seed*4.4;
    VOLATILE_VAR long lx0 = seed*1000, lx1 = seed*1001, lx2 = seed*1002;
    VOLATILE_VAR long ly0 = seed*2000, ly1 = seed*2001, ly2 = seed*2002;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state_a, &&state_b, &&state_c, 
        &&state_d, &&state_e, &&state_f,
        &&state_g, &&state_h
    };
    
    uint64_t checksum = 0;
    void* state = &&state_a;
    int counter = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Extensive arithmetic operations */
        x0 = x1 * x2 - x3;
        x1 = x4 ^ x0 + counter;
        x2 = y0 & x1 | x3;
        x3 = y1 + x2 * 7;
        x4 = y2 - x3 / 4;
        y0 = y3 * y4 + x0;
        y1 = y0 ^ x1 - counter;
        y2 = y1 & x2 | 0x7F;
        y3 = y2 + x3 * 11;
        y4 = y3 - x4 / 5;
        
        fx0 = fx1 * 2.1f + fx2;
        fx1 = fx0 - fx2 * 1.9f;
        fx2 = fx3 * 2.2f + fx1;
        fx3 = fx2 - fx0 * 0.8f;
        
        dx0 = dx1 * 2.01 + dx2;
        dx1 = dx0 * 1.99 - dx2;
        dx2 = dx3 * 2.02 + dx1;
        dx3 = dx2 - dx0 * 0.98;
        
        lx0 = lx1 + lx2 * counter;
        lx1 = ly0 ^ lx0;
        lx2 = lx1 & 0x7FFF;
        ly0 = ly1 + ly2;
        ly1 = lx0 ^ ly0;
        ly2 = ly1 & 0x3FFF;
        
        /* Force all variables to be register candidates */
        asm volatile("" : : 
            "r"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4),
            "r"(y0), "r"(y1), "r"(y2), "r"(y3), "r"(y4),
            "r"(fx0), "r"(fx1), "r"(fx2), "r"(fx3),
            "r"(dx0), "r"(dx1), "r"(dx2), "r"(dx3),
            "r"(lx0), "r"(lx1), "r"(lx2),
            "r"(ly0), "r"(ly1), "r"(ly2)
        );
        
        /* Computed goto - creates very complex CFG */
        goto *state;
        
        state_a:
            x0 = x1 + y0;
            state = labels[(counter + 1) % 8];
            goto next_iteration;
        state_b:
            x1 = x2 * y1;
            state = labels[(counter + 3) % 8];
            goto next_iteration;
        state_c:
            x2 = x3 - y2;
            state = labels[(counter + 5) % 8];
            goto next_iteration;
        state_d:
            x3 = x4 ^ y3;
            state = labels[(counter + 7) % 8];
            goto next_iteration;
        state_e:
            x4 = x0 & y4;
            state = labels[(counter + 2) % 8];
            goto next_iteration;
        state_f:
            y0 = y1 + x0;
            state = labels[(counter + 4) % 8];
            goto next_iteration;
        state_g:
            y1 = y2 * x1;
            state = labels[(counter + 6) % 8];
            goto next_iteration;
        state_h:
            y2 = y3 - x2;
            state = labels[(counter) % 8];
            goto next_iteration;
            
        next_iteration:
        counter = (counter + 1) & 0xF;
    }
    
    checksum = x0 + x1 + x2 + x3 + x4 + y0 + y1 + y2 + y3 + y4 +
               (uint64_t)fx0 + (uint64_t)fx1 + (uint64_t)fx2 + (uint64_t)fx3 +
               (uint64_t)dx0 + (uint64_t)dx1 + (uint64_t)dx2 + (uint64_t)dx3 +
               lx0 + lx1 + lx2 + ly0 + ly1 + ly2;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = LOOP_ITERATIONS;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = LOOP_ITERATIONS;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    uint64_t total_checksum = 0;
    
    /* Run all test functions to maximize coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}
