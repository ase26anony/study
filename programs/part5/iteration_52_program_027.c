/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* Complex arithmetic stress function with register pressure */
int stress_computation(int seed, int n) {
    int result = 0;
    
    /* Multi-use temporary value - candidate for rematerialization */
    int base_computation = seed * vol_a + vol_b / (vol_c + 1);
    
    /* Complex floating-point chain creating many virtual registers */
    float fp_chain = vol_f1;
    for (int i = 0; i < n; i++) {
        /* Long dependency chain with multiple temporaries */
        fp_chain = fp_chain * vol_f2 + vol_f3 / (fp_chain + 1.0f) - 
                   (float)(i % (vol_a + 1)) * 0.5f;
        
        /* Inline assembly clobbering multiple registers */
        /* This reduces available physical registers */
        asm volatile (
            "# Clobber hard registers to increase pressure"
            :
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
        
        /* Multi-use of base_computation in different contexts */
        /* This creates patterns where RA might want to rematerialize */
        int use1 = base_computation + i * vol_d;
        int use2 = base_computation - i / (vol_b + 1);
        int use3 = base_computation * (i % (vol_c + 2));
        
        /* Address computation with multiple offsets */
        /* Base address that might be rematerialized */
        char buffer[256];
        char *base_addr = &buffer[i % 128];
        
        /* Multiple uses with different offsets */
        char val1 = *(base_addr + (use1 % 64));
        char val2 = *(base_addr + (use2 % 32));
        char val3 = *(base_addr + (use3 % 16));
        
        result += val1 + val2 + val3 + (int)fp_chain;
        
        /* Opaque function call creating unanalyzable values */
        if (i % 7 == 0) {
            int opaque = get_external_value();
            result += opaque * (vol_a + vol_b + vol_c);
        }
    }
    
    /* Switch statement spreading base_computation uses */
    /* This creates spatial separation encouraging rematerialization */
    switch (seed % 4) {
        case 0:
            result += base_computation * 2;
            break;
        case 1:
            result += base_computation / 2;
            break;
        case 2:
            result += base_computation + vol_d;
            break;
        case 3:
            result += base_computation - vol_a;
            break;
    }
    
    return result;
}

/* Second stress function with different patterns */
int stress_computation2(int iterations) {
    volatile int counter = iterations;
    int total = 0;
    
    while (counter > 0) {
        /* Complex integer arithmetic chain */
        int a = rand() % 100 + 1;
        int b = rand() % 100 + 1;
        int c = rand() % 100 + 1;
        int d = rand() % 100 + 1;
        int e = rand() % 100 + 1;
        int f = rand() % 100 + 1;
        
        /* Non-trivial expression creating many temporaries */
        int complex_expr = a * b + c / d - e % f;
        complex_expr = complex_expr * (a + b) - (c * d) / (e + 1);
        complex_expr = complex_expr + (a % b) * (c % d) - (e % f);
        
        /* Multi-use value in different computation contexts */
        int multi_use = complex_expr * 2;
        total += multi_use + (multi_use / 3) - (multi_use % 5);
        
        /* Another inline assembly clobber */
        asm volatile (
            "# More register clobbering"
            :
            :
            : "memory", "r0", "r1", "r2", "r3", "r4"
        );
        
        counter--;
    }
    
    return total;
}

/* Main test harness */
int main(int argc, char *argv[]) {
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    srand(time(NULL));
    
    /* Call stress functions multiple times */
    int total_result = 0;
    
    for (int i = 0; i < 5; i++) {
        /* Varying inputs to create different compilation patterns */
        int result1 = stress_computation(rand() % 100, iterations / (i + 1) + 10);
        int result2 = stress_computation2(iterations / ((i % 3) + 2) + 5);
        
        total_result += result1 + result2;
        
        /* Prevent loop unrolling from eliminating register pressure */
        volatile int barrier = i;
        (void)barrier;
    }
    
    printf("Final result: %d\n", total_result);
    
    /* Additional test with array addressing patterns */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * i;
    }
    
    /* Multiple base address computations with offsets */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Base address that might be rematerialized */
        int *base = &array[i % 128];
        
        /* Multiple offset uses - potential rematerialization candidates */
        sum += base[0] + base[1] + base[2] + base[3] + base[4];
        sum += base[5] + base[6] + base[7] + base[8] + base[9];
    }
    
    printf("Array sum: %d\n", sum);
    
    return total_result > 0 ? 0 : 1;
}

/* Dummy external function implementation */
int get_external_value(void) {
    static int counter = 0;
    return rand() % 1000 + (counter++ % 7);
}
