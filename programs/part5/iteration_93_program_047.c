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

#define ARRAY_SIZE 100
#define BUFFER_SIZE 256

/* Structure for testing pointer post-increment with field access */
struct TestStruct {
    int id;
    volatile int value;
    char name[16];
};

/* Function 1: Copy with post-increment in while loop condition */
void copy_with_postinc(char *dest, const char *src) {
    while ((*dest++ = *src++) != '\0') {
        /* Empty body - all work done in condition */
    }
}

/* Function 2: Summation with post-increment in for loop */
int sum_array_postinc(const int *arr, int n) {
    int sum = 0;
    const int *end = arr + n;
    for (const int *p = arr; p < end; sum += *p++) {
        /* Loop uses post-increment in update expression */
    }
    return sum;
}

/* Function 3: Mixed volatile and non-volatile pointers */
int volatile_sum(volatile int *varr, int *arr, int n) {
    int total = 0;
    volatile int *vend = varr + n;
    
    /* Use post-increment on volatile pointer */
    while (varr < vend) {
        total += *varr++;
    }
    
    /* Use post-increment on non-volatile pointer */
    int *end = arr + n;
    while (arr < end) {
        total += *arr++;
    }
    
    return total;
}

/* Function 4: Search with post-increment in while condition */
int find_value_postinc(const int *arr, int n, int target) {
    const int *end = arr + n;
    while (arr < end && *arr++ != target) {
        /* Search using post-increment in condition */
    }
    return (arr[-1] == target) ? (arr - 1 - (arr - n)) : -1;
}

/* Function 5: Structure pointer post-increment in loop */
void process_structs(struct TestStruct *sarr, int n) {
    struct TestStruct *end = sarr + n;
    
    /* Post-increment on structure pointer accessing fields */
    for (struct TestStruct *sptr = sarr; sptr < end; sptr++) {
        /* Access field then increment pointer (simulated post-increment) */
        int temp = sptr->value;
        sptr->id = temp * 2;
    }
}

/* Function 6: Complex control flow with post-increment */
int conditional_postinc(int *arr, int n, int threshold) {
    int count = 0;
    int *end = arr + n;
    
    for (int *p = arr; p < end; ) {
        if (*p > threshold) {
            /* Post-increment in taken path */
            count += *p++;
            
            /* Nested if with another post-increment */
            if (p < end && *p < 0) {
                count -= *p++;
            }
        } else {
            /* Post-increment in not-taken path */
            p++;
        }
    }
    
    return count;
}

/* Function 7: Switch statement with post-increment */
int switch_postinc(int *arr, int n) {
    int result = 0;
    int *end = arr + n;
    
    for (int *p = arr; p < end; ) {
        switch (*p % 4) {
            case 0:
                /* Fall through with post-increment */
                result += *p++;
                /* FALLTHROUGH */
            case 1:
                result -= *p++;
                break;
            case 2:
                /* Comma expression with post-increment */
                result = (result = *p++, p++, result * 2);
                break;
            case 3:
                p++;  /* Skip this element */
                break;
        }
    }
    
    return result;
}

/* Function 8: Nested loops with post-increment */
void matrix_process(int matrix[][10], int rows) {
    for (int i = 0; i < rows; i++) {
        /* Inner loop uses pointer with post-increment */
        int *row_end = matrix[i] + 10;
        for (int *p = matrix[i]; p < row_end; ) {
            /* Multiple accesses with post-increment */
            int val1 = *p++;
            if (p < row_end) {
                int val2 = *p++;
                *p = val1 + val2;  /* Store result */
                p++;  /* Move past result */
            }
        }
    }
}

/* Function 9: Byte buffer copy with post-increment */
void buffer_copy(volatile uint8_t *dest, const uint8_t *src, size_t len) {
    const uint8_t *end = src + len;
    
    /* Tight loop likely to generate auto-inc RTL */
    while (src < end) {
        *dest++ = *src++;
    }
}

/* Function 10: String operations with post-increment */
int string_ops(const char *str) {
    int len = 0;
    const char *p = str;
    
    /* strlen-like loop */
    while (*p++ != '\0') {
        len++;
    }
    
    /* Reset and process */
    p = str;
    int sum = 0;
    while (*p != '\0') {
        /* Access with zero offset then increment */
        sum += *p;
        p++;
    }
    
    return len + sum;
}

int main(void) {
    /* Test data arrays */
    char source[BUFFER_SIZE] = "Test string for auto-inc-dec optimization";
    char destination[BUFFER_SIZE];
    
    int data[ARRAY_SIZE];
    volatile int vdata[ARRAY_SIZE];
    
    struct TestStruct structs[10];
    
    int matrix[5][10];
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i;
        vdata[i] = i * 2;
    }
    
    for (int i = 0; i < 10; i++) {
        structs[i].id = i;
        structs[i].value = i * 10;
        snprintf(structs[i].name, sizeof(structs[i].name), "Item%d", i);
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Execute test functions */
    
    /* 1. Copy with post-increment */
    copy_with_postinc(destination, source);
    printf("Copy result: %s\n", destination);
    
    /* 2. Summation with post-increment */
    int sum = sum_array_postinc(data, ARRAY_SIZE);
    printf("Array sum: %d\n", sum);
    
    /* 3. Mixed volatile/non-volatile */
    int vsum = volatile_sum(vdata, data, ARRAY_SIZE);
    printf("Volatile sum: %d\n", vsum);
    
    /* 4. Search with post-increment */
    int found = find_value_postinc(data, ARRAY_SIZE, 42);
    printf("Found 42 at index: %d\n", found);
    
    /* 5. Structure processing */
    process_structs(structs, 10);
    printf("First struct id after processing: %d\n", structs[0].id);
    
    /* 6. Conditional post-increment */
    int cond_result = conditional_postinc(data, ARRAY_SIZE, 50);
    printf("Conditional result: %d\n", cond_result);
    
    /* 7. Switch with post-increment */
    int switch_result = switch_postinc(data, 20);
    printf("Switch result: %d\n", switch_result);
    
    /* 8. Nested loops */
    matrix_process(matrix, 5);
    printf("Matrix[0][0] after processing: %d\n", matrix[0][0]);
    
    /* 9. Buffer copy */
    uint8_t src_buf[50], dest_buf[50];
    for (int i = 0; i < 50; i++) src_buf[i] = i;
    buffer_copy((volatile uint8_t *)dest_buf, src_buf, 50);
    printf("Buffer copy check: %d\n", dest_buf[25]);
    
    /* 10. String operations */
    int str_result = string_ops(source);
    printf("String operations result: %d\n", str_result);
    
    /* Additional tight loops for RTL generation */
    
    /* Loop with pointer increment in comma expression */
    {
        int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        int *ptr = arr;
        int total = 0;
        for (int i = 0; i < 10; i++) {
            total = (total += *ptr, ptr++, total);
        }
        printf("Comma expression total: %d\n", total);
    }
    
    /* Array access with index zero and pointer increment */
    {
        volatile int *vptr = vdata;
        int zero_index_sum = 0;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            zero_index_sum += vptr[0];  /* Zero offset access */
            vptr++;  /* Increment after access */
        }
        printf("Zero index sum: %d\n", zero_index_sum);
    }
    
    return 0;
}
