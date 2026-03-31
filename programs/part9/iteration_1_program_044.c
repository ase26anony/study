/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) patterns */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* separate increment - should be merged */
    }
    return sum;
}

/* Pattern 2: Using restrict to help alias analysis */
void copy_restrict(int *restrict dst, const int *restrict src, int n) {
    const int *s = src;
    int *d = dst;
    
    while (n-- > 0) {
        *d = *s;        /* Simple (mem (reg)) access */
        d = d + 1;      /* Separate increment */
        s = s + 1;      /* Separate increment */
    }
}

/* Pattern 3: Char pointer traversal - different mode (QImode) */
int count_chars(const char *str) {
    const char *p = str;
    int count = 0;
    
    while (*p != '\0') {
        if (*p == 'a')  /* Simple char dereference */
            count++;
        p = p + 1;      /* Separate increment */
    }
    return count;
}

/* Pattern 4: Mixed operations in loop */
void fill_sequence(short *arr, int n) {
    short *p = arr;
    
    for (int i = 0; i < n; i++) {
        *p = (short)i;  /* Store with simple address */
        p = p + 1;      /* Increment separately */
    }
}

/* Pattern 5: Double pointer increment in same loop */
void copy_and_increment(int *dst, const int *src, int n, int *out_sum) {
    const int *s = src;
    int *d = dst;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int val = *s;   /* Load with (mem (reg)) */
        sum += val;
        *d = val;       /* Store with (mem (reg)) */
        s = s + 1;      /* Increment source */
        d = d + 1;      /* Increment destination */
    }
    
    *out_sum = sum;
}

/* Pattern 6: While loop with pointer comparison */
int find_value(const int *arr, int n, int target) {
    const int *p = arr;
    const int *end = arr + n;
    
    while (p < end) {
        if (*p == target)  /* Simple dereference */
            return 1;
        p = p + 1;         /* Separate increment */
    }
    return 0;
}

/* Pattern 7: Local array with pointer traversal */
int sum_local_array(void) {
    int local_arr[16];
    int *p = local_arr;
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 16; i++) {
        local_arr[i] = i;
    }
    
    /* Traverse with pointer */
    for (int i = 0; i < 16; i++) {
        sum += *p;      /* Should generate (mem (reg p)) */
        p = p + 1;      /* Separate increment */
    }
    
    return sum;
}

/* Pattern 8: Nested loops with pointer reset */
void matrix_sum_rows(int matrix[4][4], int *row_sums) {
    for (int i = 0; i < 4; i++) {
        int *row = matrix[i];  /* Get pointer to row */
        int sum = 0;
        
        for (int j = 0; j < 4; j++) {
            sum += *row;       /* Simple dereference */
            row = row + 1;     /* Separate increment */
        }
        
        row_sums[i] = sum;
    }
}

/* Main test function */
int main(void) {
    int arr1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int arr2[10] = {0};
    int arr3[4][4];
    int row_sums[4];
    
    /* Initialize matrix */
    int val = 1;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            arr3[i][j] = val++;
        }
    }
    
    /* Test Pattern 1 */
    int sum1 = sum_array_int(arr1, 10);
    printf("Sum1: %d\n", sum1);
    
    /* Test Pattern 2 */
    copy_restrict(arr2, arr1, 10);
    int sum2 = sum_array_int(arr2, 10);
    printf("Sum2: %d\n", sum2);
    
    /* Test Pattern 3 */
    const char *test_str = "auto_increment_test_string";
    int char_count = count_chars(test_str);
    printf("Char count: %d\n", char_count);
    
    /* Test Pattern 4 */
    short short_arr[20];
    fill_sequence(short_arr, 20);
    printf("Short arr[5]: %d\n", (int)short_arr[5]);
    
    /* Test Pattern 5 */
    int arr4[10];
    int out_sum;
    copy_and_increment(arr4, arr1, 10, &out_sum);
    printf("Copy sum: %d\n", out_sum);
    
    /* Test Pattern 6 */
    int found = find_value(arr1, 10, 5);
    printf("Found 5: %s\n", found ? "yes" : "no");
    
    /* Test Pattern 7 */
    int local_sum = sum_local_array();
    printf("Local sum: %d\n", local_sum);
    
    /* Test Pattern 8 */
    matrix_sum_rows(arr3, row_sums);
    printf("Row sums: %d %d %d %d\n", 
           row_sums[0], row_sums[1], row_sums[2], row_sums[3]);
    
    /* Additional pattern in main itself */
    int *p = arr1;
    int main_sum = 0;
    for (int i = 0; i < 10; i++) {
        main_sum += *p;  /* Should generate (mem (reg p)) */
        p = p + 1;       /* Separate increment */
    }
    printf("Main sum: %d\n", main_sum);
    
    /* Final checksum */
    int total = sum1 + sum2 + char_count + short_arr[5] + 
                out_sum + found + local_sum + 
                row_sums[0] + main_sum;
    printf("Total checksum: %d\n", total);
    
    return 0;
}
