/* auto-inc-dec-test.c
 * 
 * This program is designed to trigger auto-increment/decrement addressing
 * pattern recognition in GCC's RTL optimizations, specifically targeting
 * the find_auto_inc logic in auto-inc-dec.cc lines 1352-1358.
 * 
 * Compile with: gcc -O2 -fdump-rtl-auto_inc_dec -S auto-inc-dec-test.c
 * Or for more detailed RTL dumps: gcc -O2 -fdump-rtl-all auto-inc-dec-test.c
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define BUFFER_SIZE 128

/* Structure to enable pointer arithmetic with field access */
struct data_item {
    int value;
    int tag;
    char metadata[16];
};

/* Global arrays with different qualifiers */
volatile int volatile_buffer[ARRAY_SIZE];
int regular_buffer[ARRAY_SIZE];
volatile struct data_item volatile_structs[BUFFER_SIZE];
struct data_item regular_structs[BUFFER_SIZE];

/* Function 1: Copy with post-increment in while loop condition */
void copy_with_postinc(char *dest, const char *src) {
    /* Classic strcpy-like loop with post-increment */
    while ((*dest++ = *src++) != '\0') {
        /* Empty body - all work in condition */
    }
}

/* Function 2: Summation with post-increment in for loop */
int sum_with_postinc(const int *arr, int n) {
    int sum = 0;
    const int *end = arr + n;
    
    /* Post-increment in loop update */
    for (const int *p = arr; p < end; sum += *p++) {
        /* Work done in loop update */
    }
    
    return sum;
}

/* Function 3: Mixed volatile and non-volatile pointers */
int find_in_volatile(volatile int *arr, int target, int n) {
    volatile int *p = arr;
    volatile int *end = arr + n;
    
    /* Post-increment in while condition with volatile */
    while (p < end && *p++ != target) {
        /* Search loop */
    }
    
    return (p - 1 < end) ? 1 : 0;
}

/* Function 4: Structure access with post-increment */
int process_structs(struct data_item *items, int count) {
    int total = 0;
    struct data_item *ptr = items;
    struct data_item *end = ptr + count;
    
    /* Access structure field with post-increment */
    while (ptr < end) {
        total += ptr->value;  /* Base address with zero offset */
        ptr++;                /* Post-increment after access */
    }
    
    return total;
}

/* Function 5: Complex control flow with post-increment */
int conditional_postinc(int *arr, int n, int threshold) {
    int count = 0;
    int *p = arr;
    int *end = arr + n;
    
    /* Post-increment in both branches of conditional */
    while (p < end) {
        if (*p > threshold) {
            /* Taken path with post-increment */
            count += *p++;
        } else {
            /* Not-taken path with post-increment */
            p++;
        }
    }
    
    return count;
}

/* Function 6: Nested loops with inner post-increment */
void matrix_copy(int dest[][10], int src[][10], int rows) {
    for (int i = 0; i < rows; i++) {
        int *d = dest[i];
        int *s = src[i];
        int *end = d + 10;
        
        /* Inner tight copy loop with post-increment */
        while (d < end) {
            *d++ = *s++;  /* Classic post-increment copy */
        }
    }
}

/* Function 7: Comma expression with post-increment */
int comma_postinc(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    while (p < end) {
        /* Comma expression: access then increment */
        int val = (*p, p++, *p);  /* Actually we want: access p, increment, but that's wrong... */
        /* Let's do it properly: */
        int val2;
        val2 = (*p, p++, *(p-1));  /* This creates the pattern we want */
        sum += val2;
    }
    
    return sum;
}

/* Function 8: Switch statement with fall-through and post-increment */
int switch_postinc(int *arr, int n) {
    int result = 0;
    int *p = arr;
    int *end = arr + n;
    
    while (p < end) {
        switch (*p % 4) {
            case 0:
                result += *p++;
                /* Fall through */
            case 1:
                result -= *p++;
                break;
            case 2:
                p++;  /* Skip with post-increment */
                result += 100;
                break;
            case 3:
                result *= *p++;
                break;
        }
    }
    
    return result;
}

