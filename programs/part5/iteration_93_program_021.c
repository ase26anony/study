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
    char tag;
    int data[4];
};

/* Function prototypes */
void copy_with_postinc(volatile char *dest, const char *src, size_t n);
int sum_with_postinc(const volatile int *arr, int n);
int find_with_postinc(volatile int *arr, int n, int target);
void process_struct_array(struct TestStruct *arr, int count);
void mixed_qualifier_postinc(volatile int *vptr, int *regptr, int n);
void nested_loop_postinc(volatile int *matrix, int rows, int cols);

/* Test 1: Copy function using post-increment in while loop */
void copy_with_postinc(volatile char *dest, const char *src, size_t n) {
    volatile char *d = dest;
    const char *s = src;
    
    /* Classic copy loop with post-increment - should generate auto-inc RTL */
    while (n-- > 0) {
        *d++ = *s++;
    }
}

/* Test 2: Summation with post-increment in for loop */
int sum_with_postinc(const volatile int *arr, int n) {
    const volatile int *p = arr;
    int sum = 0;
    
    /* For loop with pointer post-increment in update */
    for (int i = 0; i < n; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Test 3: Search with post-increment in while condition */
int find_with_postinc(volatile int *arr, int n, int target) {
    volatile int *p = arr;
    volatile int *end = arr + n;
    
    /* While loop with post-increment in condition */
    while (p < end && *p++ != target) {
        /* Empty body - increment happens in condition */
    }
    
    return (p - 1) - arr;  /* Return index where found or n if not found */
}

/* Test 4: Structure access with post-increment */
void process_struct_array(struct TestStruct *arr, int count) {
    struct TestStruct *sptr = arr;
    
    for (int i = 0; i < count; i++) {
        /* Access structure field with pointer, then increment */
        int val = sptr->value;  /* Memory access with zero offset */
        sptr->id = val * 2;     /* Another access */
        sptr++;                 /* Post-increment equivalent */
        
        /* Alternative: comma expression with post-increment */
        if (i % 2 == 0) {
            int temp = sptr->value, dummy = (sptr++, temp);
            (void)dummy;  /* Prevent unused variable warning */
        }
    }
}

/* Test 5: Mixed volatile and non-volatile pointers */
void mixed_qualifier_postinc(volatile int *vptr, int *regptr, int n) {
    /* Use both pointers with post-increment in same expression */
    for (int i = 0; i < n; i++) {
        /* Memory access with volatile pointer */
        int volatile_val = *vptr++;
        
        /* Memory access with regular pointer */
        int reg_val = *regptr++;
        
        /* Use values to prevent dead code elimination */
        vptr[-1] = reg_val;
        regptr[-1] = volatile_val;
    }
}

/* Test 6: Nested loops with post-increment */
void nested_loop_postinc(volatile int *matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        volatile int *row_ptr = matrix + i * cols;
        
        /* Inner loop with post-increment pointer */
        for (int j = 0; j < cols; j++) {
            /* Multiple memory accesses with same base pointer */
            int val = *row_ptr;
            *row_ptr = val * 2;
            row_ptr++;  /* Post-increment */
        }
    }
}

/* Test 7: Switch statement with post-increment in cases */
int switch_postinc(volatile int *arr, int n, int mode) {
    volatile int *p = arr;
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        switch (mode) {
            case 0:
                /* Post-increment in case 0 */
                result += *p++;
                break;
                
            case 1:
                /* Post-increment with different operation */
                result -= *p++;
                break;
                
            case 2:
                /* Fall-through case with post-increment */
                result |= *p++;
                /* FALLTHROUGH */
                
            case 3:
                /* Another memory access in fall-through path */
                result ^= *p++;
                break;
                
            default:
                /* Default case also uses post-increment */
                result &= *p++;
                break;
        }
    }
    
    return result;
}

/* Test 8: String operations with post-increment */
size_t strlen_postinc(const volatile char *str) {
    const volatile char *p = str;
    
    /* While loop with post-increment in condition */
    while (*p++ != '\0') {
        /* Empty body */
    }
    
    return p - str - 1;
}

