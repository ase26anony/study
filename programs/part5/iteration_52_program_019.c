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
static int complex_volatile_chain(int iter) {
    int result = 0;
    
    /* Force many virtual registers with complex expression */
    for (int i = 0; i < iter; i++) {
        /* Complex arithmetic with volatile operands - creates many temporaries */
        int temp1 = vol_a * vol_b + vol_c / (vol_d + 1);
        int temp2 = vol_e % (vol_f + 1) - vol_g * vol_a;
        int temp3 = (vol_b << 2) | (vol_c & 0xFF);
        int temp4 = temp1 * temp2 - temp3 / (vol_d + 1);
        
        /* Multi-use temporary value - candidate for rematerialization */
        int base = temp4 * (vol_e + i);
        
        /* Use base in multiple, spatially separated contexts */
        if (i % 3 == 0) {
            result += base * 2;
        } else if (i % 3 == 1) {
            result += base / 3;
        } else {
            result += base + temp1;
        }
        
        /* Inline assembly that clobbers registers - increases register pressure */
        __asm__ volatile (
            "# Clobber hard registers to force virtual register usage\n"
            "mov r0, %0\n"
            "mov r1, %1\n"
            :
            : "r" (temp1), "r" (temp2)
            : "r0", "r1", "r2", "r3", "memory"
        );
    }
    
    return result;
}

/* Function with address computation patterns */
static int address_computation_stress(int *array, int size, int offset) {
    int sum = 0;
    
    /* Compute base address once, use with multiple offsets */
    int *base_ptr = &array[offset];
    
    /* Use base with different offsets - may trigger register recreation */
    for (int i = 0; i < size; i += 4) {
        /* Multiple uses of computed address with different offsets */
        sum += base_ptr[i];      /* offset 0 */
        sum += base_ptr[i + 1];  /* offset 1 */
        sum += base_ptr[i + 2];  /* offset 2 */
        sum += base_ptr[i + 3];  /* offset 3 */
        
        /* Complex expression using the same base */
        int val1 = *(base_ptr + i) * 2;
        int val2 = *(base_ptr + i + 1) / 3;
        int val3 = *(base_ptr + i + 2) + 5;
        int val4 = *(base_ptr + i + 3) - 7;
        
        sum += val1 + val2 + val3 + val4;
    }
    
    return sum;
}

/* Main stress function with control flow to obstruct optimization */
int stress_computation(int seed, int n) {
    int result = 0;
    volatile int vol_counter = seed;  /* Volatile loop counter */
    
    /* Array for address computation */
    int *data_array = (int *)malloc(n * sizeof(int));
    if (!data_array) return -1;
    
    /* Initialize with semi-random data */
    for (int i = 0; i < n; i++) {
        data_array[i] = (i * seed + vol_a) % 100;
    }
    
    /* Loop with volatile control - prevents optimization */
    for (vol_counter = 0; vol_counter < n; vol_counter++) {
        /* Opaque function call - compiler can't analyze result */
        int opaque = get_external_value() % 100;
        
        /* Complex expression with opaque value */
        int expr1 = (opaque * vol_b + vol_c) / (vol_d + 1);
        int expr2 = (vol_e % (opaque + 1)) * vol_f;
        int expr3 = expr1 * expr2 - opaque;
        
        /* Multi-use temporary in different control paths */
        int multi_use = expr3 * (vol_g + vol_counter);
        
        /* Switch with different uses of the same value */
        switch (vol_counter % 4) {
            case 0:
                result += multi_use * 2;
                break;
            case 1:
                result += multi_use / 3 + expr1;
                break;
            case 2:
                result += multi_use + expr2;
                break;
            case 3:
                result += multi_use - expr3;
                break;
        }
        
        /* More complex arithmetic creating register pressure */
        double fp_temp = (double)multi_use * 1.5;
        fp_temp += (double)expr1 / 2.0;
        fp_temp -= (double)expr2 * 0.75;
        fp_temp /= (double)(opaque + 1);
        
        result += (int)fp_temp;
        
        /* Inline assembly with many clobbers */
#ifdef __OPTIMIZE__
        __asm__ volatile (
            "# Stress register allocator\n"
            "add r4, %0, %1\n"
            "sub r5, %2, %3\n"
            :
            : "r" (multi_use), "r" (expr1), "r" (expr2), "r" (expr3)
            : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "memory"
        );
#endif
    }
    
    /* Call address computation function */
    result += address_computation_stress(data_array, n, seed % 10);
    
    /* Another round of complex volatile chain */
    result += complex_volatile_chain(n / 2);
    
    free(data_array);
    return result;
}

/* Simulated external function */
int get_external_value(void) {
    static int counter = 0;
    return (counter++ * 1103515245 + 12345) & 0x7FFFFFFF;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = 42;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 1000) iterations = 1000;  /* Limit for safety */
    }
    
    srand(seed);
    
    /* Initialize volatile variables with some variation */
    vol_a = rand() % 50 + 1;
    vol_b = rand() % 50 + 1;
    vol_c = rand() % 50 + 1;
    vol_d = rand() % 50 + 1;
    vol_e = rand() % 50 + 1;
    vol_f = rand() % 50 + 1;
    vol_g = rand() % 50 + 1;
    
    /* Call stress function multiple times from different contexts */
    int total = 0;
    for (int i = 0; i < 3; i++) {
        int result = stress_computation(seed + i, iterations);
        total += result;
        printf("Iteration %d: result = %d\n", i, result);
    }
    
    printf("Total: %d\n", total);
    
    /* Ensure result is used */
    return total == 0 ? 1 : 0;
}
