/* test_haifa_sched.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 256
#define M 128

/* Non-inlineable function to force scheduling considerations */
__attribute__((noinline)) int external_func(int x) {
    volatile int sink = x * 2;  /* Prevent optimization */
    return sink + (x >> 1);
}

/* Another non-inlineable function with side effects */
__attribute__((noinline)) void process_chunk(int *arr, int start, int end, int factor) {
    volatile int barrier;
    for (int i = start; i < end; ++i) {
        /* Create data dependencies */
        int prev = (i > start) ? arr[i-1] : 1;
        
        /* Mix of operations with different latencies */
        int temp = prev * factor;
        
        /* Inline assembly to create scheduling barriers */
        asm volatile("" : "+r"(temp) : : "memory");
        
        int func_result = external_func(i);
        
        /* More arithmetic with dependencies */
        arr[i] = (temp + func_result) ^ (temp >> 3);
        
        /* Another scheduling barrier */
        asm volatile("" ::: "memory");
        
        barrier = arr[i];  /* Volatile store */
    }
}

/* Function with complex control flow */
__attribute__((noinline)) int schedule_intensive_function(int *data, int size, int threshold) {
    int sum = 0;
    volatile int sink;
    
    for (int i = 0; i < size; ++i) {
        /* Multiple basic blocks with different operations */
        if (data[i] < threshold) {
            /* Branch 1: Floating point operations */
            float fval = data[i] * 0.5f;
            int ival = (int)fval;
            
            /* Inline assembly with register constraints */
            asm volatile("imul %1, %0" : "+r"(ival) : "r"(i));
            
            /* Memory operation */
            data[i] = ival + external_func(i);
            
            /* Create dependency chain */
            for (int j = 0; j < 3; ++j) {
                data[i] = (data[i] * 17) ^ 0x5A5A;
                asm volatile("" ::: "memory");
            }
        } else if (data[i] < threshold * 2) {
            /* Branch 2: Integer operations with memory */
            int val = data[i];
            
            /* Series of dependent operations */
            val = (val << 3) | (val >> 5);
            sink = val;  /* Volatile store */
            val = sink + i;  /* Volatile load dependency */
            
            /* Function call in middle of dependency chain */
            val += external_func(val);
            
            data[i] = val ^ 0x12345678;
            
            /* More operations */
            asm volatile("ror $7, %0" : "+r"(data[i]));
        } else {
            /* Branch 3: Mixed operations */
            long long lval = data[i];
            lval = lval * lval;
            lval = lval % 10007;
            
            /* Inline assembly with multiple constraints */
            asm volatile("add %%rax, %%rcx" 
                        : "=c"(lval) 
                        : "a"(i), "c"(lval)
                        : /* clobbers */);
            
            data[i] = (int)lval;
        }
        
        /* Common path with more operations */
        sum += data[i];
        
        /* Periodic function call */
        if ((i & 0x7) == 0) {
            sum = external_func(sum);
        }
    }
    
    return sum;
}

/* Function pointer to prevent inlining analysis */
typedef int (*compute_func_t)(int*, int, int);
compute_func_t func_ptr = schedule_intensive_function;

int main() {
    /* Allocate and initialize data */
    int *data = (int*)malloc(N * sizeof(int));
    int *buffer = (int*)malloc(M * sizeof(int));
    
    if (!data || !buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < N; ++i) {
        data[i] = rand() % 1000;
    }
    for (int i = 0; i < M; ++i) {
        buffer[i] = i * 3;
    }
    
    /* Process data with schedule-intensive function */
    int result1 = func_ptr(data, N, 500);
    
    /* Process buffer with different function */
    process_chunk(buffer, 0, M, 7);
    
    /* Second pass with different parameters */
    int result2 = schedule_intensive_function(buffer, M, 250);
    
    /* Mix results */
    int final_result = result1 ^ result2;
    
    /* Validate results aren't obviously wrong */
    if (final_result == 0) {
        /* This is unlikely but handle it */
        final_result = 1;
    }
    
    /* Use the result to prevent dead code elimination */
    volatile int output = final_result;
    
    /* Clean up */
    free(data);
    free(buffer);
    
    printf("Result: %d\n", output);
    
    return 0;
}
