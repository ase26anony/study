/* sel-sched-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Global arrays for memory operations */
extern int global_arr[1024];
extern short global_short_arr[2048];
int global_arr[1024];
short global_short_arr[2048];

/* Non-inlineable functions */
__attribute__((noinline)) int non_inline_func(int x, int y) {
    return x * y + (x ^ y);
}

__attribute__((noinline)) unsigned noinline_unsigned(unsigned a, unsigned b) {
    asm volatile ("" : : : "memory");
    return (a + b) * (a - b);
}

/* Pure function for loop-variant calls */
__attribute__((const)) int pure_func(int x) {
    return x * 3 + 7;
}

/* Helper function in separate compilation unit */
extern int helper_compute(int idx, int mod);

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 100;
volatile int volatile_seed = 42;

/* Main computation with nested loops */
int compute_checksum(int limit1, int limit2, int threshold) {
    register int acc = 0;  /* Hint for register allocation */
    int i, j, k;
    unsigned short us_temp;
    int local_arr[64];
    
    /* Initialize local array */
    for (k = 0; k < 64; ++k) {
        local_arr[k] = k * 3;
    }
    
    /* Outer loop with varying trip count */
    for (i = 0; i < limit1; ++i) {
        int outer_var = i * 2;
        unsigned u_i = (unsigned)i;
        
        /* Middle loop with conditional execution */
        if (__builtin_expect(i > threshold, 0)) {
            for (j = 0; j < limit2; ++j) {
                /* Mix of arithmetic operations */
                int temp = i * j + acc;
                unsigned u_temp = u_i * (unsigned)j;
                
                /* Loop-carried dependency */
                acc = global_arr[(i * 16 + j) % 1024] + acc;
                
                /* Conditional branch creating control flow */
                if (i % 8 == 0) {
                    temp = non_inline_func(temp, j);
                    /* Memory barrier */
                    asm volatile ("" ::: "memory");
                }
                
                /* Memory operations with potential aliasing */
                global_short_arr[(i + j) % 2048] = (short)(temp % 65536);
                
                /* Pure function call with loop-variant args */
                int pure_result = pure_func(temp);
                
                /* Different data type computations */
                us_temp = (unsigned short)(u_temp % 65535);
                acc += pure_result + (int)us_temp;
                
                /* Function call from another TU */
                if (j % 16 == 0) {
                    acc += helper_compute(i, j);
                }
            }
        } else {
            /* Different path with inner loop */
            for (j = 5; j < limit2 / 2; ++j) {
                short s_acc = (short)(acc % 32768);
                int idx = (i * 32 + j * 3) % 1024;
                
                /* Memory access pattern */
                int mem_val = global_arr[idx];
                acc = mem_val - local_arr[j % 64] + acc;
                
                /* Unsigned computation */
                unsigned u_acc = noinline_unsigned((unsigned)mem_val, (unsigned)j);
                acc += (int)(u_acc % 1000);
                
                /* Another conditional */
                if (j % 7 == 0) {
                    acc = acc ^ (i * j);
                }
            }
        }
        
        /* Variable with different scope */
        {
            int scope_var = outer_var * 3;
            acc += scope_var;
            /* Multiple uses of same variable */
            scope_var = pure_func(scope_var);
            acc -= scope_var / 2;
        }
    }
    
    return acc;
}

/* Warm-up function */
__attribute__((noinline)) void warmup_computation(void) {
    int dummy = 0;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            dummy += i * j + non_inline_func(i, j);
        }
    }
    /* Use dummy to prevent optimization */
    if (dummy == 0) {
        printf("Warmup complete\n");
    }
}

int main(int argc, char *argv[]) {
    /* Initialize seed data */
    int seed = volatile_seed;
    for (int i = 0; i < 1024; ++i) {
        global_arr[i] = (seed * 1103515245 + 12345) % 65536;
        seed = global_arr[i];
    }
    for (int i = 0; i < 2048; ++i) {
        global_short_arr[i] = (short)((i * 32719) % 32768);
    }
    
    /* Get loop bounds from arguments or volatile */
    int limit1 = (argc > 1) ? atoi(argv[1]) : volatile_bound;
    int limit2 = (argc > 2) ? atoi(argv[2]) : volatile_bound / 2;
    int threshold = (argc > 3) ? atoi(argv[3]) : limit1 / 3;
    
    if (limit1 <= 0) limit1 = 100;
    if (limit2 <= 0) limit2 = 50;
    if (threshold < 0) threshold = limit1 / 4;
    
    printf("Running with limits: %d, %d, threshold: %d\n", 
           limit1, limit2, threshold);
    
    /* Warm-up loop */
    warmup_computation();
    
    /* Main computation */
    int result = compute_checksum(limit1, limit2, threshold);
    
    printf("Result checksum: %d\n", result);
    printf("Hex: 0x%08x\n", (unsigned int)result);
    
    return 0;
}
