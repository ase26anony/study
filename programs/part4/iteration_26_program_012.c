/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto_inc_dec optimization pass
 * Specifically targets the find_inc(true) path with reg1_val = 0
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 16

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int *arr, int size) {
    int sum = 0;
    const int *ptr = arr;
    
    /* This should generate: mem = *(ptr + 0), then ptr = ptr + 4 */
    for (int i = 0; i < size; i++) {
        sum += *ptr;    /* Base + 0 access */
        ptr++;          /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int *buffer, int size) {
    /* Compiler's ivopts may transform this to pointer arithmetic */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* May become *(base + 0) after optimization */
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
    const int *end = arr + size * stride;
    
    while (ptr < end) {
        sum += *ptr;        /* *(ptr + 0) */
        ptr += stride;      /* Explicit increment by constant */
    }
    return sum;
}

/* Pattern 5: Struct access to ensure non-trivial element size */
struct Data {
    int values[4];
    float weight;
};

void process_struct_array(struct Data *array, int count) {
    struct Data *ptr = array;
    
    for (int i = 0; i < count; i++) {
        /* Access with base + 0, then increment by sizeof(struct Data) */
        ptr->weight *= 2.0f;
        ptr++;  /* Increment by constant struct size */
    }
}

/* Pattern 6: Mixed access patterns to test different optimization paths */
void mixed_access(int *dest, const int *src1, const int *src2, int size) {
    int *d = dest;
    const int *s1 = src1;
    const int *s2 = src2;
    
    for (int i = 0; i < size; i++) {
        /* Three separate memory accesses, each with base + 0 */
        int val1 = *s1;
        int val2 = *s2;
        *d = val1 + val2;
        
        /* Three separate increments */
        s1++;
        s2++;
        d++;
    }
}

/* Pattern 7: Reverse traversal with auto-decrement opportunity */
int sum_reverse(const int *arr, int size) {
    int sum = 0;
    const int *ptr = arr + size - 1;
    
    for (int i = size - 1; i >= 0; i--) {
        sum += *ptr;    /* Base + 0 access */
        ptr--;          /* Separate decrement instruction */
    }
    return sum;
}

/* Pattern 8: Loop with if condition that doesn't prevent optimization */
int conditional_sum(const int *arr, int size, int threshold) {
    int sum = 0;
    const int *ptr = arr;
    
    for (int i = 0; i < size; i++) {
        if (*ptr > threshold) {  /* Base + 0 access in condition */
            sum += *ptr;          /* Another base + 0 access */
        }
        ptr++;  /* Increment after accesses */
    }
    return sum;
}

int main() {
    /* Initialize test data */
    int array[N];
    int buffer[N];
    int matrix[M][N];
    struct Data struct_array[100];
    
    /* Fill with some values */
    for (int i = 0; i < N; i++) {
        array[i] = i % 100;
        buffer[i] = 0;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = j * N + i;
        }
    }
    
    for (int i = 0; i < 100; i++) {
        struct_array[i].weight = i * 0.5f;
    }
    
    /* Execute all patterns */
    int sum1 = sum_array(array, N);
    clear_buffer(buffer, N);
    fill_matrix(matrix);
    int sum2 = sum_with_stride(array, N/4, 4);
    process_struct_array(struct_array, 100);
    
    int dest[N];
    mixed_access(dest, array, buffer, N);
    
    int sum3 = sum_reverse(array, N);
    int sum4 = conditional_sum(array, N, 50);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %d %d\n", sum1, sum2, sum3, sum4);
    printf("Buffer[0] = %d, Matrix[0][0] = %d\n", buffer[0], matrix[0][0]);
    printf("Struct weight = %f\n", struct_array[0].weight);
    
    return 0;
}
