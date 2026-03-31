#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External declarations for multi-file compilation */
extern int external_array[1024];
extern volatile int g_volatile_seed;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_compute(int a, int b) {
    return (a * b) ^ (a + b);
}

__attribute__((noinline, const)) int pure_compute(int x, int y) {
    return x * x + y * y;
}

__attribute__((noinline)) void noinline_side_effect(int* ptr) {
    *ptr = (*ptr * 1103515245 + 12345) & 0x7fffffff;
}

/* Global arrays for memory operations */
int global_arr[2048];
short short_arr[4096];
unsigned int uint_arr[1024];

/* Struct with different sized members */
struct MixedData {
    int a;
    short b;
    char c;
    long d;
} mixed_structs[256];

/* Warm-up function executed once */
__attribute__((noinline)) void warm_up_computation(void) {
    volatile int warm_seed = 42;
    int temp = 0;
    
    for (int i = 0; i < 100; ++i) {
        if (__builtin_expect((i & 3) == 0, 0)) {
            temp += noinline_compute(i, warm_seed);
        } else {
            temp -= pure_compute(i, warm_seed);
        }
        
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
    
    printf("Warm-up result: %d\n", temp);
}

/* Core computation with nested loops targeting selective scheduling */
__attribute__((noinline)) long complex_nested_loops(int outer_limit, 
                                                   int inner_limit,
                                                   int conditional_limit) {
    long total_checksum = 0;
    int register reg_acc1 = 0;  /* Hint for register allocation */
    int reg_acc2 = 0;
    unsigned int u_acc = 0;
    short s_acc = 0;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < outer_limit; ++i) {
        int loop_variant = i * 7;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > conditional_limit, 0)) {
            /* Inner loop with different data type */
            for (unsigned int j = 0; j < (unsigned int)inner_limit; ++j) {
                /* Loop-carried dependency */
                reg_acc1 = global_arr[j] + reg_acc1;
                
                /* Memory operation with potential aliasing */
                uint_arr[i] = uint_arr[i] ^ (j * 3);
                
                /* Mixed data type operations */
                s_acc = (short)(s_acc + short_arr[j % 4096]);
                
                /* Function call with loop-variant arguments */
                int pure_result = pure_compute(i, j);
                u_acc += (unsigned int)pure_result;
                
                /* Optimization barrier */
                asm volatile("" ::: "memory");
                
                /* Conditional branch inside innermost loop */
                if (__builtin_expect((j % 13) == 0, 1)) {
                    reg_acc2 = noinline_compute(reg_acc1, reg_acc2);
                    /* Access struct member */
                    mixed_structs[j % 256].a = reg_acc2;
                }
                
                /* Another memory barrier */
                asm volatile("" ::: "memory");
            }
        } else {
            /* Alternative path with different operations */
            for (short k = 0; k < (short)(inner_limit / 2); ++k) {
                /* Different computation pattern */
                int idx = (i * 31 + k) % 2048;
                global_arr[idx] = global_arr[idx] * 2 + 1;
                
                /* Call to function with side effect */
                noinline_side_effect(&global_arr[idx]);
                
                /* Complex expression with multiple uses */
                reg_acc1 = reg_acc1 * 3 - reg_acc2;
                reg_acc2 = reg_acc1 / (k + 1);
                
                /* Access external array */
                if (idx < 1024) {
                    total_checksum += external_array[idx];
                }
            }
        }
        
        /* Update checksum with all accumulators */
        total_checksum += reg_acc1 + reg_acc2 + u_acc + s_acc;
        
        /* Reset some accumulators periodically */
        if ((i % 8) == 0) {
            reg_acc2 = 0;
            asm volatile("" ::: "memory");
        }
    }
    
    return total_checksum;
}

/* Secondary nested loop structure with different patterns */
__attribute__((noinline)) long secondary_loops(int limit_a, int limit_b) {
    long result = 0;
    volatile int vol_var = g_volatile_seed;
    
    /* Triple nested loops */
    for (int x = 0; x < limit_a; ++x) {
        int temp = x * x;
        
        for (int y = 0; y < limit_b; ++y) {
            /* Register hint for inner loop variable */
            register int reg_y = y;
            
            for (int z = 0; z < 16; ++z) {
                /* Complex expression with multiple operations */
                int val = (temp * reg_y) >> (z % 8);
                val = val ^ (vol_var + z);
                
                /* Conditional store */
                if (__builtin_expect(val > 1000, 0)) {
                    short_arr[z * 64] = (short)val;
                }
                
                /* Function call in deepest loop */
                result += pure_compute(val, reg_y);
                
                /* Memory operation with struct */
                mixed_structs[z].b = (short)result;
            }
            
            /* Barrier between middle loop iterations */
            asm volatile("" ::: "memory");
        }
        
        /* Update volatile variable */
        if ((x % 4) == 0) {
            g_volatile_seed = x;
        }
    }
    
    return result;
}

/* Initialize data with pseudo-random values */
void initialize_data(void) {
    /* Simple LCG for pseudo-random values */
    unsigned int seed = 123456789;
    
    for (int i = 0; i < 2048; ++i) {
        seed = seed * 1103515245 + 12345;
        global_arr[i] = (int)(seed % 1000);
        
        if (i < 4096) {
            seed = seed * 1103515245 + 12345;
            short_arr[i] = (short)(seed % 1000);
        }
        
        if (i < 1024) {
            seed = seed * 1103515245 + 12345;
            uint_arr[i] = seed;
        }
        
        if (i < 256) {
            seed = seed * 1103515245 + 12345;
            mixed_structs[i].a = (int)seed;
            mixed_structs[i].b = (short)seed;
            mixed_structs[i].c = (char)seed;
            mixed_structs[i].d = (long)seed * seed;
        }
    }
}

int main(int argc, char* argv[]) {
    /* Use arguments to make loop bounds non-constant */
    int outer_lim = (argc > 1) ? atoi(argv[1]) : 50;
    int inner_lim = (argc > 2) ? atoi(argv[2]) : 100;
    int cond_lim = (argc > 3) ? atoi(argv[3]) : 25;
    
    /* Initialize global volatile */
    g_volatile_seed = 777;
    
    printf("Initializing data...\n");
    initialize_data();
    
    printf("Starting warm-up...\n");
    warm_up_computation();
    
    printf("Running main computation...\n");
    long result1 = complex_nested_loops(outer_lim, inner_lim, cond_lim);
    
    printf("Running secondary loops...\n");
    long result2 = secondary_loops(outer_lim / 2, inner_lim / 2);
    
    long final_result = result1 + result2;
    printf("Final checksum: %ld\n", final_result);
    printf("Result1: %ld, Result2: %ld\n", result1, result2);
    
    return 0;
}
