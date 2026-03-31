/* mcf_coverage.c - Program to trigger MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Global volatile to prevent dead code elimination */
volatile int global_seed = 42;

/* Function 1: Irreducible loop with goto jumping across loop boundaries */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many local variables to increase register pressure */
    int v0 = seed + 0, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3, v4 = seed + 4;
    int v5 = seed + 5, v6 = seed + 6, v7 = seed + 7, v8 = seed + 8, v9 = seed + 9;
    int v10 = seed + 10, v11 = seed + 11, v12 = seed + 12, v13 = seed + 13, v14 = seed + 14;
    int v15 = seed + 15, v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f, f3 = seed * 0.4f, f4 = seed * 0.5f;
    double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03, d3 = seed * 0.04, d4 = seed * 0.05;
    long l0 = seed * 100L, l1 = seed * 200L, l2 = seed * 300L, l3 = seed * 400L, l4 = seed * 500L;
    
    volatile int control = seed; /* Prevent loop unrolling */
    unsigned long checksum = 0;
    
    int i = 0;
outer_loop:
    if (i >= iterations) goto end_func1;
    
    /* Complex arithmetic creating long dependency chains */
    v0 = v1 + v2; v1 = v3 * v4; v2 = v5 - v6; v3 = v7 / (v8 ? v8 : 1); v4 = v9 ^ v10;
    v5 = v11 | v12; v6 = v13 & v14; v7 = v15 << 2; v8 = v16 >> 1; v9 = v17 % (v18 ? v18 : 1);
    v10 = v19 ^ v0; v11 = v1 + v2; v12 = v3 * v4; v13 = v5 - v6; v14 = v7 / (v8 ? v8 : 1);
    v15 = v9 ^ v10; v16 = v11 | v12; v17 = v13 & v14; v18 = v15 << 3; v19 = v16 >> 2;
    
    f0 = f1 + f2; f1 = f3 * f4; f2 = f0 - f1; f3 = f2 * f4; f4 = f3 / (f0 ? f0 : 1.0f);
    d0 = d1 + d2; d1 = d3 * d4; d2 = d0 - d1; d3 = d2 * d4; d4 = d3 / (d0 ? d0 : 1.0);
    l0 = l1 + l2; l1 = l3 * l4; l2 = l0 - l1; l3 = l2 * l4; l4 = l3 / (l0 ? l0 : 1L);
    
    /* Irreducible control flow: goto jumps into middle of inner loop */
    if (control++ % 7 == 0) goto inner_middle;
    
inner_start:
    for (int j = 0; j < 5; j++) {
        v0 += j; v1 -= j; v2 *= (j + 1); v3 /= (j ? j : 1);
        if (j == 2 && (control % 11 == 0)) goto outer_loop; /* Jump to outer loop */
    }
    goto inner_end;
    
inner_middle:
    v0 = v0 * 3; v1 = v1 / 2; v2 = v2 + 100; v3 = v3 - 50;
    goto inner_start; /* Jump back to loop start */
    
inner_end:
    /* More arithmetic to keep variables live */
    v4 = v5 + v6; v5 = v7 * v8; v6 = v9 - v10; v7 = v11 / (v12 ? v12 : 1);
    f0 = f1 * 1.5f; f1 = f2 + 2.0f; f2 = f3 - 0.5f; f3 = f4 * 3.0f;
    d0 = d1 * 1.7; d1 = d2 + 3.1; d2 = d3 - 1.2; d3 = d4 * 2.5;
    
    i++;
    if (control % 13 == 0) goto outer_loop; /* Another jump to outer loop */
    goto outer_loop;
    
