/* mcf_coverage.c - Program to trigger MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Complex irreducible CFG with goto jumping across loops */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Many local variables to create register pressure */
    VOLATILE_VAR int a0 = seed, a1 = seed + 1, a2 = seed + 2, a3 = seed + 3;
    VOLATILE_VAR int a4 = seed + 4, a5 = seed + 5, a6 = seed + 6, a7 = seed + 7;
    VOLATILE_VAR int a8 = seed + 8, a9 = seed + 9, a10 = seed + 10, a11 = seed + 11;
    VOLATILE_VAR float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    VOLATILE_VAR float f3 = seed * 0.4f, f4 = seed * 0.5f, f5 = seed * 0.6f;
    VOLATILE_VAR double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03;
    VOLATILE_VAR double d3 = seed * 0.04, d4 = seed * 0.05, d5 = seed * 0.06;
    VOLATILE_VAR long l0 = seed * 100L, l1 = seed * 200L, l2 = seed * 300L;
    VOLATILE_VAR long l3 = seed * 400L, l4 = seed * 500L, l5 = seed * 600L;
    VOLATILE_VAR unsigned long checksum = 0;
    
    int i, j;
    
    /* Outer loop with label for goto target */
    outer_loop:
    for (i = 0; i < iterations; i++) {
        /* Inner loop with multiple labels */
        inner_loop:
        for (j = 0; j < 10; j++) {
            /* Complex arithmetic to create register pressure */
            a0 = a1 + a2 * a3 - a4 / (a5 + 1);
            a1 = a2 + a3 * a4 - a5 / (a6 + 1);
            a2 = a3 + a4 * a5 - a6 / (a7 + 1);
            a3 = a4 + a5 * a6 - a7 / (a8 + 1);
            
            f0 = f1 * 1.1f + f2 * 0.9f - f3 * 0.5f;
            f1 = f2 * 1.2f + f3 * 0.8f - f4 * 0.6f;
            f2 = f3 * 1.3f + f4 * 0.7f - f5 * 0.7f;
            
            d0 = d1 * 1.01 + d2 * 0.99 - d3 * 0.55;
            d1 = d2 * 1.02 + d3 * 0.98 - d4 * 0.66;
            d2 = d3 * 1.03 + d4 * 0.97 - d5 * 0.77;
            
            l0 = l1 + l2 * 3 - l3 / 2 + l4 % 7;
            l1 = l2 + l3 * 4 - l4 / 3 + l5 % 8;
            l2 = l3 + l4 * 5 - l5 / 4 + l0 % 9;
            
            /* Mark variables as used to prevent optimization */
            asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3));
            asm volatile("" : : "r"(f0), "r"(f1), "r"(f2));
            asm volatile("" : : "r"(d0), "r"(d1), "r"(d2));
            asm volatile("" : : "r"(l0), "r"(l1), "r"(l2));
            
            /* Irreducible control flow: goto jumps into outer loop */
            if ((i * j + seed) % 37 == 0) {
                checksum += a0 + a1 + a2 + a3 + a4 + a5;
                goto outer_loop;  /* Jump from inner to outer loop */
            }
            
            if ((i * j + seed) % 41 == 0) {
                checksum += (unsigned long)(f0 + f1 + f2 + f3 + f4 + f5);
                goto middle_label;  /* Jump to label between loops */
            }
        }
        
        /* More arithmetic operations */
        a4 = a5 + a6 * a7 - a8 / (a9 + 1);
        a5 = a6 + a7 * a8 - a9 / (a10 + 1);
        a6 = a7 + a8 * a9 - a10 / (a11 + 1);
        
        continue;  /* Normal loop continuation */
        
        middle_label:
        /* This label is reachable via goto from inner loop */
        a7 = a8 + a9 * a10 - a11 / (a0 + 1);
        a8 = a9 + a10 * a11 - a0 / (a1 + 1);
        checksum += a7 + a8 + a9 + a10 + a11;
    }
    
    checksum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11;
    checksum += (unsigned long)(f0 + f1 + f2 + f3 + f4 + f5);
    checksum += (unsigned long)(d0 + d1 + d2 + d3 + d4 + d5);
    checksum += l0 + l1 + l2 + l3 + l4 + l5;
    
    return checksum;
}

