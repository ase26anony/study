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
 * This often generates: mem = *(ptr + 0); ptr = ptr + 4;
 */
void pattern1(int *arr, int size) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < size; i++) {
        sum += *p;      /* Access with base + 0 offset */
        p++;            /* Separate increment instruction */
    }
    
    printf("Pattern1 sum: %d\n", sum);
}

/* Pattern 2: Indexed array access with post-increment
 * ivopts may convert this to pointer arithmetic
 */
void pattern2(char *buffer, int size) {
    int i = 0;
    
    while (i < size) {
        buffer[i] = 0;  /* May become *(buffer + 0) after optimization */
        i++;            /* Separate increment */
    }
    
    printf("Pattern2 cleared %d bytes\n", size);
}

/* Pattern 3: Nested loops with invariant base pointer
 * Inner loop accesses with base + 0 offset
 */
void pattern3(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base calculated in outer loop */
        
        for (int i = 0; i < N; i++) {
            base[i] = i * j;        /* Access relative to invariant base */
        }
    }
    
    printf("Pattern3 filled matrix\n");
}

/* Pattern 4: Explicit stride with separate increment
 * Clear separation between access and increment
 */
void pattern4(float *data, int count, int stride) {
    float total = 0.0f;
    float *ptr = data;
    
    for (int i = 0; i < count; i++) {
        total += *ptr;      /* *(ptr + 0) */
        ptr += stride;      /* Explicit increment by stride */
    }
    
    printf("Pattern4 total: %f\n", total);
}

/* Pattern 5: Struct access with pointer increment
 * Tests with non-primitive types
 */
struct Point {
    int x;
    int y;
    int z;
};

void pattern5(struct Point *points, int count) {
    struct Point *p = points;
    
    for (int i = 0; i < count; i++) {
        p->x = i;       /* Access through pointer with offset 0 */
        p->y = i * 2;
        p->z = i * 3;
        p++;            /* Increment by sizeof(struct Point) */
    }
    
    printf("Pattern5 processed %d points\n", count);
}

/* Pattern 6: Mixed access pattern that may confuse optimizers
 * but still leaves some accesses as base + 0
 */
void pattern6(int *arr1, int *arr2, int size) {
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < size; i++) {
        /* Multiple accesses with same base pointer */
        int val = *p1;          /* First access: base + 0 */
        *p2 = val;              /* Different base pointer */
        p1++;                   /* Increment first pointer */
        p2++;                   /* Increment second pointer */
    }
    
    printf("Pattern6 copied %d elements\n", size);
}

/* Pattern 7: Do-while loop with pointer access
 * Different loop structure may affect optimization
 */
void pattern7(unsigned char *data, int length) {
    unsigned char *ptr = data;
    int remaining = length;
    
    do {
        *ptr = 0xFF;        /* *(ptr + 0) */
        ptr++;
        remaining--;
    } while (remaining > 0);
    
    printf("Pattern7 filled %d bytes with 0xFF\n", length);
}

/* Pattern 8: Loop with if condition inside
 * The increment may still be separate from access
 */
void pattern8(int *values, int threshold, int count) {
    int *p = values;
    
    for (int i = 0; i < count; i++) {
        if (*p > threshold) {   /* Access with base + 0 */
            *p = threshold;
        }
        p++;                    /* Separate increment */
    }
    
    printf("Pattern8 clamped %d values\n", count);
}

int main() {
    /* Allocate and initialize test data */
    int *arr1 = (int*)malloc(N * sizeof(int));
    char *buffer = (char*)malloc(N * sizeof(char));
    int (*matrix)[N] = (int(*)[N])malloc(M * N * sizeof(int));
    float *floats = (float*)malloc(N * sizeof(float));
    struct Point *points = (struct Point*)malloc(N * sizeof(struct Point));
    int *arr2 = (int*)malloc(N * sizeof(int));
    unsigned char *bytes = (unsigned char*)malloc(N);
    int *values = (int*)malloc(N * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        if (arr1) arr1[i] = i;
        if (buffer) buffer[i] = 'A' + (i % 26);
        if (floats) floats[i] = i * 0.1f;
        if (points) {
            points[i].x = 0;
            points[i].y = 0;
            points[i].z = 0;
        }
        if (arr2) arr2[i] = 0;
        if (bytes) bytes[i] = 0;
        if (values) values[i] = i * 10;
    }
    
    /* Initialize matrix */
    if (matrix) {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                matrix[i][j] = 0;
            }
        }
    }
    
    /* Execute all patterns */
    if (arr1) pattern1(arr1, N);
    if (buffer) pattern2(buffer, N);
    if (matrix) pattern3(matrix);
    if (floats) pattern4(floats, N, 1);
    if (points) pattern5(points, N);
    if (arr1 && arr2) pattern6(arr1, arr2, N);
    if (bytes) pattern7(bytes, N);
    if (values) pattern8(values, 100, N);
    
    /* Cleanup */
    free(arr1);
    free(buffer);
    free(matrix);
    free(floats);
    free(points);
    free(arr2);
    free(bytes);
    free(values);
    
    return 0;
}
