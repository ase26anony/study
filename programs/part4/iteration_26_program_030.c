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
    const int *ptr = arr;
    int sum = 0;
    
    /* This should generate: base + 0 addressing in loop body */
    for (int i = 0; i < size; i++) {
        sum += *ptr;    /* Access with ptr + 0 */
        ptr += 1;       /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer induction */
void clear_buffer(int *buffer, int size) {
    /* Compiler's ivopts should transform this to pointer arithmetic */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* Should become *(buffer + i*4) then optimized */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 pattern */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;        /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
int sum_every_other(const int *arr, int size, int stride) {
    const int *ptr = arr;
    int sum = 0;
    
    /* Force separate increment instruction */
    for (int i = 0; i < size; i += stride) {
        sum += *ptr;    /* ptr + 0 access */
        ptr += stride;  /* Explicit increment */
    }
    return sum;
}

/* Pattern 5: Struct access to ensure proper alignment */
struct Data {
    int value;
    char tag;
    /* Padding likely added here */
};

int sum_struct_array(const struct Data *arr, int size) {
    const struct Data *ptr = arr;
    int sum = 0;
    
    for (int i = 0; i < size; i++) {
        sum += ptr->value;  /* Access with ptr + 0 */
        ptr += 1;           /* Increment by struct size */
    }
    return sum;
}

/* Pattern 6: Multiple memory references in same loop */
void copy_and_transform(int *dest, const int *src, int size) {
    const int *s = src;
    int *d = dest;
    
    for (int i = 0; i < size; i++) {
        *d = *s + 1;    /* Two memory accesses with base + 0 */
        s += 1;         /* Separate increment for source */
        d += 1;         /* Separate increment for dest */
    }
}

/* Pattern 7: Loop with pointer comparison */
int count_until(const int *arr, int limit) {
    const int *ptr = arr;
    int count = 0;
    
    /* Pointer comparison in loop condition */
    while (ptr < arr + N) {
        if (*ptr >= limit) break;  /* Access with ptr + 0 */
        count++;
        ptr += 1;                  /* Separate increment */
    }
    return count;
}

/* Main function to exercise all patterns */
int main() {
    int array[N];
    int buffer[N];
    int matrix[M][N];
    struct Data struct_array[N];
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        array[i] = i;
        buffer[i] = i * 2;
        struct_array[i].value = i * 3;
        struct_array[i].tag = 'A' + (i % 26);
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = j * N + i;
        }
    }
    
    /* Exercise different patterns */
    int sum1 = sum_array(array, N);
    printf("Sum of array: %d\n", sum1);
    
    clear_buffer(buffer, N);
    printf("Buffer cleared\n");
    
    fill_matrix(matrix);
    printf("Matrix filled\n");
    
    int sum2 = sum_every_other(array, N, 2);
    printf("Sum every other: %d\n", sum2);
    
    int sum3 = sum_struct_array(struct_array, N);
    printf("Sum of struct array: %d\n", sum3);
    
    int dest[N];
    copy_and_transform(dest, array, N);
    printf("Array copied and transformed\n");
    
    int count = count_until(array, 100);
    printf("Count until 100: %d\n", count);
    
    return 0;
}
