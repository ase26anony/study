#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SIZE 256
#define BUFFER_SIZE 128

/* Structure for testing pointer post-increment with field access */
struct Data {
    int value;
    char id;
    float weight;
};

/* Function 1: Copy with post-increment pointers (classic strcpy-like) */
void copy_with_postinc(volatile char *dest, const char *src, int n) {
    volatile char *d = dest;
    const char *s = src;
    
    // Basic post-increment in loop condition
    while (n-- > 0) {
        *d++ = *s++;  // Should generate post-increment addressing
    }
}

/* Function 2: Summation with mixed volatile/non-volatile pointers */
int sum_with_postinc(volatile int *arr, int count) {
    volatile int *p = arr;
    int sum = 0;
    
    // Post-increment in loop update with zero offset access
    for (int i = 0; i < count; i++) {
        sum += *p++;  // *p with p++ - zero offset, base pointer
    }
    
    return sum;
}

/* Function 3: Search with post-increment in condition */
int find_value(volatile int *arr, int size, int target) {
    volatile int *p = arr;
    volatile int *end = arr + size;
    int index = -1;
    
    // Post-increment in while condition
    while (p < end && *p++ != target) {
        index++;
    }
    
    // Check if we found it (p was incremented after comparison)
    if (p[-1] == target) {
        return index + 1;
    }
    return -1;
}

/* Function 4: Complex control flow with post-increment */
void process_buffer(volatile char *buf, int len, int mode) {
    volatile char *ptr = buf;
    int i = 0;
    
    // Switch with different post-increment patterns
    switch (mode) {
        case 0:
            // Simple loop with post-increment
            while (i++ < len) {
                char temp = *ptr;
                ptr++;  // Separate increment - might still combine
                buf[i] = temp;
            }
            break;
            
        case 1:
            // Post-increment in if-else branches
            for (i = 0; i < len; i++) {
                if (i % 2 == 0) {
                    *ptr++ = 'A';  // Post-increment in taken path
                } else {
                    char c = *ptr;
                    ptr++;  // Post-increment in not-taken path
                    *buf = c;
                }
            }
            break;
            
        case 2:
            // Comma expression with post-increment
            while (i < len) {
                char val = (*ptr, ptr++, *ptr);  // Access, increment, access
                buf[i++] = val;
            }
            break;
            
        default:
            // Nested loops with post-increment
            for (i = 0; i < len; i++) {
                volatile char *inner = ptr;
                for (int j = 0; j < 4; j++) {
                    *inner++ = (i + j) & 0xFF;  // Inner loop post-increment
                }
                ptr += 4;
            }
            break;
    }
}

/* Function 5: Structure access with pointer post-increment */
int process_structs(struct Data *structs, int count) {
    struct Data *sptr = structs;
    int total = 0;
    
    // Structure field access with pointer increment
    for (int i = 0; i < count; i++) {
        total += sptr->value;  // sptr->field with sptr++ after
        sptr++;  // Post-increment of structure pointer
    }
    
    return total;
}

/* Function 6: String operations with post-increment */
int string_length(const char *str) {
    const char *p = str;
    
    // Classic while(*p++) pattern
    while (*p++ != '\0') {
        // Empty - all work in condition
    }
    
    return (int)(p - str - 1);
}

/* Function 7: Array reversal with post-increment/decrement */
void reverse_array(volatile int *arr, int n) {
    volatile int *start = arr;
    volatile int *end = arr + n - 1;
    
    while (start < end) {
        // Swap with post-increment and post-decrement
        int temp = *start;
        *start++ = *end;  // Post-increment
        *end-- = temp;    // Post-decrement
    }
}

/* Function 8: Mixed qualifiers in same expression */
int mixed_qualifiers(volatile int *vptr, int *regptr, int n) {
    int sum = 0;
    
    // Mix volatile and non-volatile pointers
    for (int i = 0; i < n; i++) {
        sum += *vptr++;  // Volatile pointer post-increment
        
        // Access through regular pointer in same loop
        *regptr = sum;
        regptr++;  // Regular pointer increment
    }
    
    return sum;
}

int main(void) {
    // Test data with different storage classes
    volatile int volatile_array[SIZE];
    int regular_array[SIZE];
    volatile char buffer[BUFFER_SIZE];
    struct Data struct_array[10];
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = (char)(i % 26 + 'A');
    }
    
    for (int i = 0; i < 10; i++) {
        struct_array[i].value = i * 10;
        struct_array[i].id = (char)('A' + i);
        struct_array[i].weight = (float)i * 1.5f;
    }
    
    // Test 1: Copy with post-increment
    volatile char dest[BUFFER_SIZE];
    copy_with_postinc(dest, (const char*)buffer, 50);
    printf("Copy test: dest[0]=%c, dest[49]=%c\n", dest[0], dest[49]);
    
    // Test 2: Summation with post-increment
    int sum1 = sum_with_postinc(volatile_array, 100);
    printf("Sum test 1: %d\n", sum1);
    
    // Test 3: Search with post-increment
    volatile_array[75] = 9999;
    int found = find_value(volatile_array, 100, 9999);
    printf("Search test: found at index %d\n", found);
    
    // Test 4: Complex control flow
    process_buffer(buffer, 32, 0);
    printf("Buffer process test: buffer[0]=%c\n", buffer[0]);
    
    // Test 5: Structure processing
    int struct_sum = process_structs(struct_array, 10);
    printf("Structure sum: %d\n", struct_sum);
    
    // Test 6: String length
    const char *test_str = "Hello, World!";
    int len = string_length(test_str);
    printf("String length: %d (expected: %lu)\n", len, strlen(test_str));
    
    // Test 7: Array reversal
    int test_rev[5] = {1, 2, 3, 4, 5};
    reverse_array((volatile int*)test_rev, 5);
    printf("Reverse test: [%d, %d, %d, %d, %d]\n", 
           test_rev[0], test_rev[1], test_rev[2], test_rev[3], test_rev[4]);
    
    // Test 8: Mixed qualifiers
    int mixed_sum = mixed_qualifiers(volatile_array, regular_array, 50);
    printf("Mixed qualifiers sum: %d\n", mixed_sum);
    
    // Additional tight loops that should generate good RTL patterns
    volatile int *p1 = volatile_array;
    volatile int *p2 = (volatile int*)regular_array;
    
    // Loop with multiple post-increment operations
    for (int i = 0; i < 50; i++) {
        *p2++ = *p1++ + 1;  // Two post-increments in one expression
    }
    
    // Test the result
    printf("Final test: regular_array[0]=%d, regular_array[49]=%d\n",
           regular_array[0], regular_array[49]);
    
    return 0;
}
