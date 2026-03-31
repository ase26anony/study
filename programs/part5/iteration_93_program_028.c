#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 100
#define BUFFER_SIZE 50

/* Structure for testing pointer post-increment */
struct Data {
    int value;
    int count;
    char id;
};

/* Function 1: Copy with post-increment in loop condition */
void copy_with_postinc(char *dest, const char *src) {
    // Classic strcpy-like loop with post-increment
    while ((*dest++ = *src++) != '\0') {
        // Empty body - all work in condition
    }
}

/* Function 2: Sum array with post-increment in update */
int sum_array_postinc(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    const int *end = arr + n;
    
    // Post-increment in loop update
    for (; p < end; sum += *p++);
    
    return sum;
}

/* Function 3: Mixed volatile and non-volatile with post-increment */
int process_mixed_buffers(volatile int *vsrc, int *dest, int n) {
    int processed = 0;
    volatile int *vptr = vsrc;
    int *dptr = dest;
    
    // Post-increment on both volatile and non-volatile pointers
    while (processed < n) {
        *dptr++ = (int)*vptr++;
        processed++;
    }
    
    return processed;
}

/* Function 4: Nested loops with post-increment */
void matrix_process(int matrix[][5], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int *row_ptr = matrix[i];
        int *end = row_ptr + cols;
        
        // Inner loop with post-increment
        while (row_ptr < end) {
            // Access with zero offset then increment
            int val = *row_ptr;
            row_ptr++;
            
            // Simple processing
            if (val < 0) {
                val = -val;
            }
        }
    }
}

/* Function 5: Structure access with post-increment */
int process_structs(struct Data *data, int count) {
    int total = 0;
    struct Data *ptr = data;
    struct Data *end = data + count;
    
    // Post-increment accessing structure fields
    while (ptr < end) {
        total += ptr->value * ptr->count;
        ptr++;  // Post-increment after access
    }
    
    return total;
}

/* Function 6: Comma expression with post-increment */
int find_value(const int *arr, int n, int target) {
    const int *ptr = arr;
    const int *end = arr + n;
    
    // Comma expression: access then increment
    while (ptr < end && (*(ptr++) != target));
    
    return (ptr - arr - 1);
}

/* Function 7: Switch with post-increment in cases */
int process_with_switch(const char *str) {
    const char *p = str;
    int count[3] = {0};
    
    while (*p) {
        switch (*p++) {  // Post-increment in switch expression
            case 'a':
                count[0]++;
                break;
            case 'b':
                count[1]++;
                // Fall through with post-increment
            case 'c':
                count[2]++;
                break;
        }
    }
    
    return count[0] + count[1] + count[2];
}

/* Function 8: Complex control flow with post-increment */
void complex_control_flow(int *arr, int n, int threshold) {
    int *ptr = arr;
    int *end = arr + n;
    
    while (ptr < end) {
        // Post-increment in if condition
        if ((*ptr++) > threshold) {
            // Another post-increment in the taken path
            int temp = *ptr;
            ptr++;
            
            if (temp < 0) {
                // Nested condition with pointer access
                while (ptr < end && *ptr++ != 0);
            }
        } else {
            // Post-increment in not-taken path
            int val = *ptr;
            ptr++;
            
            // Loop with post-increment in condition
            for (int i = 0; i < val && ptr < end; i++) {
                *ptr++ = i;
            }
        }
    }
}

/* Main function with various test cases */
int main() {
    // Test arrays with different qualifiers
    volatile int volatile_buffer[ARRAY_SIZE];
    int regular_buffer[ARRAY_SIZE];
    char string_buffer[BUFFER_SIZE];
    struct Data data_array[20];
    
    // Initialize test data
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_buffer[i] = i * 2;
        regular_buffer[i] = i;
    }
    
    // Initialize string
    const char *test_string = "abcabcdeffghijklmnop";
    strcpy(string_buffer, test_string);
    
    // Initialize struct array
    for (int i = 0; i < 20; i++) {
        data_array[i].value = i;
        data_array[i].count = i % 5;
        data_array[i].id = 'A' + (i % 26);
    }
    
    // Test 1: String copy with post-increment
    char dest[BUFFER_SIZE];
    copy_with_postinc(dest, string_buffer);
    printf("Copy test: %s\n", dest);
    
    // Test 2: Array sum with post-increment
    int sum = sum_array_postinc(regular_buffer, ARRAY_SIZE);
    printf("Array sum: %d\n", sum);
    
    // Test 3: Mixed volatile/non-volatile processing
    int processed = process_mixed_buffers(volatile_buffer, regular_buffer, 50);
    printf("Processed items: %d\n", processed);
    
    // Test 4: Matrix processing
    int matrix[3][5] = {{1, 2, 3, 4, 5},
                       {6, 7, 8, 9, 10},
                       {11, 12, 13, 14, 15}};
    matrix_process(matrix, 3, 5);
    
    // Test 5: Structure processing
    int struct_total = process_structs(data_array, 20);
    printf("Structure total: %d\n", struct_total);
    
    // Test 6: Find value
    int find_idx = find_value(regular_buffer, ARRAY_SIZE, 42);
    printf("Found 42 at index: %d\n", find_idx);
    
    // Test 7: Switch with post-increment
    int switch_count = process_with_switch(test_string);
    printf("Switch processed chars: %d\n", switch_count);
    
    // Test 8: Complex control flow
    complex_control_flow(regular_buffer, ARRAY_SIZE, 25);
    
    // Additional tight loops likely to generate auto-inc RTL
    
    // Byte copy loop
    unsigned char src_bytes[100];
    unsigned char dst_bytes[100];
    unsigned char *s = src_bytes;
    unsigned char *d = dst_bytes;
    unsigned char *end_s = src_bytes + 100;
    
    while (s < end_s) {
        *d++ = *s++;  // Classic post-increment byte copy
    }
    
    // Search loop with post-increment
    const char *search_ptr = test_string;
    while (*search_ptr && *search_ptr++ != 'f');
    
    // Pointer arithmetic in expression
    int *p1 = regular_buffer;
    int *p2 = regular_buffer + 10;
    int diff = 0;
    
    while (p1 < p2) {
        diff += (*p1++ - *p2--);  // Post-increment and post-decrement
    }
    
    printf("Final diff: %d\n", diff);
    
    return 0;
}
