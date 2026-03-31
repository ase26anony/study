/* test_auto_inc_dec.c
 * This program contains loop patterns designed to trigger the uncovered
 * lines in GCC's auto_inc_dec pass where find_inc(true) is called with
 * reg1_val = 0 (base + 0 addressing mode).
 */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int *arr, int size) {
    int sum = 0;
    const int *ptr = arr;
    
    /* This should generate: mem = *(ptr + 0), then ptr = ptr + 4 */
    for (int i = 0; i < size; i++) {
        sum += *ptr;  /* Access with base + 0 offset */
        ptr++;        /* Separate increment instruction */
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
        int *base = &matrix[j][0];  /* Base calculated in outer loop */
        
        /* Inner loop accesses with base + 0 */
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
        sum += *ptr;      /* *(ptr + 0) */
        ptr += stride;    /* Candidate increment for find_inc */
    }
    return sum;
}

/* Pattern 5: Pointer arithmetic with different data types */
void process_chars(char *str, int len) {
    char *p = str;
    
    /* char* increment by 1 */
    for (int i = 0; i < len; i++) {
        *p = (*p) + 1;  /* Access with offset 0 */
        p++;            /* Increment by 1 */
    }
}

/* Pattern 6: Struct access to test with larger increments */
struct Point {
    int x;
    int y;
    int z;
};

void translate_points(struct Point *points, int count, int dx, int dy, int dz) {
    struct Point *ptr = points;
    
    /* Struct access with increment by sizeof(struct Point) */
    for (int i = 0; i < count; i++) {
        ptr->x += dx;  /* Multiple accesses with same base */
        ptr->y += dy;
        ptr->z += dz;
        ptr++;         /* Increment by struct size */
    }
}

/* Pattern 7: Mixed patterns to increase coverage */
void mixed_access(int *arr1, int *arr2, int size) {
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < size; i++) {
        /* Two memory accesses with potential auto-inc opportunities */
        int val = *p1 + *p2;
        *p1 = val;
        *p2 = val;
        
        p1++;
        p2++;
    }
}

/* Main function to exercise all patterns */
int main() {
    /* Initialize test data */
    int arr[N];
    int buffer[N];
    int matrix[M][N];
    char str[] = "Hello, World!";
    struct Point points[20];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        buffer[i] = i * 2;
    }
    
    for (int i = 0; i < 20; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
    }
    
    /* Exercise Pattern 1 */
    int total = sum_array(arr, N);
    printf("Sum of array: %d\n", total);
    
    /* Exercise Pattern 2 */
    clear_buffer(buffer, N);
    printf("Buffer cleared\n");
    
    /* Exercise Pattern 3 */
    fill_matrix(matrix);
    printf("Matrix filled\n");
    
    /* Exercise Pattern 4 */
    total = sum_with_stride(arr, N/2, 2);
    printf("Sum with stride 2: %d\n", total);
    
    /* Exercise Pattern 5 */
    process_chars(str, sizeof(str) - 1);
    printf("Processed string: %s\n", str);
    
    /* Exercise Pattern 6 */
    translate_points(points, 20, 1, 2, 3);
    printf("Points translated\n");
    
    /* Exercise Pattern 7 */
    mixed_access(arr, buffer, N);
    printf("Mixed access completed\n");
    
    return 0;
}