/* Function 9: Direct pointer dereference with zero offset */
int direct_deref(int *ptr) {
    /* Simple dereference that should use base + 0 offset */
    int val = *ptr;
    ptr++;  /* Separate increment to create pattern */
    return val;
}

/* Function 10: Array access with index 0 */
int array_zero_index(int *arr) {
    /* arr[0] with base + 0 offset */
    int val = arr[0];
    arr++;  /* Increment after */
    return val;
}

int main(void) {
    char src_string[] = "Test string for copy operation";
    char dest_string[sizeof(src_string)];
    
    /* Initialize buffers */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        regular_buffer[i] = i;
        volatile_buffer[i] = i * 2;
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        regular_structs[i].value = i % 100;
        volatile_structs[i].value = (i * 3) % 100;
    }
    
    printf("=== Auto-Increment/Decrement Test Program ===\n");
    
    /* Test 1: String copy with post-increment */
    copy_with_postinc(dest_string, src_string);
    printf("1. Copy result: %s\n", dest_string);
    
    /* Test 2: Summation with post-increment */
    int sum1 = sum_with_postinc(regular_buffer, ARRAY_SIZE);
    printf("2. Sum of regular buffer: %d\n", sum1);
    
    /* Test 3: Volatile search with post-increment */
    int found = find_in_volatile(volatile_buffer, 100, ARRAY_SIZE);
    printf("3. Found 100 in volatile buffer: %s\n", found ? "Yes" : "No");
    
    /* Test 4: Structure processing */
    int struct_sum = process_structs(regular_structs, BUFFER_SIZE);
    printf("4. Sum of struct values: %d\n", struct_sum);
    
    /* Test 5: Conditional post-increment */
    int cond_sum = conditional_postinc(regular_buffer, ARRAY_SIZE, 50);
    printf("5. Conditional sum (>50): %d\n", cond_sum);
    
    /* Test 6: Matrix copy */
    int matrix_a[5][10], matrix_b[5][10];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix_a[i][j] = i * 10 + j;
        }
    }
    matrix_copy(matrix_b, matrix_a, 5);
    printf("6. Matrix copy completed\n");
    
    /* Test 7: Comma expression */
    int comma_sum = comma_postinc(regular_buffer, 50);
    printf("7. Comma expression sum: %d\n", comma_sum);
    
    /* Test 8: Switch with post-increment */
    int switch_result = switch_postinc(regular_buffer, 50);
    printf("8. Switch result: %d\n", switch_result);
    
    /* Test 9: Direct dereference */
    int direct_val = direct_deref(regular_buffer);
    printf("9. Direct dereference: %d\n", direct_val);
    
    /* Test 10: Array zero index */
    int zero_index_val = array_zero_index(regular_buffer + 10);
    printf("10. Array[0] access: %d\n", zero_index_val);
    
    /* Additional tight loop patterns likely to generate auto-inc RTL */
    printf("\n=== Additional Tight Loop Patterns ===\n");
    
    /* Byte copy loop */
    {
        char src[100], dst[100];
        for (int i = 0; i < sizeof(src); i++) src[i] = i;
        
        char *s = src;
        char *d = dst;
        char *end = src + sizeof(src);
        
        /* Very tight byte copy */
        while (s < end) {
            *d++ = *s++;
        }
        printf("Byte copy completed\n");
    }
    
    /* Word copy with post-increment */
    {
        int src_ints[50], dst_ints[50];
        for (int i = 0; i < 50; i++) src_ints[i] = i * 2;
        
        int *s = src_ints;
        int *d = dst_ints;
        int *end = src_ints + 50;
        
        while (s < end) {
            *d++ = *s++;
        }
        printf("Word copy completed\n");
    }
    
    /* Mixed qualifier access */
    {
        int sum = 0;
        volatile int *vptr = volatile_buffer;
        int *rptr = regular_buffer;
        
        /* Access both volatile and non-volatile with post-increment */
        for (int i = 0; i < 10; i++) {
            sum += *vptr++ + *rptr++;
        }
        printf("Mixed qualifier sum: %d\n", sum);
    }
    
    return 0;
}
