#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 100
#define BUFFER_SIZE 50

/* Structure for testing pointer post-increment with field access */
struct Data {
    int value;
    int count;
    char id;
};

/* Function 1: Copy with post-increment pointers (tight loop) */
void copy_with_postinc(char *dest, const char *src) {
    // Classic strcpy-like loop that should generate post-increment
    while ((*dest++ = *src++) != '\0') {
        // Empty body - all work in condition
    }
}

/* Function 2: Summation with post-increment in loop */
int sum_array_postinc(const int *arr, int n) {
    int sum = 0;
    const int *end = arr + n;
    
    // Loop with post-increment in update statement
    for (const int *p = arr; p < end; sum += *p++) {
        // All work in for statement
    }
    
    return sum;
}

/* Function 3: Search with post-increment in condition */
int find_value_postinc(const int *arr, int n, int target) {
    const int *p = arr;
    const int *end = arr + n;
    
    // Post-increment in while condition
    while (p < end && *p++ != target) {
        // Empty - search happens in condition
    }
    
    return (p - 1) - arr;  // Return index of found element
}

/* Function 4: Mixed volatile and non-volatile pointers */
int process_volatile_mix(volatile int *varr, int *normal_arr, int n) {
    int result = 0;
    
    // Use volatile pointer with post-increment
    volatile int *vp = varr;
    for (int i = 0; i < n; i++) {
        // Comma expression: access then increment
        int temp = *vp;
        vp++;
        result += temp;
    }
    
    // Use normal pointer with post-increment
    int *np = normal_arr;
    for (int i = 0; i < n; i++) {
        result -= *np++;
    }
    
    return result;
}

/* Function 5: Structure pointer post-increment */
int process_structs(struct Data *data, int count) {
    int total = 0;
    struct Data *ptr = data;
    
    // Post-increment structure pointer
    for (int i = 0; i < count; i++) {
        // Access field, then increment pointer
        total += ptr->value;
        ptr++;
    }
    
    return total;
}

/* Function 6: Nested loops with post-increment */
void matrix_process(int matrix[][10], int rows) {
    for (int i = 0; i < rows; i++) {
        int *row_ptr = matrix[i];
        int sum = 0;
        
        // Inner loop with post-increment
        for (int j = 0; j < 10; j++) {
            sum += *row_ptr++;
        }
        
        // Store sum in first element
        matrix[i][0] = sum;
    }
}

/* Function 7: Switch case with post-increment */
int switch_with_postinc(int *arr, int index) {
    int result = 0;
    int *ptr = arr;
    
    switch (index) {
        case 0:
            // Post-increment in case 0
            result = *ptr++;
            // Fall through
        case 1:
            // Post-increment in case 1
            result += *ptr++;
            break;
        case 2:
            // Different pattern
            result = ptr[0];
            ptr++;
            break;
        default:
            // Loop with post-increment
            for (int i = 0; i < 3; i++) {
                result += *ptr++;
            }
    }
    
    return result;
}

/* Function 8: Conditional paths with post-increment */
int conditional_postinc(int *arr, int n, int threshold) {
    int *ptr = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        if (*ptr > threshold) {
            // Taken path: post-increment
            sum += *ptr++;
        } else {
            // Not-taken path: also post-increment
            sum -= *ptr++;
        }
    }
    
    return sum;
}

/* Function 9: Zero offset array access (reg1_val = 0 pattern) */
int zero_offset_access(int **ptr_array, int count) {
    int total = 0;
    
    for (int i = 0; i < count; i++) {
        // Access element 0 with pointer that gets incremented
        int *ptr = ptr_array[i];
        total += ptr[0];  // Zero offset
        // Simulate pointer increment in separate step
        ptr_array[i] = ptr + 1;
    }
    
    return total;
}

/* Function 10: Byte buffer processing with post-increment */
int process_byte_buffer(volatile unsigned char *buffer, int size) {
    volatile unsigned char *p = buffer;
    int checksum = 0;
    
    // Tight byte processing loop
    for (int i = 0; i < size; i++) {
        checksum ^= *p++;  // Post-increment volatile pointer
    }
    
    return checksum;
}

int main() {
    // Test data setup
    char source[BUFFER_SIZE] = "Test string for post-increment operations";
    char destination[BUFFER_SIZE];
    
    int int_array[ARRAY_SIZE];
    volatile int volatile_array[ARRAY_SIZE];
    
    struct Data struct_array[20];
    int matrix[5][10];
    
    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 2;
        volatile_array[i] = i * 3;
    }
    
    for (int i = 0; i < 20; i++) {
        struct_array[i].value = i * 10;
        struct_array[i].count = i;
        struct_array[i].id = 'A' + i;
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    // Test 1: String copy with post-increment
    copy_with_postinc(destination, source);
    printf("Copy test: %s\n", destination);
    
    // Test 2: Array summation
    int sum = sum_array_postinc(int_array, ARRAY_SIZE);
    printf("Array sum: %d\n", sum);
    
    // Test 3: Search with post-increment
    int index = find_value_postinc(int_array, ARRAY_SIZE, 50);
    printf("Found value 50 at index: %d\n", index);
    
    // Test 4: Mixed volatile/non-volatile
    int mix_result = process_volatile_mix(volatile_array, int_array, ARRAY_SIZE);
    printf("Mixed volatile result: %d\n", mix_result);
    
    // Test 5: Structure processing
    int struct_total = process_structs(struct_array, 20);
    printf("Structure total: %d\n", struct_total);
    
    // Test 6: Nested matrix processing
    matrix_process(matrix, 5);
    printf("Matrix row sums: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", matrix[i][0]);
    }
    printf("\n");
    
    // Test 7: Switch with post-increment
    int switch_result = switch_with_postinc(int_array, 2);
    printf("Switch result: %d\n", switch_result);
    
    // Test 8: Conditional paths
    int cond_result = conditional_postinc(int_array, 10, 10);
    printf("Conditional result: %d\n", cond_result);
    
    // Test 9: Zero offset access
    int *ptr_array[5];
    for (int i = 0; i < 5; i++) {
        ptr_array[i] = &int_array[i * 10];
    }
    int zero_offset_result = zero_offset_access(ptr_array, 5);
    printf("Zero offset result: %d\n", zero_offset_result);
    
    // Test 10: Byte buffer with volatile
    unsigned char byte_buffer[100];
    for (int i = 0; i < 100; i++) {
        byte_buffer[i] = i;
    }
    int checksum = process_byte_buffer((volatile unsigned char *)byte_buffer, 100);
    printf("Byte buffer checksum: %d\n", checksum);
    
    // Additional complex pattern: Multiple post-increment in same expression
    int *p1 = int_array;
    int *p2 = &int_array[50];
    int complex_result = 0;
    
    // Multiple pointers with post-increment
    for (int i = 0; i < 25; i++) {
        complex_result += *p1++ + *p2++;  // Two post-increments in one expression
    }
    printf("Complex pattern result: %d\n", complex_result);
    
    // Test with different data patterns to avoid dead code elimination
    volatile int test_counter = 0;
    int *volatile volatile_ptr = int_array;
    
    // Loop that should generate auto-increment RTL
    for (int i = 0; i < 10; i++) {
        test_counter += *volatile_ptr++;
    }
    printf("Volatile pointer test: %d\n", test_counter);
    
    return 0;
}
