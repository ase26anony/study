/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __OPTIMIZE__
#define USE_AGGRESSIVE_PATTERNS 1
#else
#define USE_AGGRESSIVE_PATTERNS 0
#endif

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* Complex arithmetic with long dependency chain */
static int complex_arithmetic(int a, int b, int c, int d, int e, int f, int g) {
    /* Multi-use temporary value - candidate for rematerialization */
    int base = a * b + c - d;
    
    /* Complex expression creating many temporaries */
    int result = (base * c + d / (e ? e : 1) - f % (g ? g : 1)) 
                 * (a + b - c + d - e + f - g);
    
    /* Use base in different context */
    if (result > 1000) {
        result += base * 2;
    } else {
        result -= base / 2;
    }
    
    return result;
}

/* Floating point stress with volatile accesses */
static float fp_stress(float f1, float f2, float f3, float f4, float f5) {
    /* Force many FP temporaries */
    float t1 = vol_f1 * f1 + vol_f2 * f2;
    float t2 = f3 / (vol_f3 + 0.1f) - f4 * 0.5f;
    float t3 = t1 * t2 + f5 / (t1 ? t1 : 1.0f);
    float t4 = t3 - t2 + t1 * vol_f1;
    
    /* Multi-use of t3 in different expressions */
    float result = t4 * 2.0f + t3 / 3.0f;
    
    /* Another use of t3 with different computation */
    if (result > 0.0f) {
        result += t3 * 0.25f;
    } else {
        result -= t3 * 0.75f;
    }
    
    return result;
}

/* Function with inline assembly clobbering registers */
static int asm_clobber_stress(int x, int y, int z) {
    int result = x;
    
#if USE_AGGRESSIVE_PATTERNS
    /* Clobber multiple registers to increase pressure */
    asm volatile (
        "mov %0, %0\n\t"
        :
        : "r"(result)
        : "r0", "r1", "r2", "r3", "memory"
    );
#endif
    
    /* Complex computation after clobber */
    result = (result * y + z / (x ? x : 1)) * (y - z + x % 7);
    
    return result;
}

/* Address computation with multiple offsets */
static int address_computation(int *array, int index, int n) {
    /* Base address computation - candidate for rematerialization */
    int *base = &array[index];
    
    int sum = 0;
    
    /* Use base with different offsets */
    sum += base[0] * 2;      /* offset 0 */
    sum += base[1] * 3;      /* offset 1 */
    sum += base[-1] * 4;     /* offset -1 */
    sum += base[2] * 5;      /* offset 2 */
    sum += base[-2] * 6;     /* offset -2 */
    
    /* Another use of base in different computation */
    if (n > 0) {
        sum += base[n % 5] * 7;
    }
    
    return sum;
}

/* Main stress function with control flow */
int stress_computation(int seed, int n) {
    int result = seed;
    volatile int vol_counter = 0;  /* Prevent loop optimizations */
    
    /* Opaque function call - compiler can't analyze value */
    int opaque = rand() % 100;
    
    /* Loop with carried dependency and volatile */
    for (vol_counter = 0; vol_counter < n; vol_counter++) {
        /* Complex arithmetic with opaque value */
        int a = result + opaque + vol_a;
        int b = vol_b * vol_counter + vol_c;
        int c = vol_d - result;
        
        /* Multi-branch control flow */
        if (vol_counter % 3 == 0) {
            result += complex_arithmetic(a, b, c, vol_a, vol_b, vol_c, vol_d);
        } else if (vol_counter % 3 == 1) {
            /* Use same values in different pattern */
            float fp_val = fp_stress(a * 0.1f, b * 0.2f, c * 0.3f, 
                                    result * 0.4f, opaque * 0.5f);
            result += (int)fp_val;
        } else {
            result += asm_clobber_stress(a, b, c);
        }
        
        /* Prevent tail recursion optimization */
        opaque = (opaque * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Array access pattern */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i + result + opaque;
    }
    
    /* Address computation stress */
    result += address_computation(array, 50, n);
    
    return result;
}

/* Secondary stress function with different pattern */
int stress_computation2(int seed, int n) {
    int result = seed;
    
    /* Different arithmetic pattern */
    for (int i = 0; i < n; i++) {
        /* Long chain of dependent computations */
        int t1 = result * 3 + i;
        int t2 = t1 / (vol_a + 1) - vol_b;
        int t3 = t2 * vol_c + vol_d % (t1 ? t1 : 1);
        int t4 = t3 - t2 + t1 * i;
        
        /* Multi-use of t2 in spatially separated contexts */
        if (i % 2 == 0) {
            result += t2 * 2 + t4;
        } else {
            result -= t2 / 2 + t3;
        }
        
        /* Another computation using t2 */
        result += (t2 % 17) * 3;
        
        /* Volatile update to prevent optimization */
        vol_a = (vol_a + 1) & 0xFF;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = 42;
    
    /* Use command line or default */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 1000) iterations = 1000;
    }
    
    srand(seed);
    
    /* Initialize volatile variables */
    vol_a = rand() % 100 + 1;
    vol_b = rand() % 100 + 1;
    vol_c = rand() % 100 + 1;
    vol_d = rand() % 100 + 1;
    
    /* Call stress functions multiple times */
    int result1 = 0, result2 = 0;
    
    for (int i = 0; i < 10; i++) {
        result1 += stress_computation(seed + i, iterations);
        result2 += stress_computation2(seed - i, iterations / 2);
        
        /* Modify volatiles between calls */
        vol_a = (vol_a * 13 + 7) % 256;
        vol_b = (vol_b * 17 + 11) % 256;
    }
    
    /* Final computation mixing both results */
    int final_result = result1 * 3 - result2 * 2;
    
    /* Use result to prevent elimination */
    printf("Result: %d (seed=%d, iterations=%d)\n", 
           final_result, seed, iterations);
    
    return final_result != 0 ? 0 : 1;
}
