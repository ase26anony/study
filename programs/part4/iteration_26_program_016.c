/* test_auto_inc_dec.c
 * This program contains loops designed to trigger the uncovered lines
 * in GCC's auto_inc_dec optimization pass (lines 1352-1358).
 * The patterns aim to create memory references with base + 0 addressing
 * and separate increment instructions that find_inc(true) can merge.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50

/* Pattern 1: Simple pointer traversal with post-increment */
int pattern1_simple_pointer(int *arr, int size) {
    int sum = 0;
    int *p = arr;
    
    /* Classic *ptr++ pattern - access uses ptr + 0, then ptr is incremented */
    for (int i = 0; i < size; i++) {
        sum += *p;  /* Should become *(p + 0) after optimization */
        p++;        /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void pattern2_indexed_with_postinc(int *buffer, int size) {
    int i = 0;
    
    /* Loop where index is used then incremented */
    while (i < size) {
        buffer[i] = i * 2;  /* Base + (i * scale) may become base_ptr + 0 */
        i++;                /* Separate increment */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void pattern3_nested_invariant_base(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base[i] - should become *(base + 0) */
        for (int i = 0; i < N; i++) {
            base[i] = i + j;  /* Access relative to invariant base */
            /* Compiler may create separate increment of i or pointer */
        }
    }
}

/* Pattern 4: Explicit pointer arithmetic with stride */
int pattern4_explicit_stride(int *arr, int size, int stride) {
    int total = 0;
    int *ptr = arr;
    int *end = arr + size * stride;
    
    /* Explicit increment separate from access */
    for (; ptr < end; ptr += stride) {
        total += *ptr;  /* *(ptr + 0) */
    }
    return total;
}

/* Pattern 5: Struct access to ensure proper scaling */
struct Data {
    int values[4];
    float weight;
};

int pattern5_struct_array(struct Data *array, int count) {
    int sum = 0;
    struct Data *ptr = array;
    
    /* Access struct field, then increment pointer */
    for (int i = 0; i < count; i++) {
        sum += ptr->values[0];  /* Access through pointer */
        ptr++;                  /* Separate increment by sizeof(struct Data) */
    }
    return sum;
}

/* Pattern 6: Char pointer with byte access */
void pattern6_char_pointer(char *buffer, int size) {
    char *p = buffer;
    char *end = buffer + size;
    
    /* Char access with explicit post-increment */
    while (p < end) {
        *p = (p - buffer) % 256;
        p++;  /* Increment by 1 */
    }
}

/* Pattern 7: Mixed access pattern to avoid over-optimization */
int pattern7_mixed_access(int *arr1, int *arr2, int size) {
    int result = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    /* Two pointers incremented separately */
    for (int i = 0; i < size; i++) {
        result += *p1 + *p2;  /* Two *(ptr + 0) accesses */
        p1++;
        p2++;
    }
    return result;
}

/* Pattern 8: Do-while loop to ensure at least one iteration */
void pattern8_dowhile(double *data, int size) {
    double *ptr = data;
    int count = size;
    
    /* Do-while ensures the loop runs even if size=1 */
    do {
        *ptr = 0.0;
        ptr++;
        count--;
    } while (count > 0);
}

int main() {
    /* Initialize test data */
    int arr1[N];
    int arr2[N];
    int matrix[M][N];
    struct Data struct_arr[N/4];
    char char_buf[N*2];
    double double_arr[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = N - i;
    }
    
    for (int i = 0; i < N/4; i++) {
        for (int j = 0; j < 4; j++) {
            struct_arr[i].values[j] = i * 4 + j;
        }
        struct_arr[i].weight = i * 0.1f;
    }
    
    /* Test all patterns */
    int sum1 = pattern1_simple_pointer(arr1, N);
    pattern2_indexed_with_postinc(arr2, N);
    pattern3_nested_invariant_base(matrix);
    int sum4 = pattern4_explicit_stride(arr1, N/2, 2);
    int sum5 = pattern5_struct_array(struct_arr, N/4);
    pattern6_char_pointer(char_buf, N*2);
    int sum7 = pattern7_mixed_access(arr1, arr2, N);
    pattern8_dowhile(double_arr, N);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %d %d\n", sum1, sum4, sum5, sum7);
    
    return 0;
}
