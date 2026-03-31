/* Program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Volatile variables to prevent optimization */
volatile int vol_counter = 0;
volatile float vol_float = 1.5f;
volatile double vol_double = 2.5;

/* Arrays with potential aliasing */
static float data_f[SIZE];
static double data_d[SIZE];
static int data_i[SIZE];

/* Function with memory aliasing - restrict and non-restrict pointers */
static inline void process_chunk(float *restrict dst_f, double *restrict dst_d,
                                 const float *src_f, const double *src_d,
                                 int *modifiable_i, int start, int end) {
    /* Mixed operations with carried dependencies */
    float f_acc = vol_float;
    double d_acc = vol_double;
    int i_acc = vol_counter;
    
    for (int i = start; i < end; i++) {
        /* Multiple independent operations */
        float f1 = src_f[i] * 1.1f;
        float f2 = src_f[(i + 1) % SIZE] * 0.9f;
        
        double d1 = src_d[i] / 1.3;
        double d2 = src_d[(i + 2) % SIZE] * 1.7;
        
        int i1 = modifiable_i[i] + 3;
        int i2 = modifiable_i[(i + 3) % SIZE] - 2;
        
        /* Carried dependency chain */
        f_acc = f_acc * f1 + f2;
        d_acc = d_acc / d1 + d2;
        i_acc = i_acc * i1 + i2;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            f_acc = f_acc * 0.5f;
            d_acc = d_acc * 0.5;
            i_acc = i_acc >> 1;
        } else if (i % 13 == 0) {
            f_acc = f_acc + 10.0f;
            d_acc = d_acc - 5.0;
            i_acc = i_acc ^ 0x55AA;
        }
        
        /* Memory stores with potential aliasing */
        dst_f[i] = f_acc;
        dst_d[i] = d_acc;
        modifiable_i[i] = i_acc;
        
        /* Inline assembly with memory clobber to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Store final accumulators to volatile to prevent dead code elimination */
    vol_float = f_acc;
    vol_double = d_acc;
    vol_counter = i_acc;
}

/* Main hot loop function */
static inline void compute_loop(int iterations) {
    /* Local arrays for processing */
    float local_f[SIZE];
    double local_d[SIZE];
    int local_i[SIZE];
    
    /* Initialize local arrays */
    for (int i = 0; i < SIZE; i++) {
        local_f[i] = (float)i * 0.1f;
        local_d[i] = (double)i * 0.2;
        local_i[i] = i * 3;
    }
    
    /* Multiple passes to create scheduling region */
    for (int pass = 0; pass < iterations; pass++) {
        /* Process in chunks to create more complex control flow */
        for (int chunk = 0; chunk < SIZE; chunk += 64) {
            int end = chunk + 64;
            if (end > SIZE) end = SIZE;
            
            /* Call with mixed pointer types to create aliasing concerns */
            process_chunk(local_f, local_d,
                         data_f, data_d,
                         local_i, chunk, end);
        }
        
        /* Additional mixed operations between passes */
        float temp_f = 0.0f;
        double temp_d = 0.0;
        for (int i = 0; i < SIZE; i++) {
            temp_f += local_f[i] * 0.3f;
            temp_d += local_d[i] / 1.1;
            
            /* Another conditional inside outer loop */
            if (i % 11 == 0) {
                local_i[i] = (local_i[i] * 2) ^ 0xFF;
            }
        }
        
        /* Use results to prevent optimization */
        vol_float += temp_f;
        vol_double += temp_d;
    }
}

/* Secondary computation with different pattern */
static inline void compute_alternate(int iterations) {
    double mix_acc = 0.0;
    int int_acc = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex mixed-type expressions */
        mix_acc = (mix_acc * 1.1) + (data_f[i % SIZE] * 0.7f);
        int_acc = (int_acc * 3) ^ data_i[i % SIZE];
        
        /* Floating point division - expensive operation */
        if (mix_acc != 0.0) {
            mix_acc = 1.0 / mix_acc;
        }
        
        /* Integer division with remainder */
        int_acc = int_acc / 7 + (int_acc % 13);
        
        /* Store to array with stride */
        data_d[i % SIZE] = mix_acc;
        data_i[(i * 3) % SIZE] = int_acc;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    vol_double = mix_acc;
    vol_counter = int_acc;
}

int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        data_f[i] = (float)(i + 1) * 0.25f;
        data_d[i] = (double)(i + 2) * 0.33;
        data_i[i] = i * 5 + 1;
    }
    
    /* Perform multiple computation phases */
    uint64_t checksum = 0;
    
    /* Phase 1: Main hot loop */
    compute_loop(ITERATIONS);
    
    /* Phase 2: Alternate computation */
    compute_alternate(ITERATIONS / 2);
    
    /* Phase 3: Another round with different parameters */
    compute_loop(ITERATIONS / 4);
    
    /* Phase 4: Final mixed computation */
    compute_alternate(ITERATIONS / 8);
    
    /* Create checksum from results to prevent optimization */
    for (int i = 0; i < SIZE; i++) {
        checksum ^= *(uint64_t*)&data_f[i];
        checksum ^= *(uint64_t*)&data_d[i];
        checksum ^= (uint64_t)data_i[i];
    }
    
    checksum ^= (uint64_t)vol_counter;
    checksum ^= *(uint64_t*)&vol_float;
    checksum ^= *(uint64_t*)&vol_double;
    
    printf("Checksum: %016llx\n", (unsigned long long)checksum);
    
    return 0;
}
