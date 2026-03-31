/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Specifically targets (mem (reg)) patterns followed by register increment
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    const int *end = arr + n;
    int sum = 0;
    
    /* This should generate (mem (reg)) patterns */
    while (p < end) {
        sum += *p;      /* mem access via register */
        p++;            /* increment of same register */
    }
    return sum;
}

/* Pattern 2: Copy with restrict pointers - helps alias analysis */
void copy_int_restrict(int *restrict dst, const int *restrict src, int n) {
    const int *s = src;
    int *d = dst;
    const int *end = src + n;
    
    /* Classic memcpy-like pattern */
    while (s < end) {
        *d = *s;        /* Two (mem (reg)) accesses */
        d++;            /* Post-increment */
        s++;            /* Post-increment */
    }
}

/* Pattern 3: Char pointer traversal - different mode (QImode) */
int count_chars(const char *str) {
    const char *p = str;
    int count = 0;
    
    while (*p != '\0') {
        count += *p;    /* mem access via register */
        p++;            /* increment of same register */
    }
    return count;
}

/* Pattern 4: Explicit split operations in same basic block */
int process_buffer(short *buf, int n) {
    short *p = buf;
    int total = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        short val = *p;     /* Load via register */
        p = p + 1;          /* Explicit increment - separate statement */
        total += val * val;
    }
    return total;
}

/* Pattern 5: Mixed operations with local pointer */
void fill_sequence(int *arr, int n) {
    int *p = arr;
    int value = 1;
    
    while (n-- > 0) {
        *p = value;         /* Store via register */
        p++;                /* Post-increment */
        value += 2;
    }
}

/* Pattern 6: Nested pointer usage */
void copy_reverse(int *dst, const int *src, int n) {
    const int *s = src;
    int *d = dst + n - 1;
    
    while (n-- > 0) {
        *d = *s;            /* Two (mem (reg)) accesses */
        s++;                /* Increment source */
        d--;                /* Decrement destination */
    }
}

/* Pattern 7: Simple while loop with pointer */
int find_max(const int *arr, int n) {
    const int *p = arr;
    const int *end = arr + n;
    int max = *p++;         /* Combined access and increment */
    
    while (p < end) {
        if (*p > max) {     /* mem access via register */
            max = *p;
        }
        p++;                /* increment */
    }
    return max;
}

/* Pattern 8: Byte-wise copy without library functions */
void byte_copy(char *dst, const char *src, size_t n) {
    char *d = dst;
    const char *s = src;
    
    while (n--) {
        *d = *s;            /* Two byte accesses via registers */
        d++;                /* Post-increment */
        s++;                /* Post-increment */
    }
}

/* Main function with various test cases */
int main(void) {
    /* Test data */
    int arr1[100];
    int arr2[100];
    char str[] = "Test string for char pointer traversal";
    short shorts[50];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        arr1[i] = i * 2;
    }
    
    for (int i = 0; i < 50; i++) {
        shorts[i] = (short)(i * 3);
    }
    
    /* Test 1: Sum array with pointer traversal */
    int sum = sum_array_int(arr1, 100);
    printf("Sum of arr1: %d\n", sum);
    
    /* Test 2: Copy with restrict pointers */
    copy_int_restrict(arr2, arr1, 100);
    
    /* Verify copy */
    int sum2 = sum_array_int(arr2, 100);
    printf("Sum of arr2 (copy): %d\n", sum2);
    
    /* Test 3: Char pointer traversal */
    int char_sum = count_chars(str);
    printf("Char sum: %d\n", char_sum);
    
    /* Test 4: Explicit split operations */
    int short_total = process_buffer(shorts, 50);
    printf("Short buffer total: %d\n", short_total);
    
    /* Test 5: Fill sequence */
    int arr3[20];
    fill_sequence(arr3, 20);
    printf("First/last of sequence: %d, %d\n", arr3[0], arr3[19]);
    
    /* Test 6: Reverse copy */
    int arr4[10];
    int arr5[10] = {0,1,2,3,4,5,6,7,8,9};
    copy_reverse(arr4, arr5, 10);
    printf("Reverse copy first: %d\n", arr4[0]);
    
    /* Test 7: Find max */
    int max_val = find_max(arr1, 100);
    printf("Max value in arr1: %d\n", max_val);
    
    /* Test 8: Byte copy */
    char src[10] = {1,2,3,4,5,6,7,8,9,10};
    char dst[10];
    byte_copy(dst, src, 10);
    printf("Byte copy check: %d\n", dst[9]);
    
    /* Additional pointer traversal in main itself */
    int *ptr = arr1;
    int local_sum = 0;
    for (int i = 0; i < 10; i++) {
        local_sum += *ptr;  /* mem access via register */
        ptr++;              /* increment */
    }
    printf("Main loop sum: %d\n", local_sum);
    
    /* Final verification */
    if (sum == sum2 && dst[9] == 10) {
        printf("All tests passed successfully!\n");
        return 0;
    } else {
        printf("Test verification failed\n");
        return 1;
    }
}
