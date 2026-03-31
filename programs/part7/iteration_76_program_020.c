/* Test program to trigger MCF fixup graph debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_VARS 30
#define MAX_ITER 10000

/* Prevent optimization */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Complex function with irreducible CFG using nested goto */
__attribute__((noinline, optimize("no-goto")))
static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Many local variables to create register pressure */
    volatile int v0 = seed + 1;
    int v1 = seed + 2, v2 = seed + 3, v3 = seed + 4, v4 = seed + 5;
    int v5 = seed + 6, v6 = seed + 7, v7 = seed + 8, v8 = seed + 9, v9 = seed + 10;
    float f0 = seed * 1.1f, f1 = seed * 1.2f, f2 = seed * 1.3f, f3 = seed * 1.4f;
    double d0 = seed * 2.1, d1 = seed * 2.2, d2 = seed * 2.3, d3 = seed * 2.4;
    long l0 = seed * 3, l1 = seed * 4, l2 = seed * 5, l3 = seed * 6;
    unsigned long checksum = 0;
    
    /* Labels for irreducible jumps */
    loop_start:
    inner_loop:
    middle_block:
    exit_path:
    
    for (int i = 0; i < iterations; i++) {
        /* Complex arithmetic to keep variables live */
        v0 = v1 + v2 * v3 - v4;
        v1 = v5 ^ v6 | v7 & v8;
        v2 = v9 * v0 / (v1 + 1);
        v3 = v4 << (v5 & 3);
        v4 = v6 >> (v7 % 4);
        
        f0 = f1 * 1.1f + f2 - f3;
        f1 = f0 * 0.9f / (f2 + 0.001f);
        f2 = f3 + f1 * 2.0f;
        f3 = f2 - f0 * 0.5f;
        
        d0 = d1 * 1.01 + d2 - d3;
        d1 = d0 / 1.02 + d3 * 0.98;
        d2 = d1 + d3 * 1.5;
        d3 = d2 - d0 * 0.75;
        
        l0 = l1 + l2 * l3;
        l1 = l0 ^ l2 | l3;
        l2 = l1 * l3 / (l0 + 1);
        l3 = l2 << (l1 & 7);
        
        /* Irreducible control flow with goto jumping across loops */
        if ((i & 0x1F) == 0) {
            goto inner_loop;
        }
        if ((i & 0x3F) == 0) {
            goto middle_block;
        }
        if ((i % 97) == 0) {
            goto exit_path;
        }
        
        /* More arithmetic */
        v5 = v6 + v7 - v8 * v9;
        v6 = v0 ^ v1 & v2 | v3;
        v7 = v4 * v5 / (v6 + 2);
        v8 = v7 << (v9 & 3);
        v9 = v8 >> (v0 % 5);
        
        continue;
        
        inner_loop:
        v0 = v0 + 1;
        v1 = v1 - 1;
        if ((i & 1) == 0) goto middle_block;
        continue;
        
        middle_block:
        v2 = v2 * 2;
        v3 = v3 / 2;
        if ((i & 3) == 0) goto loop_start;
        continue;
        
        exit_path:
        v4 = v4 ^ 0xAAAA;
        v5 = v5 | 0x5555;
        if ((i & 7) == 0) goto loop_start;
    }
    
    /* Aggregate checksum */
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               (unsigned long)f0 + (unsigned long)f1 + 
               (unsigned long)f2 + (unsigned long)f3 +
               (unsigned long)d0 + (unsigned long)d1 +
               (unsigned long)d2 + (unsigned long)d3 +
               l0 + l1 + l2 + l3;
    
    /* Keep variables alive */
    KEEP_ALIVE(v0); KEEP_ALIVE(v1); KEEP_ALIVE(v2); KEEP_ALIVE(v3);
    KEEP_ALIVE(v4); KEEP_ALIVE(v5); KEEP_ALIVE(v6); KEEP_ALIVE(v7);
    KEEP_ALIVE(v8); KEEP_ALIVE(v9);
    
    return checksum;
}

