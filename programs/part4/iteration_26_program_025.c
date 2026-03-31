/* test_auto_inc_dec.c
 * This program contains loops designed to trigger the uncovered lines
 * in GCC's auto-inc-dec pass where memory addresses have a constant
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
        sum += *ptr;  /* Access at offset 0 */
        ptr++;        /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int *buffer, int size) {
    /* Compiler's ivopts may transform this to pointer arithmetic */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* Initially base + (i * 4), may become ptr + 0 */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 pattern */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
int sum_with_stride(const int *arr, int size, int stride) {
    int sum = 0;
    const int *ptr = arr;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < size; i++) {
        sum += *ptr;      /* Access at offset 0 */
        ptr += stride;    /* Separate increment by constant */
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
    
    /* Struct access with post-increment */
    for (int i = 0; i < count; i++) {
        ptr->tag = 'A' + (i % 26);  /* Access at offset 0 */
        ptr++;                      /* Increment by struct size */
    }
}

/* Pattern 6: Multiple memory references in same loop */
void copy_and_transform(int *dest, const int *src, int size) {
    const int *s = src;
    int *d = dest;
    
    for (int i = 0; i < size; i++) {
        *d = *s + 1;  /* Two memory accesses at offset 0 */
        s++;          /* Separate increments */
        d++;
    }
}

/* Pattern 7: Loop with pointer arithmetic in condition */
int find_value(const int *arr, int size, int target) {
    const int *ptr = arr;
    const int *end = arr + size;
    
    while (ptr < end) {
        if (*ptr == target)  /* Access at offset 0 */
            return ptr - arr;
        ptr++;  /* Increment after access */
    }
    return -1;
}

/* Pattern 8: Do-while loop variant */
void initialize_range(int *start, int *end, int value) {
    int *ptr = start;
    
    if (ptr >= end) return;
    
    do {
        *ptr = value;  /* Access at offset 0 */
        ptr++;         /* Increment after access */
    } while (ptr < end);
}

int main() {
    /* Initialize test data */
    int array[N];
    int buffer[N];
    int matrix[M][N];
    struct Data struct_array[M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array[i] = i;
        buffer[i] = 0;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = 0;
        }
        for (int k = 0; k < 4; k++) {
            struct_array[j].values[k] = j * 10 + k;
        }
        struct_array[j].tag = 'X';
    }
    
    /* Execute all patterns to ensure code generation */
    int sum1 = sum_array(array, N);
    clear_buffer(buffer, N);
    fill_matrix(matrix);
    int sum2 = sum_with_stride(array, N, 1);
    process_struct_array(struct_array, M);
    
    int dest[N];
    copy_and_transform(dest, array, N);
    
    int found = find_value(array, N, 42);
    initialize_range(buffer, buffer + N, -1);
    
    /* Verify results */
    printf("Sum1: %d\n", sum1);
    printf("Sum2: %d\n", sum2);
    printf("Found 42 at index: %d\n", found);
    printf("Buffer[0]: %d, Buffer[N-1]: %d\n", buffer[0], buffer[N-1]);
    
    return 0;
}
