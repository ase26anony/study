/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) patterns */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* mem (reg p) */
        p = p + 1;      /* increment of p - should merge with above */
    }
    return sum;
}

/* Pattern 2: Using restrict to help alias analysis */
void copy_restrict(int *restrict dst, const int *restrict src, int n) {
    const int *s = src;
    int *d = dst;
    
    while (n-- > 0) {
        *d = *s;        /* mem (reg s) and mem (reg d) */
        d = d + 1;      /* increment d */
        s = s + 1;      /* increment s */
    }
}

/* Pattern 3: Char pointer traversal - simple (mem (reg)) */
int count_chars(const char *str) {
    const char *p = str;
    int count = 0;
    
    while (*p != '\0') {
        if (*p == 'a')  /* mem (reg p) */
            count++;
        p = p + 1;      /* increment p */
    }
    return count;
}

/* Pattern 4: Explicit split operations in same basic block */
int process_buffer(short *buf, int size) {
    short *ptr = buf;
    int total = 0;
    
    for (int i = 0; i < size; i++) {
        short val = *ptr;   /* mem (reg ptr) */
        total += val;
        ptr = ptr + 1;      /* increment ptr */
    }
    return total;
}

/* Pattern 5: Multiple dereferences with same base */
void double_deref(int *a, int *b, int *out, int n) {
    int *p = a;
    int *q = b;
    int *r = out;
    
    for (int i = 0; i < n; i++) {
        int x = *p;         /* mem (reg p) */
        int y = *q;         /* mem (reg q) */
        *r = x + y;         /* mem (reg r) */
        p = p + 1;          /* increment p */
        q = q + 1;          /* increment q */
        r = r + 1;          /* increment r */
    }
}

/* Pattern 6: While loop with pointer increment in body */
void fill_sequence(int *arr, int n) {
    int *p = arr;
    int value = 0;
    
    while (n-- > 0) {
        *p = value++;       /* mem (reg p) */
        p = p + 1;          /* increment p */
    }
}

/* Pattern 7: Main function with its own traversal */
int main(void) {
    const int ARRAY_SIZE = 100;
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    int array3[ARRAY_SIZE];
    char buffer[256];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = i * 2;
    }
    
    strcpy(buffer, "test string with several a characters for counting");
    
    /* Test Pattern 1 */
    int sum1 = sum_array_int(array1, ARRAY_SIZE);
    printf("Sum1: %d\n", sum1);
    
    /* Test Pattern 2 */
    copy_restrict(array3, array1, ARRAY_SIZE);
    int sum2 = sum_array_int(array3, ARRAY_SIZE);
    printf("Sum2: %d\n", sum2);
    
    /* Test Pattern 3 */
    int char_count = count_chars(buffer);
    printf("Char count: %d\n", char_count);
    
    /* Test Pattern 4 */
    short short_buf[50];
    for (int i = 0; i < 50; i++) short_buf[i] = i;
    int short_sum = process_buffer(short_buf, 50);
    printf("Short sum: %d\n", short_sum);
    
    /* Test Pattern 5 */
    double_deref(array1, array2, array3, ARRAY_SIZE);
    int sum3 = sum_array_int(array3, ARRAY_SIZE);
    printf("Sum3: %d\n", sum3);
    
    /* Test Pattern 6 */
    fill_sequence(array1, ARRAY_SIZE);
    int sum4 = sum_array_int(array1, ARRAY_SIZE);
    printf("Sum4: %d\n", sum4);
    
    /* Pattern in main itself */
    int *ptr = array1;
    int main_sum = 0;
    for (int i = 0; i < 10; i++) {
        main_sum += *ptr;   /* mem (reg ptr) */
        ptr = ptr + 1;      /* increment ptr */
    }
    printf("Main sum: %d\n", main_sum);
    
    /* Final checksum */
    int total_checksum = sum1 + sum2 + sum3 + sum4 + main_sum + char_count + short_sum;
    printf("Total checksum: %d\n", total_checksum);
    
    /* Verification */
    int expected_sum1 = (ARRAY_SIZE - 1) * ARRAY_SIZE / 2;
    if (sum1 == expected_sum1) {
        printf("Pattern 1 verification: PASS\n");
    } else {
        printf("Pattern 1 verification: FAIL (expected %d, got %d)\n", expected_sum1, sum1);
    }
    
    return 0;
}
