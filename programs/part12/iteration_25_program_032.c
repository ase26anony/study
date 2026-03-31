/* test_ddg_coverage.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100

/* Prevent optimization and create dependencies */
volatile int volatile_var = 0;
int global_array[SIZE];

/* Functions to prevent optimization */
__attribute__((noinline)) void use_value(int val) {
    volatile_var = val;
}

__attribute__((noinline)) int* get_array(void) {
    return global_array;
}

/* Test 1: Flow dependency (RAW) with carried dependency */
__attribute__((noinline)) int test_flow_dependency(void) {
    int array[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i + 1;
    }
    
    /* Flow dependency: sum depends on previous iteration's sum */
    for (int i = 0; i < SIZE; i++) {
        sum += array[i];  /* RAW: read array[i], write sum */
        array[i] = sum;   /* WAW: write array[i] */
    }
    
    /* Anti-dependency within same iteration */
    int temp = sum;
    sum = array[SIZE-1];
    array[SIZE-1] = temp;  /* WAR: read array[SIZE-1], then write it */
    
    use_value(sum);
    return sum;
}

/* Test 2: Anti-dependency (WAR) pattern */
__attribute__((noinline)) int test_anti_dependency(void) {
    int a[SIZE], b[SIZE];
    int result = 0;
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = SIZE - i;
    }
    
    /* Anti-dependency: read then write same location */
    for (int i = 1; i < SIZE - 1; i++) {
        int temp = a[i];      /* Read a[i] */
        a[i] = b[i] + 1;      /* Write a[i] - WAR dependency */
        b[i] = temp * 2;      /* Write b[i] */
        result += a[i] + b[i];
    }
    
    /* Multiple writes to same location - output dependency */
    a[0] = result;
    a[0] = a[0] * 3;          /* WAW: multiple writes to a[0] */
    
    use_value(result);
    return result;
}

/* Test 3: Output dependency (WAW) and complex flow */
__attribute__((noinline)) int test_output_dependency(void) {
    int data[SIZE];
    int accum = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 3) % 7;
    }
    
    /* Multiple writes to same elements with output dependencies */
    for (int i = 0; i < SIZE; i++) {
        data[i] = data[i] + 1;      /* Write 1 */
        if (i % 3 == 0) {
            data[i] = data[i] * 2;  /* Write 2 - WAW on data[i] */
        }
        accum += data[i];
        
        /* Flow dependency with distance */
        if (i > 0) {
            data[i] += data[i-1];   /* RAW: data[i-1] from previous iteration */
        }
    }
    
    /* More WAW dependencies */
    data[SIZE/2] = accum;
    data[SIZE/2] = data[SIZE/2] / 2;
    
    use_value(accum);
    return accum;
}

/* Test 4: Nested loops with cross-iteration dependencies */
__attribute__((noinline)) int test_nested_dependency(void) {
    int matrix[N][N];
    int total = 0;
    
    /* Initialize matrix */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = i * N + j;
        }
    }
    
    /* Nested loop with flow dependency across outer iterations */
    for (int i = 1; i < N; i++) {
        for (int j = 0; j < N; j++) {
            /* Flow dependency: uses value from previous i iteration */
            matrix[i][j] = matrix[i][j] + matrix[i-1][j];
            total += matrix[i][j];
        }
    }
    
    /* Inner loop with anti-dependency */
    for (int i = 0; i < N; i++) {
        int row_sum = 0;
        for (int j = 0; j < N; j++) {
            int old_val = matrix[i][j];  /* Read */
            matrix[i][j] = row_sum + j;  /* Write - WAR */
            row_sum += old_val;
        }
        total += row_sum;
    }
    
    use_value(total);
    return total;
}

