#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 100
#define BUFFER_SIZE 50

/* Structure for testing pointer post-increment with field access */
struct Data {
    int value;
    char id;
    float weight;
};

/* Function 1: Copy with post-increment in loop condition */
void copy_with_postinc(char *dest, const char *src) {
    // Classic strcpy-like loop with post-increment
    while ((*dest++ = *src++) != '\0') {
        // Empty body - all work in condition
    }
}

/* Function 2: Summation with post-increment in update statement */
int sum_array(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    const int *end = arr + n;
    
    // Post-increment in loop update
    for (; p < end; sum += *p++) {
        // Work done in update statement
    }
    
    return sum;
}

/* Function 3: Mixed volatile and non-volatile pointers */
int find_in_volatile(volatile int *arr, int target, int n) {
    volatile int *p = arr;
    volatile int *end = arr + n;
    
    // Post-increment in condition with volatile
    while (p < end && *p++ != target) {
        // Search loop
    }
    
    return (p - 1) - arr;  // Return index
}

/* Function 4: Complex control flow with post-increment */
int process_with_branches(int *data, int n, int threshold) {
    int count = 0;
    int *ptr = data;
    int *end = data + n;
    
    while (ptr < end) {
        // Post-increment in if condition
        if (*ptr++ > threshold) {
            count++;
            
            // Nested condition with another post-increment
            if (ptr < end && *ptr++ < 0) {
                count++;
            }
        } else {
            // Else branch also uses post-increment
            if (ptr < end && *ptr++ == 0) {
                count--;
            }
        }
    }
    
    return count;
}

/* Function 5: Structure pointer post-increment */
float sum_structure_weights(struct Data *sarr, int n) {
    float total = 0.0f;
    struct Data *sptr = sarr;
    struct Data *end = sarr + n;
    
    // Post-increment accessing structure field
    for (; sptr < end; total += sptr++->weight) {
        // Work in update statement
    }
    
    return total;
}

/* Function 6: Comma expression with post-increment */
int copy_with_comma(char *dest, const char *src, int max) {
    int i = 0;
    const char *s = src;
    char *d = dest;
    
    // Comma expression: access then increment
    while (i < max && ((*d = *s), d++, s++, *s != '\0')) {
        i++;
    }
    
    *d = '\0';
    return i;
}

/* Function 7: Switch statement with post-increment */
int process_switch(int *values, int n) {
    int result = 0;
    int *p = values;
    int *end = values + n;
    
    while (p < end) {
        switch (*p++) {
            case 0:
                // Fall through with post-increment
                result += *p++;
                break;
            case 1:
                result -= *p++;
                // No break, falls through
            case 2:
                result *= 2;
                if (p < end) {
                    result += *p++;
                }
                break;
            case 3:
                // Nested loop inside switch
                for (int i = 0; i < 2 && p < end; i++) {
                    result += *p++;
                }
                break;
            default:
                result += *p++;
                break;
        }
    }
    
    return result;
}

/* Function 8: Byte buffer copy with post-increment */
void copy_buffer(volatile char *dest, const char *src, int size) {
    volatile char *d = dest;
    const char *s = src;
    int i = 0;
    
    // Tight loop with post-increment
    while (i++ < size) {
        *d++ = *s++;
    }
}

int main() {
    // Test data arrays
    char source[BUFFER_SIZE] = "Test string for post-increment operations";
    char destination[BUFFER_SIZE];
    
    int numbers[ARRAY_SIZE];
    volatile int volatile_numbers[ARRAY_SIZE];
    
    struct Data struct_array[20];
    
    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        numbers[i] = i * 2;
        volatile_numbers[i] = i * 3;
    }
    
    for (int i = 0; i < 20; i++) {
        struct_array[i].value = i;
        struct_array[i].id = 'A' + (i % 26);
        struct_array[i].weight = i * 1.5f;
    }
    
    // Test 1: String copy with post-increment
    copy_with_postinc(destination, source);
    printf("Copy test: %s\n", destination);
    
    // Test 2: Array summation
    int sum = sum_array(numbers, ARRAY_SIZE);
    printf("Sum of numbers: %d\n", sum);
    
    // Test 3: Volatile array search
    int index = find_in_volatile(volatile_numbers, 150, ARRAY_SIZE);
    printf("Found 150 at index (approx): %d\n", index);
    
    // Test 4: Complex control flow
    int count = process_with_branches(numbers, ARRAY_SIZE, 50);
    printf("Count above threshold: %d\n", count);
    
    // Test 5: Structure processing
    float weight_sum = sum_structure_weights(struct_array, 20);
    printf("Total weight: %.2f\n", weight_sum);
    
    // Test 6: Comma expression copy
    char buffer[20];
    int copied = copy_with_comma(buffer, source, 10);
    printf("Copied %d chars: %s\n", copied, buffer);
    
    // Test 7: Switch with post-increment
    int switch_result = process_switch(numbers, 10);
    printf("Switch result: %d\n", switch_result);
    
    // Test 8: Volatile buffer copy
    volatile char volatile_buffer[BUFFER_SIZE];
    copy_buffer(volatile_buffer, source, 20);
    printf("Volatile copy done\n");
    
    // Additional tight loops that should generate auto-inc
    // Pointer arithmetic in loop
    int *p1 = numbers;
    int *p2 = numbers + ARRAY_SIZE/2;
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        *p2++ = *p1++ + 1;
    }
    
    // Post-decrement test
    int reverse_sum = 0;
    int *rp = numbers + ARRAY_SIZE - 1;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        reverse_sum += *rp--;
    }
    printf("Reverse sum: %d\n", reverse_sum);
    
    // Array access with index zero and post-increment
    int *ptr = numbers;
    int zero_index_test = 0;
    for (int i = 0; i < 5; i++) {
        // Access arr[0] equivalent through pointer
        zero_index_test += ptr[0];
        ptr++;  // Post-increment after access
    }
    printf("Zero index test sum: %d\n", zero_index_test);
    
    return 0;
}
