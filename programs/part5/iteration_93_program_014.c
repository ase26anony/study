/* auto-inc-dec-test.c
 * 
 * This program is designed to generate RTL patterns that trigger the
 * auto-increment/decrement optimization pass in GCC, specifically targeting
 * the uncovered lines in auto-inc-dec.cc (lines 1352-1358).
 * 
 * The code uses post-increment/decrement pointer arithmetic in various
 * contexts to create opportunities for auto-inc-dec optimization.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 100
#define BUFFER_SIZE 256

/* Structure for testing pointer post-increment with field access */
struct TestStruct {
    int value;
    int data;
    char id;
};

/* Function 1: Copy with post-increment pointers (classic strcpy-like loop) */
void copy_with_postinc(volatile char *dest, const char *src, int n) {
    volatile char *d = dest;
    const char *s = src;
    
    /* Tight loop with post-increment in assignment */
    while (n-- > 0) {
        *d++ = *s++;  /* This should generate post-increment addressing */
    }
}

/* Function 2: Summation with mixed volatile/non-volatile pointers */
int sum_with_postinc(volatile int *arr, int size) {
    volatile int *p = arr;
    int sum = 0;
    int *end = (int*)(arr + size);  /* Mixed pointer types */
    
    /* Loop with post-increment in the update expression */
    for (; p < end; sum += *p++) {
        /* Empty body - all work in loop header */
    }
    
    return sum;
}

/* Function 3: Search with post-increment in condition */
int find_value(volatile int *arr, int size, int target) {
    volatile int *p = arr;
    int count = 0;
    
    /* Post-increment in loop condition */
    while (count++ < size && *p++ != target) {
        /* Continue searching */
    }
    
    return (count <= size) ? (count - 1) : -1;
}

/* Function 4: Structure array traversal with post-increment */
int sum_struct_values(struct TestStruct *arr, int n) {
    struct TestStruct *ptr = arr;
    int total = 0;
    
    /* Access structure field with pointer post-increment */
    for (int i = 0; i < n; i++) {
        total += ptr->value;  /* Memory access */
        ptr++;  /* Post-increment in separate statement */
    }
    
    return total;
}

/* Function 5: Complex control flow with post-increment */
void process_buffer(volatile char *buf, int size, int mode) {
    volatile char *p = buf;
    int i = 0;
    
    /* Switch with different post-increment patterns */
    switch (mode) {
        case 0:
            /* Simple while with post-increment */
            while (i++ < size && *p++ != '\0') {
                /* Process until null terminator */
            }
            break;
            
        case 1:
            /* For loop with post-increment in update */
            for (i = 0; i < size; i++, p++) {
                volatile char temp = *p;  /* Dereference */
                /* Additional processing */
                *p = temp + 1;
            }
            break;
            
        case 2:
            /* Do-while with post-increment */
            do {
                *p = *p * 2;
                p++;
            } while (++i < size);
            break;
            
        default:
            /* Nested loops with post-increment */
            for (i = 0; i < size; i++) {
                volatile char *inner = p;
                int j = 10;
                while (j-- > 0) {
                    *inner++ = (char)i;  /* Post-increment in inner loop */
                }
                p += 10;
            }
            break;
    }
}

/* Function 6: Comma expression with post-increment */
int access_and_increment(volatile int *ptr) {
    int result;
    
    /* Comma expression: access then increment */
    result = (*ptr, ptr++, 0) ? 0 : *ptr;
    
    /* Another comma expression variant */
    return (result = *ptr, ptr++, result);
}

/* Function 7: String length with post-increment */
int string_length(const char *str) {
    const char *p = str;
    
    /* Classic strlen implementation */
    while (*p++ != '\0') {
        /* All work in condition */
    }
    
    return (int)(p - str - 1);
}

/* Function 8: Array initialization with post-increment */
void init_array(volatile int *arr, int size, int start) {
    volatile int *p = arr;
    int *end = (int*)(arr + size);
    
    /* Multiple basic blocks with conditional */
    if (start > 0) {
        for (; p < end; *p++ = start++) {
            /* Initialization in loop update */
        }
    } else {
        volatile int *q = arr;
        int count = size;
        while (count-- > 0) {
            *q++ = count;  /* Post-increment in loop body */
        }
    }
}

