/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Stress function with complex register patterns */
static int stress_computation(int seed, int n) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 1;
    volatile int v4 = seed - 1;
    
    int result = 0;
    
    /* Complex arithmetic chain creating many temporaries */
    for (int i = 0; i < n; i++) {
        /* Opaque function call to prevent constant propagation */
        int opaque = get_external_value() + i;
        
        /* Long dependency chain with volatile accesses */
        int t1 = v1 * opaque + v2 / (opaque + 1);
        int t2 = t1 - v3 % (opaque > 0 ? opaque : 1);
        int t3 = t2 + v4 * (opaque % 7);
        int t4 = t3 * t3 - t1;
        int t5 = t4 + (v1 << 2) - (v2 >> 1);
        
        /* Multi-use temporary value - candidate for rematerialization */
        int base = t5 * opaque - v3;
        
        /* Use base in multiple, spatially separated contexts */
        if (i % 3 == 0) {
            result += base * 2;
        } else if (i % 3 == 1) {
            result += base / 2;
        } else {
            result += base + opaque;
        }
        
        /* Address computation with multiple offsets */
        char buffer[256];
        char *ptr = &buffer[opaque % 128];
        
        /* Multiple uses of ptr with different offsets */
        ptr[0] = (char)(base & 0xFF);
        ptr[1] = (char)((base >> 8) & 0xFF);
        ptr[2] = (char)((base >> 16) & 0xFF);
        ptr[-1] = (char)(opaque & 0xFF);
        
        /* Inline assembly clobbering registers to increase pressure */
        #ifdef __OPTIMIZE__
        asm volatile (
            "# Force register clobbering\n"
            : 
            : "r"(t5), "r"(base)
            : "r0", "r1", "r2", "r3", "memory"
        );
        #endif
        
        /* Prevent loop unrolling with volatile loop counter update */
        v1 += (i % 5);
    }
    
    return result;
}

/* Another stress function with different pattern */
static int stress_address_calc(int iterations) {
    volatile int offset = 1;
    int array[256];
    int sum = 0;
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 256; i++) {
        array[i] = get_external_value() + i;
    }
    
    for (int i = 0; i < iterations; i++) {
        /* Base address computation that might be rematerialized */
        int *base_ptr = &array[offset % 128];
        
        /* Multiple uses with different offsets */
        sum += base_ptr[0];
        sum += base_ptr[1];
        sum += base_ptr[-1];
        sum += base_ptr[2];
        sum += base_ptr[-2];
        
        /* Complex expression using the base pointer */
        int temp = (int)(base_ptr + (i % 16)) - (int)array;
        sum += temp * base_ptr[0];
        
        /* More register pressure */
        volatile int v = i;
        sum += v * (base_ptr[1] - base_ptr[-1]);
        
        /* Update volatile to prevent optimization */
        offset += (i % 7) + 1;
    }
    
    return sum;
}

/* Simulate external function */
int get_external_value(void) {
    static int counter = 0;
    return counter++ % 100;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    int seed = 12345;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 10000) iterations = 10000;
    }
    
    srand(seed);
    
    printf("Starting stress tests...\n");
    
    /* Call stress functions multiple times from different contexts */
    int total = 0;
    
    total += stress_computation(seed, iterations / 2);
    total += stress_address_calc(iterations / 4);
    total += stress_computation(seed * 2, iterations / 3);
    total += stress_address_calc(iterations / 5);
    
    /* Additional calls with different parameters */
    for (int i = 0; i < 5; i++) {
        total += stress_computation(seed + i * 100, 50);
    }
    
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
