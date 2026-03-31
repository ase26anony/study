/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Targets the (mem (reg)) pattern in auto-inc-dec.cc lines 1352-1358
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) patterns */
    while (n-- > 0) {
        sum += *p;      /* mem access via register */
        p++;            /* increment of same register */
    }
    return sum;
}

/* Pattern 2: Char pointer with explicit increment */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    /* Split operations to encourage separate RTL instructions */
    while (n-- > 0) {
        char temp = *s;     /* (mem (reg s)) */
        s = s + 1;          /* increment s */
        *d = temp;          /* (mem (reg d)) */
        d = d + 1;          /* increment d */
    }
}

/* Pattern 3: Short pointer with post-increment in for loop */
int16_t sum_array_short(const int16_t *arr, int n) {
    const int16_t *p = arr;
    int16_t sum = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        sum += *p;      /* Simple dereference */
        p += 1;         /* Explicit increment */
    }
    return sum;
}

/* Pattern 4: Direct pointer arithmetic in basic block */
int process_int_pair(int *restrict ptr) {
    int a, b;
    
    /* Two consecutive accesses with increment between */
    a = *ptr;           /* First access: (mem (reg ptr)) */
    ptr = ptr + 1;      /* Increment */
    b = *ptr;           /* Second access: (mem (reg ptr)) */
    
    return a + b;
}

/* Pattern 5: Mixed types to test different memory modes */
void fill_buffer_int32(int32_t *buf, int32_t value, int count) {
    int32_t *p = buf;
    
    while (count-- > 0) {
        *p = value;     /* Store via register */
        p++;            /* Post-increment */
    }
}

/* Pattern 6: Nested pointer operations */
void reverse_copy(char *restrict dst, const char *restrict src, int len) {
    const char *s = src;
    char *d = dst + len - 1;
    
    while (len-- > 0) {
        char val = *s;      /* Load from src */
        s++;                /* Increment src */
        *d = val;           /* Store to dst */
        d--;                /* Decrement dst - tests auto-dec too */
    }
}

/* Pattern 7: Local pointer in main - direct traversal */
int sum_local_array(void) {
    int local_arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int *p = local_arr;
    int sum = 0;
    int i;
    
    for (i = 0; i < 10; i++) {
        sum += *p;      /* Dereference pointer */
        p++;            /* Increment pointer */
    }
    return sum;
}

/* Pattern 8: Pointer to volatile (use sparingly) */
int sum_with_volatile(volatile int *arr, int n) {
    volatile int *p = arr;
    int sum = 0;
    
    while (n-- > 0) {
        sum += *p;      /* Volatile access */
        p++;            /* Increment */
    }
    return sum;
}

/* Main function to exercise all patterns */
int main(void) {
    int int_array[20];
    char char_buffer[100];
    int16_t short_array[50];
    int32_t int32_buffer[30];
    int checksum = 0;
    int i;
    
    /* Initialize test data */
    for (i = 0; i < 20; i++) int_array[i] = i;
    for (i = 0; i < 100; i++) char_buffer[i] = (char)(i % 256);
    for (i = 0; i < 50; i++) short_array[i] = (int16_t)(i * 2);
    for (i = 0; i < 30; i++) int32_buffer[i] = i * 3;
    
    /* Test Pattern 1: Simple int pointer traversal */
    checksum += sum_array_int(int_array, 20);
    
    /* Test Pattern 2: Char buffer copy */
    char dest_buffer[100];
    copy_buffer_char(dest_buffer, char_buffer, 100);
    for (i = 0; i < 100; i++) {
        checksum += dest_buffer[i];
    }
    
    /* Test Pattern 3: Short array sum */
    checksum += sum_array_short(short_array, 50);
    
    /* Test Pattern 4: Direct pointer arithmetic */
    checksum += process_int_pair(int_array);
    
    /* Test Pattern 5: Fill buffer */
    fill_buffer_int32(int32_buffer, 42, 30);
    for (i = 0; i < 30; i++) {
        checksum += int32_buffer[i];
    }
    
    /* Test Pattern 6: Reverse copy */
    char reversed[100];
    reverse_copy(reversed, char_buffer, 100);
    for (i = 0; i < 100; i++) {
        checksum += reversed[i];
    }
    
    /* Test Pattern 7: Local array traversal */
    checksum += sum_local_array();
    
    /* Test Pattern 8: Volatile access */
    checksum += sum_with_volatile(int_array, 10);
    
    /* Additional pattern in main itself */
    {
        int *ptr = int_array;
        int local_sum = 0;
        
        /* This loop in main should also generate the pattern */
        for (i = 0; i < 20; i++) {
            local_sum += *ptr;   /* Dereference */
            ptr++;               /* Increment */
        }
        checksum += local_sum;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("All patterns executed successfully.\n");
    
    /* Verify the copy worked */
    if (memcmp(char_buffer, dest_buffer, 100) == 0) {
        printf("Copy verification: PASSED\n");
    } else {
        printf("Copy verification: FAILED\n");
    }
    
    return 0;
}
