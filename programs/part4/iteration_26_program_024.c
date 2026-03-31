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
        // Access with current ptr (base + 0 at RTL level)
        sum += *ptr;
        // Separate increment instruction
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
        // Array access: buffer[i] -> *(buffer + i*4)
        // After optimization: *(ptr + 0) where ptr = buffer + i*4
        buffer[i] = i * 2;
        // Separate increment
        i++;
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop
 * Outer loop sets base pointer, inner loop uses base + 0
 */
void pattern3_nested_invariant_base(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        // Base pointer calculated in outer loop
        int *base = &matrix[j][0];
        int *inner_ptr = base;
        
        for (int i = 0; i < N; i++) {
            // Access with base + 0 addressing
            *inner_ptr = i + j;
            // Separate increment
            inner_ptr++;
        }
    }
}

/* Pattern 4: Pointer with explicit stride increment
 * Clear separation between access and increment
 */
int pattern4_explicit_stride(int *arr, int size, int stride) {
    int total = 0;
    int *ptr = arr;
    int count = 0;
    
    while (count < size) {
        // Memory access with current ptr
        total += *ptr;
        // Explicit increment by stride (constant after propagation)
        ptr += stride;
        count++;
    }
    return total;
}

/* Pattern 5: Struct access with pointer increment
 * Tests with non-primitive types
 */
struct Data {
    int value;
    char tag;
    float weight;
};

int pattern5_struct_traversal(struct Data *data, int count) {
    int sum = 0;
    struct Data *current = data;
    int i = 0;
    
    while (i < count) {
        // Access struct member
        sum += current->value;
        // Separate increment by struct size
        current++;
        i++;
    }
    return sum;
}

/* Pattern 6: Do-while loop for different loop structure
 * Might create different RTL patterns
 */
void pattern6_dowhile_zero_offset(char *buffer, int size) {
    char *p = buffer;
    int i = 0;
    
    if (size <= 0) return;
    
    do {
        // Access with p + 0
        *p = (char)i;
        // Increment after access
        p++;
        i++;
    } while (i < size);
}

/* Pattern 7: Two-dimensional access linearized
 * Single pointer traversing 2D array
 */
void pattern7_linearized_2d(int matrix[M][N]) {
    int *linear_ptr = &matrix[0][0];
    int total_elements = M * N;
    
    for (int i = 0; i < total_elements; i++) {
        // Simple access with pointer
        linear_ptr[i] = i;
        // Note: This might optimize differently
    }
}

/* Pattern 8: Reverse traversal with decrement
 * Tests auto-decrement patterns too
 */
void pattern8_reverse_traversal(int *arr, int size) {
    int *ptr = arr + size - 1;
    
    for (int i = size - 1; i >= 0; i--) {
        // Access with ptr + 0
        *ptr = i;
        // Decrement instruction
        ptr--;
    }
}

/* Main function to exercise all patterns */
int main() {
    // Initialize test data
    int array1[N];
    int array2[N];
    int matrix[M][N];
    struct Data struct_array[N];
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        array1[i] = i;
        array2[i] = i * 2;
        struct_array[i].value = i * 3;
        struct_array[i].tag = 'A' + (i % 26);
        struct_array[i].weight = i * 0.1f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = i * N + j;
        }
    }
    
    // Execute all patterns
    int result1 = pattern1_simple_pointer(array1, N);
    pattern2_indexed_postinc(array2, N);
    pattern3_nested_invariant_base(matrix);
    int result4 = pattern4_explicit_stride(array1, N/2, 2);
    int result5 = pattern5_struct_traversal(struct_array, N);
    
    char buffer[N];
    pattern6_dowhile_zero_offset(buffer, N);
    
    pattern7_linearized_2d(matrix);
    pattern8_reverse_traversal(array1, N);
    
    // Print results to prevent optimization removal
    printf("Results: %d %d %d\n", result1, result4, result5);
    printf("Buffer[0]=%d, Matrix[0][0]=%d\n", buffer[0], matrix[0][0]);
    
    return 0;
}