/* Test 9: Comma expression sequencing */
int comma_postinc(volatile int *ptr) {
    /* Comma expression: access memory, increment pointer, return value */
    return (ptr[0], ptr++, ptr[-1]);  /* Equivalent to *ptr with side effect */
}

/* Test 10: Complex expression with multiple post-increments */
void complex_postinc(volatile int *a, volatile int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Multiple post-increments in single expression */
        *c++ = *a++ + *b++;
    }
}

int main(void) {
    /* Test data arrays - mix volatile and non-volatile */
    volatile int volatile_array[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    volatile char volatile_buffer[BUFFER_SIZE];
    char regular_buffer[BUFFER_SIZE];
    struct TestStruct struct_array[10];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        volatile_buffer[i] = 'A' + (i % 26);
        regular_buffer[i] = 'a' + (i % 26);
    }
    
    for (int i = 0; i < 10; i++) {
        struct_array[i].id = i;
        struct_array[i].value = i * 10;
        struct_array[i].tag = 'A' + i;
        for (int j = 0; j < 4; j++) {
            struct_array[i].data[j] = i * 100 + j;
        }
    }
    
    /* Test 1: Copy with post-increment */
    copy_with_postinc(volatile_buffer, regular_buffer, BUFFER_SIZE / 2);
    printf("Copy test completed\n");
    
    /* Test 2: Sum with post-increment */
    int sum1 = sum_with_postinc(volatile_array, ARRAY_SIZE);
    int sum2 = sum_with_postinc((const volatile int *)regular_array, ARRAY_SIZE);
    printf("Sum test: volatile sum = %d, regular sum = %d\n", sum1, sum2);
    
    /* Test 3: Find with post-increment */
    int target = 100;
    int index1 = find_with_postinc(volatile_array, ARRAY_SIZE, target);
    int index2 = find_with_postinc(volatile_array, ARRAY_SIZE, 9999); /* Not found */
    printf("Find test: found %d at index %d, not found case: %d\n", 
           target, index1, index2);
    
    /* Test 4: Structure processing */
    process_struct_array(struct_array, 10);
    printf("Structure processing completed\n");
    
    /* Test 5: Mixed qualifiers */
    mixed_qualifier_postinc(volatile_array, regular_array, ARRAY_SIZE / 2);
    printf("Mixed qualifier test completed\n");
    
    /* Test 6: Nested loops */
    volatile int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    nested_loop_postinc((volatile int *)matrix, 5, 5);
    printf("Nested loop test completed\n");
    
    /* Test 7: Switch with post-increment */
    int switch_result = switch_postinc(volatile_array, 10, 2);
    printf("Switch test result: %d\n", switch_result);
    
    /* Test 8: String length */
    const volatile char *test_str = "Hello, World!";
    size_t len = strlen_postinc(test_str);
    printf("String length test: '%s' length = %zu\n", "Hello, World!", len);
    
    /* Test 9: Comma expression */
    volatile int comma_test[5] = {1, 2, 3, 4, 5};
    int comma_result = comma_postinc(comma_test);
    printf("Comma expression test result: %d\n", comma_result);
    
    /* Test 10: Complex expression */
    volatile int complex_a[5] = {1, 2, 3, 4, 5};
    volatile int complex_b[5] = {10, 20, 30, 40, 50};
    int complex_c[5];
    complex_postinc(complex_a, complex_b, complex_c, 5);
    printf("Complex expression test: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", complex_c[i]);
    }
    printf("\n");
    
    /* Additional test: Array of pointers with post-increment */
    volatile int *ptr_array[5];
    int values[5] = {100, 200, 300, 400, 500};
    for (int i = 0; i < 5; i++) {
        ptr_array[i] = &values[i];
    }
    
    volatile int **pptr = ptr_array;
    int ptr_sum = 0;
    for (int i = 0; i < 5; i++) {
        ptr_sum += **pptr++;  /* Dereference pointer-to-pointer with post-increment */
    }
    printf("Pointer array sum: %d\n", ptr_sum);
    
    return 0;
}
