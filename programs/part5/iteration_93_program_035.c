#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 100
#define BUFFER_SIZE 50

/* Structure for testing pointer post-increment with field access */
struct Data {
    int value;
    char id;
    float score;
};

/* Function 1: Copy with post-increment in loop condition */
void copy_with_postinc(char *dest, const char *src) {
    // Classic strcpy-like loop with post-increment
    while ((*dest++ = *src++) != '\0') {
        // Empty body - all work in condition
    }
}

/* Function 2: Summation with mixed volatile and non-volatile */
int sum_with_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *p = arr;
    
    // Loop with post-increment in update statement
    for (int i = 0; i < n; i++) {
        sum += *p++;  // Post-increment on volatile pointer
    }
    
    return sum;
}

/* Function 3: Search with post-increment in complex control flow */
int find_value(int *array, int size, int target) {
    int *ptr = array;
    int found = -1;
    
    // Multiple basic blocks with post-increment
    if (size > 0) {
        // Post-increment in loop condition
        while (ptr < &array[size] && *ptr++ != target) {
            // Continue searching
        }
        
        // Adjust for post-increment
        found = (ptr - array) - 1;
        if (found >= size || array[found] != target) {
            found = -1;
        }
    }
    
    return found;
}

/* Function 4: Structure array processing with post-increment */
float process_structs(struct Data *sptr, int count) {
    float total = 0.0f;
    struct Data *current = sptr;
    
    // Nested control flow with post-increment
    for (int i = 0; i < count; i++) {
        // Access structure field with pointer, then increment
        total += current->score;
        
        // Post-increment in different contexts
        switch (current->id) {
            case 'A':
                // Post-increment in taken path
                current->value = *((int*)current++);
                break;
            case 'B':
                // Post-increment in not-taken path
                if (current->value > 0) {
                    current->value = *((int*)current);
                }
                current++;
                break;
            default:
                // Comma expression with post-increment
                current->value = (temp = *((int*)current), current++, temp);
                break;
        }
    }
    
    return total;
}

/* Function 5: Byte buffer copy with volatile */
void copy_buffer(volatile char *dst, volatile char *src, int len) {
    volatile char *d = dst;
    volatile char *s = src;
    
    // Tight loop with post-increment
    int i = 0;
    while (i++ < len) {
        *d++ = *s++;  // Both pointers post-increment
    }
}

/* Function 6: Mixed qualifiers in same expression */
int mixed_qualifiers_postinc(volatile int *vptr, int *regptr, int n) {
    int result = 0;
    
    // Use both volatile and non-volatile pointers
    for (int i = 0; i < n; i++) {
        // Access volatile, then regular with post-increment
        int volatile_val = *vptr++;
        int regular_val = *regptr++;
        
        result += volatile_val * regular_val;
    }
    
    return result;
}

/* Function 7: Array zeroing with post-decrement */
void zero_array(int *arr, int size) {
    int *p = &arr[size - 1];
    
    // Post-decrement loop
    while (size-- > 0) {
        *p-- = 0;  // Post-decrement
    }
}

/* Function 8: String concatenation with post-increment */
void concat_strings(char *dest, const char *src1, const char *src2) {
    // Copy first string
    while ((*dest++ = *src1++) != '\0');
    
    // Back up for null terminator
    dest--;
    
    // Copy second string
    while ((*dest++ = *src2++) != '\0');
}

int main() {
    // Test data setup
    char source[BUFFER_SIZE] = "Test string for post-increment operations";
    char destination[BUFFER_SIZE * 2];
    
    volatile int volatile_array[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    int mixed_array[ARRAY_SIZE];
    
    struct Data struct_array[20];
    
    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i + 1;
        mixed_array[i] = i * 3;
    }
    
    for (int i = 0; i < 20; i++) {
        struct_array[i].value = i * 10;
        struct_array[i].id = (i % 3) == 0 ? 'A' : ((i % 3) == 1 ? 'B' : 'C');
        struct_array[i].score = i * 1.5f;
    }
    
    printf("=== Testing Post-Increment/Decrement Patterns ===\n");
    
    // Test 1: String copy with post-increment
    copy_with_postinc(destination, source);
    printf("1. Copy result: %s\n", destination);
    
    // Test 2: Summation with volatile
    int sum1 = sum_with_postinc(volatile_array, ARRAY_SIZE);
    printf("2. Volatile array sum: %d\n", sum1);
    
    // Test 3: Search with post-increment
    int search_target = 42;
    int found_idx = find_value(regular_array, ARRAY_SIZE, search_target);
    printf("3. Found %d at index: %d\n", search_target, found_idx);
    
    // Test 4: Structure processing
    float struct_total = process_structs(struct_array, 20);
    printf("4. Structure total score: %.2f\n", struct_total);
    
    // Test 5: Buffer copy with volatile
    volatile char vol_src[10] = "Volatile!";
    volatile char vol_dst[10];
    copy_buffer(vol_dst, vol_src, 9);
    vol_dst[9] = '\0';
    printf("5. Volatile copy: %s\n", (char*)vol_dst);
    
    // Test 6: Mixed qualifiers
    int mixed_result = mixed_qualifiers_postinc(volatile_array, regular_array, 10);
    printf("6. Mixed qualifiers result: %d\n", mixed_result);
    
    // Test 7: Array zeroing with post-decrement
    int test_zero[10] = {1,2,3,4,5,6,7,8,9,10};
    zero_array(test_zero, 10);
    printf("7. Array zeroing - first element: %d\n", test_zero[0]);
    
    // Test 8: String concatenation
    char str1[] = "Hello, ";
    char str2[] = "World!";
    char result[50];
    concat_strings(result, str1, str2);
    printf("8. Concatenated: %s\n", result);
    
    // Additional complex pattern: Nested loops with post-increment
    printf("\n=== Additional Complex Patterns ===\n");
    
    // Matrix-like processing
    int matrix[5][5];
    int *flat_ptr = &matrix[0][0];
    
    // Initialize with post-increment
    for (int i = 0; i < 25; i++) {
        *flat_ptr++ = i;
    }
    
    // Process with pointer arithmetic
    int matrix_sum = 0;
    int *row_ptr = &matrix[0][0];
    for (int row = 0; row < 5; row++) {
        int *col_ptr = row_ptr;
        for (int col = 0; col < 5; col++) {
            // Post-increment in inner loop
            matrix_sum += *col_ptr++;
        }
        row_ptr += 5;  // Next row
    }
    printf("Matrix sum: %d\n", matrix_sum);
    
    // Test with pointer to array[0] pattern (zero offset)
    int *ptr_to_first = &regular_array[0];
    int first_value = *ptr_to_first++;  // Dereference with zero offset, then increment
    printf("First value (zero offset): %d, next value: %d\n", 
           first_value, *ptr_to_first);
    
    // Comma expression test
    int temp_array[5] = {10, 20, 30, 40, 50};
    int *cptr = temp_array;
    int comma_result = (temp = *cptr, cptr++, temp);  // Explicit comma expression
    printf("Comma expression result: %d, next: %d\n", comma_result, *cptr);
    
    return 0;
}
