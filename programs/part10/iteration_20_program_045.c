/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* indices, int size, int* counter) {
    double sum = 0.0;
    double temp1, temp2;
    volatile double vol_var = 0.0; /* Prevent optimization of critical dependencies */
    
    /* Complex loop with multiple carried dependencies */
    for (int i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) with floating point */
        temp1 = arr[i-1] * 1.5;      /* Read arr[i-1] */
        arr[i] = temp1 + (double)i;  /* Write arr[i] - depends on previous iteration */
        
        /* 2. ANTI-DEPENDENCY (WAR) with integer */
        int idx = indices[i];        /* Read indices[i] */
        indices[i] = idx + i;        /* Write indices[i] - anti-dep on previous read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) with pointer arithmetic */
        double* ptr = &arr[i];
        *ptr = *ptr * 2.0;           /* First write to arr[i] */
        if (i % 3 == 0) {
            *ptr = *ptr / 1.5;       /* Second write to same location - output dep */
        }
        
        /* 4. MEMORY DEPENDENCY with aliasing */
        /* Create ambiguous pointer to force conservative memory dependency */
        double* alias_ptr = (double*)((char*)arr + (i % 2) * sizeof(double));
        temp2 = *alias_ptr;          /* May alias with arr[i] or arr[i-1] */
        
        /* 5. MIXED DATA TYPES in dependency chain */
        /* Integer -> Float conversion dependency */
        float f_temp = (float)(indices[i] % 7);
        arr[i] += (double)f_temp;
        
        /* 6. CONTROL DEPENDENCY */
        /* Branch creates control flow dependencies */
        if (arr[i] > 100.0) {
            arr[i] = 100.0;
            (*counter)++;            /* Memory update with potential dependencies */
        }
        
        /* 7. ACCUMULATOR with carried dependency */
        sum += arr[i] * 0.1;         /* Loop-carried dependency through sum */
        
        /* Use volatile to prevent reordering across critical operations */
        vol_var = arr[i];
    }
    
    /* Final computation with dependencies on loop results */
    for (int j = 0; j < 4; j++) {
        sum = sum * 0.99 + (double)indices[j];
    }
    
    return sum;
}

/* Helper with pointer arithmetic to create complex addressing */
__attribute__((noinline, noclone))
void init_arrays(double* arr, int* indices, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = (double)(i % 10);
        indices[i] = i * 2;
    }
}

int main(void) {
    const int SIZE = 512;
    double* array = (double*)malloc(SIZE * sizeof(double));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    int counter = 0;
    
    if (!array || !indices) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    init_arrays(array, indices, SIZE);
    
    /* Execute the complex loop */
    double result = compute_loop(array, indices, SIZE, &counter);
    
    /* Use results to prevent dead code elimination */
    volatile double vol_result = result;
    printf("Result: %f, Counter: %d\n", result, counter);
    
    /* Verify with simple check */
    double check = 0.0;
    for (int i = 0; i < 10; i++) {
        check += array[i];
    }
    printf("Array sum[0..9]: %f\n", check);
    
    free(array);
    free(indices);
    
    return 0;
}
