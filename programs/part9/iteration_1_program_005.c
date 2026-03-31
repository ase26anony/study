/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference followed by increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) pattern */
    while (n-- > 0) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* separate increment - should trigger find_inc() */
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict qualifier */
void copy_buffer_restrict(char *restrict dst, const char *restrict src, int n) {
    /* Classic memcpy pattern - should generate post-increment */
    while (n-- > 0) {
        *dst = *src;    /* Simple (mem (reg)) access */
        dst = dst + 1;  /* Separate increment statements */
        src = src + 1;
    }
}

/* Pattern 3: Mixed statements in same basic block */
int process_chars(const char *data, int len) {
    const char *ptr = data;
    int count = 0;
    
    for (int i = 0; i < len; i++) {
        char val = *ptr;        /* Load via register */
        ptr = ptr + 1;          /* Increment in next statement */
        if (val > 0) count++;
    }
    return count;
}

/* Pattern 4: Pointer arithmetic split across operations */
short sum_shorts(const short *arr, int n) {
    const short *p = arr;
    short total = 0;
    
    while (n > 0) {
        short temp = *p;    /* (mem (reg)) pattern */
        total += temp;
        p = p + 1;          /* Plain increment */
        n--;
    }
    return total;
}

/* Pattern 5: Local pointer with no complex addressing */
void fill_sequence(int *buffer, int size) {
    int *ptr = buffer;
    int value = 1;
    
    for (int i = 0; i < size; i++) {
        *ptr = value;       /* Store via register */
        ptr = ptr + 1;      /* Increment separately */
        value += 2;
    }
}

/* Pattern 6: While loop with pointer post-increment */
int find_max(const int *data, int n) {
    const int *p = data;
    int max = *p;           /* Initial load */
    p = p + 1;              /* Increment */
    
    while (--n > 0) {
        int val = *p;       /* Load via register */
        p = p + 1;          /* Separate increment */
        if (val > max) max = val;
    }
    return max;
}

/* Pattern 7: Main function with its own traversal */
int main(void) {
    const int ARRAY_SIZE = 100;
    const int BUFFER_SIZE = 256;
    
    /* Test data */
    int int_array[ARRAY_SIZE];
    char src_buffer[BUFFER_SIZE];
    char dst_buffer[BUFFER_SIZE];
    short short_array[ARRAY_SIZE];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3;
        short_array[i] = (short)(i * 2);
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        src_buffer[i] = (char)(i % 128);
    }
    
    /* Test Pattern 1: Simple pointer traversal */
    int sum1 = sum_array_int(int_array, ARRAY_SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test Pattern 2: Restrict pointer copy */
    copy_buffer_restrict(dst_buffer, src_buffer, BUFFER_SIZE);
    
    /* Verify copy */
    if (memcmp(src_buffer, dst_buffer, BUFFER_SIZE) == 0) {
        printf("Buffer copy successful\n");
    }
    
    /* Test Pattern 3: Char processing */
    int count = process_chars(src_buffer, BUFFER_SIZE);
    printf("Positive chars: %d\n", count);
    
    /* Test Pattern 4: Short array sum */
    short short_sum = sum_shorts(short_array, ARRAY_SIZE);
    printf("Sum of shorts: %d\n", (int)short_sum);
    
    /* Test Pattern 5: Fill sequence */
    int fill_buffer[50];
    fill_sequence(fill_buffer, 50);
    
    /* Test Pattern 6: Find maximum */
    int max_val = find_max(int_array, ARRAY_SIZE);
    printf("Maximum value: %d\n", max_val);
    
    /* Additional test in main: Pointer traversal */
    int *ptr = int_array;
    int local_sum = 0;
    
    /* This loop in main should also generate the pattern */
    for (int i = 0; i < 10; i++) {
        int val = *ptr;     /* Should be (mem (reg)) */
        local_sum += val;
        ptr = ptr + 1;      /* Separate increment */
    }
    printf("Local sum: %d\n", local_sum);
    
    /* Calculate final checksum */
    int checksum = sum1 + count + short_sum + max_val + local_sum;
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
