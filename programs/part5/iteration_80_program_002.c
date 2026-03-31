/* sel_sched_test.c - Test program to trigger selective scheduling dump logic */
#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
int global_arr[1024];
unsigned short global_short_arr[2048];
volatile int volatile_seed = 42;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_func(int x, int y) {
    return x * y + (x ^ y);
}

__attribute__((noinline, const)) int pure_func(int x) {
    return (x * 3) / 2;
}

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int compute_checksum(int* arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; ++i) {
        sum = arr[i] + sum;  /* Loop-carried dependency */
        if (__builtin_expect((sum & 1) == 0, 0)) {
            sum += pure_func(i);  /* Call pure function */
        }
    }
    return sum;
}

/* Secondary computation with nested loops */
__attribute__((noinline)) int nested_loop_computation(int limit1, int limit2) {
    int acc = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < limit1; ++i) {
        int temp = i * 2;
        
        /* Conditional inner loop execution */
        if (i > limit1 / 4) {
            /* Inner loop with different data type */
            for (unsigned short j = 0; j < (unsigned short)limit2; ++j) {
                /* Mix of arithmetic operations */
                int val = (int)j * 3 + (i % 7);
                acc += val;
                
                /* Memory operation with potential aliasing */
                global_short_arr[j] = (unsigned short)(val & 0xFFFF);
                
                /* Optimization barrier */
                asm volatile ("" ::: "memory");
                
                /* Conditional branch */
                if (i % 5 == 0) {
                    acc += noinline_func(i, j);
                }
                
                /* Multiple uses of same variable */
                reg_acc = acc;
                acc = reg_acc + pure_func(j);
            }
        } else {
            /* Different path with function calls */
            acc += noinline_func(i, i + 1);
        }
        
        /* Another optimization barrier */
        asm volatile ("" ::: "memory");
    }
    
    return acc;
}

/* Triple nested loop structure */
__attribute__((noinline)) int triple_nested_loop(int n1, int n2, int n3) {
    int total = 0;
    
    /* First level loop */
    for (int a = 0; a < n1; ++a) {
        int local_sum = 0;
        
        /* Second level loop */
        for (int b = 0; b < n2; ++b) {
            /* Third level loop with different counter type */
            for (unsigned c = 0; c < (unsigned)n3; ++c) {
                /* Complex expression with multiple operations */
                int idx = (a * b + c) % 1024;
                int val = global_arr[idx] + (int)c * 2;
                
                /* Conditional with __builtin_expect */
                if (__builtin_expect((val % 13) == 0, 1)) {
                    local_sum += pure_func(val);
                } else {
                    local_sum += val;
                }
                
                /* Memory store */
                global_arr[idx] = val % 256;
                
                /* Periodic optimization barrier */
                if (c % 8 == 0) {
                    asm volatile ("" ::: "memory");
                }
            }
            
            /* Function call with loop-variant arguments */
            if (b % 3 == 0) {
                local_sum += noinline_func(a, b);
            }
        }
        
        total += local_sum;
        
        /* Variable with different scope */
        {
            int scope_var = a * 7;
            total += scope_var;
        }
    }
    
    return total;
}

/* Warm-up function to trigger compilation paths */
__attribute__((noinline)) void warm_up_computation(void) {
    int warm_sum = 0;
    volatile int warm_limit = volatile_seed % 10 + 5;
    
    for (int i = 0; i < warm_limit; ++i) {
        warm_sum += i * 2;
        if (i % 4 == 0) {
            warm_sum += pure_func(i);
        }
        asm volatile ("" ::: "memory");
    }
    
    /* Use the result to prevent optimization */
    global_arr[0] = warm_sum % 256;
}

/* Main computation function */
__attribute__((noinline)) int core_computation(int param1, int param2, int param3) {
    int result = 0;
    
    /* Call different loop structures */
    result += nested_loop_computation(param1, param2);
    result += triple_nested_loop(param2 / 2, param3, param1);
    
    /* Another loop with array access */
    int arr[128];
    for (int i = 0; i < 128; ++i) {
        arr[i] = (i * 3 + param1) % 100;
    }
    
    result += compute_checksum(arr, 128);
    
    return result;
}

/* Initialize data with pseudo-random values */
void init_data(void) {
    /* Simple LCG for pseudo-random values */
    unsigned int lcg_state = 123456789;
    
    for (int i = 0; i < 1024; ++i) {
        lcg_state = lcg_state * 1103515245 + 12345;
        global_arr[i] = (int)(lcg_state % 1000);
    }
    
    for (int i = 0; i < 2048; ++i) {
        lcg_state = lcg_state * 1103515245 + 12345;
        global_short_arr[i] = (unsigned short)(lcg_state % 65536);
    }
}

int main(int argc, char* argv[]) {
    /* Use command line arguments or defaults for variability */
    int param1 = (argc > 1) ? atoi(argv[1]) : 50;
    int param2 = (argc > 2) ? atoi(argv[2]) : 30;
    int param3 = (argc > 3) ? atoi(argv[3]) : 20;
    
    /* Ensure parameters are reasonable */
    if (param1 < 10) param1 = 10;
    if (param2 < 10) param2 = 10;
    if (param3 < 10) param3 = 10;
    
    printf("Starting selective scheduling test...\n");
    printf("Parameters: %d, %d, %d\n", param1, param2, param3);
    
    /* Initialize data */
    init_data();
    
    /* Warm-up to trigger compilation paths */
    printf("Performing warm-up...\n");
    warm_up_computation();
    
    /* Main computation */
    printf("Running main computation...\n");
    int result = core_computation(param1, param2, param3);
    
    /* Verification output */
    printf("Result checksum: %d\n", result);
    printf("Global array[0]: %d\n", global_arr[0]);
    printf("Global short array[100]: %d\n", (int)global_short_arr[100]);
    
    return 0;
}
