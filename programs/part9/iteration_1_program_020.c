/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Specifically targets the (mem (reg)) pattern in find_address_incs()
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) pattern */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* separate increment - should be merged */
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict qualifier */
void copy_buffer_restrict(char *restrict dst, const char *restrict src, int n) {
    /* Classic memcpy pattern - should trigger auto-inc */
    while (n-- > 0) {
        *dst = *src;    /* Simple (mem (reg)) access */
        dst = dst + 1;  /* Separate increment */
        src = src + 1;
    }
}

/* Pattern 3: Pointer traversal with explicit increments */
int find_first_zero(const int *arr, int n) {
    const int *ptr = arr;
    int i = 0;
    
    /* Dereference and increment in separate statements */
    while (i < n) {
        if (*ptr == 0)  /* (mem (reg)) pattern */
            return i;
        ptr = ptr + 1;  /* Should merge with previous access */
        i++;
    }
    return -1;
}

/* Pattern 4: Short type with post-increment in loop body */
short sum_shorts(const short *data, int count) {
    const short *p = data;
    short total = 0;
    
    for (int i = 0; i < count; i++) {
        total += *p;    /* Access via register pointer */
        p++;            /* Post-increment - may be separate initially */
    }
    return total;
}

/* Pattern 5: Mixed operations to create basic block with pattern */
void fill_alternating(int *buf, int n, int val1, int val2) {
    int *p = buf;
    
    /* Two memory writes with pointer increments */
    for (int i = 0; i < n; i += 2) {
        *p = val1;      /* First write */
        p = p + 1;      /* Increment */
        
        if (i + 1 < n) {
            *p = val2;  /* Second write */
            p = p + 1;  /* Another increment */
        }
    }
}

/* Pattern 6: Local pointer with no function arguments */
int process_local_buffer(void) {
    char buffer[256];
    char *p = buffer;
    int checksum = 0;
    
    /* Initialize buffer */
    for (int i = 0; i < 256; i++) {
        buffer[i] = (char)(i & 0xFF);
    }
    
    /* Process with pointer arithmetic */
    for (int i = 0; i < 256; i++) {
        checksum += *p;     /* Dereference pointer */
        p = p + 1;          /* Increment separately */
    }
    
    return checksum;
}

/* Pattern 7: Double pointer increment (two increments after use) */
void double_increment_pattern(int *dst, const int *src, int n) {
    int *d = dst;
    const int *s = src;
    
    while (n-- > 0) {
        *d = *s;        /* Two (mem (reg)) accesses */
        d = d + 1;      /* First increment */
        s = s + 1;      /* Second increment */
    }
}

/* Pattern 8: Complex enough to avoid other optimizations but keep pattern */
int sum_every_other(const int *arr, int n) {
    const int *ptr = arr;
    int sum = 0;
    int i = 0;
    
    while (i < n) {
        sum += *ptr;            /* Memory access */
        ptr = ptr + 2;          /* Larger stride */
        i += 2;
        
        /* Add some conditional to prevent loop unrolling */
        if (i < n && *ptr < 0) {
            sum += *ptr;        /* Another access */
            ptr = ptr + 2;
            i += 2;
        }
    }
    return sum;
}

/* Main function with various test cases */
int main(void) {
    const int ARRAY_SIZE = 100;
    int int_array[ARRAY_SIZE];
    char char_buffer1[ARRAY_SIZE];
    char char_buffer2[ARRAY_SIZE];
    short short_array[ARRAY_SIZE];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 2;
        char_buffer1[i] = (char)(i & 0x7F);
        short_array[i] = (short)(i * 3);
    }
    
    printf("Testing auto-increment/decrement patterns...\n");
    
    /* Test 1: Simple array sum */
    int sum1 = sum_array_int(int_array, ARRAY_SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test 2: Restrict copy */
    copy_buffer_restrict(char_buffer2, char_buffer1, ARRAY_SIZE);
    printf("Buffer copy completed\n");
    
    /* Test 3: Find zero (won't find one) */
    int pos = find_first_zero(int_array, ARRAY_SIZE);
    printf("First zero at position: %d\n", pos);
    
    /* Test 4: Short sum */
    short sum2 = sum_shorts(short_array, ARRAY_SIZE);
    printf("Sum of shorts: %d\n", (int)sum2);
    
    /* Test 5: Fill alternating */
    int fill_buf[50];
    fill_alternating(fill_buf, 50, 1, 2);
    printf("Fill alternating completed\n");
    
    /* Test 6: Local buffer */
    int checksum = process_local_buffer();
    printf("Local buffer checksum: %d\n", checksum);
    
    /* Test 7: Double increment */
    int dest[ARRAY_SIZE];
    double_increment_pattern(dest, int_array, ARRAY_SIZE);
    printf("Double increment copy completed\n");
    
    /* Test 8: Every other sum */
    int sum3 = sum_every_other(int_array, ARRAY_SIZE);
    printf("Sum of every other: %d\n", sum3);
    
    /* Additional pattern in main itself */
    int *ptr = int_array;
    int main_sum = 0;
    for (int i = 0; i < 10; i++) {
        main_sum += *ptr;   /* Should generate (mem (reg)) */
        ptr = ptr + 1;      /* Separate increment */
    }
    printf("Main loop sum: %d\n", main_sum);
    
    /* Verify results */
    int expected_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += i * 2;
    }
    
    if (sum1 == expected_sum && memcmp(char_buffer1, char_buffer2, ARRAY_SIZE) == 0) {
        printf("All tests passed! Patterns should trigger auto-inc-dec optimization.\n");
        return 0;
    } else {
        printf("Test verification failed\n");
        return 1;
    }
}
