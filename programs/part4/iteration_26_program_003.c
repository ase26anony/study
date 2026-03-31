/* test_auto_inc_dec.c
 * Designed to trigger the uncovered lines in auto-inc-dec.cc:
 *   mem_insn.reg1_is_const = true;
 *   mem_insn.reg1_val = 0;
 *   if (find_inc (true)) return true;
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 16

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int* arr, int size) {
    int sum = 0;
    const int* p = arr;
    
    /* This should generate: load from (p + 0), then p += 4 */
    for (int i = 0; i < size; i++) {
        sum += *p;      /* Access at offset 0 */
        p++;            /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int* buffer, int size) {
    /* Compiler's ivopts may transform this to pointer arithmetic */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* Initially base + (i * 4), may become ptr + 0 */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int* base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 pattern */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
int sum_with_stride(const int* start, int stride, int count) {
    int total = 0;
    const int* ptr = start;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < count; i++) {
        total += *ptr;   /* Access at offset 0 */
        ptr += stride;   /* Separate increment by constant */
    }
    return total;
}

/* Pattern 5: Struct access to ensure non-trivial element size */
struct Data {
    int a;
    int b;
    float c;
    double d;
};

void process_struct_array(struct Data* array, int count) {
    struct Data* ptr = array;
    
    /* Large stride (24 bytes on typical 64-bit) */
    for (int i = 0; i < count; i++) {
        ptr->a = i;      /* Access at offset 0 */
        ptr->b = i * 2;
        ptr++;           /* Increment by sizeof(struct Data) */
    }
}

/* Pattern 6: Multiple memory references in same loop */
void copy_and_transform(int* dest, const int* src, int size) {
    const int* s = src;
    int* d = dest;
    
    for (int i = 0; i < size; i++) {
        *d = *s + 1;    /* Two memory accesses at offset 0 */
        s++;            /* Separate increments */
        d++;
    }
}

/* Pattern 7: char pointer for single-byte stride */
int count_chars(const char* str, char target) {
    int count = 0;
    const char* p = str;
    
    while (*p != '\0') {
        if (*p == target) count++;  /* Access at offset 0 */
        p++;                        /* Increment by 1 */
    }
    return count;
}

/* Pattern 8: Pointer arithmetic in loop condition */
void reverse_array(int* arr, int size) {
    int* start = arr;
    int* end = arr + size - 1;
    
    while (start < end) {
        int temp = *start;  /* Access at offset 0 */
        *start = *end;      /* Another access at offset 0 */
        *end = temp;
        start++;            /* Separate increments */
        end--;
    }
}

/* Main function to exercise all patterns */
int main() {
    int arr[N];
    int buffer[N];
    int matrix[M][N];
    struct Data struct_arr[100];
    char str[] = "test string for auto increment optimization";
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        buffer[i] = i * 2;
    }
    
    /* Exercise Pattern 1 */
    int sum = sum_array(arr, N);
    printf("Sum: %d\n", sum);
    
    /* Exercise Pattern 2 */
    clear_buffer(buffer, N);
    
    /* Exercise Pattern 3 */
    fill_matrix(matrix);
    
    /* Exercise Pattern 4 */
    sum = sum_with_stride(arr, 2, N/2);
    printf("Strided sum: %d\n", sum);
    
    /* Exercise Pattern 5 */
    process_struct_array(struct_arr, 100);
    
    /* Exercise Pattern 6 */
    copy_and_transform(buffer, arr, N);
    
    /* Exercise Pattern 7 */
    int char_count = count_chars(str, 't');
    printf("Character count: %d\n", char_count);
    
    /* Exercise Pattern 8 */
    reverse_array(arr, N);
    
    return 0;
}