end_func1:
    /* Aggregate checksum from all variables */
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
               (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4 +
               (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4 +
               l0 + l1 + l2 + l3 + l4;
    
    /* Mark variables as used to prevent optimization */
    KEEP_ALIVE(v0); KEEP_ALIVE(v1); KEEP_ALIVE(v2); KEEP_ALIVE(v3); KEEP_ALIVE(v4);
    KEEP_ALIVE(v5); KEEP_ALIVE(v6); KEEP_ALIVE(v7); KEEP_ALIVE(v8); KEEP_ALIVE(v9);
    KEEP_ALIVE(v10); KEEP_ALIVE(v11); KEEP_ALIVE(v12); KEEP_ALIVE(v13); KEEP_ALIVE(v14);
    KEEP_ALIVE(v15); KEEP_ALIVE(v16); KEEP_ALIVE(v17); KEEP_ALIVE(v18); KEEP_ALIVE(v19);
    KEEP_ALIVE(f0); KEEP_ALIVE(f1); KEEP_ALIVE(f2); KEEP_ALIVE(f3); KEEP_ALIVE(f4);
    KEEP_ALIVE(d0); KEEP_ALIVE(d1); KEEP_ALIVE(d2); KEEP_ALIVE(d3); KEEP_ALIVE(d4);
    KEEP_ALIVE(l0); KEEP_ALIVE(l1); KEEP_ALIVE(l2); KEEP_ALIVE(l3); KEEP_ALIVE(l4);
    
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    int v0 = seed, v1 = seed * 2, v2 = seed * 3, v3 = seed * 4, v4 = seed * 5;
    int v5 = seed * 6, v6 = seed * 7, v7 = seed * 8, v8 = seed * 9, v9 = seed * 10;
    float f0 = seed * 0.11f, f1 = seed * 0.22f, f2 = seed * 0.33f, f3 = seed * 0.44f;
    double d0 = seed * 0.011, d1 = seed * 0.022, d2 = seed * 0.033, d3 = seed * 0.044;
    long l0 = seed * 111L, l1 = seed * 222L, l2 = seed * 333L, l3 = seed * 444L;
    
    volatile int counter = 0;
    unsigned long checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Heavy arithmetic before switch */
        v0 = v1 + v2; v1 = v3 * v4; v2 = v5 - v6; v3 = v7 / (v8 ? v8 : 1); v4 = v9 ^ v0;
        v5 = v0 | v1; v6 = v2 & v3; v7 = v4 << 1; v8 = v5 >> 2; v9 = v6 % (v7 ? v7 : 1);
        f0 = f1 + f2; f1 = f3 * f0; f2 = f1 - f0; f3 = f2 * 1.5f;
        d0 = d1 + d2; d1 = d3 * d0; d2 = d1 - d0; d3 = d2 * 1.7;
        l0 = l1 + l2; l1 = l3 * l0; l2 = l1 - l0; l3 = l2 * 2;
        
        /* Switch with goto to different labels */
        switch (counter++ % 8) {
            case 0:
                v0 += 10;
                goto label_a;
            case 1:
                v1 -= 5;
                goto label_b;
            case 2:
                v2 *= 3;
                goto label_c;
            case 3:
                v3 /= 2;
                goto label_d;
            case 4:
                v4 ^= 0xFF;
                goto label_a;
            case 5:
                v5 |= 0xAA;
                goto label_b;
            case 6:
                v6 &= 0x55;
                goto label_c;
            case 7:
                v7 <<= 2;
                goto label_d;
        }
        
    label_a:
        v8 = v9 + v0;
        if (i % 3 == 0) goto label_d;
        
    label_b:
        v9 = v0 * v1;
        f0 = f1 + 1.0f;
        if (i % 5 == 0) goto label_a;
        
    label_c:
        f1 = f2 * 2.0f;
        d0 = d1 + 0.5;
        if (i % 7 == 0) goto label_b;
        
    label_d:
        d1 = d2 * 1.5;
        l0 = l1 + 100;
        /* Fall through to continue loop */
    }
    
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               (int)f0 + (int)f1 + (int)f2 + (int)f3 +
               (int)d0 + (int)d1 + (int)d2 + (int)d3 +
               l0 + l1 + l2 + l3;
    
    KEEP_ALIVE(v0); KEEP_ALIVE(v1); KEEP_ALIVE(v2); KEEP_ALIVE(v3); KEEP_ALIVE(v4);
    KEEP_ALIVE(v5); KEEP_ALIVE(v6); KEEP_ALIVE(v7); KEEP_ALIVE(v8); KEEP_ALIVE(v9);
    KEEP_ALIVE(f0); KEEP_ALIVE(f1); KEEP_ALIVE(f2); KEEP_ALIVE(f3);
    KEEP_ALIVE(d0); KEEP_ALIVE(d1); KEEP_ALIVE(d2); KEEP_ALIVE(d3);
    KEEP_ALIVE(l0); KEEP_ALIVE(l1); KEEP_ALIVE(l2); KEEP_ALIVE(l3);
    
    return checksum;
}

