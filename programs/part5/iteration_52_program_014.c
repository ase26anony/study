/* early-remat-test.c
 * Designed to trigger virtual register creation in GCC's early-remat pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-gcse early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* Complex arithmetic with long dependency chain */
static int complex_arithmetic_chain(int a, int b, int c, int d, int e, int f, int g) {
    /* Non-trivial arithmetic creating many temporaries */
    int t1 = a * b;
    int t2 = c + d;
    int t3 = e - f;
    int t4 = g % (a + 1);
    int t5 = t1 * t2;
    int t6 = t3 / (t4 + 1);
    int t7 = t5 + t6;
    int t8 = t7 - (a * c);
    int t9 = t8 % (b + d);
    int t10 = t9 * (e - g);
    
    /* Use volatile variables in computation */
    t10 += vol_a * vol_b;
    t10 -= vol_c / (vol_d + 1);
    
    return t10;
}

/* Floating point stress with multiple uses of temporaries */
static float fp_stress(float a, float b, float c, float d, float e, float f) {
    /* Complex floating expressions */
    float t1 = a * b + c / d;
    float t2 = e - f * a;
    float t3 = b / c + d * e;
    float t4 = t1 * t2 - t3;
    
    /* Multi-use temporary in different contexts */
    float base = t4 * vol_f1 + vol_f2 / vol_f3;
    
    /* Use base with different offsets in separate basic blocks */
    if (base > 0) {
        float r1 = base + 1.0f;
        float r2 = base * 2.0f;
        float r3 = base / 3.0f;
        return r1 + r2 - r3;
    } else {
        float r1 = base - 1.0f;
        float r2 = base / 2.0f;
        float r3 = base * 3.0f;
        return r1 - r2 + r3;
    }
}

/* Function with inline assembly clobbering registers */
static int asm_clobber_stress(int x, int y) {
    int result;
    
    /* Complex computation before asm */
    int pre = x * y + (x % (y + 1)) - (y / (x + 1));
    
    /* Inline assembly that clobbers multiple registers */
    __asm__ volatile (
        "mov %1, %%eax\n\t"
        "mov %2, %%ebx\n\t"
        "add %%ebx, %%eax\n\t"
        "imul %%eax, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r" (result)
        : "r" (pre), "r" (y)
        : "%eax", "%ebx", "memory"
    );
    
    /* More computation after asm to force register reloads */
    result += complex_arithmetic_chain(x, y, pre, result, vol_a, vol_b, vol_c);
    
    return result;
}

/* Main stress function with control flow and register pressure */
static int stress_computation(int seed, int iterations) {
    int total = 0;
    volatile int vol_counter = seed;  /* Prevent loop optimizations */
    
    /* Array for address computation patterns */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * seed;
    }
    
    /* Loop with carried dependencies and volatile */
    for (int i = 0; i < iterations; i++) {
        /* Opaque function call creates unanalyzable values */
        int opaque = get_external_value() + i;
        
        /* Complex arithmetic chain */
        int val1 = complex_arithmetic_chain(
            opaque, vol_counter, 
            array[i % 256], array[(i + 1) % 256],
            vol_a, vol_b, vol_c
        );
        
        /* Address computation with multiple offsets */
        int* base_ptr = &array[i % 256];
        
        /* Use base pointer with different offsets - candidate for remat */
        int use1 = base_ptr[0] + val1;
        int use2 = base_ptr[1] * val1;
        int use3 = base_ptr[2] - val1;
        int use4 = base_ptr[3] / (val1 + 1);
        
        /* Multi-use temporary in different control paths */
        int multi_use_temp = use1 * use2 - use3 + use4;
        
        if (opaque % 3 == 0) {
            /* Path 1: Multiple uses of temporary */
            total += multi_use_temp * 2;
            total -= multi_use_temp / 3;
        } else if (opaque % 3 == 1) {
            /* Path 2: Different uses */
            total += multi_use_temp + 100;
            total *= (multi_use_temp % 7) + 1;
        } else {
            /* Path 3: Yet another pattern */
            total += asm_clobber_stress(multi_use_temp, opaque);
        }
        
        /* Floating point stress in some iterations */
        if (i % 5 == 0) {
            float fp_val = fp_stress(
                vol_f1 + i, vol_f2 - i, 
                vol_f3 * i, (float)opaque,
                (float)val1, (float)total
            );
            total += (int)fp_val;
        }
        
        /* Update volatile to prevent optimization */
        vol_counter++;
    }
    
    return total;
}

/* Mock external function */
int get_external_value(void) {
    static int counter = 0;
    return counter++ * 1103515245 + 12345;  /* Simple LCG */
}

/* Secondary stress function for multiple call sites */
static int alternate_stress(int base) {
    /* Different pattern to create varied RTL */
    int acc = base;
    
    for (int i = 0; i < 50; i++) {
        /* Nested complex expressions */
        acc = acc * (vol_a + i) - (vol_b % (i + 1)) + (vol_c / (acc + 1));
        
        /* More inline asm with clobbers */
        int temp;
        __asm__ volatile (
            "mov %1, %%ecx\n\t"
            "lea (%%ecx, %%ecx, 2), %%edx\n\t"
            "mov %%edx, %0\n\t"
            : "=r" (temp)
            : "r" (acc)
            : "%ecx", "%edx"
        );
        
        acc = temp + get_external_value();
    }
    
    return acc;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = time(NULL);
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    srand(seed);
    
    /* Initialize volatile variables with some variation */
    vol_a = rand() % 100 + 1;
    vol_b = rand() % 100 + 1;
    vol_c = rand() % 100 + 1;
    vol_d = rand() % 100 + 1;
    
    /* Call stress functions from multiple sites */
    int result1 = stress_computation(seed, iterations);
    int result2 = alternate_stress(result1);
    int result3 = stress_computation(result2, iterations / 2);
    
    /* Use results to prevent elimination */
    int final_result = result1 + result2 - result3;
    
    printf("Result: %d (seed: %d, iterations: %d)\n", 
           final_result, seed, iterations);
    
    return final_result != 0 ? 0 : 1;
}