int main(void) {
    /* Test data arrays - mix volatile and non-volatile */
    volatile int volatile_array[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    volatile char buffer[BUFFER_SIZE];
    struct TestStruct struct_array[20];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    /* Initialize buffer with pattern */
    for (int i = 0; i < BUFFER_SIZE - 1; i++) {
        buffer[i] = (char)('A' + (i % 26));
    }
    buffer[BUFFER_SIZE - 1] = '\0';
    
    /* Initialize structure array */
    for (int i = 0; i < 20; i++) {
        struct_array[i].value = i * 10;
        struct_array[i].data = i * 20;
        struct_array[i].id = (char)('A' + i);
    }
    
    /* Test 1: Copy with post-increment */
    volatile char dest_buffer[BUFFER_SIZE];
    copy_with_postinc(dest_buffer, (const char*)buffer, 50);
    printf("Copy test: dest[0] = %c, dest[49] = %c\n", 
           dest_buffer[0], dest_buffer[49]);
    
    /* Test 2: Summation with post-increment */
    int sum1 = sum_with_postinc(volatile_array, ARRAY_SIZE);
    int sum2 = sum_with_postinc((volatile int*)regular_array, ARRAY_SIZE);
    printf("Sum test 1: %d, Sum test 2: %d\n", sum1, sum2);
    
    /* Test 3: Search with post-increment */
    int found_index = find_value(volatile_array, ARRAY_SIZE, 50);
    printf("Search test: value 50 found at index %d\n", found_index);
    
    /* Test 4: Structure traversal */
    int struct_sum = sum_struct_values(struct_array, 20);
    printf("Structure sum: %d\n", struct_sum);
    
    /* Test 5: Complex control flow */
    volatile char work_buffer[50];
    for (int i = 0; i < 50; i++) {
        work_buffer[i] = (char)i;
    }
    process_buffer(work_buffer, 50, 0);
    printf("Process buffer: first char = %d\n", (int)work_buffer[0]);
    
    /* Test 6: Comma expression */
    volatile int test_val = 42;
    int comma_result = access_and_increment(&test_val);
    printf("Comma expression test: %d\n", comma_result);
    
    /* Test 7: String length */
    const char *test_string = "Hello, auto-inc-dec!";
    int len = string_length(test_string);
    printf("String length: %d (expected: %zu)\n", len, strlen(test_string));
    
    /* Test 8: Array initialization */
    volatile int init_array_test[10];
    init_array(init_array_test, 10, 5);
    printf("Init array test: [0]=%d, [9]=%d\n", init_array_test[0], init_array_test[9]);
    
    /* Additional tight loop patterns */
    /* Pattern 1: Pointer difference calculation */
    {
        volatile int *p1 = volatile_array;
        volatile int *p2 = volatile_array + ARRAY_SIZE;
        int count = 0;
        while (p1 < p2) {
            if (*p++ > 25) {  /* Post-increment in condition */
                count++;
            }
        }
        printf("Count > 25: %d\n", count);
    }
    
    /* Pattern 2: Mixed pointer types in expression */
    {
        int *reg_ptr = regular_array;
        volatile int *vol_ptr = volatile_array;
        int mixed_sum = 0;
        
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Access both volatile and non-volatile with post-increment */
            mixed_sum += *reg_ptr++ + *vol_ptr++;
        }
        printf("Mixed pointer sum: %d\n", mixed_sum);
    }
    
    /* Pattern 3: Post-increment with zero offset (targeting specific lines) */
    {
        volatile int *ptr = &volatile_array[0];
        int temp;
        
        /* Direct dereference with following increment */
        temp = *ptr;  /* mem_insn.mem_loc = address_of_x, offset 0 */
        ptr++;        /* Should be combined into post-increment addressing */
        
        /* Array access with index 0 */
        temp = volatile_array[0];  /* Base + zero offset */
        
        printf("Zero offset tests completed\n");
    }
    
    return 0;
}
