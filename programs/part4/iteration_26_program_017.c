/* test_auto_inc_dec.c
 * 
 * This program contains loops designed to trigger the uncovered lines
 * in GCC's auto_inc_dec pass (lines 1352-1358 in auto-inc-dec.cc).
 * The patterns aim to create memory references with base + 0 addressing
 * and separate increment instructions that find_inc(true) can merge.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50

/* Pattern 1: Simple pointer traversal with post-increment */
int pattern1_simple_pointer_postinc(int *arr, int size) {
    int sum = 0;
    int *p = arr;
    
    /* Classic *ptr++ pattern - access uses *(ptr + 0), then ptr is incremented */
    for (int i = 0; i < size; i++) {
        sum += *p;  /* Should become base + 0 access */
        p++;        /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void pattern2_indexed_access_postinc(int *buffer, int size) {
    int i = 0;
    
    /* Indexed access - compiler may transform to pointer with zero offset */
    while (i < size) {
        buffer[i] = 0;  /* May become *(base + 0) after optimization */
        i++;            /* Separate increment */
    }
}

/* Pattern 3: Pointer arithmetic with explicit stride */
int pattern3_pointer_with_stride(int *arr, int size, int stride) {
    int total = 0;
    int *ptr = arr;
    int *end = arr + size;
    
    /* Explicit pointer increment separate from access */
    while (ptr < end) {
        total += *ptr;   /* base + 0 access */
        ptr += stride;   /* Constant increment (stride=1 for int*) */
    }
    return total;
}

/* Pattern 4: Nested loops with invariant base in inner loop */
void pattern4_nested_loops_invariant_base(int matrix[M][N]) {
    /* Outer loop calculates base address */
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base calculated in outer loop */
        
        /* Inner loop accesses with base + 0 */
        for (int i = 0; i < N; i++) {
            base[i] = i;  /* May become *(base + 0) in RTL */
        }
    }
}

/* Pattern 5: Struct access with pointer increment */
struct Data {
    int value;
    char tag;
    float weight;
};

int pattern5_struct_pointer_traversal(struct Data *data, int count) {
    int sum = 0;
    struct Data *ptr = data;
    
    /* Struct pointer traversal - fixed stride based on struct size */
    for (int i = 0; i < count; i++) {
        sum += ptr->value;  /* base + 0 access for struct member */
        ptr++;              /* Increment by sizeof(struct Data) */
    }
    return sum;
}

/* Pattern 6: Mixed access patterns to avoid over-optimization */
void pattern6_mixed_access(int *src, int *dst, int size) {
    int *s = src;
    int *d = dst;
    
    /* Two separate pointers, each with their own increment */
    for (int i = 0; i < size; i++) {
        *d = *s;  /* Both should be base + 0 accesses */
        s++;      /* Separate increment */
        d++;      /* Separate increment */
    }
}

/* Pattern 7: Do-while loop to ensure at least one iteration */
int pattern7_dowhile_loop(int *arr, int size) {
    int sum = 0;
    int *p = arr;
    int count = size;
    
    /* Do-while ensures loop runs at least once */
    if (count > 0) {
        do {
            sum += *p;  /* base + 0 access */
            p++;        /* Separate increment */
            count--;
        } while (count > 0);
    }
    return sum;
}

/* Pattern 8: Loop with if condition inside - tests robustness */
int pattern8_conditional_increment(int *arr, int size, int threshold) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < size; i++) {
        if (*ptr > threshold) {  /* base + 0 access in condition */
            sum += *ptr;         /* Another base + 0 access */
        }
        ptr++;  /* Separate increment */
    }
    return sum;
}

/* Main function to run all patterns */
int main() {
    /* Initialize test data */
    int array1[N];
    int array2[N];
    int matrix[M][N];
    struct Data struct_array[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = i;
        array2[i] = 0;
        struct_array[i].value = i * 2;
        struct_array[i].tag = 'A' + (i % 26);
        struct_array[i].weight = i * 0.5f;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = j * N + i;
        }
    }
    
    /* Execute all patterns */
    int result1 = pattern1_simple_pointer_postinc(array1, N);
    pattern2_indexed_access_postinc(array2, N);
    int result3 = pattern3_pointer_with_stride(array1, N, 1);
    pattern4_nested_loops_invariant_base(matrix);
    int result5 = pattern5_struct_pointer_traversal(struct_array, N);
    pattern6_mixed_access(array1, array2, N);
    int result7 = pattern7_dowhile_loop(array1, N);
    int result8 = pattern8_conditional_increment(array1, N, 50);
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d %d %d %d %d\n", 
           result1, result3, result5, result7, result8);
    
    return 0;
}
