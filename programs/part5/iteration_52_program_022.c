/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* Complex arithmetic with volatile variables - creates many temporaries */
static int complex_volatile_chain(int seed) {
    /* Force many virtual registers with complex expression */
    int t1 = vol_a * vol_b + seed;
    int t2 = vol_c / vol_a - vol_d % (vol_b + 1);
    int t3 = t1 * t2 - vol_a + vol_c % (vol_b | 1);
    
    /* Multi-use temporary value - candidate for rematerialization */
    int base = t3 * vol_a + vol_b - vol_c;
    
    /* Use base in multiple separated contexts */
    int result = 0;
    if (seed % 2) {
        result += base * 2;
    } else {
        result += base / 2;
    }
    
    /* Another use of base with different computation */
    result += (base + vol_a) * (base - vol_b);
    
    return result;
}

/* Floating point stress with inline assembly clobbers */
static float fp_stress_with_clobbers(float init) {
    float a = init + vol_f1;
    float b = vol_f2 * a - vol_f3;
    
    /* Inline assembly that clobbers multiple hard registers */
    /* This reduces available physical registers */
    asm volatile (
        "# Clobber hard registers to increase pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        :
        : "r" ((int)a), "r" ((int)b)
        : "r0", "r1", "r2", "r3", "memory"
    );
    
    /* Complex FP chain that needs many temporaries */
    float c = b * vol_f1 / vol_f2 + a;
    float d = c - vol_f3 * b + a / (vol_f1 + 0.1f);
    float e = d * c + b / (a + 0.01f) - vol_f2;
    
    /* Multi-use temporary */
    float fp_base = e * a + b - c * d;
    
    /* Use in multiple offset computations */
    float r1 = fp_base * 1.5f;
    float r2 = fp_base * 2.5f;
    float r3 = fp_base * 3.5f;
    
    return r1 + r2 - r3;
}

/* Loop with volatile dependencies and opaque calls */
static int loop_stress(int iterations) {
    int sum = 0;
    volatile int vol_counter = iterations;
    
    for (volatile int i = 0; i < vol_counter; i = i + 1) {
        /* Opaque function call creates unanalyzable values */
        int opaque = get_external_value();
        
        /* Complex expression with loop-carried dependency */
        int temp = (opaque * vol_a + i) % (vol_b + 1);
        temp = temp * vol_c - opaque / (vol_d | 1);
        
        /* Multi-use base computation */
        int loop_base = temp + i * vol_a - opaque;
        
        /* Use base with different offsets in control flow */
        if (i % 3 == 0) {
            sum += loop_base + 10;
        } else if (i % 3 == 1) {
            sum += loop_base * 2 - 5;
        } else {
            sum += loop_base / 2 + 20;
        }
        
        /* Another use of related computation */
        sum += (loop_base + opaque) * (i % 7);
    }
    
    return sum;
}

/* Address computation with multiple offsets */
static int address_computation_stress(int *array, int size) {
    int total = 0;
    
    for (int i = 0; i < size - 4; i++) {
        /* Compute base address - candidate for rematerialization */
        int *base = &array[i];
        
        /* Use base with multiple different offsets */
        int val1 = base[0] * vol_a;
        int val2 = base[1] + vol_b;
        int val3 = base[2] - vol_c;
        int val4 = base[3] / (vol_d | 1);
        
        /* Complex expression using all values */
        total += (val1 * val2) + (val3 - val4) * (i % 5);
        
        /* Force another computation with base */
        total += base[0] + base[1] - base[2] + base[3];
    }
    
    return total;
}

/* Main stress function combining all patterns */
int stress_computation(int seed, int n) {
    int result = 0;
    
    /* Mix different stress patterns */
    result += complex_volatile_chain(seed);
    
    float fp_res = fp_stress_with_clobbers(seed * 0.1f);
    result += (int)fp_res;
    
    result += loop_stress(n % 10 + 5);
    
    /* Create array for address computation */
    int arr[50];
    for (int i = 0; i < 50; i++) {
        arr[i] = (i * seed + vol_a) % 100;
    }
    
    result += address_computation_stress(arr, 50);
    
    /* Final complex expression to use all intermediate results */
    result = (result * vol_a + seed) % (vol_b * 1000);
    
    return result;
}

/* Mock external function */
int get_external_value(void) {
    static int counter = 0;
    return (counter++ * 1103515245 + 12345) & 0x7fffffff;
}

int main(int argc, char **argv) {
    int seed = 42;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    srand(seed);
    
    /* Initialize volatile variables with random-ish values */
    vol_a = (rand() % 100) + 1;
    vol_b = (rand() % 100) + 1;
    vol_c = (rand() % 100) + 1;
    vol_d = (rand() % 100) + 1;
    
    vol_f1 = (rand() % 100) / 10.0f + 0.5f;
    vol_f2 = (rand() % 100) / 10.0f + 0.5f;
    vol_f3 = (rand() % 100) / 10.0f + 0.5f;
    
    int total = 0;
    
    /* Call stress function multiple times from different contexts */
    for (int i = 0; i < 10; i++) {
        int res = stress_computation(seed + i, i * 5);
        total += res;
        
        /* Vary the call pattern */
        if (i % 2 == 0) {
            res = stress_computation(seed - i, i * 3);
            total -= res / 2;
        } else {
            res = stress_computation(seed * i, i * 7);
            total += res * 2;
        }
    }
    
    printf("Result: %d\n", total);
    
    return 0;
}
