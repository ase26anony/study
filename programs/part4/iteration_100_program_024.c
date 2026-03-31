/* test_haifa_sched.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 256
#define ITERATIONS 1000

/* Non-inlineable function to force scheduling around calls */
int __attribute__((noinline)) external_func(int x) {
    volatile int sink = x * 2;  /* Prevent optimization */
    return sink + 1;
}

/* Another non-inlineable function with side effects */
void __attribute__((noinline)) process_chunk(int *arr, int start, int end, int factor) {
    volatile int barrier;
    for (int i = start; i < end; i++) {
        /* Complex dependency chain */
        int prev = (i > 0) ? arr[i-1] : 1;
        int temp = prev * factor;
        
        /* Inline assembly to create scheduling barriers */
        asm volatile("" : "+r"(temp) : : "memory");
        
        int func_result = external_func(i);
        
        /* More dependencies */
        temp = temp + func_result;
        temp = temp ^ (temp >> 3);
        
        /* Memory store with dependency */
        arr[i] = temp;
        
        /* Artificial memory barrier */
        barrier = arr[i];
    }
}

/* Function with complex control flow */
int __attribute__((noinline)) schedule_intensive_work(int *data, int size) {
    int sum = 0;
    volatile int sink;
    
    /* Multiple basic blocks with different scheduling pressures */
    for (int i = 0; i < size; i++) {
        if (i % 3 == 0) {
            /* Branch 1: Floating point intensive */
            double a = data[i] * 1.5;
            double b = a * a;
            double c = b / 3.14159;
            
            /* Inline assembly with register constraints */
            int result;
            asm volatile ("cvtsd2si %1, %0" : "=r"(result) : "x"(c));
            
            /* Create dependency chain */
            result = result + external_func(i);
            data[i] = result;
            
            /* Memory clobber */
            asm volatile("" ::: "memory");
            
        } else if (i % 3 == 1) {
            /* Branch 2: Integer and memory intensive */
            int x = data[i];
            int y = data[(i * 7) % size];
            
            /* Complex integer operations */
            x = (x * y) + (x >> 4);
            x = x ^ (x << 3);
            x = x % 1023;
            
            /* Function call in the middle */
            x = x + external_func(x);
            
            /* Store with volatile read */
            data[i] = x;
            sink = data[i];
            
        } else {
            /* Branch 3: Mixed operations */
            int val = data[i];
            
            /* Series of dependent operations */
            val = val * 3;
            asm volatile("addl $1, %0" : "+r"(val));
            val = val & 0xFF;
            val = val | (val << 8);
            
            /* Conditional within branch */
            if (val > 1000) {
                val = val / 2;
                asm volatile("" : "+r"(val) : : "cc");
            }
            
            data[i] = val;
        }
        
        /* Accumulate result with dependency */
        sum += data[i];
        
        /* Periodic function pointer call */
        if (i % 32 == 0) {
            int (*func_ptr)(int) = external_func;
            sum += func_ptr(i);
        }
    }
    
    return sum;
}

/* Function with nested loops and pointer chasing */
void __attribute__((noinline)) pointer_chasing_work(int **arr, int rows, int cols) {
    volatile int accumulator = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row = arr[i];
        
        /* Unrolled loop with dependencies */
        for (int j = 0; j < cols - 3; j += 4) {
            /* Create dependency chain across unrolled iterations */
            int t0 = row[j];
            int t1 = t0 + row[j+1];
            int t2 = t1 * row[j+2];
            int t3 = t2 - row[j+3];
            
            /* Inline assembly barriers between dependent ops */
            asm volatile("" : "+r"(t0), "+r"(t1), "+r"(t2), "+r"(t3));
            
            row[j] = t0;
            row[j+1] = t1;
            row[j+2] = t2;
            row[j+3] = t3;
            
            accumulator += t3;
        }
        
        /* Function call with side effect */
        external_func(i);
    }
    
    /* Use the volatile to prevent dead code elimination */
    if (accumulator == 0) {
        printf("Never happens\n");
    }
}

int main() {
    int *data = (int*)malloc(N * sizeof(int));
    int **matrix = (int**)malloc(16 * sizeof(int*));
    
    /* Initialize data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        data[i] = rand() % 1000;
    }
    
    for (int i = 0; i < 16; i++) {
        matrix[i] = (int*)malloc(64 * sizeof(int));
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = rand() % 500;
        }
    }
    
    int total_sum = 0;
    
    /* Execute multiple times to increase scheduling opportunities */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call different scheduling-intensive functions */
        total_sum += schedule_intensive_work(data, N);
        
        /* Process chunks with different factors */
        process_chunk(data, 0, N/2, iter % 7 + 2);
        process_chunk(data, N/2, N, iter % 5 + 3);
        
        /* Pointer chasing work */
        pointer_chasing_work(matrix, 16, 64);
        
        /* Mix in some direct computation */
        for (int i = 0; i < N; i++) {
            data[i] = (data[i] * 1103515245 + 12345) & 0x7fffffff;
            asm volatile("" : "+r"(data[i]));
        }
    }
    
    /* Final validation */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum ^= data[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Total sum: %d\n", total_sum);
    
    /* Cleanup */
    for (int i = 0; i < 16; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(data);
    
    return 0;
}
