#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100

/* Prevent optimization and create dependencies */
volatile int volatile_var = 0;
int global_array[SIZE];

/* Functions to prevent optimization */
__attribute__((noinline)) int use_value(int val) {
    volatile_var = val;
    return val + 1;
}

__attribute__((noinline)) void modify_array(int* arr, int idx, int val) {
    arr[idx] = val + volatile_var;
}

/* Test 1: Flow dependency (RAW) - carried across iterations */
__attribute__((noinline)) int test_flow_dependency() {
    int array[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i % 100;
    }
    
    /* Main loop with flow dependency */
    for (int i = 1; i < SIZE; i++) {
        /* Flow dependency: array[i] depends on array[i-1] */
        array[i] = array[i-1] + array[i] * 2;
        sum += array[i];
    }
    
    /* Additional flow dependency with volatile */
    int acc = sum;
    for (int i = 0; i < N; i++) {
        acc = acc + volatile_var;  /* Flow dependency through volatile */
        array[i % SIZE] = acc;
    }
    
    return sum + acc;
}

/* Test 2: Anti-dependency (WAR) */
__attribute__((noinline)) int test_anti_dependency() {
    int array[SIZE];
    int temp[SIZE];
    int result = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i;
        temp[i] = SIZE - i;
    }
    
    /* Loop with anti-dependency */
    for (int i = 0; i < SIZE - 1; i++) {
        int read_val = array[i];          /* Read array[i] */
        array[i] = temp[i] * 3;           /* Overwrite array[i] - WAR with previous read */
        result += read_val + array[i];
        
        /* Another anti-dependency pattern */
        float fval = (float)array[i];
        array[i] = (int)fval * 2;         /* WAR: array[i] read as float, then overwritten */
    }
    
    return result;
}

/* Test 3: Output dependency (WAW) */
__attribute__((noinline)) int test_output_dependency() {
    int array[SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        array[i] = 1;
    }
    
    /* Loop with output dependencies */
    for (int i = 0; i < SIZE; i++) {
        /* Multiple writes to same location - WAW */
        array[i] = i * 2;
        if (i % 3 == 0) {
            array[i] = array[i] + volatile_var;  /* Another write - WAW */
        }
        array[i] = array[i] * 3;                 /* Third write - WAW */
        
        sum += array[i];
    }
    
    /* Mixed WAW with different data types */
    double darray[SIZE/2];
    for (int i = 0; i < SIZE/2; i++) {
        darray[i] = i * 1.5;
        darray[i] = darray[i] * 2.0;  /* WAW with double */
        array[i*2] = (int)darray[i];  /* Store as int */
    }
    
    return sum;
}

/* Test 4: Nested loops with dependencies */
__attribute__((noinline)) int test_nested_dependency() {
    int matrix[64][64];
    int result = 0;
    
    /* Initialize matrix */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = i * 64 + j;
        }
    }
    
    /* Nested loops with flow dependency across outer iterations */
    for (int i = 1; i < 63; i++) {
        for (int j = 1; j < 63; j++) {
            /* Dependencies on previous iteration of both loops */
            matrix[i][j] = matrix[i-1][j] + matrix[i][j-1] - matrix[i-1][j-1];
            result += matrix[i][j];
        }
    }
    
    /* Another nested pattern with anti-dependency */
    for (int i = 0; i < 63; i++) {
        int row_sum = 0;
        for (int j = 0; j < 64; j++) {
            row_sum += matrix[i][j];      /* Read */
            matrix[i][j] = row_sum;       /* Write - WAR */
        }
        result += row_sum;
    }
    
    return result;
}

/* Test 5: Complex control flow with dependencies */
__attribute__((noinline)) int test_control_flow_dependency() {
    int array[SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i % 10;
    }
    
    /* Loop with control flow creating complex dependencies */
    for (int i = 1; i < SIZE; i++) {
        if (i % 2 == 0) {
            /* Even indices: flow dependency */
            array[i] = array[i-1] + array[i];
            sum += array[i];
        } else {
            /* Odd indices: anti-dependency */
            int temp = array[i];
            array[i] = sum % 100;
            sum += temp;
        }
        
        /* Additional output dependency in some iterations */
        if (i % 7 == 0) {
            array[i] = array[i] * 2;      /* WAW */
            array[i] = array[i] + 1;      /* Another WAW */
        }
    }
    
    return sum;
}

/* Test 6: Pointer aliasing creating ambiguous dependencies */
__attribute__((noinline)) int test_pointer_aliasing() {
    int data[SIZE];
    int* ptr1 = &data[0];
    int* ptr2 = &data[SIZE/2];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i;
    }
    
    /* Loop with potential pointer aliasing */
    int sum = 0;
    for (int i = 0; i < SIZE/2; i++) {
        /* These may alias, creating potential dependencies */
        *ptr1 = *ptr1 + *ptr2;
        *ptr2 = *ptr1 * 2;
        
        ptr1++;
        ptr2--;
        
        sum += data[i];
    }
    
    return sum;
}

int main() {
    srand(time(NULL));
    
    /* Initialize global array */
    for (int i = 0; i < SIZE; i++) {
        global_array[i] = rand() % 1000;
    }
    
    /* Run all tests */
    int result = 0;
    
    result += test_flow_dependency();
    printf("Test 1 complete\n");
    
    result += test_anti_dependency();
    printf("Test 2 complete\n");
    
    result += test_output_dependency();
    printf("Test 3 complete\n");
    
    result += test_nested_dependency();
    printf("Test 4 complete\n");
    
    result += test_control_flow_dependency();
    printf("Test 5 complete\n");
    
    result += test_pointer_aliasing();
    printf("Test 6 complete\n");
    
    /* Final computation to use all results */
    volatile_var = result % 1000;
    printf("Final result: %d\n", result);
    
    return 0;
}
