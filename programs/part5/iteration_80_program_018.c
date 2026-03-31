/* sel_sched_test.c - Test program for selective scheduling dump coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global arrays with different types to create varied RTL */
volatile int g_volatile_seed = 42;
int g_array_int[1024];
unsigned int g_array_uint[1024];
short g_array_short[2048];
float g_array_float[1024];

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) 
int noinline_func1(int x, int y) {
    return (x * y) ^ (x + y);
}

__attribute__((noinline))
unsigned noinline_func2(unsigned a, unsigned b) {
    return (a & b) | (a ^ b);
}

__attribute__((noinline))
short noinline_func3(short x, short y) {
    return (short)(x + y * 2);
}

/* Pure function for loop-invariant computations */
__attribute__((const))
int pure_multiply(int a, int b) {
    return a * b;
}

/* Helper function with loop-carried dependency */
__attribute__((noinline))
int accumulate_loop(int *arr, int n, int init) {
    int acc = init;
    for (int i = 0; i < n; ++i) {
        acc = arr[i] + acc;
        /* Memory barrier to create scheduling boundary */
        asm volatile ("" ::: "memory");
    }
    return acc;
}

/* Complex nested loop structure */
__attribute__((noinline))
int complex_nested_loops(int outer_limit, int inner_limit, int threshold) {
    int total = 0;
    unsigned u_total = 0;
    short s_total = 0;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < outer_limit; ++i) {
        /* Loop-carried dependency */
        int local_acc = i;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > threshold, 0)) {
            /* Inner loop with different counter type */
            for (unsigned j = 0; j < (unsigned)inner_limit; ++j) {
                /* Mixed operations creating varied RTL */
                int temp = g_array_int[j] * i;
                unsigned u_temp = g_array_uint[j] + j;
                short s_temp = g_array_short[j * 2];
                
                /* Function calls with loop-variant arguments */
                temp = noinline_func1(temp, i);
                u_temp = noinline_func2(u_temp, j);
                s_temp = noinline_func3(s_temp, (short)i);
                
                /* Conditional branch inside innermost loop */
                if (j % 8 == 0) {
                    temp = pure_multiply(temp, 3);
                    /* Another memory barrier */
                    asm volatile ("" ::: "memory");
                }
                
                /* Update accumulators with different scopes */
                register int reg_acc = local_acc;  /* register hint */
                reg_acc += temp;
                local_acc = reg_acc;
                u_total += u_temp;
                s_total += s_temp;
                
                /* Access with potential aliasing */
                g_array_float[j] = (float)(temp * 0.5);
            }
        } else {
            /* Different path with simpler loop */
            for (short k = 0; k < (short)(inner_limit / 2); ++k) {
                int idx = (i * 16 + k) % 1024;
                g_array_int[idx] = g_array_int[idx] + k;
                /* Memory operation barrier */
                asm volatile ("" ::: "memory");
            }
        }
        
        /* Update total with complex expression */
        total += local_acc * (i % 4 + 1);
    }
    
    /* Combine different type accumulators */
    return total + (int)u_total + (int)s_total;
}

/* Another loop pattern with struct access */
struct DataPoint {
    int x;
    int y;
    int z;
};

struct DataPoint g_data[256];

__attribute__((noinline))
int struct_access_loop(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Struct access with different field operations */
        g_data[i % 256].x = i;
        g_data[i % 256].y = g_data[i % 256].x * 2;
        g_data[i % 256].z = g_data[i % 256].y + g_data[i % 256].x;
        
        /* Complex computation with multiple uses of same variable */
        int val = g_data[i % 256].z;
        val = val * val - val;
        val = noinline_func1(val, i);
        sum += val;
        
        /* Conditional with __builtin_expect */
        if (__builtin_expect((i & 0xF) == 0, 1)) {
            asm volatile ("" ::: "memory");
            sum += g_volatile_seed;
        }
    }
    
    return sum;
}

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline))
void warm_up_computation(void) {
    int warm_sum = 0;
    /* Simple warm-up loop */
    for (int i = 0; i < 100; ++i) {
        warm_sum += i * i;
        if (i % 10 == 0) {
            warm_sum = noinline_func1(warm_sum, i);
        }
    }
    /* Prevent optimization */
    asm volatile ("" : : "r"(warm_sum));
}

/* Initialize data with pseudo-random values */
void initialize_data(void) {
    /* Simple LCG for pseudo-random values */
    unsigned seed = g_volatile_seed;
    
    for (int i = 0; i < 1024; ++i) {
        seed = seed * 1103515245 + 12345;
        g_array_int[i] = (int)(seed % 1000);
        g_array_uint[i] = seed % 500;
        g_array_float[i] = (float)(seed % 100) * 0.1f;
    }
    
    for (int i = 0; i < 2048; ++i) {
        seed = seed * 1103515245 + 12345;
        g_array_short[i] = (short)(seed % 30000);
    }
    
    for (int i = 0; i < 256; ++i) {
        seed = seed * 1103515245 + 12345;
        g_data[i].x = (int)(seed % 100);
        g_data[i].y = (int)(seed % 200);
        g_data[i].z = (int)(seed % 300);
    }
}

/* Main computation driver */
int main(int argc, char *argv[]) {
    /* Use arguments to make loop bounds non-constant */
    int outer_loops = (argc > 1) ? atoi(argv[1]) : 50;
    int inner_loops = (argc > 2) ? atoi(argv[2]) : 100;
    int threshold = (argc > 3) ? atoi(argv[3]) : 25;
    
    if (outer_loops <= 0) outer_loops = 50;
    if (inner_loops <= 0) inner_loops = 100;
    
    /* Initialize data */
    initialize_data();
    
    /* Warm-up to potentially trigger different compilation paths */
    warm_up_computation();
    
    /* Main computation with nested loops */
    int result1 = complex_nested_loops(outer_loops, inner_loops, threshold);
    
    /* Another loop pattern */
    int result2 = struct_access_loop(outer_loops * 2);
    
    /* Loop with accumulation */
    int result3 = accumulate_loop(g_array_int, 
                                 (inner_loops < 1024) ? inner_loops : 1024, 
                                 result1);
    
    /* Final checksum */
    int final_result = result1 + result2 * 3 - result3;
    
    printf("Computation result: %d\n", final_result);
    printf("Breakdown: r1=%d, r2=%d, r3=%d\n", result1, result2, result3);
    
    return 0;
}
