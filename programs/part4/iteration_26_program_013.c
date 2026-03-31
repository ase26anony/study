/* test_auto_inc_dec.c
 * 
 * This program contains loop patterns designed to trigger the uncovered
 * lines in GCC's auto_inc_dec pass (lines 1352-1358 in auto-inc-dec.cc).
 * The patterns aim to create memory references with base + 0 addressing
 * and separate increment instructions that find_inc(true) can merge.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50

/* Pattern 1: Simple pointer traversal with post-increment
 * This should generate: *(ptr + 0) access followed by ptr += sizeof(int)
 */
int pattern1_simple_pointer(int *arr, int size) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + size;
    
    while (ptr < end) {
        /* Access with current ptr (base + 0 at RTL level) */
        sum += *ptr;
        /* Separate increment instruction */
        ptr++;
    }
    return sum;
}

/* Pattern 2: Indexed array access with post-increment
 * ivopts should transform this to pointer arithmetic
 */
void pattern2_indexed_postinc(int *buffer, int size) {
    int i = 0;
    while (i < size) {
        /* Access with base + (i * scale) - may become base + 0 after ivopts */
        buffer[i] = i * 2;
        /* Separate increment */
        i++;
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop
 * Inner loop has constant base pointer with offset 0
 */
void pattern3_nested_invariant(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        /* Base calculated in outer loop, invariant in inner loop */
        int *base = &matrix[j][0];
        for (int i = 0; i < N; i++) {
            /* Access: base[i] = i -> *(base + i*4) */
            base[i] = i + j;
            /* Increment happens through i, but strength reduction
               may create separate pointer increment */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment
 * Clear base + 0 access followed by ptr += stride
 */
int pattern4_explicit_stride(int *arr, int size, int stride) {
    int total = 0;
    int *ptr = arr;
    int count = size;
    
    while (count-- > 0) {
        /* Base + 0 access */
        total += *ptr;
        /* Explicit increment by stride */
        ptr += stride;
    }
    return total;
}

/* Pattern 5: Struct access with pointer arithmetic
 * Larger stride may still trigger the optimization
 */
struct Data {
    int a;
    int b;
    float c;
};

int pattern5_struct_traversal(struct Data *data, int count) {
    int sum = 0;
    struct Data *ptr = data;
    
    for (int i = 0; i < count; i++) {
        /* Access struct member - base + 0 for the load of ptr */
        sum += ptr->a;
        /* ptr++ increments by sizeof(struct Data) */
        ptr++;
    }
    return sum;
}

/* Pattern 6: Do-while loop often generates cleaner RTL
 * May help avoid loop header complexity
 */
int pattern6_dowhile(int *arr, int size) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + size;
    
    if (ptr >= end) return 0;
    
    do {
        sum += *ptr;
        ptr++;
    } while (ptr < end);
    
    return sum;
}

/* Pattern 7: Char pointer with byte access
 * Different scale factor (1 byte) */
int pattern7_char_traversal(char *str, int len) {
    int count = 0;
    char *p = str;
    char *end = str + len;
    
    while (p < end) {
        if (*p == 'a') count++;
        p++;
    }
    return count;
}

/* Pattern 8: Mixed patterns to increase coverage */
void pattern8_mixed(int *dest, int *src, int n) {
    int *d = dest;
    int *s = src;
    
    for (int i = 0; i < n; i++) {
        /* Two memory accesses with base + 0 pattern */
        int val = *s;
        *d = val * 2;
        
        /* Separate increments */
        d++;
        s++;
    }
}

int main() {
    /* Initialize test data */
    int arr[N];
    int buffer[N];
    int matrix[M][N];
    struct Data data[N];
    char str[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        buffer[i] = 0;
        str[i] = 'a' + (i % 26);
        data[i].a = i * 2;
        data[i].b = i * 3;
        data[i].c = i * 1.5f;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = j * N + i;
        }
    }
    
    /* Execute all patterns to ensure code is generated */
    int result = 0;
    
    result += pattern1_simple_pointer(arr, N);
    pattern2_indexed_postinc(buffer, N);
    pattern3_nested_invariant(matrix);
    result += pattern4_explicit_stride(arr, N, 1);
    result += pattern5_struct_traversal(data, N);
    result += pattern6_dowhile(arr, N);
    result += pattern7_char_traversal(str, N);
    
    int dest[N];
    pattern8_mixed(dest, arr, N);
    
    printf("Result: %d\n", result);
    printf("Buffer[0] = %d\n", buffer[0]);
    printf("Matrix[0][0] = %d\n", matrix[0][0]);
    printf("Dest[0] = %d\n", dest[0]);
    
    return 0;
}