/* Function with switch and goto creating irreducible regions */
__attribute__((noinline))
static unsigned long test_switch_goto(int iterations, int seed) {
    volatile int s0 = seed * 11;
    int s1 = seed * 12, s2 = seed * 13, s3 = seed * 14, s4 = seed * 15;
    int s5 = seed * 16, s6 = seed * 17, s7 = seed * 18, s8 = seed * 19, s9 = seed * 20;
    float sf0 = seed * 3.1f, sf1 = seed * 3.2f, sf2 = seed * 3.3f;
    double sd0 = seed * 4.1, sd1 = seed * 4.2, sd2 = seed * 4.3;
    long sl0 = seed * 50, sl1 = seed * 51, sl2 = seed * 52;
    unsigned long checksum = 0;
    
    /* Labels for switch goto targets */
    case_a_return:
    case_b_loop:
    case_c_exit:
    default_handler:
    
    for (int i = 0; i < iterations; i++) {
        /* Register pressure operations */
        s0 = s1 + s2 - s3 * s4;
        s1 = s5 ^ s6 | s7 & s8;
        s2 = s9 * s0 / (s1 + 1);
        s3 = s4 << (s5 & 3);
        s4 = s6 >> (s7 % 4);
        
        sf0 = sf1 * 1.5f + sf2;
        sf1 = sf0 * 0.8f / (sf2 + 0.001f);
        sf2 = sf1 * 2.0f - sf0;
        
        sd0 = sd1 * 1.1 + sd2;
        sd1 = sd0 / 1.05 + sd2 * 0.95;
        sd2 = sd1 * 1.2 - sd0;
        
        sl0 = sl1 + sl2;
        sl1 = sl0 ^ sl2;
        sl2 = sl1 * 3 / (sl0 + 1);
        
        /* Switch with goto to external labels - creates irreducible CFG */
        switch (i & 0x7) {
            case 0:
                s5 = s6 + s7;
                if ((i & 0xF) == 0) goto case_a_return;
                break;
            case 1:
                s6 = s7 - s8;
                if ((i % 13) == 0) goto case_b_loop;
                break;
            case 2:
                s7 = s8 * s9;
                if ((i & 0x1F) == 0) goto case_c_exit;
                break;
            case 3:
                s8 = s9 / (s0 + 1);
                if ((i % 17) == 0) goto default_handler;
                break;
            default:
                s9 = s0 ^ s1;
                if ((i & 0x3F) == 0) goto case_a_return;
                break;
        }
        
        /* More operations */
        s5 = s6 * s7 - s8;
        s6 = s9 & s0 | s1;
        s7 = s2 + s3 * s4;
        s8 = s5 << (s6 & 7);
        s9 = s7 >> (s8 % 3);
        
        continue;
        
        case_a_return:
        s0 = s0 + 100;
        s1 = s1 - 50;
        if ((i & 1) == 0) continue;
        goto case_b_loop;
        
        case_b_loop:
        s2 = s2 * 3;
        s3 = s3 / 3;
        if ((i & 2) == 0) continue;
        goto case_c_exit;
        
        case_c_exit:
        s4 = s4 ^ 0xCCCC;
        s5 = s5 | 0x3333;
        if ((i & 4) == 0) continue;
        goto default_handler;
        
        default_handler:
        s6 = s6 + 999;
        s7 = s7 - 888;
        if ((i & 8) == 0) continue;
    }
    
    checksum = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 +
               (unsigned long)sf0 + (unsigned long)sf1 + (unsigned long)sf2 +
               (unsigned long)sd0 + (unsigned long)sd1 + (unsigned long)sd2 +
               sl0 + sl1 + sl2;
    
    KEEP_ALIVE(s0); KEEP_ALIVE(s1); KEEP_ALIVE(s2); KEEP_ALIVE(s3);
    KEEP_ALIVE(s4); KEEP_ALIVE(s5); KEEP_ALIVE(s6); KEEP_ALIVE(s7);
    KEEP_ALIVE(s8); KEEP_ALIVE(s9);
    
    return checksum;
}

