/* test_haifa_sched.c - Program to trigger instruction scheduler cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to prevent inlining */
int __attribute__((noinline)) external_compute(int x, int y) {
    return (x * y) ^ (x + y);
}

/* Another non-inlineable function */
void __attribute__((noinline)) side_effect_func(volatile int* ptr) {
    *ptr += 1;
    asm volatile("" ::: "memory");
}

/* Complex function with schedule-intensive operations */
int schedule_intensive_loop(int* arr, int size, int factor) {
    volatile int sink = 0;
    int result = 0;
    int i;
    
    /* Initial computation with dependencies */
    int temp = arr[0];
    sink = temp;
    temp = sink + factor;
    
    /* Loop with mixed operations and dependencies */
    for (i = 1; i < size; ++i) {
        /* Create data dependency chain */
        int prev = arr[i-1];
        int curr = arr[i];
        
        /* Memory barrier to force scheduling constraints */
        asm volatile("" ::: "memory");
        
        /* Arithmetic with varying latencies */
        int prod = prev * factor;
        int sum = prod + curr;
        
        /* Function call with side effects */
        side_effect_func(&sink);
        
        /* More arithmetic with dependencies */
        int shifted = sum << 2;
        int masked = shifted & 0xFF;
        
        /* External function call */
        int ext = external_compute(masked, i);
        
        /* Conditional store with volatile */
        if (ext > 100) {
            arr[i] = ext;
            sink = arr[i];
        } else {
            arr[i] = masked;
            sink = arr[i] * 2;
        }
        
        /* More operations to extend basic block */
        result += arr[i];
        result ^= (i * 7);
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
    }
    
    return result;
}

/* Function with complex control flow */
int multi_branch_scheduler(int x, int y, int z) {
    volatile int vsink;
    int result = 0;
    
    /* Branch 1: Floating point intensive */
    if (x > y) {
        double a = x * 1.5;
        double b = y * 2.5;
        vsink = (int)a;
        
        for (int i = 0; i < 8; ++i) {
            a = a * b + i;
            b = b - 0.5;
            asm volatile("" ::: "memory");
        }
        
        result = (int)a + (int)b;
        vsink = result;
    }
    /* Branch 2: Integer and memory intensive */
    else if (x > z) {
        int arr[16];
        for (int i = 0; i < 16; ++i) {
            arr[i] = x + i * y;
        }
        
        /* Complex dependency chain */
        int acc = 0;
        for (int i = 0; i < 16; ++i) {
            acc = acc * 3 + arr[i];
            side_effect_func(&vsink);
            acc ^= (i << 3);
        }
        
        result = acc;
        vsink = result;
    }
    /* Branch 3: Mixed operations */
    else {
        result = x;
        for (int i = 0; i < 12; ++i) {
            result = external_compute(result, y + i);
            result += z * i;
            
            /* Inline assembly with register constraints */
            asm volatile("addl %1, %0" : "+r"(result) : "r"(i));
            asm volatile("" ::: "memory");
        }
    }
    
    return result;
}

/* Main driver */
int main() {
    const int SIZE = 128;
    int* data = (int*)malloc(SIZE * sizeof(int));
    int i, total = 0;
    
    /* Initialize data */
    srand(time(NULL));
    for (i = 0; i < SIZE; ++i) {
        data[i] = rand() % 256;
    }
    
    /* Call schedule-intensive functions multiple times */
    for (i = 0; i < 3; ++i) {
        int factor = 3 + i;
        
        /* First intensive function */
        int r1 = schedule_intensive_loop(data, SIZE, factor);
        
        /* Second with complex control flow */
        int r2 = multi_branch_scheduler(data[0], data[1], data[2]);
        
        total += r1 + r2;
        
        /* Modify data for next iteration */
        for (int j = 0; j < SIZE; j += 4) {
            data[j] = (data[j] * factor) ^ r1;
        }
    }
    
    /* Final validation */
    int checksum = 0;
    for (i = 0; i < SIZE; ++i) {
        checksum ^= data[i];
    }
    checksum ^= total;
    
    printf("Result checksum: %d\n", checksum);
    
    free(data);
    return 0;
}
