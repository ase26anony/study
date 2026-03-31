#include <stdio.h>
#include <stdlib.h>

/* External declarations for multi-file testing */
extern int global_array[1024];
extern volatile int g_volatile_seed;

/* Non-inlineable functions */
__attribute__((noinline)) int non_inline_func(int x, int y) {
    return x * y + (x ^ y);
}

__attribute__((noinline, const)) int pure_func(int a, int b) {
    return (a * b) >> 1;
}

/* Helper with side effects */
__attribute__((noinline)) void side_effect_func(int* ptr) {
    *ptr += (*ptr % 7);
}

/* Warm-up function */
__attribute__((noinline)) void warm_up(int iterations) {
    register int acc = 0;
    for (int i = 0; i < iterations; ++i) {
        acc += i * 3;
        if (__builtin_expect(i % 16 == 0, 0)) {
            acc -= pure_func(i, i + 1);
        }
    }
    asm volatile("" : "+r"(acc) : : "memory");
}

/* Core computation with nested loops */
unsigned long long core_computation(int N, int M, int K, short* data) {
    unsigned long long checksum = 0;
    int outer_acc = 0;
    unsigned loop_counter = 0;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < N; ++i) {
        int inner_acc = 0;
        unsigned short mod_result = 0;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > (N / 3), 0)) {
            /* First inner loop with different data type */
            for (unsigned j = 0; j < (unsigned)M; ++j) {
                /* Loop-carried dependency */
                inner_acc = data[j % 1024] + inner_acc;
                
                /* Memory operation with potential aliasing */
                global_array[j % 1024] += i;
                
                /* Mixed arithmetic operations */
                mod_result = (j * 7 + i) % 256;
                
                /* Conditional branch */
                if (__builtin_expect((i * j) % K == 0, 1)) {
                    /* Function call with loop-variant arguments */
                    inner_acc += non_inline_func(i, j);
                    
                    /* Optimization barrier */
                    asm volatile("" ::: "memory");
                }
                
                /* Pure function call */
                inner_acc ^= pure_func(i, mod_result);
                
                /* Multiple uses of same variable */
                loop_counter += mod_result;
                loop_counter *= 2;
            }
        }
        
        /* Second inner loop with different characteristics */
        for (short s = 0; s < (short)(M / 2); ++s) {
            /* Different data type computations */
            int temp = s * i;
            
            /* Register hint variable */
            register int reg_var = temp & 0xFF;
            
            /* Complex expression with multiple operations */
            temp = (temp << 3) | (temp >> 5);
            temp += reg_var * reg_var;
            
            /* Conditional with side effect */
            if (temp % 11 == 0) {
                side_effect_func(&inner_acc);
                checksum += temp;
            }
            
            /* Another optimization barrier */
            asm volatile("" ::: "memory");
            
            /* Use variable across operations */
            reg_var = pure_func(reg_var, s);
            inner_acc += reg_var;
        }
        
        /* Update outer accumulator */
        outer_acc += inner_acc;
        checksum += outer_acc;
        
        /* Volatile dependency to prevent optimization */
        if (g_volatile_seed > 0) {
            outer_acc ^= g_volatile_seed;
        }
    }
    
    return checksum + loop_counter;
}

/* Initialize data with pseudo-random values */
void init_data(short* data, int size, int seed) {
    int lcg = seed;
    for (int i = 0; i < size; ++i) {
        lcg = (lcg * 1103515245 + 12345) & 0x7FFFFFFF;
        data[i] = (short)(lcg % 1000);
    }
}

int main(int argc, char* argv[]) {
    /* Use arguments for variability */
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int M = (argc > 2) ? atoi(argv[2]) : 50;
    int K = (argc > 3) ? atoi(argv[3]) : 7;
    
    /* Volatile seed */
    g_volatile_seed = (argc > 4) ? atoi(argv[4]) : 42;
    
    /* Local arrays */
    short data_array[1024];
    init_data(data_array, 1024, g_volatile_seed);
    
    /* Warm-up execution */
    warm_up(100);
    
    /* Main computation */
    unsigned long long result = core_computation(N, M, K, data_array);
    
    /* Verifiable output */
    printf("Result checksum: %llu\n", result);
    
    return 0;
}
