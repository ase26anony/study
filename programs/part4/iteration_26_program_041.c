/* test_auto_inc_dec.c
 * This program contains loop patterns designed to trigger the uncovered
 * lines in GCC's auto-inc-dec pass where memory addresses have a constant
 * offset of zero (base + 0) and a separate increment instruction exists.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int *arr, int size) {
    int sum = 0;
    const int *ptr = arr;
    
    /* This should generate: load from (ptr + 0), then ptr += 4 */
    for (int i = 0; i < size; i++) {
        sum += *ptr;    /* Access at offset 0 */
        ptr++;          /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int *buffer, int size) {
    /* Compiler's ivopts may transform this to pointer arithmetic */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* May become *(base + 0) with separate i increment */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
int sum_with_stride(const int *arr, int size, int stride) {
    int sum = 0;
    const int *ptr = arr;
    const int *end = arr + size * stride;
    
    while (ptr < end) {
        sum += *ptr;        /* Access at offset 0 */
        ptr += stride;      /* Separate constant increment */
    }
    return sum;
}

/* Pattern 5: Struct access to ensure non-trivial element size */
struct Data {
    int values[4];
    char tag;
};

void process_struct_array(struct Data *array, int count) {
    struct Data *ptr = array;
    
    for (int i = 0; i < count; i++) {
        /* Access struct at offset 0, then increment by sizeof(struct Data) */
        ptr->tag = 'A';
        ptr++;  /* Increment by constant struct size */
    }
}

/* Pattern 6: Pointer arithmetic in loop condition */
void copy_arrays(int *dest, const int *src, int size) {
    int *d = dest;
    const int *s = src;
    
    /* Classic copy loop pattern */
    for (int i = 0; i < size; i++) {
        *d = *s;  /* Both accesses at offset 0 */
        d++;
        s++;      /* Two separate increment instructions */
    }
}

/* Pattern 7: Mixed patterns to increase coverage */
void mixed_access_patterns() {
    int buffer1[N];
    int buffer2[N];
    int *matrix[M];
    
    /* Initialize matrix of pointers */
    for (int i = 0; i < M; i++) {
        matrix[i] = malloc(N * sizeof(int));
    }
    
    /* Multiple loops with different patterns */
    clear_buffer(buffer1, N);
    
    int *p1 = buffer1;
    int *p2 = buffer2;
    for (int i = 0; i < N; i++) {
        *p2 = *p1 + i;  /* Two memory accesses at offset 0 */
        p1++;
        p2++;           /* Two increment instructions */
    }
    
    /* Clean up */
    for (int i = 0; i < M; i++) {
        free(matrix[i]);
    }
}

int main() {
    int arr[N];
    int matrix[M][N];
    struct Data struct_arr[20];
    
    /* Initialize test data */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
    
    /* Execute all patterns */
    int total = sum_array(arr, N);
    printf("Sum: %d\n", total);
    
    clear_buffer(arr, N);
    
    fill_matrix(matrix);
    
    total = sum_with_stride(arr, N/2, 2);
    printf("Strided sum: %d\n", total);
    
    process_struct_array(struct_arr, 20);
    
    mixed_access_patterns();
    
    return 0;
}