/* Complex switch with goto creating irreducible regions */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int b0 = seed, b1 = seed * 2, b2 = seed * 3, b3 = seed * 4;
    VOLATILE_VAR int b4 = seed * 5, b5 = seed * 6, b6 = seed * 7, b7 = seed * 8;
    VOLATILE_VAR int b8 = seed * 9, b9 = seed * 10, b10 = seed * 11, b11 = seed * 12;
    VOLATILE_VAR float g0 = seed * 0.15f, g1 = seed * 0.25f, g2 = seed * 0.35f;
    VOLATILE_VAR float g3 = seed * 0.45f, g4 = seed * 0.55f, g5 = seed * 0.65f;
    VOLATILE_VAR unsigned long checksum = 0;
    
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Long dependency chain */
        b0 = b1 + b2;
        b1 = b2 + b3;
        b2 = b3 + b4;
        b3 = b4 + b5;
        b4 = b5 + b6;
        b5 = b6 + b7;
        b6 = b7 + b8;
        b7 = b8 + b9;
        b8 = b9 + b10;
        b9 = b10 + b11;
        b10 = b11 + b0;
        b11 = b0 + b1;
        
        g0 = g1 * 1.5f - g2;
        g1 = g2 * 1.6f - g3;
        g2 = g3 * 1.7f - g4;
        g3 = g4 * 1.8f - g5;
        g4 = g5 * 1.9f - g0;
        g5 = g0 * 2.0f - g1;
        
        /* Complex switch with goto to different labels */
        switch ((i + seed) % 7) {
            case 0:
                checksum += b0 + b1;
                goto label_alpha;
            case 1:
                checksum += b2 + b3;
                goto label_beta;
            case 2:
                checksum += b4 + b5;
                goto label_gamma;
            case 3:
                checksum += b6 + b7;
                /* Fall through */
            case 4:
                checksum += b8 + b9;
                goto label_delta;
            case 5:
                checksum += b10 + b11;
                goto label_alpha;
            default:
                checksum += (unsigned long)(g0 + g1 + g2);
                break;
        }
        
        /* More operations after switch */
        b0 = b1 * b2 - b3;
        continue;
        
        label_alpha:
        b1 = b2 * b3 - b4;
        checksum += b1;
        continue;
        
        label_beta:
        b2 = b3 * b4 - b5;
        checksum += b2;
        goto label_delta;
        
        label_gamma:
        b3 = b4 * b5 - b6;
        checksum += b3;
        /* Fall through to delta */
        
        label_delta:
        b4 = b5 * b6 - b7;
        checksum += b4;
        /* Loop continues */
    }
    
    checksum += b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 + b11;
    checksum += (unsigned long)(g0 + g1 + g2 + g3 + g4 + g5);
    
    return checksum;
}

/* Computed goto state machine creating complex CFG */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int c0 = seed, c1 = seed + 100, c2 = seed + 200, c3 = seed + 300;
    VOLATILE_VAR int c4 = seed + 400, c5 = seed + 500, c6 = seed + 600, c7 = seed + 700;
    VOLATILE_VAR int c8 = seed + 800, c9 = seed + 900, c10 = seed + 1000;
    VOLATILE_VAR double e0 = seed * 0.001, e1 = seed * 0.002, e2 = seed * 0.003;
    VOLATILE_VAR double e3 = seed * 0.004, e4 = seed * 0.005, e5 = seed * 0.006;
    VOLATILE_VAR unsigned long checksum = 0;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, 
        &&state4, &&state5, &&state6, &&state7
    };
    
    int state = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Heavy arithmetic creating register pressure */
        c0 = c1 * 2 - c2 + c3 / (c4 % 5 + 1);
        c1 = c2 * 3 - c3 + c4 / (c5 % 6 + 1);
        c2 = c3 * 4 - c4 + c5 / (c6 % 7 + 1);
        c3 = c4 * 5 - c5 + c6 / (c7 % 8 + 1);
        c4 = c5 * 6 - c6 + c7 / (c8 % 9 + 1);
        c5 = c6 * 7 - c7 + c8 / (c9 % 10 + 1);
        c6 = c7 * 8 - c8 + c9 / (c10 % 11 + 1);
        
        e0 = e1 * 1.11 + e2 * 0.89 - e3;
        e1 = e2 * 1.12 + e3 * 0.88 - e4;
        e2 = e3 * 1.13 + e4 * 0.87 - e5;
        e3 = e4 * 1.14 + e5 * 0.86 - e0;
        e4 = e5 * 1.15 + e0 * 0.85 - e1;
        e5 = e0 * 1.16 + e1 * 0.84 - e2;
        
        /* Mark as used */
        asm volatile("" : : "r"(c0), "r"(c1), "r"(c2), "r"(c3), "r"(c4), "r"(c5), "r"(c6));
        asm volatile("" : : "r"(e0), "r"(e1), "r"(e2), "r"(e3), "r"(e4), "r"(e5));
        
        /* Update state based on computation */
        state = (c0 + c1 + c2 + i) % 8;
        
        /* Computed goto - creates irreducible CFG */
        goto *labels[state];
        
        state0:
            checksum += c0 + c1;
            c7 = c0 + c1;
            goto loop_end;
        state1:
            checksum += c2 + c3;
            c8 = c2 + c3;
            goto loop_end;
        state2:
            checksum += c4 + c5;
            c9 = c4 + c5;
            goto loop_end;
        state3:
            checksum += c6 + c7;
            c10 = c6 + c7;
            goto loop_end;
        state4:
            checksum += (unsigned long)(e0 + e1);
            c0 = (int)(e0 * 100);
            goto loop_end;
        state5:
            checksum += (unsigned long)(e2 + e3);
            c1 = (int)(e2 * 100);
            goto loop_end;
        state6:
            checksum += (unsigned long)(e4 + e5);
            c2 = (int)(e4 * 100);
            goto loop_end;
        state7:
            checksum += c8 + c9 + c10;
            c3 = c8 + c9;
            /* Fall through */
        
        loop_end:
        /* Continue loop */
    }
    
    checksum += c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9 + c10;
    checksum += (unsigned long)(e0 + e1 + e2 + e3 + e4 + e5);
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = 42;
    unsigned long total_checksum = 0;
    
    /* Parse command line for iteration count */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    /* Run all test functions with different CFG patterns */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    /* Additional test with mixed patterns */
    for (int i = 0; i < iterations / 100; i++) {
        VOLATILE_VAR int x = i * seed;
        VOLATILE_VAR float y = i * seed * 0.5f;
        VOLATILE_VAR double z = i * seed * 0.1;
        
        /* Create artificial register pressure */
        for (int j = 0; j < 50; j++) {
            x = x * 3 - j;
            y = y * 1.1f + j * 0.01f;
            z = z * 1.01 - j * 0.001;
            
            /* Use inline asm to keep variables live */
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
        }
        
        total_checksum += x + (unsigned long)y + (unsigned long)z;
    }
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}
