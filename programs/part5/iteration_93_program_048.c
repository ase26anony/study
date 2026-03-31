/* auto-inc-dec-test.c
 * 
 * This program is designed to generate RTL patterns that trigger the
 * auto-increment/decrement optimization in GCC's RTL passes, specifically
 * targeting the uncovered lines in auto-inc-dec.cc (lines 1352-1358).
 * 
 * The code uses various post-increment/decrement patterns in loops,
 * conditionals, and with volatile qualifiers to create opportunities
 * for the find_auto_inc pass to combine memory accesses with address
 * register updates.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define BUFFER_SIZE 128

/* Structure to enable pointer arithmetic with post-increment */
struct DataBlock {
    int values[8];
    volatile int status;
    int counter;
};

/* Function 1: Copy with post-increment in loop condition */
void copy_with_postinc(char *dest, const char *src, size_t n) {
    /* Classic strcpy-like loop - should generate post-increment RTL */
    while (n-- > 0) {
        *dest++ = *src++;
    }
}

/* Function 2: Summation with post-increment in update statement */
int sum_array_postinc(const int *arr, int size) {
    int sum = 0;
    const int *end = arr + size;
    
    /* Post-increment in loop update */
    for (const int *p = arr; p < end; sum += *p++) {
        /* Empty body - all work in loop header */
    }
    return sum;
}

/* Function 3: Search with post-increment in loop condition */
int find_value_postinc(const int *arr, int size, int target) {
    const int *p = arr;
    const int *end = arr + size;
    
    /* Post-increment in while condition */
    while (p < end && *p++ != target) {
        /* Continue searching */
    }
    
    return (p - 1) - arr;  /* Return index where found (or size if not found) */
}

/* Function 4: Mixed volatile and non-volatile pointers */
int process_volatile_buffer(volatile int *vptr, int *regptr, int size) {
    int result = 0;
    
    /* Use volatile pointer with post-increment */
    for (int i = 0; i < size; i++) {
        result += *vptr++;  /* Post-increment on volatile pointer */
    }
    
    /* Also use regular pointer with post-increment */
    for (int i = 0; i < size; i++) {
        *regptr++ = result;  /* Post-increment on regular pointer */
    }
    
    return result;
}

/* Function 5: Structure access with pointer post-increment */
int process_struct_array(struct DataBlock *blocks, int count) {
    int total = 0;
    struct DataBlock *end = blocks + count;
    
    /* Access structure field with pointer post-increment */
    for (struct DataBlock *sptr = blocks; sptr < end; sptr++) {
        /* Comma expression: access then increment */
        volatile int status_val;
        status_val = sptr->status, sptr->counter++;
        
        /* Another post-increment pattern in loop */
        for (int i = 0; i < 8; i++) {
            total += sptr->values[i];
        }
    }
    
    return total;
}

/* Function 6: Nested loops with inner post-increment */
void matrix_copy(int dest[][8], int src[][8], int rows) {
    for (int i = 0; i < rows; i++) {
        int *dptr = dest[i];
        int *sptr = src[i];
        int *end = dptr + 8;
        
        /* Inner loop with post-increment */
        while (dptr < end) {
            *dptr++ = *sptr++;  /* Classic copy with post-increment */
        }
    }
}

/* Function 7: Switch statement with post-increment in cases */
int switch_with_postinc(int *arr, int size, int mode) {
    int result = 0;
    int *ptr = arr;
    int *end = arr + size;
    
    switch (mode) {
        case 0:
            /* Post-increment in while condition */
            while (ptr < end && *ptr++ >= 0) {
                result++;
            }
            break;
            
        case 1:
            /* Post-increment in for loop update */
            for (; ptr < end; ptr++) {
                result += *ptr;  /* Simple dereference - offset 0 */
            }
            break;
            
        case 2:
            /* Comma expression with post-increment */
            while (ptr < end) {
                int val;
                val = *ptr, ptr++, result += val;  /* Access then increment */
            }
            break;
            
        default:
            /* Do nothing */
            break;
    }
    
    return result;
}

