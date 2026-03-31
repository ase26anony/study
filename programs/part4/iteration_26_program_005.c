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
    
    /* This should generate: load from (p + 0), then p += 4 */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* Access with offset 0 */
        p++;            /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
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
int sum_with_stride(const int* start, int stride, int n) {
    int total = 0;
    const int* ptr = start;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < n; i++) {
        total += *ptr;      /* *(ptr + 0) */
        ptr += stride;      /* Candidate increment for find_inc */
    }
    return total;
}

/* Pattern 5: Struct access to ensure proper scaling */
struct Data {
    int values[4];
    float weight;
};

float process_structs(const struct Data* array, int n) {
    float total = 0.0f;
    const struct Data* p = array;
    
    for (int i = 0; i < n; i++) {
        /* Access struct member - compiler may use base + 0 */
        total += p->weight;
        p++;  /* Increment by sizeof(struct Data) */
    }
    return total;
}

/* Pattern 6: Multiple memory references in same loop */
void copy_and_transform(int* dest, const int* src, int n) {
    const int* s = src;
    int* d = dest;
    
    for (int i = 0; i < n; i++) {
        *d = *s + 1;    /* Two memory accesses with offset 0 */
        s++;
        d++;
    }
}

/* Pattern 7: Loop with pointer arithmetic in condition */
int find_value(const int* arr, int n, int target) {
    const int* p = arr;
    const int* end = arr + n;
    
    while (p < end) {
        if (*p == target)  /* *(p + 0) */
            return p - arr;
        p++;  /* Separate increment */
    }
    return -1;
}

/* Pattern 8: Using different data types to test various scales */
void process_chars(char* str, int length) {
    char* p = str;
    
    for (int i = 0; i < length; i++) {
        *p = (*p >= 'a' && *p <= 'z') ? *p - 32 : *p;
        p++;
    }
}

/* Main function to exercise all patterns */
int main() {
    int arr[SIZE];
    int buffer[SIZE];
    int matrix[10][INNER_SIZE];
    struct Data structs[SIZE];
    char str[] = "Hello, World!";
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
        buffer[i] = i * 2;
        structs[i].weight = i * 0.5f;
    }
    
    /* Test each pattern */
    int sum1 = sum_array(arr, SIZE);
    printf("Sum of array: %d\n", sum1);
    
    clear_buffer(buffer, SIZE);
    printf("Buffer cleared\n");
    
    fill_matrix(matrix, 10);
    printf("Matrix filled\n");
    
    int sum2 = sum_with_stride(arr, 2, SIZE/2);
    printf("Sum with stride 2: %d\n", sum2);
    
    float struct_sum = process_structs(structs, SIZE);
    printf("Struct weight sum: %.2f\n", struct_sum);
    
    int dest[SIZE];
    copy_and_transform(dest, arr, SIZE);
    printf("Array copied and transformed\n");
    
    int found = find_value(arr, SIZE, 42);
    printf("Found 42 at index: %d\n", found);
    
    process_chars(str, sizeof(str) - 1);
    printf("Uppercase string: %s\n", str);
    
    return 0;
}
