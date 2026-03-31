#include <stdio.h>
#include <string.h>

#define SIZE 100
#define PATTERN_SIZE 50

/* Function prototypes */
int sum_array_postinc(int *arr, int n);
int find_value_postinc(int *arr, int n, int target);
void copy_array_postinc(int *dest, int *src, int n);
void process_mixed_pointers(volatile int *vptr, int *ptr, int n);
int complex_control_flow(int *arr, int n, int threshold);

/* Main function with various post-increment patterns */
int main(void) {
    /* Non-volatile arrays */
    int array1[SIZE];
    int array2[SIZE];
    int result_array[SIZE];
    
    /* Volatile arrays */
    volatile int volatile_array[SIZE];
    volatile int volatile_buffer[PATTERN_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i * 2;
        array2[i] = i * 3;
        volatile_array[i] = i * 4;
    }
    
    for (int i = 0; i < PATTERN_SIZE; i++) {
        volatile_buffer[i] = i * 5;
    }
    
    printf("Testing post-increment/decrement patterns for auto-inc-dec optimization\n\n");
    
    /* Test 1: Simple summation with post-increment in loop */
    int sum1 = sum_array_postinc(array1, SIZE);
    printf("Sum of array1 (post-increment): %d\n", sum1);
    
    /* Test 2: Copy array using post-increment */
    copy_array_postinc(result_array, array2, SIZE);
    int sum2 = sum_array_postinc(result_array, SIZE);
    printf("Sum of copied array2 (post-increment copy): %d\n", sum2);
    
    /* Test 3: Find value with post-increment */
    int target = 42;
    int found_index = find_value_postinc(array1, SIZE, target);
    printf("Found %d at index (or -1): %d\n", target, found_index);
    
    /* Test 4: Mixed volatile and non-volatile pointers */
    process_mixed_pointers(volatile_array, array1, SIZE);
    
    /* Test 5: Complex control flow with post-increment */
    int complex_result = complex_control_flow(array1, SIZE, 50);
    printf("Complex control flow result: %d\n", complex_result);
    
    /* Test 6: String/byte copying with post-increment */
    char src_str[] = "Test string for post-increment copying";
    char dest_str[100];
    char *s = src_str, *d = dest_str;
    
    /* Classic K&R string copy with post-increment */
    while ((*d++ = *s++) != '\0')
        ;
    printf("Copied string: %s\n", dest_str);
    
    /* Test 7: Nested loops with post-increment */
    int matrix[10][10];
    int row_sum[10] = {0};
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Process matrix with post-increment in inner loop */
    for (int i = 0; i < 10; i++) {
        int *row_ptr = matrix[i];
        int *end_ptr = row_ptr + 10;
        while (row_ptr < end_ptr) {
            row_sum[i] += *row_ptr++;
        }
        printf("Row %d sum: %d\n", i, row_sum[i]);
    }
    
    /* Test 8: Switch statement with post-increment */
    int switch_test = 0;
    int *ptr = array1;
    
    for (int i = 0; i < 5; i++) {
        switch (i % 3) {
            case 0:
                switch_test += *ptr++;
                break;
            case 1:
                switch_test -= *ptr++;
                break;
            case 2:
                switch_test *= *ptr++;
                break;
        }
    }
    printf("Switch test result: %d\n", switch_test);
    
    /* Test 9: Comma expression with post-increment */
    int temp_sum = 0;
    int *p = array1;
    int *q = array2;
    
    for (int i = 0; i < 10; i++) {
        /* Comma expression: access then increment */
        temp_sum += (temp_sum = *p++, *q++);
    }
    printf("Comma expression test: %d\n", temp_sum);
    
    return 0;
}

/* Function 1: Sum array using post-increment in loop condition */
int sum_array_postinc(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    /* Post-increment in loop condition */
    while (p < end) {
        sum += *p++;
    }
    
    return sum;
}

/* Function 2: Find value using post-increment in search */
int find_value_postinc(int *arr, int n, int target) {
    int *p = arr;
    int *end = arr + n;
    int index = 0;
    
    /* Post-increment in while condition */
    while (p < end && *p++ != target) {
        index++;
    }
    
    return (index < n) ? index : -1;
}

/* Function 3: Copy array using post-increment */
void copy_array_postinc(int *dest, int *src, int n) {
    int *d = dest;
    int *s = src;
    int *end = src + n;
    
    /* Classic copy loop with post-increment */
    while (s < end) {
        *d++ = *s++;
    }
}

/* Function 4: Process mixed volatile and non-volatile pointers */
void process_mixed_pointers(volatile int *vptr, int *ptr, int n) {
    volatile int *v_end = vptr + n;
    int *p_end = ptr + n;
    
    /* Process volatile array with post-increment */
    volatile int *vp = vptr;
    int volatile_sum = 0;
    while (vp < v_end) {
        volatile_sum += *vp++;
    }
    printf("Volatile array sum: %d\n", volatile_sum);
    
    /* Process non-volatile array with post-increment */
    int *p = ptr;
    int non_volatile_sum = 0;
    while (p < p_end) {
        non_volatile_sum += *p++;
    }
    printf("Non-volatile array sum: %d\n", non_volatile_sum);
    
    /* Mixed access in same loop */
    vp = vptr;
    p = ptr;
    int mixed_sum = 0;
    for (int i = 0; i < n; i++) {
        mixed_sum += *vp++ + *p++;
    }
    printf("Mixed pointers sum: %d\n", mixed_sum);
}

/* Function 5: Complex control flow with post-increment */
int complex_control_flow(int *arr, int n, int threshold) {
    int result = 0;
    int *p = arr;
    int *end = arr + n;
    
    /* Complex loop with if-else and post-increment */
    while (p < end) {
        if (*p < threshold) {
            /* Post-increment in taken path */
            result += *p++;
        } else {
            /* Post-increment in not-taken path */
            result -= *p++;
        }
        
        /* Additional condition with post-increment */
        if (p < end && *p % 2 == 0) {
            result += *p++ * 2;
        }
    }
    
    /* Nested loop with post-increment */
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        int *row = matrix[i];
        for (int j = 0; j < 5; j++) {
            *row++ = i * j;
        }
    }
    
    return result;
}
