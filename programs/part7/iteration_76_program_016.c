/* mcf_coverage.c - Program to trigger GCC's MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Function 1: Irreducible loops with goto jumping across loop boundaries */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many local variables to increase register pressure */
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
    int i;
    
    /* Outer loop */
    for (i = 0; i < iterations; i++) {
        /* Create irreducible region with goto jumping into inner loop */
        if (i % 3 == 0) {
            goto inner_loop_start;
        }
        
        /* Block A: Long arithmetic chain */
        v0 = v1 + v2;
        v1 = v3 * v4;
        v2 = v5 - v6;
        v3 = v7 ^ v8;
        v4 = v9 & v0;
        f0 = f1 * f2 + f3;
        f1 = f2 - f0 * 0.5f;
        d0 = d1 * 1.1 + d2;
        d1 = d0 * 0.9 - d2;
        l0 = l1 + l2 * l3;
        l1 = l4 - l5 / (l6 + 1);
        l2 = l7 ^ l8 | l9;
        
        /* Force register pressure with inline asm */
        asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
        asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2));
        asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3), "r"(l4));
        
        if (i % 7 == 0) {
            goto outer_loop_end;
        }
        
    inner_loop_start:
        /* Inner loop - but entered via goto from above */
        for (int j = 0; j < 5; j++) {
            /* Block B: Different arithmetic chain */
            v5 = v6 + v7 * j;
            v6 = v8 - v9 / (j + 1);
            v7 = v0 ^ v1;
            v8 = v2 | v3;
            v9 = v4 & v5;
            f2 = f3 * j + f0;
            f3 = f1 - f2 * 0.3f;
            d2 = d0 * j + d1;
            l3 = l4 + l5 * j;
            l4 = l6 - l7 / (j + 1);
            l5 = l8 ^ l9;
            l6 = l0 | l1;
            l7 = l2 & l3;
            
            asm volatile("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9));
            asm volatile("" : : "r"(f2), "r"(f3));
            asm volatile("" : : "r"(d2));
            asm volatile("" : : "r"(l5), "r"(l6), "r"(l7), "r"(l8), "r"(l9));
            
            if (j % 2 == 0 && i % 5 == 0) {
                goto cross_jump;  /* Jump out of inner loop */
            }
        }
        
        if (i % 11 == 0) {
            continue;  /* Skip the cross_jump label */
        }
        
    cross_jump:
        /* Block C: Mixed operations */
        v0 = v5 + v6 - v7;
        v1 = v8 * v9 / (v0 + 1);
        f0 = f1 + f2 * f3;
        d0 = d1 + d2 * 0.5;
        l0 = l3 + l4 - l5 * l6;
        
    outer_loop_end:
        /* Final block with checksum update */
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        checksum += (unsigned long)(f0 + f1 + f2 + f3);
        checksum += (unsigned long)(d0 + d1 + d2);
        checksum += l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9;
    }
    
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int a0 = seed, a1 = seed+1, a2 = seed+2, a3 = seed+3, a4 = seed+4;
    VOLATILE_VAR int b0 = seed+5, b1 = seed+6, b2 = seed+7, b3 = seed+8, b4 = seed+9;
    VOLATILE_VAR float fa0 = seed*0.11f, fa1 = seed*0.22f, fa2 = seed*0.33f;
    VOLATILE_VAR double da0 = seed*0.011, da1 = seed*0.022, da2 = seed*0.033;
    VOLATILE_VAR long la0 = seed*111L, la1 = seed*222L, la2 = seed*333L;
    VOLATILE_VAR long lb0 = seed*444L, lb1 = seed*555L, lb2 = seed*666L;
    
    unsigned long checksum = 0;
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex switch with goto to different labels */
        switch (state % 7) {
            case 0:
                a0 = a1 + a2 * i;
                a1 = a3 - a4 / (i + 1);
                fa0 = fa1 * i + fa2;
                da0 = da1 * 1.5 + da2;
                la0 = la1 + la2 * i;
                if (i % 3 == 0) goto label_b;
                break;
                
            case 1:
                a2 = a3 ^ a4;
                a3 = a0 | a1;
                fa1 = fa2 - fa0 * 0.7f;
                da1 = da0 * 0.8 - da2;
                la1 = la2 - la0 / (i + 1);
                if (i % 5 == 0) goto label_c;
                break;
                
            case 2:
            label_b:
                b0 = b1 + b2 * i;
                b1 = b3 - b4;
                fa2 = fa0 * 0.3f + fa1;
                da2 = da0 + da1 * i;
                la2 = lb0 + lb1 * i;
                if (i % 7 == 0) goto label_d;
                break;
                
            case 3:
            label_c:
                b2 = b3 ^ b4;
                b3 = b0 | b1;
                fa0 = fa1 + fa2 * 2.0f;
                da0 = da1 * 1.1 + da2;
                lb0 = lb1 + lb2 * i;
                if (i % 11 == 0) goto label_a;
                break;
                
            case 4:
            label_d:
                a4 = b0 + b1 - b2;
                b4 = a0 * a1 / (a2 + 1);
                fa1 = fa0 * fa2;
                da1 = da0 + da2 * 0.6;
                lb1 = la0 + la1 - la2;
                if (i % 13 == 0) goto label_b;
                break;
                
            case 5:
            label_a:
                a0 = b2 + b3 - b4;
                a1 = a2 * a3 / (a4 + 1);
                fa2 = fa0 + fa1 * 0.9f;
                da2 = da0 * 0.7 + da1;
                lb2 = lb0 + lb1 * lb2;
                break;
                
            case 6:
                /* Default case with more arithmetic */
                a2 = a3 + a4 * i;
                a3 = b0 - b1;
                fa0 = fa1 * 1.5f + fa2;
                da0 = da1 * 2.0 - da2;
                la0 = lb0 ^ lb1 | lb2;
                if (i % 17 == 0) goto label_c;
                break;
        }
        
        /* Force all variables to be live */
        asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4),
                         "r"(b0), "r"(b1), "r"(b2), "r"(b3), "r"(b4));
        asm volatile("" : : "r"(fa0), "r"(fa1), "r"(fa2));
        asm volatile("" : : "r"(da0), "r"(da1), "r"(da2));
        asm volatile("" : : "r"(la0), "r"(la1), "r"(la2),
                         "r"(lb0), "r"(lb1), "r"(lb2));
        
        /* Update state and checksum */
        state = (state * 1103515245 + 12345) & 0x7fffffff;
        checksum += a0 + a1 + a2 + a3 + a4 + b0 + b1 + b2 + b3 + b4;
        checksum += (unsigned long)(fa0 + fa1 + fa2);
        checksum += (unsigned long)(da0 + da1 + da2);
        checksum += la0 + la1 + la2 + lb0 + lb1 + lb2;
    }
    
    return checksum;
}

