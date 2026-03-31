/* auto-inc-dec-test.c
 * 
 * This program is designed to generate RTL patterns that trigger the
 * auto-increment/decrement optimization in GCC's RTL passes, specifically
 * targeting the uncovered lines in auto-inc-dec.cc (lines 1352-1358).
 * 
 * The code uses post-increment/decrement pointer arithmetic in various
 * contexts to create opportunities for auto-inc-dec addressing modes.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define BUFFER_SIZE 128

/* Structure for testing pointer post-increment with field access */
struct TestStruct {
    int id;
    volatile int value;
    char name[16];
};

/* Function 1: Copy with post-increment in while loop condition
 * This creates a tight loop with *d++ = *s++ pattern
 */
void copy_with_postinc(char *dest, const char *src) {
    while ((*dest++ = *src++) != '\0') {
        /* Empty body - all work done in condition */
    }
}

/* Function 2: Summation with post-increment in for loop update
 * Uses pointer arithmetic with zero offset (p[0] equivalent to *p)
 */
int sum_array_postinc(const int *arr, int n) {
    int sum = 0;
    const int *end = arr + n;
    for (const int *p = arr; p < end; sum += *p++) {
        /* Work done in loop update */
    }
    return sum;
}

/* Function 3: Search with post-increment in while loop condition
 * Mixes volatile and non-volatile access patterns
 */
int find_value_postinc(volatile int *arr, int n, int target) {
    volatile int *p = arr;
    volatile int *end = arr + n;
    
    while (p < end && *p++ != target) {
        /* Search done in condition */
    }
    
    return (p - 1) - arr;  /* Return index where found or n */
}

/* Function 4: Structure array processing with post-increment
 * Uses sptr->field with pointer increment
 */
int sum_struct_values(struct TestStruct *arr, int n) {
    int total = 0;
    struct TestStruct *end = arr + n;
    
    for (struct TestStruct *sptr = arr; sptr < end; total += sptr++->value) {
        /* Work done in loop update with structure field access */
    }
    return total;
}

/* Function 5: Mixed volatile/non-volatile copy with comma expression
 * Uses (temp = *ptr, ptr++, temp) pattern
 */
void mixed_copy_postinc(volatile int *dest, const int *src, int n) {
    for (int i = 0; i < n; i++) {
        /* Comma expression with post-increment */
        int temp = (*src, src++, *src);  /* Actually increments src */
        *dest++ = temp;  /* Post-increment on volatile pointer */
    }
}

/* Function 6: Nested loops with inner post-increment
 * Creates complex control flow graph
 */
void matrix_copy_postinc(int dest[][4], const int src[][4], int rows) {
    for (int i = 0; i < rows; i++) {
        const int *s = src[i];
        int *d = dest[i];
        const int *end = s + 4;
        
        /* Inner tight loop with post-increment */
        while (s < end) {
            *d++ = *s++;
        }
    }
}

/* Function 7: Switch statement with post-increment in cases
 * Tests auto-inc-dec in different control flow paths
 */
int process_buffer_postinc(volatile char *buf, int size, int mode) {
    volatile char *p = buf;
    volatile char *end = buf + size;
    int count = 0;
    
    while (p < end) {
        switch (mode) {
            case 0:  /* Count spaces */
                if (*p++ == ' ') count++;
                break;
            case 1:  /* Count digits */
                if (*p++ >= '0' && *(p-1) <= '9') count++;
                break;
            case 2:  /* Skip and count */
                p++;  /* Just increment */
                count++;
                break;
            default: /* Copy with post-increment in comma expr */
                count += (*p, p++, 1);
        }
    }
    return count;
}

/* Function 8: String concatenation with double post-increment
 * Tests complex addressing patterns
 */
void concat_strings_postinc(char *dest, const char *src1, const char *src2) {
    /* Copy first string */
    while ((*dest++ = *src1++) != '\0');
    
    dest--;  /* Back up over null terminator */
    
    /* Copy second string with post-increment in condition */
    while ((*dest++ = *src2++) != '\0');
}

/* Function 9: Array reversal with post-increment/decrement
 * Uses both ++ and -- operations
 */
void reverse_array_postinc(int *arr, int n) {
    int *start = arr;
    int *end = arr + n - 1;
    
    while (start < end) {
        /* Swap with post-increment/decrement */
        int temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}

/* Function 10: Mixed qualifiers in complex expression
 * Tests volatile/non-volatile interaction
 */
int complex_expr_postinc(volatile int *vptr, int *ptr, int n) {
    int result = 0;
    volatile int *vend = vptr + n;
    
    /* Mixed pointers in loop */
    for (; vptr < vend; result += *vptr++ + *ptr++) {
        /* Work in loop update with dual post-increment */
    }
    return result;
}

int main(void) {
    /* Test data arrays - mix volatile and non-volatile */
    char source[BUFFER_SIZE] = "Test string for auto-inc-dec optimization";
    char dest[BUFFER_SIZE];
    volatile int volatile_array[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    struct TestStruct struct_array[10];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    for (int i = 0; i < 10; i++) {
        struct_array[i].id = i;
        struct_array[i].value = i * 10;
        snprintf(struct_array[i].name, 16, "Struct%d", i);
    }
    
    printf("=== Auto-Inc-Dec Test Program ===\n\n");
    
    /* Test 1: String copy with post-increment */
    copy_with_postinc(dest, source);
    printf("1. String copy result: %s\n", dest);
    
    /* Test 2: Array summation */
    int sum = sum_array_postinc(regular_array, ARRAY_SIZE);
    printf("2. Array sum: %d\n", sum);
    
    /* Test 3: Volatile array search */
    int index = find_value_postinc(volatile_array, ARRAY_SIZE, 100);
    printf("3. Found value 100 at index: %d\n", index);
    
    /* Test 4: Structure processing */
    int struct_sum = sum_struct_values(struct_array, 10);
    printf("4. Structure values sum: %d\n", struct_sum);
    
    /* Test 5: Mixed copy */
    volatile int mixed_dest[ARRAY_SIZE];
    mixed_copy_postinc(mixed_dest, regular_array, 50);
    printf("5. Mixed copy completed\n");
    
    /* Test 6: Matrix copy */
    int matrix_src[3][4] = {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}};
    int matrix_dest[3][4];
    matrix_copy_postinc(matrix_dest, matrix_src, 3);
    printf("6. Matrix copy completed\n");
    
    /* Test 7: Buffer processing with switch */
    volatile char buffer[] = "Test 123 buffer 456";
    int count = process_buffer_postinc(buffer, sizeof(buffer)-1, 1);
    printf("7. Digits in buffer: %d\n", count);
    
    /* Test 8: String concatenation */
    char concat_result[50];
    concat_strings_postinc(concat_result, "Hello ", "World!");
    printf("8. Concatenated: %s\n", concat_result);
    
    /* Test 9: Array reversal */
    int rev_array[] = {1, 2, 3, 4, 5};
    reverse_array_postinc(rev_array, 5);
    printf("9. Reversed array: %d %d %d %d %d\n", 
           rev_array[0], rev_array[1], rev_array[2], rev_array[3], rev_array[4]);
    
    /* Test 10: Complex expression */
    int complex_result = complex_expr_postinc(volatile_array, regular_array, 10);
    printf("10. Complex expression result: %d\n", complex_result);
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
