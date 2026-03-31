/* test_auto_inc_dec.c
 * This program contains loop patterns designed to trigger the uncovered
 * lines in GCC's auto-inc-dec pass where memory addresses have a constant
 * offset of zero (base + 0) and a separate increment instruction exists.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 16

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int *arr, int size) {
    int sum = 0;
    const int *p = arr;
    
    /* Classic *ptr++ pattern - access uses ptr + 0, then ptr is incremented */
    for (int i = 0; i < size; i++) {
        sum += *p;      /* Should become *(p + 0) after optimization */
        p++;            /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int *buffer, int size) {
    /* Simple indexed access - ivopts may convert to pointer arithmetic */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* May become *(base + 0) with separate i increment */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses base[i] - base is invariant within inner loop */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;        /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit pointer increment with stride */
int sum_strided(const int *arr, int size, int stride) {
    int sum = 0;
    const int *ptr = arr;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < size; i++) {
        sum += *ptr;    /* *(ptr + 0) */
        ptr += stride;  /* Separate increment by constant stride */
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
    
    /* Access struct through pointer, then increment */
    for (int i = 0; i < count; i++) {
        ptr->tag = 'A';     /* Access at ptr + 0 */
        ptr++;              /* Increment by sizeof(struct Data) */
    }
}

/* Pattern 6: Multiple memory references in same loop */
void copy_and_scale(int *dest, const int *src, int size) {
    const int *s = src;
    int *d = dest;
    
    /* Two memory accesses, each with base + 0 pattern */
    for (int i = 0; i < size; i++) {
        *d = *s * 2;    /* Both *s and *d are *(ptr + 0) */
        s++;            /* Separate increment for source */
        d++;            /* Separate increment for destination */
    }
}

/* Pattern 7: Loop with pointer comparison */
int count_until(const int *arr, int limit) {
    const int *p = arr;
    int count = 0;
    
    /* Pointer comparison in loop condition */
    while (*p != limit) {
        count++;
        p++;    /* Increment after access */
    }
    return count;
}

/* Pattern 8: Do-while loop variant */
void reverse_array(int *arr, int size) {
    int *start = arr;
    int *end = arr + size - 1;
    
    /* Do-while with pointer increments */
    do {
        int temp = *start;
        *start = *end;
        *end = temp;
        start++;    /* Increment after access */
        end--;      /* Decrement after access */
    } while (start < end);
}

int main() {
    /* Initialize test data */
    int array[N];
    int buffer[N];
    int matrix[M][N];
    struct Data struct_array[32];
    
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
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 4; j++) {
            struct_array[i].values[j] = i + j;
        }
        struct_array[i].tag = 'X';
    }
    
    /* Execute all patterns to ensure code is generated */
    int sum1 = sum_array(array, N);
    clear_buffer(buffer, N);
    fill_matrix(matrix);
    int sum2 = sum_strided(array, N, 2);
    process_struct_array(struct_array, 32);
    
    int dest[N];
    copy_and_scale(dest, array, N);
    
    int count = count_until(array, 100);
    reverse_array(array, N);
    
    /* Verify results */
    printf("Sum1: %d\n", sum1);
    printf("Sum2: %d\n", sum2);
    printf("Count: %d\n", count);
    printf("First element after reverse: %d\n", array[0]);
    
    return 0;
}
