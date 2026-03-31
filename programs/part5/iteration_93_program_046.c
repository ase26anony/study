#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SIZE 256
#define PATTERN_SIZE 16

/* Structure to test pointer post-increment with field access */
struct Data {
    int value;
    int count;
    char id;
};

/* Function using post-increment in array indexing */
int sum_array_postinc(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    /* Post-increment in loop condition - should generate auto-inc RTL */
    while (p < end) {
        sum += *p++;
    }
    
    return sum;
}

/* Function using post-increment with volatile */
int sum_volatile_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *p = arr;
    volatile int *end = arr + n;
    
    /* Mix of volatile and control flow */
    for (; p < end; ) {
        if (*p > 0) {
            sum += *p++;  /* Post-increment in taken path */
        } else {
            p++;  /* Just increment without access in not-taken path */
        }
    }
    
    return sum;
}

/* String copy with post-increment (classic example) */
void copy_string_postinc(char *dest, const char *src) {
    /* Tight loop likely to generate auto-inc addressing */
    while ((*dest++ = *src++) != '\0');
}

/* Function with post-increment in structure access */
int process_structs(struct Data *sptr, int n) {
    int total = 0;
    struct Data *end = sptr + n;
    
    /* Post-increment accessing structure field */
    while (sptr < end) {
        total += sptr->value;
        sptr++;  /* Post-increment after field access */
    }
    
    return total;
}

/* Function with post-increment in switch statement */
int process_with_switch(int *arr, int n, int mode) {
    int result = 0;
    int *p = arr;
    int *end = arr + n;
    
    while (p < end) {
        switch (mode) {
            case 0:
                /* Post-increment in case 0 */
                result += *p++;
                break;
            case 1:
                /* Different post-increment pattern */
                result -= *p++;
                /* Fall through */
            case 2:
                /* Another memory access with offset 0 */
                result *= *p;
                p++;
                break;
            default:
                /* Simple increment */
                p++;
        }
    }
    
    return result;
}

/* Function using comma expression for sequencing */
int find_value_postinc(int *arr, int n, int target) {
    int *p = arr;
    int *end = arr + n;
    int found = 0;
    
    /* Comma expression: access then increment */
    while (p < end && (found = (*p, p++, *p != target))) {
        /* Loop continues searching */
    }
    
    return found;
}

/* Nested loops with post-increment */
void matrix_process(int matrix[][SIZE], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int *row = matrix[i];
        int *end = row + cols;
        
        /* Inner loop with post-increment */
        while (row < end) {
            *row++ *= 2;  /* Post-increment with assignment */
        }
    }
}

/* Function with multiple basic blocks and post-increment */
int complex_control_flow(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            /* Taken path with post-increment */
            sum += *p++;
        } else {
            /* Not-taken path - still increment */
            int temp = *p;
            p++;
            sum -= temp;
        }
        
        /* Additional control flow */
        if (sum > 1000) {
            /* Early exit with post-increment */
            sum += *p++;
            break;
        }
    }
    
    return sum;
}

int main() {
    /* Non-volatile arrays */
    int array[SIZE];
    int dest[SIZE];
    char str1[] = "Test string for post-increment copy";
    char str2[100];
    
    /* Volatile arrays */
    volatile int volatile_array[SIZE];
    volatile struct Data volatile_structs[PATTERN_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i;
        volatile_array[i] = i * 2;
    }
    
    for (int i = 0; i < PATTERN_SIZE; i++) {
        volatile_structs[i].value = i * 3;
        volatile_structs[i].count = i;
        volatile_structs[i].id = 'A' + i;
    }
    
    /* Test 1: Sum array with post-increment */
    int sum1 = sum_array_postinc(array, SIZE);
    printf("Sum of array (post-increment): %d\n", sum1);
    
    /* Test 2: Sum volatile array with post-increment */
    int sum2 = sum_volatile_postinc(volatile_array, SIZE);
    printf("Sum of volatile array: %d\n", sum2);
    
    /* Test 3: String copy with post-increment */
    copy_string_postinc(str2, str1);
    printf("Copied string: %s\n", str2);
    
    /* Test 4: Process structs */
    struct Data structs[PATTERN_SIZE];
    for (int i = 0; i < PATTERN_SIZE; i++) {
        structs[i].value = i * 4;
    }
    int struct_sum = process_structs(structs, PATTERN_SIZE);
    printf("Sum of struct values: %d\n", struct_sum);
    
    /* Test 5: Switch statement with post-increment */
    int switch_result = process_with_switch(array, SIZE, 0);
    printf("Switch processing result: %d\n", switch_result);
    
    /* Test 6: Complex control flow */
    int complex_result = complex_control_flow(array, SIZE);
    printf("Complex control flow result: %d\n", complex_result);
    
    /* Test 7: Find value with comma expression */
    int find_result = find_value_postinc(array, SIZE, 100);
    printf("Find result: %d\n", find_result);
    
    /* Test 8: Matrix processing */
    int matrix[10][SIZE];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * j;
        }
    }
    matrix_process(matrix, 10, SIZE);
    
    /* Verify some results */
    int verify_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        verify_sum += array[i];
    }
    
    printf("Verification sum: %d\n", verify_sum);
    printf("All tests completed.\n");
    
    return 0;
}
