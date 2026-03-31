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

/* Structure to enable pointer arithmetic on fields */
struct Data {
    int value;
    int count;
    char id;
};

/* Function 1: Copy with post-increment pointers (classic strcpy-like) */
void copy_with_postinc(char *dest, const char *src, int n) {
    char *d = dest;
    const char *s = src;
    
    /* Basic post-increment in loop condition */
    while (n-- > 0) {
        *d++ = *s++;  /* Target pattern: mem access followed by increment */
    }
}

/* Function 2: Summation with mixed volatile and non-volatile pointers */
int sum_with_postinc(volatile int *varr, int *arr, int n) {
    volatile int *vp = varr;
    int *p = arr;
    int sum = 0;
    
    /* Loop with post-increment in update statement */
    for (int i = 0; i < n; i++) {
        sum += *vp++;  /* Volatile pointer post-increment */
        sum += *p++;   /* Non-volatile pointer post-increment */
    }
    
    return sum;
}

/* Function 3: Search with post-increment in condition */
int find_value(const int *arr, int n, int target) {
    const int *p = arr;
    const int *end = arr + n;
    
    /* Post-increment in loop condition */
    while (p < end && *p++ != target) {
        /* Empty body - increment happens in condition */
    }
    
    return (p - 1) - arr;  /* Return index where found or n */
}

/* Function 4: Structure array processing with post-increment */
int process_structs(struct Data *sptr, int count) {
    int total = 0;
    struct Data *end = sptr + count;
    
    /* Nested logic with post-increment */
    while (sptr < end) {
        /* Access field then increment pointer */
        total += sptr->value;
        sptr++;  /* Post-increment after field access */
        
        /* Alternative: could use sptr++->value in the access */
    }
    
    return total;
}

/* Function 5: Complex control flow with post-increment */
int conditional_postinc(int *arr, int n, int threshold) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Post-increment in both branches of conditional */
        if (*p > threshold) {
            sum += *p++;  /* Taken path */
        } else {
            sum -= *p++;  /* Not-taken path */
        }
    }
    
    return sum;
}

/* Function 6: Switch statement with fall-through and post-increment */
int switch_postinc(char *str) {
    char *p = str;
    int count = 0;
    
    while (*p) {
        switch (*p++) {  /* Post-increment in switch expression */
            case 'a':
            case 'e':
            case 'i':
            case 'o':
            case 'u':
                count++;  /* Fall through intended */
                break;
            default:
                /* Continue with post-incremented pointer */
                break;
        }
    }
    
    return count;
}

/* Function 7: Comma expression with post-increment */
int comma_postinc(int *ptr) {
    int temp;
    
    /* Comma expression: access then increment */
    temp = (*ptr, ptr++, 0);  /* Returns 0, but has side effects */
    
    /* More practical comma expression */
    temp = (temp = *ptr, ptr++, temp);  /* Access, increment, use value */
    
    return temp;
}

/* Function 8: Byte-wise copy with post-increment (tight loop) */
void byte_copy(volatile unsigned char *dst, 
               volatile unsigned char *src, 
               int length) {
    volatile unsigned char *d = dst;
    volatile unsigned char *s = src;
    
    /* Very tight loop likely to generate auto-inc RTL */
    while (length-- > 0) {
        *d++ = *s++;
    }
}

/* Main function that exercises all patterns */
int main() {
    /* Test data */
    char src_str[] = "Test string for auto-increment patterns";
    char dest_str[100];
    
    volatile int volatile_arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int regular_arr[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    struct Data structs[5] = {
        {100, 1, 'A'},
        {200, 2, 'B'},
        {300, 3, 'C'},
        {400, 4, 'D'},
        {500, 5, 'E'}
    };
    
    int search_arr[8] = {5, 10, 15, 20, 25, 30, 35, 40};
    
    volatile unsigned char vol_buffer1[50];
    volatile unsigned char vol_buffer2[50];
    
    /* Execute test functions */
    
    /* 1. Copy with post-increment */
    copy_with_postinc(dest_str, src_str, strlen(src_str) + 1);
    printf("Copy result: %s\n", dest_str);
    
    /* 2. Summation with mixed pointers */
    int sum = sum_with_postinc(volatile_arr, regular_arr, 10);
    printf("Sum result: %d\n", sum);
    
    /* 3. Search with post-increment */
    int found_idx = find_value(search_arr, 8, 25);
    printf("Found 25 at index: %d\n", found_idx);
    
    /* 4. Structure processing */
    int struct_sum = process_structs(structs, 5);
    printf("Structure sum: %d\n", struct_sum);
    
    /* 5. Conditional post-increment */
    int cond_sum = conditional_postinc(regular_arr, 10, 50);
    printf("Conditional sum: %d\n", cond_sum);
    
    /* 6. Switch with post-increment */
    int vowel_count = switch_postinc(src_str);
    printf("Vowel count: %d\n", vowel_count);
    
    /* 7. Comma expression (less practical, but tests pattern) */
    int comma_val = comma_postinc(regular_arr);
    printf("Comma expression result: %d\n", comma_val);
    
    /* 8. Byte copy with volatile */
    for (int i = 0; i < 50; i++) {
        vol_buffer1[i] = i;
    }
    byte_copy(vol_buffer2, vol_buffer1, 50);
    printf("Byte copy completed, first byte: %d\n", (int)vol_buffer2[0]);
    
    /* Additional tight loop patterns */
    
    /* Loop with pointer arithmetic in condition */
    char *ptr = dest_str;
    int zero_count = 0;
    while (*ptr++ != '\0') {
        /* Count characters - increment in condition */
    }
    
    /* Array access with index variable */
    int idx = 0;
    int arr_sum = 0;
    while (idx < 10) {
        arr_sum += regular_arr[idx++];  /* Post-increment of index */
    }
    printf("Array sum with index post-inc: %d\n", arr_sum);
    
    /* Nested loop with inner post-increment */
    int matrix[3][4] = {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}};
    int matrix_sum = 0;
    for (int i = 0; i < 3; i++) {
        int *row_ptr = matrix[i];
        for (int j = 0; j < 4; j++) {
            matrix_sum += *row_ptr++;  /* Inner loop post-increment */
        }
    }
    printf("Matrix sum: %d\n", matrix_sum);
    
    return 0;
}
