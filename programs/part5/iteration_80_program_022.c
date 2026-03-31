/* main.c - Primary file with nested loops and complex control flow */
#include <stdio.h>
#include <stdlib.h>

/* External declarations for multi-file compilation stress */
extern int global_array[1024];
extern volatile int g_volatile_seed;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int compute_hash(int a, int b) {
    return (a * 31 + b) ^ 0x5A5A5A5A;
}

__attribute__((noinline, const)) int pure_transform(int x) {
    return (x * 3) / 2;
}

__attribute__((noinline)) void memory_barrier() {
    asm volatile ("" ::: "memory");
}

/* Helper function with loop-carried dependencies */
__attribute__((noinline)) int process_chunk(int* data, int start, int end, int init) {
    register int acc = init;  /* Hint register allocation */
    int temp;
    
    for (int i = start; i < end; ++i) {
        /* Mix of arithmetic operations */
        temp = data[i] * 2;
        if (__builtin_expect((i & 0x3F) == 0, 0)) {
            /* Uncommon path with function call */
            temp = pure_transform(temp);
        }
        acc += temp;
        
        /* Memory barrier to create scheduling boundary */
        if (i % 16 == 0) {
            memory_barrier();
        }
    }
    return acc;
}

/* Core computation with nested loops */
int complex_computation(int N, int M, int K, int* arr) {
    int total = 0;
    unsigned short outer_counter;
    int inner_acc;
    
    /* Outer loop with varying data types */
    for (outer_counter = 0; outer_counter < (unsigned short)N; ++outer_counter) {
        int outer_val = outer_counter * 3;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(outer_counter > K, 1)) {
            /* First inner loop with different counter type */
            register int j;  /* Register hint */
            for (j = 0; j < M; ++j) {
                /* Loop-carried dependency */
                inner_acc = arr[j] + inner_acc;
                
                /* Mixed operations with conditional */
                if (j % 7 == 0) {
                    inner_acc = compute_hash(inner_acc, outer_val);
                    asm volatile ("" ::: "memory");  /* Scheduling boundary */
                } else {
                    inner_acc = pure_transform(inner_acc);
                }
                
                /* Access global array with potential aliasing */
                if ((j + outer_counter) % 11 == 0) {
                    inner_acc ^= global_array[j & 1023];
                }
            }
            total += inner_acc;
        }
        
        /* Second inner loop with different structure */
        if (outer_counter % 4 == 0) {
            int k;
            int local_acc = 0;
            for (k = M - 1; k >= 0; --k) {
                /* Reverse traversal with different pattern */
                int idx = (k + outer_counter) % 256;
                local_acc = arr[idx] - local_acc;
                
                /* Function call with loop-variant arguments */
                if (k % 5 == 0) {
                    local_acc = compute_hash(local_acc, k);
                }
                
                /* Memory operation */
                global_array[idx] = local_acc & 0xFF;
            }
            total ^= local_acc;
        }
    }
    
    return total;
}

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline)) void warm_up_computation() {
    int dummy_array[64];
    int i;
    
    /* Simple warm-up loop */
    for (i = 0; i < 64; ++i) {
        dummy_array[i] = i * i;
        if (i % 8 == 0) {
            asm volatile ("" ::: "memory");
        }
    }
    
    /* Call process_chunk to ensure it's compiled */
    int result = process_chunk(dummy_array, 0, 64, 0);
    printf("Warm-up result: %d\n", result);
}

int main(int argc, char** argv) {
    /* Use arguments for variability */
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int M = (argc > 2) ? atoi(argv[2]) : 50;
    int K = (argc > 3) ? atoi(argv[3]) : 25;
    
    /* Initialize data with pseudo-random values */
    int* data_array = (int*)malloc(1024 * sizeof(int));
    if (!data_array) return 1;
    
    /* Simple LCG for pseudo-random values */
    unsigned int seed = 123456789;
    for (int i = 0; i < 1024; ++i) {
        seed = (1103515245 * seed + 12345) & 0x7FFFFFFF;
        data_array[i] = (int)(seed % 1000);
    }
    
    /* Warm-up execution */
    warm_up_computation();
    
    /* Main computation with nested loops */
    int checksum = complex_computation(N, M, K, data_array);
    
    /* Additional computation with different parameters */
    int secondary = process_chunk(data_array, 0, 512, checksum);
    
    /* Final result */
    int final_result = checksum ^ secondary;
    printf("Final checksum: %d (N=%d, M=%d, K=%d)\n", 
           final_result, N, M, K);
    
    free(data_array);
    return 0;
}
