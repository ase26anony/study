/* test_omp_array_section.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Complex base expressions for array sections */
struct Data {
    int *array;
    int offset;
};

/* Function to ensure bounds aren't constant folded */
int get_bound(int base, int argc, char **argv) {
    volatile int v = base;
    if (argc > 1) v += atoi(argv[1]);
    return v;
}

/* Function expecting pointer - may cause type warnings */
void process_ptr(int *p, int n) {
    for (int i = 0; i < n; i++) p[i] += 1;
}

int main(int argc, char **argv) {
    int n = 100;
    int m = 50;
    
    /* Dynamic bounds to prevent constant folding */
    int start = get_bound(10, argc, argv);
    int length = get_bound(20, argc, argv);
    int offset = get_bound(5, argc, argv);
    
    /* Allocate arrays */
    int *arr1 = (int *)malloc(n * sizeof(int));
    int *arr2 = (int *)malloc(n * sizeof(int));
    int *ptr = arr1;
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        arr1[i] = i;
        arr2[i] = n - i;
    }
    
    /* Struct with array member */
    struct Data data;
    data.array = arr2;
    data.offset = offset;
    
    /* Conditional expression as base - triggers op_prio parenthesization */
    int cond = (argc > 2);
    
    /* STRATEGY 1: OpenMP target data with complex array sections */
    #pragma omp target data \
        map(to: (cond ? arr1 : arr2)[start:length]) \
        map(tofrom: (ptr + offset)[0:m]) \
        map(alloc: data.array[data.offset:length])
    {
        /* Inside data region: target compute with array sections */
        #pragma omp target \
            map(tofrom: (cond ? arr1 : arr2)[start:length]) \
            map(alloc: (ptr + offset)[0:m/2])
        {
            int *base = cond ? arr1 : arr2;
            for (int i = 0; i < length; i++) {
                base[start + i] *= 2;
            }
        }
        
        /* STRATEGY 2: Potential type warning/error context */
        /* Passing array section to function expecting pointer */
        /* This may trigger diagnostics during compilation */
        #ifdef DELIBERATE_TYPE_CHECK
        process_ptr(arr1[start:length], length);  /* Array section in non-OpenMP context */
        #endif
    }
    
    /* STRATEGY 3: Task depend with array sections */
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: arr1[0:n/2])
        {
            for (int i = 0; i < n/2; i++) arr1[i] += 1;
        }
        
        #pragma omp task depend(in: arr1[0:n/2]) depend(out: arr2[n/4:3*n/4])
        {
            for (int i = n/4; i < 3*n/4; i++) arr2[i] = arr1[i % (n/2)];
        }
        
        #pragma omp task depend(in: arr2[n/4:3*n/4])
        {
            int sum = 0;
            for (int i = n/4; i < 3*n/4; i++) sum += arr2[i];
            printf("Checksum: %d\n", sum);
        }
    }
    
    /* STRATEGY 4: Multiple OpenMP clauses with different array sections */
    int *dynamic_arr = (int *)malloc(200 * sizeof(int));
    for (int i = 0; i < 200; i++) dynamic_arr[i] = i;
    
    #pragma omp target enter data \
        map(to: dynamic_arr[get_bound(0,argc,argv):get_bound(100,argc,argv)])
    
    #pragma omp target \
        map(tofrom: dynamic_arr[(argc>3?50:25):75]) \
        map(alloc: (dynamic_arr + 150)[0:get_bound(25,argc,argv)])
    {
        /* Simple computation to prevent dead code elimination */
        for (int i = 0; i < 75; i++) {
            int idx = (argc > 3 ? 50 : 25) + i;
            dynamic_arr[idx] += 3;
        }
    }
    
    #pragma omp target exit data \
        map(from: dynamic_arr[get_bound(0,argc,argv):get_bound(100,argc,argv)])
    
    /* Final checksum to ensure execution */
    int total = 0;
    for (int i = 0; i < n; i++) {
        total += arr1[i] + arr2[i];
    }
    for (int i = 0; i < 200; i++) {
        total += dynamic_arr[i];
    }
    printf("Total checksum: %d\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(dynamic_arr);
    
    return 0;
}