/* Function 3: Computed goto state machine */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int x0 = seed, x1 = seed+10, x2 = seed+20, x3 = seed+30;
    VOLATILE_VAR int y0 = seed+40, y1 = seed+50, y2 = seed+60, y3 = seed+70;
    VOLATILE_VAR float fx0 = seed*0.15f, fx1 = seed*0.25f, fx2 = seed*0.35f;
    VOLATILE_VAR double dx0 = seed*0.015, dx1 = seed*0.025, dx2 = seed*0.035;
    VOLATILE_VAR long lx0 = seed*150L, lx1 = seed*250L, lx2 = seed*350L;
    VOLATILE_VAR long ly0 = seed*450L, ly1 = seed*550L, ly2 = seed*650L;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, 
        &&state4, &&state5, &&state6, &&state7
    };
    
    unsigned long checksum = 0;
    int state = seed % 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Computed goto - creates very complex CFG */
        goto *labels[state];
        
    state0:
        x0 = x1 + x2 * i;
        x1 = x3 - x0 / (i + 1);
        fx0 = fx1 * i + fx2;
        dx0 = dx1 * 1.2 + dx2;
        lx0 = lx1 + lx2 * i;
        state = (i % 3 == 0) ? 2 : 1;
        goto end_state;
        
    state1:
        x2 = x3 ^ x0;
        x3 = x1 | x2;
        fx1 = fx2 - fx0 * 0.6f;
        dx1 = dx0 * 0.9 - dx2;
        lx1 = lx2 - lx0 / (i + 1);
        state = (i % 5 == 0) ? 3 : 0;
        goto end_state;
        
    state2:
        y0 = y1 + y2 * i;
        y1 = y3 - y0;
        fx2 = fx0 * 0.4f + fx1;
        dx2 = dx0 + dx1 * i;
        lx2 = ly0 + ly1 * i;
        state = (i % 7 == 0) ? 4 : 1;
        goto end_state;
        
    state3:
        y2 = y3 ^ y0;
        y3 = y1 | y2;
        fx0 = fx1 + fx2 * 1.8f;
        dx0 = dx1 * 1.3 + dx2;
        ly0 = ly1 + ly2 * i;
        state = (i % 11 == 0) ? 5 : 2;
        goto end_state;
        
    state4:
        x0 = y0 + y1 - y2;
        x1 = x2 * x3 / (x0 + 1);
        fx1 = fx0 * fx2;
        dx1 = dx0 + dx2 * 0.7;
        ly1 = lx0 + lx1 - lx2;
        state = (i % 13 == 0) ? 6 : 3;
        goto end_state;
        
    state5:
        x2 = y2 + y3 - y0;
        x3 = x0 * x1 / (x2 + 1);
        fx2 = fx0 + fx1 * 1.1f;
        dx2 = dx0 * 0.8 + dx1;
        ly2 = ly0 + ly1 * ly2;
        state = (i % 17 == 0) ? 7 : 4;
        goto end_state;
        
    state6:
        y0 = x0 + x1 * i;
        y1 = x2 - x3;
        fx0 = fx1 * 1.7f + fx2;
        dx0 = dx1 * 2.1 - dx2;
        lx0 = ly0 ^ ly1 | ly2;
        state = (i % 19 == 0) ? 0 : 5;
        goto end_state;
        
    state7:
        y2 = x2 + x3 * i;
        y3 = y0 - y1;
        fx1 = fx0 * 0.5f + fx2;
        dx1 = dx0 * 1.4 + dx2;
        lx1 = lx2 ^ ly0 | ly1;
        state = (i % 23 == 0) ? 1 : 6;
        /* Fall through */
        
    end_state:
        /* Force register pressure */
        asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3),
                         "r"(y0), "r"(y1), "r"(y2), "r"(y3));
        asm volatile("" : : "r"(fx0), "r"(fx1), "r"(fx2));
        asm volatile("" : : "r"(dx0), "r"(dx1), "r"(dx2));
        asm volatile("" : : "r"(lx0), "r"(lx1), "r"(lx2),
                         "r"(ly0), "r"(ly1), "r"(ly2));
        
        checksum += x0 + x1 + x2 + x3 + y0 + y1 + y2 + y3;
        checksum += (unsigned long)(fx0 + fx1 + fx2);
        checksum += (unsigned long)(dx0 + dx1 + dx2);
        checksum += lx0 + lx1 + lx2 + ly0 + ly1 + ly2;
    }
    
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
    
    /* Run all test functions to maximize coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}
