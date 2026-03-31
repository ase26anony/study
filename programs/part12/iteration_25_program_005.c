#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define INNER_SIZE 128

/* Prevent optimization of dependencies */
volatile int global_seed = 42;

/* Functions to prevent optimization and create aliasing */
__attribute__((noinline)) 
int* get_array_ptr(int* arr, int index) {
    return &arr[index];
}

__attribute__((noinline))
void modify_through_pointer(int* ptr, int value) {
    *ptr = value;
}

/* Test 1: Flow (RAW) dependency with carried dependency across iterations */
__attribute__((noinline))
int test_flow_dependency(int* arr) {
    int sum = 0;
    /* Classic accumulation with flow dependency */
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];  /* Read arr[i] */
        arr[i] = sum;   /* Write to arr[i] - creates flow dep with next iteration */
    }
    return sum;
}

/* Test 2: Anti (WAR) dependency within same iteration */
__attribute__((noinline))
int test_anti_dependency(int* arr, int* brr) {
    int temp = 0;
    for (int i = 0; i < SIZE; i++) {
        temp = arr[i];      /* Read arr[i] */
        arr[i] = brr[i];    /* Write to arr[i] - anti-dependency on previous read */
        brr[i] = temp;      /* Write to brr[i] */
    }
    return temp;
}

/* Test 3: Output (WAW) dependency */
__attribute__((noinline))
int test_output_dependency(int* arr) {
    int result = 0;
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;             /* First write */
        arr[i] = arr[i] * 2;    /* Second write - output dependency */
        result += arr[i];
    }
    return result;
}

/* Test 4: Nested loops with cross-iteration dependencies */
__attribute__((noinline))
int test_nested_dependency(int arr[][INNER_SIZE]) {
    int total = 0;
    /* Outer loop dependency carried to inner loop */
    for (int i = 1; i < SIZE/INNER_SIZE; i++) {
        for (int j = 0; j < INNER_SIZE; j++) {
            /* Flow dependency across outer loop iterations */
            arr[i][j] = arr[i-1][j] + arr[i][j];
            total += arr[i][j];
        }
    }
    return total;
}

/* Test 5: Mixed data types and operations */
__attribute__((noinline))
float test_mixed_types(float* farr, double* darr) {
    float fsum = 0.0f;
    double dsum = 0.0;
    
    for (int i = 1; i < SIZE; i++) {
        /* Flow dependency with different data types */
        farr[i] = farr[i-1] * 1.5f + farr[i];
        darr[i] = darr[i-1] / 2.0 + darr[i];
        
        fsum += farr[i];
        dsum += darr[i];
    }
    return fsum + (float)dsum;
}

/* Test 6: Control flow with dependencies */
__attribute__((noinline))
int test_control_flow_dependency(int* arr, int* brr) {
    int sum = 0;
    for (int i = 1; i < SIZE; i++) {
        if (i % 3 == 0) {
            /* Flow dependency in one path */
            arr[i] = arr[i-1] + brr[i];
            sum += arr[i];
        } else if (i % 3 == 1) {
            /* Anti-dependency in another path */
            int temp = arr[i];
            arr[i] = i * 2;
            brr[i] = temp;
            sum += brr[i];
        } else {
            /* Output dependency in third path */
            arr[i] = i;
            arr[i] = arr[i] * 3;
            sum += arr[i];
        }
    }
    return sum;
}

/* Test 7: Pointer aliasing creates complex dependencies */
__attribute__((noinline))
int test_pointer_aliasing(int* arr) {
    int* ptr1 = arr;
    int* ptr2 = arr + SIZE/2;
    int sum = 0;
    
    for (int i = 0; i < SIZE/2; i++) {
        /* Potential aliasing creates conservative dependencies */
        *ptr1 = *ptr2 + i;
        *ptr2 = *ptr1 - i;
        
        sum += *ptr1 + *ptr2;
        ptr1++;
        ptr2++;
    }
    return sum;
}

/* Test 8: Loop with multiple carried dependencies */
__attribute__((noinline))
int test_multiple_dependencies(int* arr, int* brr) {
    int x = 0, y = 0, z = 0;
    
    for (int i = 0; i < SIZE; i++) {
        /* Multiple flow dependencies */
        x = arr[i] + x;      /* Flow dep on x */
        y = brr[i] + y;      /* Flow dep on y */
        z = x + y + z;       /* Flow dep on z, also depends on x and y */
        
        arr[i] = x;
        brr[i] = y;
    }
    return x + y + z;
}

int main() {
    /* Initialize arrays with different patterns */
    int arr1[SIZE], arr2[SIZE], brr[SIZE];
    float farr[SIZE];
    double darr[SIZE];
    int nested_arr[SIZE/INNER_SIZE][INNER_SIZE];
    
    srand(time(NULL));
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        brr[i] = rand() % 100;
        farr[i] = (float)(rand() % 100) / 10.0f;
        darr[i] = (double)(rand() % 100) / 10.0;
    }
    
    for (int i = 0; i < SIZE/INNER_SIZE; i++) {
        for (int j = 0; j < INNER_SIZE; j++) {
            nested_arr[i][j] = rand() % 100;
        }
    }
    
    /* Execute all tests to trigger DDG construction */
    int result = 0;
    
    result += test_flow_dependency(arr1);
    result += test_anti_dependency(arr1, brr);
    result += test_output_dependency(arr2);
    result += test_nested_dependency(nested_arr);
    
    float float_result = test_mixed_types(farr, darr);
    result += (int)float_result;
    
    result += test_control_flow_dependency(arr1, brr);
    result += test_pointer_aliasing(arr2);
    result += test_multiple_dependencies(arr1, arr2);
    
    /* Use volatile to prevent optimization of final result */
    volatile int final_result = result;
    printf("Final checksum: %d\n", final_result);
    
    return 0;
}
