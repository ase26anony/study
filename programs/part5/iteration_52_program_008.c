/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Volatile variables to prevent optimization */
volatile int vol_a = 1;
volatile int vol_b = 2;
volatile int vol_c = 3;
volatile int vol_d = 4;
volatile int vol_e = 5;
volatile int vol_f = 6;
volatile int vol_g = 7;

/* Complex arithmetic with volatile variables and long dependency chain */
int complex_volatile_chain(int seed) {
    /* Force many virtual registers with complex expression */
    int t1 = vol_a * vol_b + seed;
    int t2 = vol_c / vol_d - t1;
    int t3 = vol_e % vol_f * t2;
    int t4 = t1 + t2 * t3 - vol_g;
    int t5 = t4 % vol_a + vol_b / vol_c;
    int t6 = t3 * t5 - t2 + t1 % vol_d;
    
    /* Multi-use temporary value */
    int base = t6 * seed - vol_e;
    
    /* Use base in multiple, spatially separated contexts */
    int result = 0;
    if (seed % 3 == 0) {
        result = base + vol_a * 2;  /* First use */
    } else if (seed % 3 == 1) {
        result = base - vol_b / 2;  /* Second use */
    } else {
        result = base * 3 + vol_c;  /* Third use */
    }
    
    /* Another complex chain */
    int u1 = result * vol_d + vol_e % vol_f;
    int u2 = u1 / vol_g - vol_a * vol_b;
    int u3 = u2 % vol_c + vol_d - vol_e;
    
    return u3 + base;  /* Use base again */
}

/* Function with inline assembly clobbering registers */
int asm_clobber_stress(int x, int y) {
    int result;
    
    /* Complex computation before asm */
    int a = x * y + vol_a;
    int b = a / vol_b - y % vol_c;
    int c = b * vol_d + x % vol_e;
    
    /* Inline assembly that clobbers multiple hard registers */
    /* This increases register pressure */
#ifdef __OPTIMIZE__
    asm volatile (
        "mov %0, %1\n\t"
        "add %0, %2\n\t"
        : "=r" (result)
        : "r" (c), "r" (a)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );
#else
    result = c + a;  /* Fallback without optimization */
#endif
    
    /* More computations after asm to force register allocation */
    int d = result * vol_f - vol_g;
    int e = d / vol_a + result % vol_b;
    int f = e * vol_c - d / vol_d;
    
    return f;
}

/* Address computation with multiple offsets */
int address_computation_stress(int *array, int index, int n) {
    /* Compute base address (forces address register) */
    int *base_ptr = &array[index];
    
    /* Use base with multiple offsets - candidate for rematerialization */
    int sum = 0;
    sum += base_ptr[0] * 2;    /* First offset */
    sum += base_ptr[1] - 3;    /* Second offset */
    sum += base_ptr[2] / 4;    /* Third offset */
    sum += base_ptr[3] + 5;    /* Fourth offset */
    sum += base_ptr[4] % 6;    /* Fifth offset */
    
    /* Complex loop with volatile to prevent optimization */
    volatile int vol_i = 0;
    for (vol_i = 0; vol_i < n; vol_i++) {
        /* Loop-carried dependency with volatile */
        sum += base_ptr[vol_i % 5] * (vol_i + 1);
        
        /* More complex arithmetic inside loop */
        int temp = sum * vol_a - vol_b;
        sum = temp % vol_c + vol_d;
    }
    
    return sum;
}

/* Main stress function combining all patterns */
int stress_computation(int seed, int n) {
    int result = 0;
    
    /* Use external function for opaque values */
    int opaque = get_external_value() + seed;
    
    /* Complex expression chain with opaque values */
    int chain1 = complex_volatile_chain(opaque);
    int chain2 = asm_clobber_stress(chain1, opaque);
    
    /* Create array for address computation */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = (i * opaque) % 97;
    }
    
    /* Multiple calls with different parameters */
    for (int i = 0; i < n; i++) {
        /* Vary the index to create different patterns */
        int idx = (opaque + i * 7) % 90;
        
        /* Mix different computation patterns */
        if (i % 3 == 0) {
            result += complex_volatile_chain(idx);
        } else if (i % 3 == 1) {
            result += asm_clobber_stress(idx, result);
        } else {
            result += address_computation_stress(array, idx, 5);
        }
        
        /* Prevent loop unrolling with volatile */
        volatile int barrier = i;
        (void)barrier;
    }
    
    return result;
}

/* External function implementation */
int get_external_value(void) {
    static int counter = 0;
    return rand() % 100 + counter++;
}

/* Test harness */
int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    /* Initialize with command line or random values */
    int iterations = 10;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    
    /* Multiple calls to stress the register allocator */
    int final_result = 0;
    for (int i = 0; i < iterations; i++) {
        int seed = rand() % 1000;
        int n = 5 + (rand() % 10);
        
        final_result += stress_computation(seed, n);
        
        /* Print progress to prevent optimization */
        if (i % 100 == 0) {
            printf("Iteration %d: %d\n", i, final_result);
        }
    }
    
    printf("Final result: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
