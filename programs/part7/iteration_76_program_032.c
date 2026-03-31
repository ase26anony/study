/* mcf_coverage.c - Program to trigger MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
__attribute__((noinline, noipa))
static uint64_t test_irreducible_goto(int iterations, int seed) {
    /* Many local variables to create register pressure */
    volatile int v0 = seed + 1;
    volatile int v1 = seed + 2;
    volatile int v2 = seed + 3;
    volatile int v3 = seed + 4;
    volatile int v4 = seed + 5;
    volatile int v5 = seed + 6;
    volatile int v6 = seed + 7;
    volatile int v7 = seed + 8;
    volatile int v8 = seed + 9;
    volatile int v9 = seed + 10;
    volatile float f0 = seed * 0.1f;
    volatile float f1 = seed * 0.2f;
    volatile float f2 = seed * 0.3f;
    volatile float f3 = seed * 0.4f;
    volatile double d0 = seed * 0.01;
    volatile double d1 = seed * 0.02;
    volatile double d2 = seed * 0.03;
    volatile double d3 = seed * 0.04;
    volatile long l0 = seed * 100L;
    volatile long l1 = seed * 200L;
    volatile long l2 = seed * 300L;
    volatile long l3 = seed * 400L;
    volatile long l4 = seed * 500L;
    volatile long l5 = seed * 600L;
    volatile long l6 = seed * 700L;
    volatile long l7 = seed * 800L;
    volatile long l8 = seed * 900L;
    volatile long l9 = seed * 1000L;
    
    uint64_t checksum = 0;
    
    /* Create irreducible loop with goto jumping across boundaries */
    int i = 0;
    outer_loop:
    while (i < iterations) {
        /* Force all variables to be live across the loop */
        asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
        asm volatile("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9));
        asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3));
        asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3), "r"(l4));
        asm volatile("" : : "r"(l5), "r"(l6), "r"(l7), "r"(l8), "r"(l9));
        
        if (i % 7 == 0) {
            goto inner_block_a;
        } else if (i % 7 == 1) {
            goto inner_block_b;
        } else if (i % 7 == 2) {
            goto inner_block_c;
        } else {
            goto inner_block_d;
        }
        
        inner_block_a:
        /* Complex arithmetic chain */
        v0 = v1 + v2 * v3 - v4 / (v5 + 1);
        v1 = v2 + v3 * v4 - v5 / (v6 + 1);
        v2 = v3 + v4 * v5 - v6 / (v7 + 1);
        v3 = v4 + v5 * v6 - v7 / (v8 + 1);
        f0 = f1 * 1.1f + f2 * 0.9f - f3;
        d0 = d1 * 1.01 + d2 * 0.99 - d3;
        l0 = l1 + l2 - l3 * l4 / (l5 + 1);
        if (i % 3 == 0) goto outer_loop;
        else goto inner_block_b;
        
        inner_block_b:
        v4 = v5 + v6 * v7 - v8 / (v9 + 1);
        v5 = v6 + v7 * v8 - v9 / (v0 + 1);
        v6 = v7 + v8 * v9 - v0 / (v1 + 1);
        f1 = f2 * 1.2f + f3 * 0.8f - f0;
        d1 = d2 * 1.02 + d3 * 0.98 - d0;
        l1 = l2 + l3 - l4 * l5 / (l6 + 1);
        if (i % 5 == 0) goto inner_block_d;
        else goto inner_block_c;
        
        inner_block_c:
        v7 = v8 + v9 * v0 - v1 / (v2 + 1);
        v8 = v9 + v0 * v1 - v2 / (v3 + 1);
        v9 = v0 + v1 * v2 - v3 / (v4 + 1);
        f2 = f3 * 1.3f + f0 * 0.7f - f1;
        d2 = d3 * 1.03 + d0 * 0.97 - d1;
        l2 = l3 + l4 - l5 * l6 / (l7 + 1);
        if (i % 11 == 0) goto outer_loop;
        else goto inner_block_d;
        
        inner_block_d:
        f3 = f0 * 1.4f + f1 * 0.6f - f2;
        d3 = d0 * 1.04 + d1 * 0.96 - d2;
        l3 = l4 + l5 - l6 * l7 / (l8 + 1);
        l4 = l5 + l6 - l7 * l8 / (l9 + 1);
        l5 = l6 + l7 - l8 * l9 / (l0 + 1);
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        checksum += (uint64_t)(f0 * 100) + (uint64_t)(f1 * 100) + 
                   (uint64_t)(f2 * 100) + (uint64_t)(f3 * 100);
        checksum += (uint64_t)(d0 * 100) + (uint64_t)(d1 * 100) + 
                   (uint64_t)(d2 * 100) + (uint64_t)(d3 * 100);
        checksum += l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9;
        i++;
        if (i % 13 == 0) goto outer_loop;
    }
    
    return checksum;
}

