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
void copy_with_postinc(char *dest, const char *src, size_t n) {
    // Classic strcpy-like loop with post-increment
    while (n-- && (*dest++ = *src++) != '\0') {
        // Empty body - all work in condition
    }
    *dest = '\0';
}

/* Function 2: Summation with mixed volatile/non-volatile pointers */
int sum_with_postinc(volatile int *varr, int *arr, int size) {
    int sum = 0;
    volatile int *vp = varr;
    int *p = arr;
    
    // Post-increment in loop update
    for (int i = 0; i < size; i++) {
        sum += *vp++ + *p++;
    }
    
    // Additional post-increment in if condition
    if (size > 0 && (*varr++ > 0)) {
        sum += 100;
    }
    
    return sum;
}

/* Function 3: Search with post-increment in while condition */
int* find_value(int *array, int size, int target) {
    int *ptr = array;
    
    // Post-increment in while condition
    while (ptr < array + size && *ptr++ != target) {
        // Continue searching
    }
    
    return (ptr > array && *(ptr-1) == target) ? ptr-1 : NULL;
}

/* Function 4: Structure array processing with post-increment */
float process_structs(struct Data *sptr, int count) {
    float total_weight = 0.0f;
    struct Data *current = sptr;
    
    // Post-increment accessing structure field
    for (int i = 0; i < count; i++) {
        total_weight += current->weight;
        current++;  // Post-increment equivalent in separate statement
        
        // Comma expression with post-increment
        int val = (current->value, current++, val);
    }
    
    // Nested loop with inner post-increment
    for (int i = 0; i < count; i++) {
        for (struct Data *inner = sptr; inner < sptr + count; ) {
            if (inner->id == 'A') {
                total_weight += inner->weight;
            }
            inner++;  // Post-increment in inner loop
        }
    }
    
    return total_weight;
}

/* Function 5: Byte buffer processing with switch and fall-through */
int process_buffer(volatile unsigned char *buf, int len) {
    volatile unsigned char *p = buf;
    int result = 0;
    int i = 0;
    
    // Switch with post-increment in cases
    while (i < len) {
        switch (*p++) {  // Post-increment in switch expression
            case 0x01:
                result += 1;
                // Fall through
            case 0x02:
                result += 2;
                p++;  // Additional increment
                break;
            case 0x03:
                // Comma expression: access then increment
                result += (temp = *p, p++, temp);
                break;
            default:
                // Simple post-increment
                result += *p++;
        }
        i++;
    }
    
    return result;
}

/* Function 6: Array reversal with post-increment/decrement */
void reverse_array(int *arr, int size) {
    int *start = arr;
    int *end = arr + size - 1;
    
    // Post-increment and pre-decrement in loop
    while (start < end) {
        // Swap with post-increment/decrement
        int temp = *start;
        *start++ = *end;  // Post-increment after store
        *end-- = temp;    // Post-decrement after store
    }
}

/* Function 7: Mixed qualifiers in complex expression */
int mixed_qualifier_test(volatile short *vptr, short *ptr, int n) {
    int sum = 0;
    
    // Both volatile and non-volatile with post-increment
    for (int i = 0; i < n; i++) {
        // Access with zero offset (ptr[0] equivalent)
        sum += vptr[0] + ptr[0];
        vptr++;  // Post-increment
        ptr++;   // Post-increment
    }
    
    // Conditional with post-increment
    if (n > 10 && (*vptr++ > 1000)) {
        sum += 500;
    }
    
    return sum;
}

int main() {
    // Test data
    char source[BUFFER_SIZE] = "Test string for copy operation";
    char destination[BUFFER_SIZE];
    
    volatile int volatile_array[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    
    struct Data struct_array[20];
    volatile unsigned char byte_buffer[100];
    
    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    for (int i = 0; i < 20; i++) {
        struct_array[i].value = i;
        struct_array[i].id = (i % 2) ? 'A' : 'B';
        struct_array[i].weight = i * 1.5f;
    }
    
    for (int i = 0; i < 100; i++) {
        byte_buffer[i] = i % 16;
    }
    
    printf("Starting auto-increment/decrement pattern tests...\n");
    
    // Test 1: String copy with post-increment
    copy_with_postinc(destination, source, BUFFER_SIZE);
    printf("1. Copy test: '%s'\n", destination);
    
    // Test 2: Summation with mixed pointers
    int sum1 = sum_with_postinc(volatile_array, regular_array, ARRAY_SIZE);
    printf("2. Sum test: %d\n", sum1);
    
    // Test 3: Search with post-increment
    int *found = find_value(regular_array, ARRAY_SIZE, 150);
    printf("3. Search test: %s\n", found ? "Found" : "Not found");
    
    // Test 4: Structure processing
    float weight_sum = process_structs(struct_array, 20);
    printf("4. Structure test: %.2f\n", weight_sum);
    
    // Test 5: Buffer processing with switch
    int buffer_result = process_buffer(byte_buffer, 100);
    printf("5. Buffer test: %d\n", buffer_result);
    
    // Test 6: Array reversal
    int test_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    reverse_array(test_array, 10);
    printf("6. Reverse test: ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", test_array[i]);
    }
    printf("\n");
    
    // Test 7: Mixed qualifier test
    volatile short vshorts[50];
    short shorts[50];
    for (int i = 0; i < 50; i++) {
        vshorts[i] = i * 10;
        shorts[i] = i * 20;
    }
    int mixed_result = mixed_qualifier_test(vshorts, shorts, 50);
    printf("7. Mixed qualifier test: %d\n", mixed_result);
    
    // Additional tight loop patterns
    printf("\nAdditional tight loop patterns:\n");
    
    // Tight copy loop
    char src[50] = "Tight loop test pattern";
    char dst[50];
    char *s = src, *d = dst;
    while ((*d++ = *s++) != '\0') ;  // Classic K&R style
    
    printf("Tight copy: '%s'\n", dst);
    
    // Summation with pointer arithmetic
    int nums[100];
    for (int i = 0; i < 100; i++) nums[i] = i + 1;
    
    int total = 0;
    for (int *p = nums; p < &nums[100]; total += *p++) ;
    printf("Pointer sum: %d\n", total);
    
    // Volatile pointer in loop
    volatile int *vp = volatile_array;
    int volatile_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_sum += *vp++;
    }
    printf("Volatile pointer sum: %d\n", volatile_sum);
    
    printf("\nAll tests completed.\n");
    
    return 0;
}
