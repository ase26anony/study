#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA to influence scheduling */
__attribute__((noinline)) 
static void use_vla(int size) {
    int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2 + (i % 3);
    }
    /* Use the VLA to prevent optimization */
    asm volatile ("" : : "r"(vla) : "memory");
}

/* Another helper with alloca */
__attribute__((noinline))
static void use_alloca(int size) {
    int* dyn_arr = (int*)alloca(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        dyn_arr[i] = (i * 3) % 17;
    }
    asm volatile ("" : : "r"(dyn_arr) : "memory");
}

int main(void) {
    /* Initialize with random data */
    srand(time(NULL));
    
    double array_fp[ARRAY_SIZE];
    int array_int[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_fp[i] = (double)rand() / RAND_MAX * 100.0;
        array_int[i] = rand() % 1000;
    }
    
    double total_fp = 0.0;
    long long total_int = 0;
    
    /* Primary outer loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency chain basic block */
        double chain_fp = array_fp[outer % ARRAY_SIZE];
        int chain_int = array_int[outer % ARRAY_SIZE];
        
        /* Long chain of dependent FP operations */
        chain_fp = chain_fp + array_fp[(outer + 1) % ARRAY_SIZE];
        chain_fp = chain_fp * 1.234567;
        chain_fp = chain_fp / (array_fp[(outer + 2) % ARRAY_SIZE] + 0.001);
        chain_fp = sqrt(fabs(chain_fp));
        chain_fp = sin(chain_fp * 0.01);
        chain_fp = chain_fp * chain_fp + chain_fp;
        
        /* Long chain of dependent integer operations */
        chain_int = chain_int + array_int[(outer + 3) % ARRAY_SIZE];
        chain_int = chain_int * 3;
        chain_int = chain_int / ((array_int[(outer + 4) % ARRAY_SIZE] % 10) + 1);
        chain_int = chain_int % 1000;
        chain_int = chain_int * chain_int - chain_int;
        chain_int = (chain_int << 3) | (chain_int >> 5);
        
        /* Memory operations with varying addressing */
        array_fp[(outer + 10) % ARRAY_SIZE] = chain_fp;
        array_int[(outer + 20) % ARRAY_SIZE] = chain_int;
        
        total_fp += chain_fp;
        total_int += chain_int;
        
        /* Use VLA helper between patterns */
        use_vla((outer % 20) + 5);
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_limit = (rand() % INNER_BASE) + 10;
        for (int i = 0; i < 5; i++) {
            int dynamic_limit = (rand() % 15) + 5;
            for (int j = 0; j < dynamic_limit; j++) {
                /* Mixed operations in inner loop */
                double temp = array_fp[(i + j + outer) % ARRAY_SIZE];
                temp = temp * temp - sin(temp);
                array_fp[(i * j + outer) % ARRAY_SIZE] = temp;
                
                int temp_int = array_int[(i * 3 + j * 7) % ARRAY_SIZE];
                temp_int = (temp_int * 11) % 997;
                temp_int = temp_int ^ (temp_int >> 3);
                array_int[(i + j * 2) % ARRAY_SIZE] = temp_int;
                
                total_fp += temp;
                total_int += temp_int;
            }
        }
        
        /* Use alloca helper */
        use_alloca((outer % 15) + 3);
        
        /* Pattern 3: Inline assembly barriers */
        double barrier_fp = array_fp[outer % ARRAY_SIZE];
        int barrier_int = array_int[outer % ARRAY_SIZE];
        
        barrier_fp = barrier_fp * 2.5 + 1.0;
        asm volatile ("" ::: "memory");
        
        barrier_fp = sin(barrier_fp) * cos(barrier_fp);
        barrier_int = barrier_int * 3 + 7;
        asm volatile ("" ::: "memory");
        
        barrier_fp = sqrt(fabs(barrier_fp)) + barrier_fp;
        barrier_int = (barrier_int << 2) | (barrier_int >> 6);
        asm volatile ("" ::: "memory");
        
        barrier_fp = barrier_fp / (array_fp[(outer + 5) % ARRAY_SIZE] + 0.5);
        barrier_int = barrier_int % ((array_int[(outer + 6) % ARRAY_SIZE] % 20) + 1);
        
        total_fp += barrier_fp;
        total_int += barrier_int;
        
        /* Pattern 4: __builtin_expect with cold path */
        int condition = (outer == 42);  /* Rare condition */
        if (__builtin_expect(condition, 0)) {
            /* Cold path - complex operations */
            double cold_fp = 0.0;
            for (int k = 0; k < 100; k++) {
                cold_fp += sin(array_fp[(outer + k) % ARRAY_SIZE] * 0.01);
                cold_fp = sqrt(fabs(cold_fp));
            }
            
            int cold_int = 0;
            for (int k = 0; k < 50; k++) {
                cold_int += array_int[(outer + k * 2) % ARRAY_SIZE];
                cold_int = (cold_int * 13) % 999;
            }
            
            total_fp += cold_fp;
            total_int += cold_int;
            
            /* Another VLA in cold path */
            use_vla(30);
        }
        
        /* More mixed operations */
        for (int i = 0; i < 3; i++) {
            double mix_fp = array_fp[(outer + i * 7) % ARRAY_SIZE];
            int mix_int = array_int[(outer + i * 11) % ARRAY_SIZE];
            
            for (int j = 0; j < 4; j++) {
                mix_fp = mix_fp * 1.1 + sin(mix_fp);
                mix_int = mix_int * 2 - (mix_int % 17);
                
                /* Function call as scheduling barrier */
                mix_fp = sqrt(fabs(mix_fp + j * 0.1));
                
                /* Memory store with complex addressing */
                array_fp[(outer + i + j * 3) % ARRAY_SIZE] = mix_fp;
                array_int[(outer + i * 2 + j) % ARRAY_SIZE] = mix_int;
            }
            
            total_fp += mix_fp;
            total_int += mix_int;
        }
    }
    
    /* Final checksum calculation */
    double final_fp = 0.0;
    long long final_int = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_fp += array_fp[i] * (i + 1);
        final_int += array_int[i] * (i % 37);
    }
    
    final_fp += total_fp;
    final_int += total_int;
    
    /* Print results to prevent optimization */
    printf("Final FP result: %f\n", final_fp);
    printf("Final INT result: %lld\n", final_int);
    
    return 0;
}
