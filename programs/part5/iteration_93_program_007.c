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

/* Function 1: Copy with post-increment in loop condition */
void copy_with_postinc(char *dest, const char *src) {
    // Classic K&R style copy - should generate post-increment addressing
    while ((*dest++ = *src++) != '\0') {
        /* empty body */
    }
}

/* Function 2: Summation with mixed volatile/non-volatile pointers */
int sum_with_postinc(volatile int *varr, int *arr, int n) {
    int sum = 0;
    volatile int *vp = varr;
    int *p = arr;
    
    // Post-increment in loop update statement
    for (int i = 0; i < n; i++) {
        sum += *vp++ + *p++;
    }
    return sum;
}

/* Function 3: Search with post-increment in condition */
int find_value(volatile int *arr, int target, int n) {
    volatile int *p = arr;
    int i = 0;
    
    // Post-increment in while condition
    while (i < n && *p++ != target) {
        i++;
    }
    return (i < n) ? i : -1;
}

/* Function 4: Structure array processing with post-increment */
int process_structs(struct Data *sptr, int count) {
    int total = 0;
    struct Data *current = sptr;
    
    // Post-increment accessing structure fields
    for (int i = 0; i < count; i++) {
        total += current->value * current->count;
        current++;  // Post-increment equivalent: current = &current[1]
    }
    return total;
}

/* Function 5: Complex control flow with post-increment */
int complex_control_flow(volatile char *buffer, int size) {
    volatile char *ptr = buffer;
    int result = 0;
    
    for (int i = 0; i < size; i++) {
        // Post-increment in switch cases
        switch (*ptr++) {
            case 'A':
            case 'a':
                result += 1;
                // Fall through with post-increment in expression
                {
                    char temp = *ptr;
                    ptr++;
                    result += temp;
                }
                break;
                
            case 'B':
            case 'b':
                // Comma expression with post-increment
                result += (temp = *ptr, ptr++, temp * 2);
                break;
                
            case 'C':
            case 'c':
                // Nested if with post-increment
                if (ptr < buffer + size - 1) {
                    result += *ptr++;
                    if (*ptr == 'X') {
                        ptr++;  // Skip X
                    }
                }
                break;
                
            default:
                // Simple post-increment
                result += *ptr++;
                break;
        }
    }
    return result;
}

/* Function 6: String concatenation with post-increment */
void concat_strings(char *dest, const char *src1, const char *src2) {
    char *d = dest;
    const char *s = src1;
    
    // First string with post-increment
    while ((*d++ = *s++) != '\0');
    
    // Back up for null terminator
    d--;
    
    // Second string with post-increment
    s = src2;
    while ((*d++ = *s++) != '\0');
}

/* Function 7: Byte buffer processing with pointer arithmetic */
int process_buffer(volatile uint8_t *buf, int len) {
    volatile uint8_t *p = buf;
    volatile uint8_t *end = buf + len;
    int checksum = 0;
    
    // Tight loop with pointer comparison and post-increment
    while (p < end) {
        checksum += *p++;
    }
    
    return checksum;
}

/* Function 8: Array initialization with post-increment */
void init_array(int *arr, int value, int n) {
    int *p = arr;
    int *end = arr + n;
    
    // Post-increment in loop body
    while (p < end) {
        *p++ = value++;
    }
}

int main() {
    // Test data
    char source[] = "Test string for auto-increment addressing modes";
    char dest[100];
    
    volatile int varray[ARRAY_SIZE];
    int array[ARRAY_SIZE];
    
    volatile char buffer[BUFFER_SIZE];
    struct Data structs[10];
    
    // Initialize test data
    for (int i = 0; i < ARRAY_SIZE; i++) {
        varray[i] = i * 2;
        array[i] = i * 3;
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 10; i++) {
        structs[i].value = i;
        structs[i].count = i * 2;
        structs[i].id = 'A' + i;
    }
    
    // Test 1: String copy with post-increment
    copy_with_postinc(dest, source);
    printf("Copy test: %s\n", dest);
    
    // Test 2: Summation with mixed pointers
    int sum = sum_with_postinc(varray, array, ARRAY_SIZE);
    printf("Sum test: %d\n", sum);
    
    // Test 3: Search with post-increment
    int found = find_value(varray, 50, ARRAY_SIZE);
    printf("Search test: Found 50 at index %d\n", found);
    
    // Test 4: Structure processing
    int struct_total = process_structs(structs, 10);
    printf("Structure test: Total = %d\n", struct_total);
    
    // Test 5: Complex control flow
    int complex_result = complex_control_flow(buffer, BUFFER_SIZE);
    printf("Complex control flow test: Result = %d\n", complex_result);
    
    // Test 6: String concatenation
    char concat_result[100];
    concat_strings(concat_result, "Hello, ", "World!");
    printf("Concatenation test: %s\n", concat_result);
    
    // Test 7: Buffer processing
    volatile uint8_t byte_buf[20];
    for (int i = 0; i < 20; i++) {
        byte_buf[i] = i;
    }
    int checksum = process_buffer(byte_buf, 20);
    printf("Buffer checksum: %d\n", checksum);
    
    // Test 8: Array initialization
    int init_arr[15];
    init_array(init_arr, 10, 15);
    printf("Array initialization: ");
    for (int i = 0; i < 15; i++) {
        printf("%d ", init_arr[i]);
    }
    printf("\n");
    
    // Additional tight loop tests that should generate auto-inc RTL
    
    // Test 9: Memory set with post-increment pattern
    volatile char test_buf[64];
    volatile char *tp = test_buf;
    int j = 0;
    while (j < 64) {
        *tp++ = j++ & 0xFF;
    }
    
    // Test 10: Pointer difference with post-increment
    volatile int *p1 = varray;
    volatile int *p2 = varray + ARRAY_SIZE/2;
    int diff = 0;
    while (p1 < p2) {
        diff += *p1++ - *p2++;
    }
    printf("Pointer difference: %d\n", diff);
    
    return 0;
}
