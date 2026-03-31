/* main.c - Primary file with nested loops for selective scheduling */
#include <stdio.h>
#include <stdlib.h>

/* External declarations for multi-file compilation stress */
extern int external_array[1024];
extern void init_external_data(void);

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_multiply(int a, int b) {
    return a * b;
}

__attribute__((noinline)) float noinline_divide(float a, float b) {
    if (b != 0.0f) return a / b;
    return 0.0f;
}

/* Pure function for const attribute */
__attribute__((const)) int pure_compute(int x, int y) {
    return (x * x) + (y * y);
}

/* Global arrays for memory operations with potential aliasing */
int global_arr[2048];
float float_arr[1024];
short short_arr[4096];

/* Volatile variables to prevent optimization */
volatile int volatile_seed = 42;
volatile int volatile_limit;

/* Core computation function with nested loops */
__attribute__((noinline)) 
long long compute_checksum(int outer_bound, int inner_bound, int conditional_limit) {
    long long checksum = 0;
    int i, j, k;
    unsigned u_counter;
    short s_temp;
    float f_acc = 0.0f;
    
    /* Outer loop with int counter */
    for (i = 0; i < outer_bound; ++i) {
        int loop_carried = i;  /* Loop-carried dependency */
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > conditional_limit, 0)) {
            /* Nested loop with unsigned counter */
            for (u_counter = 0; u_counter < (unsigned)inner_bound; u_counter += 2) {
                /* Mix of arithmetic operations */
                int temp = noinline_multiply(i, (int)u_counter);
                temp += pure_compute(i, u_counter);
                
                /* Memory operations with different types */
                global_arr[u_counter] = temp;
                short_arr[u_counter] = (short)(temp % 256);
                
                /* Loop-carried accumulation */
                loop_carried += global_arr[u_counter % 512];
                
                /* Optimization barrier */
                asm volatile ("" ::: "memory");
                
                /* Conditional branch inside innermost loop */
                if (u_counter % 8 == 0) {
                    f_acc += noinline_divide((float)temp, (float)(u_counter + 1));
                    checksum += (long long)f_acc;
                }
                
                /* Register hint variable */
                register int reg_var = loop_carried * 3;
                checksum += reg_var;
                
                /* Another optimization barrier */
                asm volatile ("" ::: "memory");
            }
        } else {
            /* Different path with short counter */
            for (s_temp = 0; s_temp < (short)(inner_bound / 4); ++s_temp) {
                /* Different computation pattern */
                float f_temp = float_arr[s_temp % 256] * 2.0f;
                int idx = (i * 32 + s_temp) % 2048;
                
                /* Memory store with potential aliasing */
                global_arr[idx] = (int)f_temp + s_temp;
                
                /* Function call with loop-variant arguments */
                checksum += noinline_multiply(i, s_temp);
                
                /* Complex conditional */
                if ((i % 3 == 0) && (s_temp % 5 == 0)) {
                    checksum += pure_compute(i, s_temp * 2);
                }
            }
        }
        
        /* Triple nested loop for deeper scheduling regions */
        if (i % 4 == 0) {
            for (j = 0; j < 8; ++j) {
                int local_acc = 0;
                for (k = 0; k < 16; ++k) {
                    /* Multiple uses of same variable */
                    int multi_use = j * k + i;
                    local_acc += multi_use;
                    multi_use *= 2;
                    local_acc -= multi_use / 3;
                    checksum += local_acc;
                }
                
                /* Memory load after computation */
                checksum += global_arr[j % 2048];
                
                /* Conditional with __builtin_expect */
                if (__builtin_expect(j > 4, 1)) {
                    checksum += short_arr[j * 2] * 3;
                }
            }
        }
        
        /* Update checksum with loop-carried value */
        checksum += loop_carried;
    }
    
    return checksum;
}

/* Warm-up function */
__attribute__((noinline))
void warmup_computation(void) {
    int temp_sum = 0;
    volatile int warmup_limit = 10;
    
    /* Simple warm-up loop */
    for (int w = 0; w < warmup_limit; ++w) {
        temp_sum += pure_compute(w, w * 2);
        asm volatile ("" ::: "memory");
    }
    
    /* Prevent optimization */
    if (temp_sum > 1000) {
        printf("Warmup completed\n");
    }
}

/* Initialize data with pseudo-random values using LCG */
void init_data(void) {
    unsigned int lcg_state = volatile_seed;
    
    for (int i = 0; i < 2048; ++i) {
        /* Simple LCG: state = state * 1103515245 + 12345 */
        lcg_state = lcg_state * 1103515245U + 12345U;
        global_arr[i] = (int)(lcg_state % 1000);
        
        if (i < 1024) {
            lcg_state = lcg_state * 1103515245U + 12345U;
            float_arr[i] = (float)(lcg_state % 100) / 10.0f;
        }
        
        if (i < 4096) {
            lcg_state = lcg_state * 1103515245U + 12345U;
            short_arr[i] = (short)(lcg_state % 256);
        }
    }
    
    /* Initialize external data if available */
    init_external_data();
}

int main(int argc, char *argv[]) {
    /* Use arguments for variability, preventing constant propagation */
    int outer_bound = (argc > 1) ? atoi(argv[1]) : 50;
    int inner_bound = (argc > 2) ? atoi(argv[2]) : 100;
    int conditional_limit = (argc > 3) ? atoi(argv[3]) : 25;
    
    /* Set volatile limit from arguments */
    volatile_limit = conditional_limit;
    
    /* Initialize data */
    init_data();
    
    /* Warm-up execution */
    warmup_computation();
    
    /* Main computation with nested loops */
    long long result = compute_checksum(
        outer_bound + volatile_seed % 10,  /* Prevent constant folding */
        inner_bound + (volatile_seed % 20),
        conditional_limit
    );
    
    /* Print verifiable result */
    printf("Computation checksum: %lld\n", result);
    
    /* Additional verification loop */
    long long verify_sum = 0;
    for (int v = 0; v < 100; ++v) {
        verify_sum += global_arr[v % 2048];
    }
    printf("Verification sum: %lld\n", verify_sum);
    
    return 0;
}
