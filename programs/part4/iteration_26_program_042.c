/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto_inc_dec optimization pass
 * Specifically targets the find_inc(true) path with reg1_val = 0
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define INNER_SIZE 50

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int* arr, int n) {
    int sum = 0;
    const int* p = arr;
    
    /* This should generate: mem = *(p + 0), then p = p + 4 */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* Access with base + 0 offset */
        p++;            /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer induction */
void clear_buffer(int* buffer, int n) {
    /* Compiler's ivopts may transform this to pointer arithmetic */
    for (int i = 0; i < n; i++) {
        buffer[i] = 0;  /* May become *(ptr + 0) after optimization */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[][INNER_SIZE], int rows) {
    for (int j = 0; j < rows; j++) {
        int* base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 */
        for (int i = 0; i < INNER_SIZE; i++) {
            base[i] = i * j;        /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
int sum_with_stride(const int* arr, int n, int stride) {
    int sum = 0;
    const int* ptr = arr;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < n; i++) {
        sum += *ptr;    /* *(ptr + 0) */
        ptr += stride;  /* Candidate increment for find_inc */
    }
    return sum;
}

/* Pattern 5: Struct access to ensure non-trivial element size */
struct Data {
    int a;
    int b;
    float c;
    char d[4];
};

void process_struct_array(struct Data* array, int n) {
    struct Data* ptr = array;
    
    /* Access struct through pointer, increment separately */
    for (int i = 0; i < n; i++) {
        ptr->a = i;     /* Access through ptr + 0 */
        ptr->b = i * 2;
        ptr++;          /* Increment by sizeof(struct Data) */
    }
}

/* Pattern 6: Mixed access patterns to test different optimization paths */
void mixed_access(int* dest, const int* src1, const int* src2, int n) {
    int* d = dest;
    const int* s1 = src1;
    const int* s2 = src2;
    
    for (int i = 0; i < n; i++) {
        /* Multiple memory accesses with same base pattern */
        *d = *s1 + *s2;  /* Three *(base + 0) accesses */
        d++;
        s1++;
        s2++;
    }
}

/* Pattern 7: Loop with pointer comparison */
int count_until(const int* arr, int limit, int n) {
    int count = 0;
    const int* p = arr;
    const int* end = arr + n;
    
    while (p < end) {
        if (*p > limit)  /* *(p + 0) */
            count++;
        p++;             /* Separate increment */
    }
    return count;
}

/* Main function to exercise all patterns */
int main() {
    int array[SIZE];
    int buffer[SIZE];
    int matrix[SIZE][INNER_SIZE];
    struct Data struct_array[SIZE];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i;
        buffer[i] = 0;
        struct_array[i].a = 0;
        struct_array[i].b = 0;
    }
    
    /* Exercise Pattern 1 */
    int total = sum_array(array, SIZE);
    printf("Sum: %d\n", total);
    
    /* Exercise Pattern 2 */
    clear_buffer(buffer, SIZE);
    printf("Buffer[0]: %d\n", buffer[0]);
    
    /* Exercise Pattern 3 */
    fill_matrix(matrix, SIZE / 2);
    printf("Matrix[0][0]: %d\n", matrix[0][0]);
    
    /* Exercise Pattern 4 */
    total = sum_with_stride(array, SIZE / 2, 2);
    printf("Strided sum: %d\n", total);
    
    /* Exercise Pattern 5 */
    process_struct_array(struct_array, SIZE);
    printf("Struct array[0].a: %d\n", struct_array[0].a);
    
    /* Exercise Pattern 6 */
    mixed_access(buffer, array, array, SIZE);
    printf("Mixed buffer[0]: %d\n", buffer[0]);
    
    /* Exercise Pattern 7 */
    int count = count_until(array, SIZE / 2, SIZE);
    printf("Count above %d: %d\n", SIZE / 2, count);
    
    return 0;
}
