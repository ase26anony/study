/* test_auto_inc_dec.c
 * 
 * This program contains loop patterns designed to trigger the uncovered
 * lines in GCC's auto_inc_dec pass (lines 1352-1358 in auto-inc-dec.cc).
 * The patterns create memory accesses with base+0 addressing where
 * find_inc(true) can find a preceding increment instruction.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50

/* Pattern 1: Simple pointer traversal with post-increment */
int pattern1_sum_array(int *arr, int size) {
    int sum = 0;
    int *p = arr;
    
    /* Classic *ptr++ pattern - access uses *(ptr + 0), then ptr is incremented */
    for (int i = 0; i < size; i++) {
        sum += *p;  /* Should become base + 0 at RTL level */
        p++;        /* Separate increment instruction for find_inc to find */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer induction variable */
void pattern2_clear_buffer(char *buffer, int size) {
    /* Simple indexed access - ivopts may create a pointer with zero offset */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* May become *(ptr + 0) after optimization */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void pattern3_fill_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 pattern */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
int pattern4_stride_access(int *arr, int size, int stride) {
    int total = 0;
    int *ptr = arr;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < size; i++) {
        total += *ptr;   /* *(ptr + 0) */
        ptr += stride;   /* Clear increment for find_inc */
    }
    return total;
}

/* Pattern 5: Struct access to ensure non-trivial element size */
struct Data {
    int a;
    int b;
    float c;
    char d[4];
};

int pattern5_struct_array(struct Data *array, int count) {
    int sum = 0;
    struct Data *ptr = array;
    
    /* Struct access - larger stride may still trigger optimization */
    for (int i = 0; i < count; i++) {
        sum += ptr->a;  /* Access with potential base + 0 */
        ptr++;          /* Increment by sizeof(struct Data) */
    }
    return sum;
}

/* Pattern 6: Multiple memory references in same loop */
void pattern6_multiple_refs(int *src, int *dst, int size) {
    int *s = src;
    int *d = dst;
    
    /* Two memory references, each with base + 0 pattern */
    for (int i = 0; i < size; i++) {
        *d = *s;  /* Both *(ptr + 0) accesses */
        s++;
        d++;
    }
}

/* Pattern 7: Loop with pointer arithmetic in condition */
int pattern7_pointer_condition(int *start, int *end) {
    int sum = 0;
    int *ptr = start;
    
    /* Pointer comparison in loop condition */
    while (ptr < end) {
        sum += *ptr;  /* *(ptr + 0) */
        ptr++;        /* Separate increment */
    }
    return sum;
}

/* Pattern 8: Access with compile-time constant offset 0 */
int pattern8_explicit_zero_offset(int *arr, int size) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < size; i++) {
        /* Explicit *(ptr + 0) - most direct way to get reg1_val = 0 */
        sum += *(ptr + 0);
        ptr += 1;
    }
    return sum;
}

int main() {
    /* Initialize test data */
    int arr1[N];
    char buffer[N];
    int matrix[M][N];
    struct Data struct_arr[N];
    int arr2[N];
    
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        buffer[i] = 'A' + (i % 26);
        arr2[i] = N - i;
        struct_arr[i].a = i * 2;
        struct_arr[i].b = i * 3;
        struct_arr[i].c = i * 1.5f;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = j * N + i;
        }
    }
    
    /* Execute all patterns to ensure code is generated */
    int sum1 = pattern1_sum_array(arr1, N);
    pattern2_clear_buffer(buffer, N);
    pattern3_fill_matrix(matrix);
    int sum4 = pattern4_stride_access(arr1, N/2, 2);
    int sum5 = pattern5_struct_array(struct_arr, N);
    pattern6_multiple_refs(arr1, arr2, N);
    int sum7 = pattern7_pointer_condition(arr1, arr1 + N);
    int sum8 = pattern8_explicit_zero_offset(arr1, N);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %d %d\n", sum1, sum4, sum5, sum7 + sum8);
    
    return 0;
}
