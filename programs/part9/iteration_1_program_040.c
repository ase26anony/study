/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Specifically targets (mem (reg)) patterns followed by register increment
 */

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
        p = p + 1;      /* increment in separate statement */
    }
    return sum;
}

/* Pattern 2: Pointer traversal with restrict qualifier */
void copy_buffer_restrict(char *restrict dst, const char *restrict src, int n) {
    /* Classic memcpy pattern - should generate optimal auto-inc */
    while (n-- > 0) {
        *dst = *src;    /* Simple (mem (reg)) access */
        dst = dst + 1;  /* Separate increment */
        src = src + 1;
    }
}

/* Pattern 3: Mixed types to test different memory modes */
short sum_array_short(const short *arr, int n) {
    const short *p = arr;
    short sum = 0;
    
    /* Tight loop with pointer increment */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* HImode memory access */
        p++;            /* Post-increment */
    }
    return sum;
}

/* Pattern 4: Explicit split operations in basic block */
int process_char_buffer(const char *buf, int len) {
    const char *ptr = buf;
    int count = 0;
    
    /* Force separate statements in same basic block */
    while (len > 0) {
        char c = *ptr;          /* Load via register */
        ptr = ptr + 1;          /* Increment separately */
        if (c == 'A') count++;
        len--;
    }
    return count;
}

/* Pattern 5: Double pointer increment pattern */
void fill_buffer_alternating(int *buf, int n, int val1, int val2) {
    int *p = buf;
    
    for (int i = 0; i < n; i += 2) {
        *p = val1;      /* First store */
        p = p + 1;      /* Increment */
        
        *p = val2;      /* Second store */
        p = p + 1;      /* Increment again */
    }
}

/* Pattern 6: Local pointer with no function arguments */
int sum_local_array(void) {
    int local_arr[100];
    int *p = local_arr;
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        local_arr[i] = i;
    }
    
    /* Pointer traversal */
    for (int i = 0; i < 100; i++) {
        sum += *p;      /* Dereference */
        p++;            /* Post-increment */
    }
    return sum;
}

/* Pattern 7: Nested pointer operations */
void transform_array(int *dst, const int *src, int n) {
    const int *s = src;
    int *d = dst;
    
    for (int i = 0; i < n; i++) {
        int val = *s;   /* Load from source */
        s = s + 1;      /* Increment source pointer */
        
        *d = val * 2;   /* Store to destination */
        d = d + 1;      /* Increment destination pointer */
    }
}

/* Pattern 8: Main function with its own pointer traversal */
int main(void) {
    int int_array[50];
    char char_buffer[100];
    short short_array[30];
    int dest_array[50];
    
    /* Initialize test data */
    for (int i = 0; i < 50; i++) {
        int_array[i] = i * 2;
    }
    
    memset(char_buffer, 'A', sizeof(char_buffer));
    char_buffer[49] = 'B';  /* One different character */
    
    for (int i = 0; i < 30; i++) {
        short_array[i] = (short)(i * 3);
    }
    
    /* Test Pattern 1: Simple int array sum */
    int sum1 = sum_array_int(int_array, 50);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test Pattern 2: Restrict copy */
    char dest_buffer[100];
    copy_buffer_restrict(dest_buffer, char_buffer, 100);
    printf("Copy completed, first char: %c\n", dest_buffer[0]);
    
    /* Test Pattern 3: Short array sum */
    short sum3 = sum_array_short(short_array, 30);
    printf("Sum of short array: %d\n", (int)sum3);
    
    /* Test Pattern 4: Char buffer processing */
    int count_a = process_char_buffer(char_buffer, 100);
    printf("Count of 'A': %d\n", count_a);
    
    /* Test Pattern 5: Alternating fill */
    fill_buffer_alternating(dest_array, 50, 1, 2);
    printf("Fill completed, first two values: %d, %d\n", dest_array[0], dest_array[1]);
    
    /* Test Pattern 6: Local array sum */
    int sum6 = sum_local_array();
    printf("Sum of local array: %d\n", sum6);
    
    /* Test Pattern 7: Transform array */
    transform_array(dest_array, int_array, 50);
    printf("Transform completed, first value: %d\n", dest_array[0]);
    
    /* Additional pattern in main: Pointer traversal */
    int *ptr = int_array;
    int main_sum = 0;
    for (int i = 0; i < 50; i++) {
        main_sum += *ptr;   /* Direct dereference */
        ptr = ptr + 1;      /* Separate increment */
    }
    printf("Main pointer traversal sum: %d\n", main_sum);
    
    /* Verify results */
    int expected_sum = 0;
    for (int i = 0; i < 50; i++) {
        expected_sum += i * 2;
    }
    
    if (main_sum == expected_sum && sum1 == expected_sum) {
        printf("All tests passed successfully!\n");
        return 0;
    } else {
        printf("Test verification failed\n");
        return 1;
    }
}
