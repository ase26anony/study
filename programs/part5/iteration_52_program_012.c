/* Compile with: gcc -O2 -fno-omit-frame-pointer -funroll-loops -fno-gcse */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile float fv1 = 1.1f, fv2 = 2.2f, fv3 = 3.3f;

/* External function to create opaque values */
extern int rand(void);

/* Stress function with complex arithmetic and register pressure */
int stress_computation(int seed, int n) {
    int result = 0;
    
    /* Complex arithmetic chain creating many temporaries */
    int a = rand() % 100 + seed;
    int b = rand() % 100 + v1;
    int c = rand() % 100 + v2;
    int d = rand() % 100 + v3;
    int e = rand() % 100 + v4;
    int f = rand() % 100 + v5;
    
    /* Multi-use temporary value - candidate for rematerialization */
    int base = a * b + c / (d ? d : 1) - e % (f ? f : 1);
    
    /* Inline assembly clobbering registers to increase pressure */
    asm volatile (
        "# Clobber hard registers\n"
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );
    
    /* Complex floating-point expressions creating dependency chains */
    float fa = fv1 * seed;
    float fb = fv2 * a;
    float fc = fv3 * b;
    float fd = (float)c / (d ? d : 1.0f);
    float fe = (float)e * 0.5f;
    float ff = (float)f * 2.0f;
    
    /* Long dependency chain without intermediate stores */
    float fchain = fa * fb + fc / fd - fe * ff + 
                   fa / fb - fc * fd + fe / ff;
    
    /* Use base in multiple, spatially separated contexts */
    for (int i = 0; i < n; i++) {
        /* Different uses of base in control flow */
        if (i % 3 == 0) {
            result += base * i;
        } else if (i % 3 == 1) {
            result -= base / (i ? i : 1);
        } else {
            result ^= base + i;
        }
        
        /* More complex arithmetic with volatile accesses */
        int temp = v1 * i + v2 * (i + 1) - v3 * (i + 2) + 
                   v4 / (i ? i : 1) - v5 % (i ? i : 1);
        
        /* Use fchain in computation */
        result += (int)(fchain * temp) % 256;
        
        /* Address computation with multiple offsets */
        int array[100];
        int *ptr = &array[i % 100];
        
        /* Multiple uses of ptr with different offsets */
        ptr[0] = base + i;
        ptr[1] = base - i;
        ptr[2] = base * i;
        ptr[3] = base ^ i;
        
        result += ptr[0] + ptr[1] - ptr[2] + ptr[3];
    }
    
    /* Another complex expression using all variables */
    int final = (a * b * c) / (d ? d : 1) + 
                (e % f) * (base % 100) - 
                (int)(fchain * 100) % 256;
    
    return result + final;
}

/* Second stress function with different patterns */
int stress_memory_access(int seed, int n) {
    int result = 0;
    int data[256];
    
    /* Initialize with volatile-dependent values */
    for (int i = 0; i < 256; i++) {
        data[i] = (i * v1 + v2 - v3) % 256;
    }
    
    /* Loop with volatile bounds to prevent optimization */
    volatile int vol_bound = n;
    for (int i = 0; i < vol_bound; i++) {
        /* Complex address computation */
        int idx1 = (i * seed + v1) % 256;
        int idx2 = (i * v2 + v3) % 256;
        int idx3 = (i * v4 + v5) % 256;
        
        /* Multi-use base address computation */
        int *base_ptr = &data[idx1];
        
        /* Use base_ptr with multiple offsets */
        int val1 = base_ptr[0] * base_ptr[1];
        int val2 = base_ptr[2] / (base_ptr[3] ? base_ptr[3] : 1);
        int val3 = base_ptr[4] % (base_ptr[5] ? base_ptr[5] : 1);
        
        /* Complex expression chain */
        result += val1 + val2 - val3 + 
                  (idx2 * idx3) / (i ? i : 1) - 
                  (seed * i) % 256;
        
        /* Another inline assembly clobber */
        asm volatile (
            "# More register clobbering\n"
            : 
            : 
            : "memory", "cc", 
              "r16", "r17", "r18", "r19", "r20"
        );
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    int seed = time(NULL);
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    srand(seed);
    
    int total = 0;
    
    /* Call stress functions multiple times from different contexts */
    for (int i = 0; i < 10; i++) {
        total += stress_computation(seed + i, iterations / 10);
        total += stress_memory_access(seed - i, iterations / 10);
        
        /* Vary the call pattern */
        if (i % 2 == 0) {
            total ^= stress_computation(total, 50);
        } else {
            total |= stress_memory_access(total, 50);
        }
    }
    
    printf("Result: %d (seed: %d)\n", total, seed);
    return total != 0 ? 0 : 1;
}
