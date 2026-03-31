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

/* Function 1: Copy with post-increment in loop */
void copy_with_postinc(char *dest, const char *src, size_t n) {
    // Classic strcpy-like loop with post-increment
    while (n-- > 0) {
        *dest++ = *src++;
    }
}

/* Function 2: Summation with post-increment */
int sum_array(const int *arr, int size) {
    int sum = 0;
    const int *end = arr + size;
    
    // Post-increment in loop condition
    while (arr < end) {
        sum += *arr++;
    }
    return sum;
}

/* Function 3: Search with post-increment */
int find_value(const int *arr, int size, int target) {
    const int *p = arr;
    const int *end = arr + size;
    
    // Post-increment in condition with early exit
    while (p < end && *p++ != target) {
        // Empty body - all work in condition
    }
    
    return (p - 1) - arr;  // Return index where found
}

/* Function 4: Mixed volatile and non-volatile pointers */
int process_volatile(volatile int *vptr, int *regptr, int size) {
    int result = 0;
    
    // Use volatile pointer with post-increment
    for (int i = 0; i < size; i++) {
        result += *vptr++;
    }
    
    // Use regular pointer with post-increment
    int *end = regptr + size;
    while (regptr < end) {
        result += *regptr++;
    }
    
    return result;
}

/* Function 5: Structure pointer post-increment */
int process_structs(struct Data *sptr, int count) {
    int total = 0;
    
    // Post-increment on structure pointer
    for (int i = 0; i < count; i++) {
        total += sptr->value;
        sptr++;  // Post-increment after access
    }
    
    return total;
}

/* Function 6: Complex control flow with post-increment */
int complex_control_flow(int *arr, int size, int threshold) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + size;
    
    // Nested control structures
    for (int i = 0; i < size; i++) {
        if (*ptr > threshold) {
            // Taken path with post-increment
            sum += *ptr++;
            
            // Switch with fallthrough
            switch (sum % 3) {
                case 0:
                    sum += *ptr++;  // Post-increment in switch case
                    break;
                case 1:
                    // Fall through with post-increment
                case 2:
                    sum -= *ptr++;  // Post-increment in fallthrough case
                    break;
            }
        } else {
            // Not-taken path also with post-increment
            sum -= *ptr++;
        }
        
        // Inner loop with post-increment
        int *inner = ptr;
        while (inner < end && *inner != 0) {
            sum += *inner++;
        }
    }
    
    return sum;
}

/* Function 7: Comma expression with post-increment */
int comma_expression_test(int *ptr) {
    // Use comma to sequence access and increment
    int val = (*ptr++, *ptr);  // First increment, then access
    return val;
}

/* Function 8: Array indexing with post-increment */
int array_index_postinc(int *arr, int *index) {
    // Array access with variable index followed by increment
    int val = arr[*index];
    (*index)++;  // Post-increment of index
    
    // Also test arr[0] with pointer increment
    int *ptr = arr;
    int zero_index_val = ptr[0];
    ptr++;  // Post-increment
    
    return val + zero_index_val;
}

int main() {
    // Test data
    char source[BUFFER_SIZE] = "Test string for post-increment operations";
    char destination[BUFFER_SIZE];
    
    int data_array[ARRAY_SIZE];
    volatile int volatile_array[ARRAY_SIZE];
    
    struct Data struct_array[20];
    
    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data_array[i] = i * 2;
        volatile_array[i] = i * 3;
    }
    
    for (int i = 0; i < 20; i++) {
        struct_array[i].value = i * 5;
        struct_array[i].count = i;
        struct_array[i].id = 'A' + i;
    }
    
    // Test 1: Copy with post-increment
    copy_with_postinc(destination, source, strlen(source) + 1);
    printf("Copy test: %s\n", destination);
    
    // Test 2: Summation with post-increment
    int sum = sum_array(data_array, ARRAY_SIZE);
    printf("Sum of array: %d\n", sum);
    
    // Test 3: Search with post-increment
    int search_index = find_value(data_array, ARRAY_SIZE, 50);
    printf("Found value 50 at index: %d\n", search_index);
    
    // Test 4: Mixed volatile/non-volatile
    int mixed_result = process_volatile(volatile_array, data_array, 10);
    printf("Mixed volatile/non-volatile result: %d\n", mixed_result);
    
    // Test 5: Structure pointer post-increment
    int struct_sum = process_structs(struct_array, 20);
    printf("Structure sum: %d\n", struct_sum);
    
    // Test 6: Complex control flow
    int complex_result = complex_control_flow(data_array, ARRAY_SIZE, 25);
    printf("Complex control flow result: %d\n", complex_result);
    
    // Test 7: Comma expression
    int comma_result = comma_expression_test(data_array);
    printf("Comma expression result: %d\n", comma_result);
    
    // Test 8: Array indexing with post-increment
    int index = 5;
    int array_index_result = array_index_postinc(data_array, &index);
    printf("Array index post-increment result: %d (new index: %d)\n", 
           array_index_result, index);
    
    // Additional tight loops likely to generate auto-inc RTL
    
    // Byte copy loop
    unsigned char byte_src[100];
    unsigned char byte_dst[100];
    for (int i = 0; i < 100; i++) byte_src[i] = i;
    
    unsigned char *s = byte_src;
    unsigned char *d = byte_dst;
    unsigned char *end_s = byte_src + 100;
    
    // This should generate post-increment addressing
    while (s < end_s) {
        *d++ = *s++;
    }
    
    // Verify the copy
    int copy_ok = memcmp(byte_src, byte_dst, 100) == 0;
    printf("Byte copy verification: %s\n", copy_ok ? "PASS" : "FAIL");
    
    // String processing with post-increment
    char str[] = "Testing,1,2,3";
    char *p = str;
    int comma_count = 0;
    
    // Post-increment in loop condition
    while (*p != '\0') {
        if (*p++ == ',') {
            comma_count++;
        }
    }
    printf("Comma count in string: %d\n", comma_count);
    
    return 0;
}
