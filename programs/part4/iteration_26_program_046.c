/* test_auto_inc_dec.c
 * 
 * This program contains loop patterns designed to trigger the uncovered
 * lines in GCC's auto_inc_dec pass (lines 1352-1358 in auto-inc-dec.cc).
 * The patterns create memory accesses with base+0 addressing where
 * find_inc(true) can find a preceding increment instruction.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50

/* Pattern 1: Simple pointer traversal with post-increment
 * This often generates: mem = *(ptr + 0), followed by ptr = ptr + 4
 */
int pattern1_sum(const int *arr, int size) {
    int sum = 0;
    const int *ptr = arr;
    const int *end = arr + size;
    
    while (ptr < end) {
        // Access with current ptr (offset 0), then increment
        sum += *ptr;  // Should become *(ptr + 0) at RTL level
        ptr++;        // Separate increment instruction
    }
    return sum;
}

/* Pattern 2: Array indexing with post-increment of index
 * ivopts may transform this to pointer arithmetic with base+0
 */
void pattern2_zero_buffer(int *buffer, int size) {
    int i = 0;
    while (i < size) {
        buffer[i] = 0;  // May become *(buffer + (i*4)) then optimized
        i++;            // Separate increment
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop
 * Outer loop sets base pointer, inner loop uses it with offset 0
 */
void pattern3_matrix_init(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  // Base for inner loop
        
        for (int i = 0; i < N; i++) {
            // Access with base pointer (offset 0 in inner loop)
            base[i] = i * j;
            // Increment happens through array indexing,
            // but strength reduction may create separate pointer increment
        }
    }
}

/* Pattern 4: Explicit stride with separate increment
 * Very clear pattern for find_inc to match
 */
float pattern4_dot_product(const float *a, const float *b, int size) {
    float dot = 0.0f;
    const float *pa = a;
    const float *pb = b;
    int count = size;
    
    while (count-- > 0) {
        dot += *pa * *pb;  // Two base+0 accesses
        pa += 1;           // Explicit increment
        pb += 1;           // Explicit increment
    }
    return dot;
}

/* Pattern 5: Struct access with pointer increment
 * Tests with non-primitive types
 */
struct Point {
    int x;
    int y;
    int z;
};

int pattern5_sum_points(const struct Point *points, int count) {
    int total = 0;
    const struct Point *ptr = points;
    
    for (int i = 0; i < count; i++) {
        total += ptr->x + ptr->y;  // Accesses at ptr + 0 and ptr + 4
        ptr++;                     // Increment by sizeof(struct Point)
    }
    return total;
}

/* Pattern 6: Mixed access pattern that may confuse optimizers
 * but still generate base+0 patterns
 */
void pattern6_alternating(int *dest, const int *src1, const int *src2, int size) {
    int *d = dest;
    const int *s1 = src1;
    const int *s2 = src2;
    
    for (int i = 0; i < size; i++) {
        *d = *s1 + *s2;  // Three base+0 accesses
        d++;
        s1++;
        s2++;
    }
}

/* Pattern 7: Do-while loop often generates different control flow
 * that might expose the pattern differently
 */
int pattern7_find_value(const int *arr, int size, int value) {
    const int *ptr = arr;
    int remaining = size;
    
    if (remaining <= 0) return -1;
    
    do {
        if (*ptr == value)  // base+0 access
            return (int)(ptr - arr);
        ptr++;              // increment
    } while (--remaining > 0);
    
    return -1;
}

/* Pattern 8: Loop with multiple statements that all use the pointer
 * Tests if auto_inc_dec can handle multiple memory references
 */
void pattern8_copy_and_transform(int *dest, const int *src, int size) {
    int *d = dest;
    const int *s = src;
    
    for (int i = 0; i < size; i++) {
        int val = *s;        // First access: base+0
        val = val * 2 + 1;   // Transformation
        *d = val;            // Second access: base+0
        d++;                 // Increment
        s++;                 // Increment
    }
}

int main() {
    // Initialize test data
    int arr1[N];
    int arr2[N];
    float farr1[N];
    float farr2[N];
    int matrix[M][N];
    struct Point points[N];
    
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = N - i;
        farr1[i] = i * 0.5f;
        farr2[i] = i * 1.5f;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = j * N + i;
        }
    }
    
    // Execute all patterns
    int sum1 = pattern1_sum(arr1, N);
    pattern2_zero_buffer(arr2, N);
    pattern3_matrix_init(matrix);
    float dot = pattern4_dot_product(farr1, farr2, N);
    int point_sum = pattern5_sum_points(points, N);
    
    int dest[N];
    pattern6_alternating(dest, arr1, arr2, N);
    
    int index = pattern7_find_value(arr1, N, 42);
    
    int transformed[N];
    pattern8_copy_and_transform(transformed, arr1, N);
    
    // Print results to prevent dead code elimination
    printf("Results:\n");
    printf("  Sum1: %d\n", sum1);
    printf("  Dot product: %f\n", dot);
    printf("  Point sum: %d\n", point_sum);
    printf("  Found 42 at index: %d\n", index);
    printf("  Transformed[10]: %d\n", transformed[10]);
    
    return 0;
}