/* Test 5: Mixed data types and operations */
__attribute__((noinline)) int test_mixed_types(void) {
    float f_array[SIZE];
    double d_array[SIZE];
    int i_array[SIZE];
    int result = 0;
    
    for (int i = 0; i < SIZE; i++) {
        f_array[i] = i * 1.5f;
        d_array[i] = i * 2.5;
        i_array[i] = i;
    }
    
    /* Mixed type dependencies */
    for (int i = 1; i < SIZE; i++) {
        /* Flow dependency with type conversion */
        f_array[i] = f_array[i-1] * 1.1f;  /* RAW on f_array[i-1] */
        
        /* Anti-dependency with different types */
        double temp = d_array[i];          /* Read d_array[i] */
        d_array[i] = f_array[i] * 2.0;     /* Write d_array[i] - WAR */
        
        /* Output dependency on integer array */
        i_array[i] = (int)temp;
        i_array[i] = i_array[i] + (int)d_array[i];  /* WAW on i_array[i] */
        
        result += i_array[i];
    }
    
    use_value(result);
    return result;
}

/* Test 6: Control flow with dependencies */
__attribute__((noinline)) int test_control_flow_deps(void) {
    int data[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 10;
    }
    
    /* Complex control flow creating various dependencies */
    for (int i = 1; i < SIZE; i++) {
        if (i % 2 == 0) {
            /* Even indices: flow dependency chain */
            data[i] = data[i] + data[i-1];  /* RAW on data[i-1] */
            sum += data[i];
        } else {
            /* Odd indices: anti-dependency pattern */
            int old = data[i];              /* Read data[i] */
            data[i] = sum * 2;              /* Write data[i] - WAR */
            sum += old;
        }
        
        /* Additional output dependency */
        if (i % 3 == 0) {
            data[i] = data[i] * 3;          /* WAW on data[i] */
        }
    }
    
    /* Loop with carried dependency and if condition */
    int acc = 0;
    for (int i = 0; i < SIZE; i++) {
        if (data[i] > 50) {
            acc = acc + data[i];  /* Flow dependency through acc */
        } else {
            data[i] = acc;        /* Write data[i] depending on acc */
        }
    }
    
    use_value(sum + acc);
    return sum + acc;
}

/* Test 7: Pointer aliasing creating ambiguous dependencies */
__attribute__((noinline)) int test_pointer_aliasing(void) {
    int buffer[2 * SIZE];
    int *p1 = &buffer[0];
    int *p2 = &buffer[SIZE/2];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 2 * SIZE; i++) {
        buffer[i] = i;
    }
    
    /* Pointer access creating potential dependencies */
    for (int i = 0; i < SIZE; i++) {
        /* These may alias, creating potential dependencies */
        p1[i] = p2[i] + 1;        /* Could be RAW if p1 and p2 overlap */
        p2[i] = p1[i] * 2;        /* Could be WAR/WAW */
        sum += p1[i] + p2[i];
    }
    
    /* Force pointer chasing dependency */
    int *ptr = &buffer[0];
    for (int i = 0; i < SIZE; i++) {
        *ptr = *ptr + *(ptr + 1);  /* Flow dependency through memory */
        ptr++;
        sum += *ptr;
    }
    
    use_value(sum);
    return sum;
}

int main(void) {
    int total_result = 0;
    
    srand(time(NULL));
    
    printf("Starting DDG coverage tests...\n");
    
    /* Run all tests to trigger various DDG edge creations */
    total_result += test_flow_dependency();
    total_result += test_anti_dependency();
    total_result += test_output_dependency();
    total_result += test_nested_dependency();
    total_result += test_mixed_types();
    total_result += test_control_flow_deps();
    total_result += test_pointer_aliasing();
    
    printf("Total result: %d\n", total_result);
    printf("Volatile var: %d\n", volatile_var);
    
    /* Use the result to prevent dead code elimination */
    if (total_result > 0) {
        printf("Tests completed successfully.\n");
    } else {
        printf("Warning: Result is zero.\n");
    }
    
    return total_result != 0 ? 0 : 1;
}
