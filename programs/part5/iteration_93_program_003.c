#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SIZE 256
#define PATTERN_SIZE 100

/* Structure for testing pointer post-increment */
struct Data {
    int values[10];
    volatile int flags[10];
    int count;
};

/* Function 1: Copy with post-increment in loop */
void copy_with_postinc(volatile char *dest, const char *src, int n) {
    volatile char *d = dest;
    const char *s = src;
    
    // Tight loop with post-increment in condition
    while (n-- > 0) {
        *d++ = *s++;  // Post-increment on both pointers
    }
}

/* Function 2: Summation with mixed volatile/non-volatile */
int sum_with_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *p = arr;
    
    // For loop with post-increment in update
    for (int i = 0; i < n; i++) {
        sum += *p++;  // Post-increment after access
    }
    
    return sum;
}

/* Function 3: Search with post-increment in condition */
int find_value(volatile int *arr, int n, int target) {
    volatile int *p = arr;
    volatile int *end = arr + n;
    
    // While loop with post-increment in condition
    while (p < end && *p++ != target) {
        // Empty body - increment happens in condition
    }
    
    return (p - 1) - arr;  // Return found position
}

/* Function 4: Complex control flow with post-increment */
void process_data(struct Data *data, volatile int *output) {
    volatile int *out_ptr = output;
    int *val_ptr = data->values;
    volatile int *flag_ptr = data->flags;
    
    // Switch with post-increment in cases
    switch (data->count) {
        case 0:
            // Fall through with post-increment
        case 1:
            *out_ptr++ = *val_ptr++;  // Post-increment
            *out_ptr++ = *flag_ptr++; // Post-increment on volatile
            break;
            
        case 2:
            // If-else with post-increment
            if (*val_ptr > 0) {
                *out_ptr++ = *val_ptr++;  // Taken path
            } else {
                *out_ptr++ = *flag_ptr++; // Not-taken path
            }
            break;
            
        default:
            // Nested loop with post-increment
            for (int i = 0; i < data->count; i++) {
                for (int j = 0; j < 3; j++) {
                    *out_ptr++ = *val_ptr++;  // Inner loop post-increment
                }
            }
            break;
    }
}

/* Function 5: String operations with post-increment */
int string_length(const char *str) {
    const char *p = str;
    
    // While loop with post-increment accessing zero offset
    while (*p++ != '\0') {
        // Simple dereference with zero offset
    }
    
    return p - str - 1;
}

/* Function 6: Comma expression with post-increment */
int copy_with_comma(volatile int *dest, volatile int *src, int n) {
    volatile int *d = dest;
    volatile int *s = src;
    int count = 0;
    
    while (n-- > 0) {
        // Comma expression: access then increment
        int temp = (*s, s++, temp);  // Access, increment, use
        *d = temp;
        d++;
        count++;
    }
    
    return count;
}

/* Function 7: Array indexing with post-increment on pointer */
void transform_array(volatile int *arr, int n) {
    volatile int *ptr = arr;
    int i = 0;
    
    // Mixed array indexing and pointer arithmetic
    while (i < n) {
        // Access with zero offset then increment
        arr[0] = *ptr;  // Zero offset access
        ptr++;
        i++;
        
        // If with post-increment in condition
        if (i < n && *ptr++ > 0) {  // Post-increment in condition
            arr[i] = 1;
        }
    }
}

int main() {
    // Test data
    volatile char buffer1[SIZE];
    volatile char buffer2[SIZE];
    volatile int numbers[PATTERN_SIZE];
    volatile int results[PATTERN_SIZE];
    struct Data data;
    
    // Initialize data
    const char *test_string = "Hello, World!";
    int len = strlen(test_string);
    
    // Initialize arrays
    for (int i = 0; i < PATTERN_SIZE; i++) {
        numbers[i] = i * 2;
        results[i] = 0;
    }
    
    for (int i = 0; i < 10; i++) {
        data.values[i] = i * 10;
        data.flags[i] = i % 2;
    }
    data.count = 5;
    
    printf("Testing auto-increment/decrement patterns:\n");
    
    // Test 1: Copy with post-increment
    copy_with_postinc(buffer1, test_string, len + 1);
    printf("1. Copied string: %s\n", (char*)buffer1);
    
    // Test 2: Summation with post-increment
    int sum = sum_with_postinc(numbers, PATTERN_SIZE);
    printf("2. Sum of numbers: %d\n", sum);
    
    // Test 3: Search with post-increment
    int found_at = find_value(numbers, PATTERN_SIZE, 50);
    printf("3. Found 50 at index: %d\n", found_at);
    
    // Test 4: Complex control flow
    process_data(&data, results);
    printf("4. Processed %d data items\n", data.count);
    
    // Test 5: String length with post-increment
    int str_len = string_length(test_string);
    printf("5. String length: %d\n", str_len);
    
    // Test 6: Comma expression
    int copied = copy_with_comma(results, numbers, 10);
    printf("6. Copied %d items using comma expressions\n", copied);
    
    // Test 7: Array transformation
    transform_array(numbers, 20);
    printf("7. Transformed first 20 array elements\n");
    
    // Additional tight loops for RTL generation
    volatile int *src_ptr = numbers;
    volatile int *dst_ptr = results + 10;
    
    // Tight copy loop likely to generate auto-inc
    for (int i = 0; i < 50; i++) {
        *dst_ptr++ = *src_ptr++;  // Classic post-increment pattern
    }
    
    // Pointer arithmetic in while loop
    volatile int *p = numbers;
    volatile int *end = numbers + PATTERN_SIZE;
    int zero_count = 0;
    
    while (p < end) {
        if (*p++ == 0) {  // Post-increment in condition
            zero_count++;
        }
    }
    printf("8. Found %d zeros in array\n", zero_count);
    
    // Test with different data patterns
    volatile short short_arr[100];
    volatile short *sp = short_arr;
    
    // Post-increment with different type
    for (int i = 0; i < 100; i++) {
        *sp++ = i;  // Post-increment on short pointer
    }
    
    printf("All tests completed successfully.\n");
    
    return 0;
}
