/* test_auto_inc_dec.c
 * This program contains loop patterns designed to trigger the uncovered
 * lines in GCC's auto_inc_dec pass (lines 1352-1358 in auto-inc-dec.cc).
 * The patterns aim to create memory references with base + 0 addressing
 * where find_inc(true) can find a preceding increment instruction.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 16

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int *arr, int size) {
    int sum = 0;
    const int *p = arr;
    
    /* Classic pattern: *p++ results in base + 0 access followed by increment */
    for (int i = 0; i < size; i++) {
        sum += *p;  /* Memory access with offset 0 relative to p */
        p++;        /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int *buffer, int size) {
    /* Simple indexed access - ivopts may transform to pointer arithmetic */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* May become *(buffer + 0) after optimization */
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

/* Pattern 4: Explicit pointer arithmetic with stride */
int sum_with_stride(const int *arr, int size, int stride) {
    int sum = 0;
    const int *ptr = arr;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < size; i++) {
        sum += *ptr;    /* base + 0 access */
        ptr += stride;  /* explicit increment by constant */
    }
    return sum;
}

/* Pattern 5: Struct access to ensure non-trivial element size */
struct Data {
    int values[4];
    char tag;
    int id;
};

void process_struct_array(struct Data *array, int count) {
    struct Data *ptr = array;
    
    /* Access struct through pointer, increment by struct size */
    for (int i = 0; i < count; i++) {
        ptr->id = i;        /* Memory access with offset 0 */
        ptr->tag = 'A' + i;
        ptr++;              /* Increment by sizeof(struct Data) */
    }
}

/* Pattern 6: Multiple memory references in same loop */
void copy_and_transform(int *dest, const int *src, int size) {
    const int *s = src;
    int *d = dest;
    
    /* Two memory references, both with base + 0 pattern */
    for (int i = 0; i < size; i++) {
        *d = *s + 1;  /* Two memory accesses: *(d + 0) and *(s + 0) */
        s++;
        d++;
    }
}

/* Pattern 7: Loop with pointer increment in middle */
void modify_alternate(int *arr, int size) {
    int *ptr = arr;
    
    for (int i = 0; i < size; i += 2) {
        *ptr = i;      /* Access with offset 0 */
        ptr += 2;      /* Increment by 2 elements */
    }
}

int main() {
    /* Initialize test data */
    int array[N];
    int buffer[N];
    int matrix[M][N];
    struct Data struct_array[10];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array[i] = i;
        buffer[i] = i * 2;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = 0;
        }
    }
    
    for (int i = 0; i < 10; i++) {
        struct_array[i].id = -1;
        struct_array[i].tag = 'Z';
    }
    
    /* Execute all patterns to ensure code is generated */
    int result1 = sum_array(array, N);
    printf("Sum of array: %d\n", result1);
    
    clear_buffer(buffer, N);
    printf("Buffer cleared\n");
    
    fill_matrix(matrix);
    printf("Matrix filled\n");
    
    int result2 = sum_with_stride(array, N/2, 2);
    printf("Sum with stride 2: %d\n", result2);
    
    process_struct_array(struct_array, 10);
    printf("Struct array processed\n");
    
    int dest[N];
    copy_and_transform(dest, array, N);
    printf("Array copied and transformed\n");
    
    modify_alternate(array, N);
    printf("Alternate elements modified\n");
    
    return 0;
}