/* Function 3: Computed goto simulating state machine */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    int v[MANY_VARS];
    float f[MANY_VARS/2];
    double d[MANY_VARS/2];
    
    /* Initialize arrays */
    for (int i = 0; i < MANY_VARS; i++) {
        v[i] = seed + i;
    }
    for (int i = 0; i < MANY_VARS/2; i++) {
        f[i] = seed * (i + 1) * 0.1f;
        d[i] = seed * (i + 1) * 0.01;
    }
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, 
        &&state4, &&state5, &&state6, &&state7
    };
    
    int state = 0;
    volatile int mod = seed;
    unsigned long checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Heavy arithmetic mixing all variables */
        for (int j = 0; j < MANY_VARS - 1; j++) {
            v[j] = v[j+1] + v[j] * (mod % 5 + 1);
            mod = (mod * 1103515245 + 12345) & 0x7fffffff;
        }
        
        for (int j = 0; j < MANY_VARS/2 - 1; j++) {
            f[j] = f[j+1] + f[j] * ((mod % 10) * 0.1f);
            d[j] = d[j+1] + d[j] * ((mod % 10) * 0.01);
            mod = (mod * 1103515245 + 12345) & 0x7fffffff;
        }
        
        /* Computed goto based on state */
        goto *labels[state];
        
    state0:
        v[0] = v[1] + v[2];
        state = (mod % 7) + 1;
        mod = (mod * 1103515245 + 12345) & 0x7fffffff;
        continue;
        
    state1:
        v[3] = v[4] * v[5];
        f[0] = f[1] + 1.0f;
        state = (mod % 6) + 2;
        mod = (mod * 1103515245 + 12345) & 0x7fffffff;
        continue;
        
    state2:
        v[6] = v[7] - v[8];
        d[0] = d[1] * 2.0;
        state = (mod % 5) + 3;
        mod = (mod * 1103515245 + 12345) & 0x7fffffff;
        continue;
        
    state3:
        v[9] = v[10] / (v[11] ? v[11] : 1);
        f[2] = f[3] - 0.5f;
        state = (mod % 4) + 4;
        mod = (mod * 1103515245 + 12345) & 0x7fffffff;
        continue;
        
    state4:
        v[12] = v[13] ^ v[14];
        d[2] = d[3] / (d[4] ? d[4] : 1.0);
        state = (mod % 3) + 5;
        mod = (mod * 1103515245 + 12345) & 0x7fffffff;
        continue;
        
    state5:
        v[15] = v[16] | v[17];
        f[4] = f[5] * 3.0f;
        state = (mod % 2) + 6;
        mod = (mod * 1103515245 + 12345) & 0x7fffffff;
        continue;
        
    state6:
        v[18] = v[19] & v[20];
        d[4] = d[5] + 1.5;
        state = (mod % 1) + 7;
        mod = (mod * 1103515245 + 12345) & 0x7fffffff;
        continue;
        
    state7:
        v[21] = v[22] << (mod % 4);
        v[23] = v[24] >> (mod % 4);
        state = 0;
        mod = (mod * 1103515245 + 12345) & 0x7fffffff;
        continue;
    }
    
    /* Compute checksum */
    for (int i = 0; i < MANY_VARS; i++) {
        checksum += v[i];
    }
    for (int i = 0; i < MANY_VARS/2; i++) {
        checksum += (int)f[i] + (int)d[i];
    }
    
    /* Keep variables alive */
    for (int i = 0; i < MANY_VARS; i++) {
        KEEP_ALIVE(v[i]);
    }
    for (int i = 0; i < MANY_VARS/2; i++) {
        KEEP_ALIVE(f[i]);
        KEEP_ALIVE(d[i]);
    }
    
    return checksum;
}

/* Main function */
int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = global_seed;
    
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
    
    /* Call all test functions to trigger different CFG patterns */
    unsigned long sum1 = test_irreducible_goto(iterations, seed);
    unsigned long sum2 = test_switch_goto(iterations, seed + 1);
    unsigned long sum3 = test_computed_goto(iterations / 10, seed + 2);
    
    /* Final checksum to prevent dead code elimination */
    unsigned long total = sum1 + sum2 + sum3;
    printf("Checksum: %lu\n", total);
    
    return (int)(total % 256);
}
