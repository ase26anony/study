/* test-annotate-expr.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
extern void use(int val);
void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Function to create data dependencies */
int process_data(int *arr, int n) {
    int sum = 0;
    
    /* 1. annot_expr_no_vector_kind - #pragma GCC ivdep */
    #pragma GCC ivdep
    for (int i = 1; i < n; i++) {
        /* Potential dependency broken by ivdep */
        arr[i] = arr[i] + arr[i-1];
        sum += arr[i];
    }
    use(sum);
    
    /* 2. annot_expr_vector_kind - #pragma GCC vector */
    int b[100];
    #pragma GCC vector
    for (int i = 0; i < n && i < 100; i++) {
        b[i] = arr[i] * 2 + 7;
        sum += b[i];
    }
    use(sum);
    
    /* 3. annot_expr_parallel_kind - #pragma GCC parallel */
    int partial[4] = {0, 0, 0, 0};
    #pragma GCC parallel
    for (int i = 0; i < n; i++) {
        /* Independent accumulations */
        partial[i % 4] += arr[i] * 3;
    }
    for (int i = 0; i < 4; i++) {
        sum += partial[i];
    }
    use(sum);
    
    return sum;
}

/* Function with maybe-infinite loop annotation */
int unrolled_computation(int n) {
    int result = 1;
    int values[10];
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        values[i] = i + 1;
    }
    
    /* 4. annot_expr_maybe_infinite_kind - #pragma GCC unroll */
    #pragma GCC unroll
    for (int i = 0; i < n && i < 10; i++) {
        result *= values[i] + 5;
        use(result);
    }
    
    return result;
}

/* Nested loop with annotation */
void nested_annotated_loop(int size) {
    int matrix[50][50];
    volatile int sink;
    
    /* Initialize */
    for (int i = 0; i < 50 && i < size; i++) {
        for (int j = 0; j < 50 && j < size; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Outer loop with ivdep */
    #pragma GCC ivdep
    for (int i = 1; i < size && i < 49; i++) {
        /* Inner loop with vector hint */
        #pragma GCC vector
        for (int j = 0; j < size && j < 50; j++) {
            matrix[i][j] += matrix[i-1][j] * 2;
        }
        sink = matrix[i][0];
    }
    (void)sink;
}

/* Conditional context with annotated loop */
int conditional_annotated(int flag, int *data, int n) {
    int total = 0;
    
    if (flag > 0) {
        /* Parallel annotation in conditional context */
        #pragma GCC parallel
        for (int i = 0; i < n; i++) {
            total += data[i] * data[i];
            use(total);
        }
    } else {
        /* Vector annotation in else branch */
        #pragma GCC vector
        for (int i = 0; i < n; i++) {
            total -= data[i];
            use(total);
        }
    }
    
    /* Maybe-infinite in loop with variable bound */
    int m = (n > 5) ? 5 : n;
    #pragma GCC unroll
    for (int i = 0; i < m; i++) {
        total += i * 100;
        use(total);
    }
    
    return total;
}

int main(void) {
    int data[100];
    int result = 0;
    
    /* Initialize data with pattern */
    for (int i = 0; i < 100; i++) {
        data[i] = (i * 7 + 3) % 19;
    }
    
    /* Test all annotation kinds */
    result += process_data(data, 50);
    
    /* Test maybe-infinite annotation */
    result += unrolled_computation(8);
    
    /* Test nested annotated loops */
    nested_annotated_loop(40);
    
    /* Test conditional contexts */
    result += conditional_annotated(1, data, 30);
    result += conditional_annotated(0, data, 20);
    
    /* Final result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