__attribute__((noinline, noipa))
static uint64_t test_switch_goto(int iterations, int seed) {
    /* Another set of many variables */
    volatile int a0 = seed, a1 = seed+1, a2 = seed+2, a3 = seed+3, a4 = seed+4;
    volatile int a5 = seed+5, a6 = seed+6, a7 = seed+7, a8 = seed+8, a9 = seed+9;
    volatile float b0 = seed*0.5f, b1 = seed*0.6f, b2 = seed*0.7f, b3 = seed*0.8f;
    volatile double c0 = seed*0.05, c1 = seed*0.06, c2 = seed*0.07, c3 = seed*0.08;
    volatile long e0 = seed*50, e1 = seed*60, e2 = seed*70, e3 = seed*80, e4 = seed*90;
    volatile long e5 = seed*100, e6 = seed*110, e7 = seed*120, e8 = seed*130, e9 = seed*140;
    
    uint64_t checksum = 0;
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Keep variables live */
        asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4));
        asm volatile("" : : "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9));
        asm volatile("" : : "r"(b0), "r"(b1), "r"(b2), "r"(b3));
        asm volatile("" : : "r"(c0), "r"(c1), "r"(c2), "r"(c3));
        asm volatile("" : : "r"(e0), "r"(e1), "r"(e2), "r"(e3), "r"(e4));
        asm volatile("" : : "r"(e5), "r"(e6), "r"(e7), "r"(e8), "r"(e9));
        
        /* Switch with goto creating irreducible flow */
        switch (state % 5) {
            case 0:
                a0 = a1 * a2 - a3 + a4;
                a1 = a2 * a3 - a4 + a5;
                b0 = b1 + b2 - b3;
                if (i % 17 == 0) goto case_0_exit;
                else goto case_1_label;
                
            case 1:
                case_1_label:
                a2 = a3 * a4 - a5 + a6;
                a3 = a4 * a5 - a6 + a7;
                b1 = b2 + b3 - b0;
                if (i % 19 == 0) goto case_1_exit;
                else goto case_2_label;
                
            case 2:
                case_2_label:
                a4 = a5 * a6 - a7 + a8;
                a5 = a6 * a7 - a8 + a9;
                b2 = b3 + b0 - b1;
                goto case_3_label;  /* Jump to another case */
                
            case 3:
                case_3_label:
                a6 = a7 * a8 - a9 + a0;
                a7 = a8 * a9 - a0 + a1;
                b3 = b0 + b1 - b2;
                if (i % 23 == 0) goto case_0_label;
                else goto case_4_label;
                
            case 4:
                case_4_label:
                a8 = a9 * a0 - a1 + a2;
                a9 = a0 * a1 - a2 + a3;
                c0 = c1 * 1.1 - c2 + c3;
                goto switch_end;
                
            case_0_label:
                a0 = a9 + a8 - a7;
                goto case_0_exit;
                
            case_0_exit:
                c1 = c2 * 1.2 - c3 + c0;
                break;
                
            case_1_exit:
                c2 = c3 * 1.3 - c0 + c1;
                break;
                
            default:
                c3 = c0 * 1.4 - c1 + c2;
                break;
        }
        switch_end:
        
        /* More arithmetic to increase register pressure */
        e0 = e1 + e2 * e3 - e4 / (e5 + 1);
        e1 = e2 + e3 * e4 - e5 / (e6 + 1);
        e2 = e3 + e4 * e5 - e6 / (e7 + 1);
        e3 = e4 + e5 * e6 - e7 / (e8 + 1);
        e4 = e5 + e6 * e7 - e8 / (e9 + 1);
        
        checksum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
        checksum += (uint64_t)(b0 * 100) + (uint64_t)(b1 * 100) + 
                   (uint64_t)(b2 * 100) + (uint64_t)(b3 * 100);
        checksum += (uint64_t)(c0 * 100) + (uint64_t)(c1 * 100) + 
                   (uint64_t)(c2 * 100) + (uint64_t)(c3 * 100);
        checksum += e0 + e1 + e2 + e3 + e4 + e5 + e6 + e7 + e8 + e9;
        
        state = (state * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return checksum;
}

__attribute__((noinline, noipa))
static uint64_t test_computed_goto(int iterations, int seed) {
    /* Yet another set of variables */
    volatile int x0 = seed, x1 = seed+10, x2 = seed+20, x3 = seed+30, x4 = seed+40;
    volatile int x5 = seed+50, x6 = seed+60, x7 = seed+70, x8 = seed+80, x9 = seed+90;
    volatile float y0 = seed*0.15f, y1 = seed*0.25f, y2 = seed*0.35f, y3 = seed*0.45f;
    volatile double z0 = seed*0.015, z1 = seed*0.025, z2 = seed*0.035, z3 = seed*0.045;
    volatile long w0 = seed*150, w1 = seed*250, w2 = seed*350, w3 = seed*450, w4 = seed*550;
    volatile long w5 = seed*650, w6 = seed*750, w7 = seed*850, w8 = seed*950, w9 = seed*1050;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&label0, &&label1, &&label2, &&label3, 
        &&label4, &&label5, &&label6, &&label7
    };
    
    uint64_t checksum = 0;
    int pc = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Force all variables live */
        asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4));
        asm volatile("" : : "r"(x5), "r"(x6), "r"(x7), "r"(x8), "r"(x9));
        asm volatile("" : : "r"(y0), "r"(y1), "r"(y2), "r"(y3));
        asm volatile("" : : "r"(z0), "r"(z1), "r"(z2), "r"(z3));
        asm volatile("" : : "r"(w0), "r"(w1), "r"(w2), "r"(w3), "r"(w4));
        asm volatile("" : : "r"(w5), "r"(w6), "r"(w7), "r"(w8), "r"(w9));
        
        /* State machine with computed goto */
        goto *labels[pc % 8];
        
        label0:
            x0 = x1 + x2 - x3 * x4 / (x5 + 1);
            x1 = x2 + x3 - x4 * x5 / (x6 + 1);
            y0 = y1 * 1.5f - y2 + y3;
            pc = (pc + 1) % 8;
            if (i % 29 == 0) goto label7;
            continue;
            
        label1:
            x2 = x3 + x4 - x5 * x6 / (x7 + 1);
            x3 = x4 + x5 - x6 * x7 / (x8 + 1);
            y1 = y2 * 1.6f - y3 + y0;
            pc = (pc + 3) % 8;
            if (i % 31 == 0) goto label0;
            continue;
            
        label2:
            x4 = x5 + x6 - x7 * x8 / (x9 + 1);
            x5 = x6 + x7 - x8 * x9 / (x0 + 1);
            y2 = y3 * 1.7f - y0 + y1;
            pc = (pc + 5) % 8;
            goto label4;
            
        label3:
            x6 = x7 + x8 - x9 * x0 / (x1 + 1);
            x7 = x8 + x9 - x0 * x1 / (x2 + 1);
            y3 = y0 * 1.8f - y1 + y2;
            pc = (pc + 7) % 8;
            if (i % 37 == 0) goto label1;
            continue;
            
        label4:
            z0 = z1 * 1.11 - z2 + z3;
            z1 = z2 * 1.12 - z3 + z0;
            w0 = w1 + w2 * w3 - w4 / (w5 + 1);
            pc = (pc + 2) % 8;
            goto label5;
            
        label5:
            z2 = z3 * 1.13 - z0 + z1;
            z3 = z0 * 1.14 - z1 + z2;
            w1 = w2 + w3 * w4 - w5 / (w6 + 1);
            pc = (pc + 4) % 8;
            if (i % 41 == 0) goto label2;
            continue;
            
        label6:
            w2 = w3 + w4 * w5 - w6 / (w7 + 1);
            w3 = w4 + w5 * w6 - w7 / (w8 + 1);
            w4 = w5 + w6 * w7 - w8 / (w9 + 1);
            pc = (pc + 6) % 8;
            goto label3;
            
        label7:
            w5 = w6 + w7 * w8 - w9 / (w0 + 1);
            w6 = w7 + w8 * w9 - w0 / (w1 + 1);
            w7 = w8 + w9 * w0 - w1 / (w2 + 1);
            pc = (pc + 1) % 8;
            if (i % 43 == 0) goto label6;
            continue;
    }
    
    /* Final computation */
    checksum += x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9;
    checksum += (uint64_t)(y0 * 100) + (uint64_t)(y1 * 100) + 
               (uint64_t)(y2 * 100) + (uint64_t)(y3 * 100);
    checksum += (uint64_t)(z0 * 100) + (uint64_t)(z1 * 100) + 
               (uint64_t)(z2 * 100) + (uint64_t)(z3 * 100);
    checksum += w0 + w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + w9;
    
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
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    uint64_t total_checksum = 0;
    
    /* Call all test functions to increase coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Final checksum: %llu\n", (unsigned long long)total_checksum);
    
    return 0;
}
