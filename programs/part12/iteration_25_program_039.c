#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100

/* Prevent optimization and create dependencies */
volatile int global_seed = 42;

/* Functions to prevent inlining and create dependencies */
__attribute__((noinline)) int use_value(int x) {
    return x + global_seed;
}

__attribute__((noinline)) void modify_value(int *x) {
    *x += global_seed;
}

/* Test 1: Flow dependency (RAW) with carried dependency across iterations */
__attribute__((noinline)) int test_flow_dependency(int *arr) {
    int sum = 0;
    volatile int temp = 0;
    
    /* Classic accumulation with flow dependency */
    for (int i = 0; i < SIZE; i++) {
        temp = sum;           /* Read sum */
        sum += arr[i];        /* Write sum - creates flow dependency with next iteration */
        arr[i] = temp;        /* Anti-dependency on temp */
    }
    
    /* Another flow dependency pattern */
    int prev = arr[0];
    for (int i = 1; i < SIZE; i++) {
        int curr = arr[i] + prev;  /* Flow: prev from previous iteration */
        arr[i] = curr;
        prev = curr;                /* Carried dependency */
    }
    
    return sum + arr[SIZE-1];
}

/* Test 2: Anti-dependency (WAR) */
__attribute__((noinline)) int test_anti_dependency(int *arr) {
    int result = 0;
    
    for (int i = 0; i < SIZE - 1; i++) {
        int read_val = arr[i];      /* Read arr[i] */
        arr[i] = arr[i+1] + i;      /* Write arr[i] - anti-dependency on read_val */
        result += read_val;
        
        /* More complex anti-dependency */
        volatile int temp = arr[i];
        arr[i] = use_value(temp);   /* WAR: temp read, then arr[i] written */
    }
    
    return result;
}

/* Test 3: Output dependency (WAW) */
__attribute__((noinline)) int test_output_dependency(int *arr) {
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        /* Multiple writes to same location */
        arr[i] = i * 2;             /* Write 1 */
        arr[i] = arr[i] + 1;        /* Write 2 - output dependency */
        
        /* Another WAW pattern */
        int x = arr[i];
        arr[i] = x * 3;             /* Write 3 */
        modify_value(&arr[i]);      /* Write 4 - through function call */
        
        sum += arr[i];
    }
    
    return sum;
}

/* Test 4: Nested loops with dependencies */
__attribute__((noinline)) int test_nested_dependency(int arr[][N]) {
    int total = 0;
    
    /* Outer loop dependency carried to inner loop */
    for (int i = 1; i < N; i++) {
        int outer_acc = arr[i-1][0];  /* Flow from previous outer iteration */
        
        for (int j = 0; j < N; j++) {
            /* Multiple dependency types */
            int read = arr[i][j];      /* Read */
            arr[i][j] = outer_acc + read + j;  /* Write with flow from outer */
            outer_acc = arr[i][j];     /* Carried dependency in inner loop */
            
            /* Anti-dependency in inner loop */
            volatile int temp = arr[i][j];
            arr[i][j] = temp * 2;      /* WAR */
        }
        
        total += outer_acc;
    }
    
    return total;
}

/* Test 5: Mixed data types and control flow */
__attribute__((noinline)) double test_mixed_dependencies(double *darr, int *iarr) {
    double dsum = 0.0;
    int isum = 0;
    
    for (int i = 1; i < SIZE; i++) {
        /* Control flow creates complex dependencies */
        if (i % 3 == 0) {
            /* Flow dependency path */
            darr[i] = darr[i-1] * 1.5;  /* Flow from previous iteration */
            isum += iarr[i];
        } else if (i % 3 == 1) {
            /* Anti-dependency path */
            double old = darr[i];        /* Read */
            darr[i] = old / 2.0 + i;     /* Write - WAR */
            iarr[i] = isum;              /* Flow dependency on isum */
        } else {
            /* Output dependency path */
            darr[i] = i * 0.25;          /* Write 1 */
            darr[i] = darr[i] + dsum;    /* Write 2 - WAW */
            isum = iarr[i];              /* Anti-dependency */
        }
        
        dsum += darr[i];
        
        /* Cross-type dependency */
        iarr[i] = (int)darr[i] + isum;   /* Depends on both darr[i] and isum */
    }
    
    return dsum + isum;
}

/* Test 6: Pointer aliasing creates ambiguous dependencies */
__attribute__((noinline)) int test_pointer_aliasing(int *a, int *b, int *c) {
    /* b and c may alias, creating potential dependencies */
    int sum = 0;
    
    for (int i = 1; i < SIZE; i++) {
        *b = a[i-1] + sum;      /* Write through pointer */
        sum += *c + i;          /* Read through potentially aliased pointer */
        a[i] = sum;             /* Flow dependency */
        
        /* Force pointer updates */
        if (i % 2 == 0) {
            b = &a[i];          /* b now aliases a */
        } else {
            c = &a[i];          /* c now aliases a */
        }
    }
    
    return sum;
}

int main() {
    /* Initialize data */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    double *darr = (double*)malloc(SIZE * sizeof(double));
    int (*nested_arr)[N] = (int(*)[N])malloc(N * N * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        darr[i] = (double)(rand() % 100) / 3.0;
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            nested_arr[i][j] = rand() % 50;
        }
    }
    
    /* Execute tests with various dependency patterns */
    int result = 0;
    result += test_flow_dependency(arr1);
    result += test_anti_dependency(arr2);
    result += test_output_dependency(arr1);
    result += test_nested_dependency(nested_arr);
    
    double dresult = test_mixed_dependencies(darr, arr2);
    result += (int)dresult;
    
    /* Test with pointer aliasing */
    int *ptr1 = &arr1[0];
    int *ptr2 = &arr2[0];
    int *ptr3 = &arr1[SIZE/2];  /* Potential alias for arr1 */
    result += test_pointer_aliasing(arr1, ptr1, ptr3);
    
    /* Use results to prevent dead code elimination */
    printf("Final result: %d\n", result);
    printf("Checksum: arr1[0]=%d, arr2[0]=%d, darr[0]=%.2f\n", 
           arr1[0], arr2[0], darr[0]);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(darr);
    free(nested_arr);
    
    return result % 256;
}