/* Function 8: String operations with post-increment */
int string_operations(char *str) {
    int len = 0;
    char *p = str;
    
    /* strlen-like loop */
    while (*p++ != '\0') {
        len++;
    }
    
    /* Convert to uppercase with post-increment */
    p = str;
    while (*p) {
        if (*p >= 'a' && *p <= 'z') {
            *p = *p - ('a' - 'A');
        }
        p++;  /* Post-increment in body */
    }
    
    return len;
}

int main(void) {
    /* Test data arrays - mix volatile and non-volatile */
    volatile int volatile_buffer[ARRAY_SIZE];
    int regular_buffer[ARRAY_SIZE];
    char string_buffer[BUFFER_SIZE];
    struct DataBlock blocks[4];
    int matrix_a[4][8], matrix_b[4][8];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_buffer[i] = i % 100;
        regular_buffer[i] = i * 2;
    }
    
    /* Initialize string */
    const char *test_string = "Hello, Auto-Increment/Decrement Test!";
    strcpy(string_buffer, test_string);
    
    /* Initialize structure array */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            blocks[i].values[j] = i * 10 + j;
        }
        blocks[i].status = i * 100;
        blocks[i].counter = 0;
    }
    
    /* Initialize matrices */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            matrix_a[i][j] = i * 8 + j;
        }
    }
    
    /* Execute all test functions to generate various RTL patterns */
    
    /* 1. Copy with post-increment */
    char copy_dest[BUFFER_SIZE];
    copy_with_postinc(copy_dest, string_buffer, strlen(string_buffer) + 1);
    printf("Copy test: %s\n", copy_dest);
    
    /* 2. Summation with post-increment */
    int sum = sum_array_postinc(regular_buffer, ARRAY_SIZE);
    printf("Sum of array: %d\n", sum);
    
    /* 3. Search with post-increment */
    int search_idx = find_value_postinc(regular_buffer, ARRAY_SIZE, 100);
    printf("Found value 100 at index: %d\n", search_idx);
    
    /* 4. Mixed volatile/non-volatile */
    int volatile_result = process_volatile_buffer(
        volatile_buffer, regular_buffer, ARRAY_SIZE / 2);
    printf("Volatile processing result: %d\n", volatile_result);
    
    /* 5. Structure access with post-increment */
    int struct_total = process_struct_array(blocks, 4);
    printf("Structure array total: %d\n", struct_total);
    
    /* 6. Nested loop matrix copy */
    matrix_copy(matrix_b, matrix_a, 4);
    printf("Matrix copy completed\n");
    
    /* 7. Switch with post-increment */
    int switch_result = switch_with_postinc(regular_buffer, 50, 1);
    printf("Switch processing result: %d\n", switch_result);
    
    /* 8. String operations */
    int str_len = string_operations(string_buffer);
    printf("String length: %d, Uppercase: %s\n", str_len, string_buffer);
    
    /* Additional complex pattern: if-else with post-increment */
    int *ptr1 = regular_buffer;
    int *ptr2 = regular_buffer + ARRAY_SIZE / 2;
    int conditional_sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE / 4; i++) {
        if (i % 2 == 0) {
            /* Taken path: post-increment */
            conditional_sum += *ptr1++;
        } else {
            /* Not-taken path: also post-increment */
            conditional_sum += *ptr2++;
        }
    }
    printf("Conditional sum: %d\n", conditional_sum);
    
    /* Final verification */
    int final_check = 0;
    int *final_ptr = regular_buffer;
    int *final_end = regular_buffer + 10;
    
    /* Simple dereference with offset 0 in loop */
    while (final_ptr < final_end) {
        final_check += *final_ptr;  /* mem_insn.reg1_val = 0 pattern */
        final_ptr++;
    }
    
    printf("Final check sum: %d\n", final_check);
    printf("All tests completed successfully.\n");
    
    return 0;
}