/* Function using computed goto (GCC extension) for state machine */
__attribute__((noinline))
static unsigned long test_computed_goto(int iterations, int seed) {
    volatile int g0 = seed * 101;
    int g1 = seed * 102, g2 = seed * 103, g3 = seed * 104, g4 = seed * 105;
    int g5 = seed * 106, g6 = seed * 107, g7 = seed * 108, g8 = seed * 109, g9 = seed * 110;
    float gf0 = seed * 5.1f, gf1 = seed * 5.2f;
    double gd0 = seed * 6.1, gd1 = seed * 6.2;
    long gl0 = seed * 200, gl1 = seed * 201;
    unsigned long checksum = 0;
    
    /* Labels for computed goto */
    static void* states[] = {
        &&state_0, &&state_1, &&state_2, &&state_3,
        &&state_4, &&state_5, &&state_6, &&state_7
    };
    
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Heavy register usage */
        g0 = g1 + g2 - g3 * g4;
        g1 = g5 ^ g6 | g7 & g8;
        g2 = g9 * g0 / (g1 + 1);
        g3 = g4 << (g5 & 3);
        g4 = g6 >> (g7 % 4);
        
        gf0 = gf1 * 2.0f + (float)g0;
        gf1 = gf0 * 0.5f / (gf1 + 0.001f);
        
        gd0 = gd1 * 1.5 + (double)g1;
        gd1 = gd0 / 1.25 + gd1 * 0.8;
        
        gl0 = gl1 + g2 * g3;
        gl1 = gl0 ^ g4 | g5;
        
        /* Update state based on complex condition */
        state = (state + (i & 0x7) + (g0 & 0x3)) & 0x7;
        
        /* Computed goto - creates very complex CFG */
        goto *states[state];
        
        state_0:
        g5 = g6 + g7;
        g6 = g8 - g9;
        state = (state + 1) & 0x7;
        if ((i & 0xF) == 0) goto *states[3];
        continue;
        
        state_1:
        g7 = g8 * g9;
        g8 = g0 / (g1 + 1);
        state = (state + 2) & 0x7;
        if ((i % 19) == 0) goto *states[0];
        continue;
        
        state_2:
        g9 = g0 ^ g1;
        g0 = g1 | g2;
        state = (state + 3) & 0x7;
        if ((i & 0x1F) == 0) goto *states[5];
        continue;
        
        state_3:
        g1 = g2 & g3;
        g2 = g3 << (g4 & 3);
        state = (state + 4) & 0x7;
        if ((i % 23) == 0) goto *states[2];
        continue;
        
        state_4:
        g3 = g4 >> (g5 % 4);
        g4 = g5 + g6 * g7;
        state = (state + 5) & 0x7;
        if ((i & 0x3F) == 0) goto *states[7];
        continue;
        
        state_5:
        g5 = g6 - g7 / (g8 + 1);
        g6 = g7 ^ g8 & g9;
        state = (state + 6) & 0x7;
        if ((i % 29) == 0) goto *states[4];
        continue;
        
        state_6:
        g7 = g8 | g9;
        g8 = g9 * g0 - g1;
        state = (state + 7) & 0x7;
        if ((i & 0x7F) == 0) goto *states[1];
        continue;
        
        state_7:
        g9 = g0 + g1 * g2;
        g0 = g1 - g2 / (g3 + 1);
        state = (state + 1) & 0x7;
        if ((i % 31) == 0) goto *states[6];
        continue;
    }
    
    checksum = g0 + g1 + g2 + g3 + g4 + g5 + g6 + g7 + g8 + g9 +
               (unsigned long)gf0 + (unsigned long)gf1 +
               (unsigned long)gd0 + (unsigned long)gd1 +
               gl0 + gl1;
    
    KEEP_ALIVE(g0); KEEP_ALIVE(g1); KEEP_ALIVE(g2); KEEP_ALIVE(g3);
    KEEP_ALIVE(g4); KEEP_ALIVE(g5); KEEP_ALIVE(g6); KEEP_ALIVE(g7);
    KEEP_ALIVE(g8); KEEP_ALIVE(g9);
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = MAX_ITER;
    int seed = 42;
    unsigned long total_checksum = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = MAX_ITER;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF test with %d iterations, seed=%d\n", iterations, seed);
    
    /* Run all test functions to maximize coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}
