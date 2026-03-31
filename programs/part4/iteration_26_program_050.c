/* test_auto_inc_dec.c
 * This program contains loop patterns designed to trigger the uncovered
 * lines in GCC's auto_inc_dec optimization pass, specifically where
 * find_inc(true) is called with reg1_val = 0.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 16

/* Pattern 1: Simple pointer traversal with post-increment
 * This should generate: base + 0 addressing in the load,
 * followed by a separate increment instruction.
 */
int pattern1_simple_pointer(int *arr, int size) {
    int sum = 0;
    int *p = arr;
    int *end = arr + size;
    
    while (p < end) {
        sum += *p;  /* Should become *(p + 0) after optimization */
        p++;        /* Separate increment - find_inc should find this */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset
 * The compiler's ivopts pass may convert the array indexing into
 * a pointer that gets incremented separately.
 */
void pattern2_indexed_post_increment(char *buffer, int size) {
    int i = 0;
    
    while (i < size) {
        buffer[i] = 0;  /* May become *(base + 0) after ivopts */
        i++;            /* Separate increment of index */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop
 * The inner loop's base pointer remains constant while an index
 * is incremented separately.
 */
void pattern3_nested_invariant_base(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        for (int i = 0; i < N; i++) {
            base[i] = i;  /* Access with base + (i * sizeof(int)) */
            /* After optimization, this may become base + 0 with
               separate increment of a pointer induction variable */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment
 * Very clear pattern with separate load and increment instructions
 */
int pattern4_explicit_stride(int *start, int stride, int count) {
    int total = 0;
    int *ptr = start;
    
    for (int i = 0; i < count; i++) {
        total += *ptr;      /* Load from current pointer */
        ptr += stride;      /* Explicit increment - good candidate for find_inc */
    }
    return total;
}

/* Pattern 5: Struct access with pointer increment
 * Using a non-trivial type to ensure proper scaling
 */
struct Data {
    int values[4];
    char tag;
};

int pattern5_struct_traversal(struct Data *array, int count) {
    int sum = 0;
    struct Data *ptr = array;
    
    for (int i = 0; i < count; i++) {
        /* Access struct member - base address calculation may simplify */
        sum += ptr->values[0];
        ptr++;  /* Increment by sizeof(struct Data) */
    }
    return sum;
}

/* Pattern 6: Mixed access pattern to avoid over-optimization
 * Sometimes simpler patterns get optimized away too early.
 * This mixes reads and writes.
 */
void pattern6_mixed_access(int *src, int *dst, int size) {
    int *s = src;
    int *d = dst;
    
    for (int i = 0; i < size; i++) {
        *d = *s;    /* Read from s, write to d */
        s++;        /* Separate increments */
        d++;
    }
}

/* Pattern 7: Do-while loop to ensure at least one iteration
 * Different loop structure might affect optimization ordering
 */
int pattern7_do_while(int *arr, int size) {
    int sum = 0;
    int *p = arr;
    int count = size;
    
    if (count <= 0) return 0;
    
    do {
        sum += *p;
        p++;
        count--;
    } while (count > 0);
    
    return sum;
}

/* Pattern 8: Reverse traversal with decrement
 * Tests auto-decrement patterns as well
 */
int pattern8_reverse_traversal(int *arr, int size) {
    int sum = 0;
    int *p = arr + size - 1;
    
    for (int i = 0; i < size; i++) {
        sum += *p;
        p--;  /* Decrement instead of increment */
    }
    return sum;
}

int main() {
    /* Initialize test data */
    int arr[N];
    char buffer[N];
    int matrix[M][N];
    struct Data struct_arr[N/4];
    
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        buffer[i] = 'A' + (i % 26);
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = j * N + i;
        }
    }
    
    for (int i = 0; i < N/4; i++) {
        for (int j = 0; j < 4; j++) {
            struct_arr[i].values[j] = i * 4 + j;
        }
        struct_arr[i].tag = 'A' + (i % 26);
    }
    
    /* Execute all patterns */
    int sum1 = pattern1_simple_pointer(arr, N);
    pattern2_indexed_post_increment(buffer, N);
    pattern3_nested_invariant_base(matrix);
    int sum4 = pattern4_explicit_stride(arr, 1, N);
    int sum5 = pattern5_struct_traversal(struct_arr, N/4);
    
    int dst[N];
    pattern6_mixed_access(arr, dst, N);
    
    int sum7 = pattern7_do_while(arr, N);
    int sum8 = pattern8_reverse_traversal(arr, N);
    
    /* Verify results (simple checks to ensure code runs) */
    printf("Pattern 1 sum: %d\n", sum1);
    printf("Pattern 4 sum: %d\n", sum4);
    printf("Pattern 5 sum: %d\n", sum5);
    printf("Pattern 7 sum: %d\n", sum7);
    printf("Pattern 8 sum: %d\n", sum8);
    printf("First few dst values: %d %d %d\n", dst[0], dst[1], dst[2]);
    
    return 0;
}
