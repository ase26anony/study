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
 * The classic *ptr++ pattern often results in base+0 addressing
 * followed by a separate increment instruction.
 */
int pattern1_sum(const int* arr, int size) {
    int sum = 0;
    const int* p = arr;
    for (int i = 0; i < size; i++) {
        sum += *p;  // Should become *(p + 0) after optimization
        p++;        // Separate increment instruction
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset
 * The compiler's ivopts pass may convert array indexing to pointer
 * arithmetic with base+0 addressing in the loop body.
 */
void pattern2_clear(int* buffer, int size) {
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  // May become *(base + 0) after ivopts
    }
}

/* Pattern 3: Nested loops with invariant base pointer
 * Outer loop sets base pointer, inner loop uses it with zero offset
 */
void pattern3_matrix_init(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int* base = &matrix[j][0];  // Base set in outer loop
        for (int i = 0; i < N; i++) {
            base[i] = i;  // Access with base + 0 in inner loop
        }
    }
}

/* Pattern 4: Explicit pointer increment with stride
 * Clear separation between access and increment
 */
int pattern4_stride_sum(const int* start, int stride, int count) {
    int total = 0;
    const int* ptr = start;
    for (int i = 0; i < count; i++) {
        total += *ptr;    // *(ptr + 0)
        ptr += stride;    // Explicit increment
    }
    return total;
}

/* Pattern 5: Struct access to ensure non-trivial element size
 * Larger element size may influence optimization decisions
 */
struct Data {
    int values[4];
    char tag;
    float weight;
};

void pattern5_struct_init(struct Data* array, int size) {
    for (int i = 0; i < size; i++) {
        array[i].tag = 'A';      // May become base + 0 access
        array[i].weight = 1.0f;  // Another base + 0 access
    }
}

/* Pattern 6: Mixed access patterns to test different optimization paths
 */
void pattern6_mixed(int* dest, const int* src1, const int* src2, int size) {
    int* d = dest;
    const int* s1 = src1;
    const int* s2 = src2;
    
    for (int i = 0; i < size; i++) {
        *d = *s1 + *s2;  // Three memory accesses, all potential base+0
        d++;
        s1++;
        s2++;
    }
}

/* Pattern 7: Loop with pointer increment in separate statement
 * Forces separate increment instruction
 */
void pattern7_separate_inc(char* buf, int size, char value) {
    char* p = buf;
    for (int i = 0; i < size; i++) {
        *p = value;  // *(p + 0)
        p = p + 1;   // Increment as separate statement
    }
}

/* Pattern 8: Access with compile-time constant offset 0
 * Using pointer dereference directly
 */
int pattern8_direct_deref(int* ptr, int size) {
    int sum = 0;
    int* current = ptr;
    for (int i = 0; i < size; i++) {
        sum += *current;  // Direct dereference, should be *(current + 0)
        current += 1;     // Separate increment
    }
    return sum;
}

int main() {
    // Initialize test data
    int arr[N];
    int buffer[N];
    int matrix[M][N];
    struct Data struct_arr[32];
    int dest[N], src1[N], src2[N];
    char char_buf[128];
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        buffer[i] = i * 2;
        src1[i] = i;
        src2[i] = N - i;
    }
    
    // Execute each pattern to ensure code is generated
    int result1 = pattern1_sum(arr, N);
    printf("Pattern 1 sum: %d\n", result1);
    
    pattern2_clear(buffer, N);
    printf("Pattern 2 cleared buffer[0]: %d\n", buffer[0]);
    
    pattern3_matrix_init(matrix);
    printf("Pattern 3 matrix[0][0]: %d\n", matrix[0][0]);
    
    int result4 = pattern4_stride_sum(arr, 2, N/2);
    printf("Pattern 4 stride sum: %d\n", result4);
    
    pattern5_struct_init(struct_arr, 32);
    printf("Pattern 5 struct tag: %c\n", struct_arr[0].tag);
    
    pattern6_mixed(dest, src1, src2, N);
    printf("Pattern 6 dest[0]: %d\n", dest[0]);
    
    pattern7_separate_inc(char_buf, 128, 'X');
    printf("Pattern 7 char_buf[0]: %c\n", char_buf[0]);
    
    int result8 = pattern8_direct_deref(arr, N);
    printf("Pattern 8 direct deref sum: %d\n", result8);
    
    return 0;
}
