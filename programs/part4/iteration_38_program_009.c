/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

/* Function with multiple loop-carried dependencies */
int loop_with_carried_deps(int n, int* restrict a, int* restrict b, int* restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependencies */
    int prod = 1;
    
    /* Loop with distance-1 dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: a[i] depends on a[i-1] */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Another distance-1 dependence through sum */
        sum = sum + a[i] * prod;
        
        /* Cross-iteration dependency through prod */
        prod = prod * 2;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum), "r"(prod) : "memory");
    }
    
    return sum + prod;
}

/* Function with nested loops and unrolling */
int nested_loop_with_unrolling(int n, int m, int* restrict x, int* restrict y) {
    volatile int acc1 = 0, acc2 = 0;
    
    /* Outer loop - will be unrolled */
    for (int i = 0; i < n; i += 2) {
        /* Inner loop with carried dependencies */
        for (int j = 1; j < m; j++) {
            /* Multiple distance-1 dependencies */
            x[j] = y[j-1] + x[j] * 3;
            y[j] = x[j-1] - y[j] / 2;
            
            /* Two separate accumulators with carried dependencies */
            acc1 = acc1 + x[j];
            acc2 = acc2 - y[j];
            
            /* Force dependence preservation */
            asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
        }
        
        /* Additional computation to create more ILP */
        x[0] = acc1 * acc2;
        y[0] = acc1 - acc2;
    }
    
    return acc1 + acc2;
}

/* Function with interleaved carried dependencies */
int interleaved_dependencies(int n, int* restrict arr1, int* restrict arr2, 
                            int* restrict arr3, int* restrict arr4) {
    volatile int sum_even = 0;
    volatile int sum_odd = 0;
    int temp = 1;
    
    /* Complex loop with multiple interleaved dependencies */
    for (int i = 2; i < n; i++) {
        /* Pattern 1: arr1[i] depends on arr1[i-2] (distance 2) */
        arr1[i] = arr1[i-2] * arr2[i] + arr3[i];
        
        /* Pattern 2: arr2[i] depends on arr2[i-1] (distance 1) */
        arr2[i] = arr2[i-1] + arr4[i] * temp;
        
        /* Pattern 3: Cross-dependency between arrays */
        arr3[i] = arr1[i-1] - arr2[i-1];
        
        /* Two separate accumulators with different update patterns */
        if (i % 2 == 0) {
            sum_even = sum_even + arr1[i] * arr2[i];
        } else {
            sum_odd = sum_odd - arr3[i] * arr4[i];
        }
        
        /* Update temp with carried dependency */
        temp = temp * 3 - arr1[i-1];
        
        /* Memory clobber to preserve all dependencies */
        asm volatile("" : : "r"(sum_even), "r"(sum_odd), "r"(temp) : "memory");
    }
    
    return sum_even + sum_odd + temp;
}

/* Function with runtime trip count (prevents unrolling) */
int runtime_trip_count(int start, int end, int* restrict data, int coeff) {
    volatile int result = 0;
    int prev = data[start];
    
    /* Loop count not known at compile time */
    for (int i = start + 1; i < end; i++) {
        /* Strong distance-1 dependence chain */
        int curr = prev * coeff + data[i];
        result = result + curr;
        prev = curr;
        
        /* Additional computation to increase ILP */
        data[i] = data[i-1] + data[i] * 2;
        
        /* Force the dependency to be preserved */
        asm volatile("" : : "r"(result), "r"(prev) : "memory");
    }
    
    return result;
}

/* Function with mixed operations for complex scheduling */
int mixed_operations(int n, int* restrict buf1, int* restrict buf2, 
                    int* restrict buf3, int* restrict buf4) {
    volatile int acc = 0;
    int shift_reg[3] = {0, 0, 0};
    
    /* Digital filter-like computation with tap delays */
    for (int i = 0; i < n; i++) {
        /* Update shift register (distance-1 dependencies) */
        shift_reg[2] = shift_reg[1];
        shift_reg[1] = shift_reg[0];
        shift_reg[0] = buf1[i];
        
        /* FIR filter computation with carried dependencies */
        int filter_out = shift_reg[0] * buf2[0] 
                       + shift_reg[1] * buf2[1] 
                       + shift_reg[2] * buf2[2];
        
        /* Feedback path (creates longer dependency chain) */
        buf3[i] = filter_out + (i > 0 ? buf3[i-1] / 4 : 0);
        
        /* Accumulator with carried dependency */
        acc = acc + buf3[i] * buf4[i];
        
        /* Non-linear operation to prevent simplifications */
        buf1[i] = (buf1[i] * buf1[i]) >> 4;
        
        /* Barrier to preserve all dependencies */
        asm volatile("" : : "r"(acc), "r"(filter_out) : "memory");
    }
    
    return acc;
}

int main() {
    const int SIZE = 1024;
    
    /* Allocate and initialize test arrays */
    int* a = (int*)malloc(SIZE * sizeof(int));
    int* b = (int*)malloc(SIZE * sizeof(int));
    int* c = (int*)malloc(SIZE * sizeof(int));
    int* x = (int*)malloc(SIZE * sizeof(int));
    int* y = (int*)malloc(SIZE * sizeof(int));
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    int* arr3 = (int*)malloc(SIZE * sizeof(int));
    int* arr4 = (int*)malloc(SIZE * sizeof(int));
    int* data = (int*)malloc(SIZE * sizeof(int));
    int* buf1 = (int*)malloc(SIZE * sizeof(int));
    int* buf2 = (int*)malloc(3 * sizeof(int));
    int* buf3 = (int*)malloc(SIZE * sizeof(int));
    int* buf4 = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = i % 100;
        b[i] = (i * 3) % 100;
        c[i] = (i * 5) % 100;
        x[i] = i % 50;
        y[i] = (i * 7) % 50;
        arr1[i] = i % 30;
        arr2[i] = (i * 11) % 30;
        arr3[i] = i % 20;
        arr4[i] = (i * 13) % 20;
        data[i] = i % 200;
        buf1[i] = i % 150;
        buf3[i] = i % 100;
        buf4[i] = (i * 17) % 100;
    }
    buf2[0] = 1; buf2[1] = 2; buf2[2] = 1;
    
    int total = 0;
    
    /* Call all test functions to ensure they're compiled */
    total += loop_with_carried_deps(SIZE, a, b, c);
    total += nested_loop_with_unrolling(64, 32, x, y);
    total += interleaved_dependencies(SIZE, arr1, arr2, arr3, arr4);
    total += runtime_trip_count(10, SIZE - 10, data, 3);
    total += mixed_operations(SIZE, buf1, buf2, buf3, buf4);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y);
    free(arr1); free(arr2); free(arr3); free(arr4);
    free(data);
    free(buf1); free(buf2); free(buf3); free(buf4);
    
    return 0;
}
