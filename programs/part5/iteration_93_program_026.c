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

#define ARRAY_SIZE 256
#define BUFFER_SIZE 128

/* Structure to enable pointer arithmetic with post-increment */
struct DataRecord {
    int id;
    volatile int value;
    char tag;
};

/* Function 1: Copy with post-increment in loop condition */
void copy_with_postinc(char *dest, const char *src, size_t n) {
    /* Classic strcpy-like loop with post-increment */
    while (n-- && (*dest++ = *src++) != '\0') {
        /* Empty body - all work in condition */
    }
}

/* Function 2: Summation with mixed volatile/non-volatile pointers */
int sum_with_postinc(volatile int *arr, int count) {
    int sum = 0;
    volatile int *p = arr;
    
    /* Post-increment in loop update */
    for (int i = 0; i < count; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Function 3: Search with post-increment in while condition */
int find_value(const int *arr, int size, int target) {
    const int *p = arr;
    const int *end = arr + size;
    
    /* Post-increment in loop condition */
    while (p < end && *p++ != target) {
        /* Continue searching */
    }
    
    return (p - 1) - arr;  /* Return index where found or size */
}

/* Function 4: Structure traversal with post-increment */
int process_structs(struct DataRecord *records, int count) {
    struct DataRecord *sptr = records;
    int total = 0;
    
    /* Access structure field with pointer increment */
    for (int i = 0; i < count; i++) {
        total += sptr->value;  /* Memory access */
        sptr++;                /* Post-increment equivalent */
    }
    
    return total;
}

/* Function 5: Complex control flow with post-increment */
void mixed_control_flow(volatile int *data, int *results, int size) {
    volatile int *src = data;
    int *dst = results;
    int i = 0;
    
    /* Nested loops with post-increment */
    while (i < size) {
        /* Switch with fall-through cases */
        switch (i % 3) {
            case 0:
                /* Post-increment in taken path */
                *dst++ = *src++ * 2;
                break;
            case 1:
                /* Comma expression with post-increment */
                *dst = (*src++, *src);  /* Access, increment, access again */
                dst++;
                src++;  /* Additional increment */
                break;
            case 2:
                /* Post-increment in both paths */
                if (*src > 100) {
                    *dst++ = *src++ / 2;
                } else {
                    *dst++ = *src++ * 3;
                }
                break;
        }
        i++;
    }
}

/* Function 6: Byte-wise copy with post-increment (tight loop) */
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

/* Function 7: Array initialization with post-increment in for loop */
void init_array(int *arr, int size, int start) {
    int *p = arr;
    
    /* Post-increment in loop update statement */
    for (int i = 0; i < size; i++) {
        *p++ = start + i;
    }
}

int main(void) {
    /* Test data arrays - mix volatile and non-volatile */
    volatile int volatile_array[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    char source_buffer[BUFFER_SIZE];
    char dest_buffer[BUFFER_SIZE];
    struct DataRecord records[50];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    /* Initialize string data */
    const char *test_string = "Auto-increment/decrement test string";
    strcpy(source_buffer, test_string);
    
    /* Initialize structure array */
    for (int i = 0; i < 50; i++) {
        records[i].id = i;
        records[i].value = i * 10;
        records[i].tag = 'A' + (i % 26);
    }
    
    printf("=== Auto-Inc/Dec Test Program ===\n");
    
    /* Test 1: String copy with post-increment */
    copy_with_postinc(dest_buffer, source_buffer, BUFFER_SIZE);
    printf("Copy test: %s\n", 
           strcmp(source_buffer, dest_buffer) == 0 ? "PASS" : "FAIL");
    
    /* Test 2: Summation with volatile pointer */
    int sum1 = sum_with_postinc(volatile_array, ARRAY_SIZE);
    int expected_sum1 = (ARRAY_SIZE - 1) * ARRAY_SIZE;  /* Sum of 0..(n-1)*2 */
    printf("Volatile sum: %d (expected: %d) %s\n", 
           sum1, expected_sum1, 
           sum1 == expected_sum1 ? "PASS" : "FAIL");
    
    /* Test 3: Search with post-increment */
    int search_target = 150;
    int found_index = find_value(regular_array, ARRAY_SIZE, search_target);
    printf("Search for %d: found at index %d\n", search_target, found_index);
    
    /* Test 4: Structure traversal */
    int struct_sum = process_structs(records, 50);
    int expected_struct_sum = (49 * 50 / 2) * 10;  /* Sum of 0..49 * 10 */
    printf("Structure sum: %d (expected: %d) %s\n",
           struct_sum, expected_struct_sum,
           struct_sum == expected_struct_sum ? "PASS" : "FAIL");
    
    /* Test 5: Mixed control flow */
    int results[ARRAY_SIZE];
    mixed_control_flow(volatile_array, results, 20);
    printf("Mixed control flow: computed %d results\n", 20);
    
    /* Test 6: Byte copy with volatile pointers */
    volatile unsigned char src_bytes[100];
    volatile unsigned char dst_bytes[100];
    for (int i = 0; i < 100; i++) {
        src_bytes[i] = i;
    }
    byte_copy(dst_bytes, src_bytes, 100);
    printf("Byte copy: %s\n",
           memcmp((void*)src_bytes, (void*)dst_bytes, 100) == 0 ? "PASS" : "FAIL");
    
    /* Test 7: Array initialization with post-increment */
    int new_array[100];
    init_array(new_array, 100, 1000);
    printf("Array init: first=%d, last=%d\n", new_array[0], new_array[99]);
    
    /* Additional tight loop patterns that might trigger auto-inc */
    {
        /* Pointer arithmetic in for loop */
        int *p = regular_array;
        int *end = regular_array + ARRAY_SIZE;
        int checksum = 0;
        
        /* Post-increment in loop condition */
        while (p < end) {
            checksum ^= *p++;  /* XOR checksum with post-increment */
        }
        printf("Checksum: %08x\n", checksum);
    }
    
    {
        /* Comma expression with post-increment */
        volatile int *vp = volatile_array;
        int temp;
        
        /* Sequence: access, increment, use result */
        temp = (*vp++, *vp);  /* Gets second element after increment */
        printf("Comma expression result: %d\n", temp);
    }
    
    printf("=== All tests completed ===\n");
    
    return 0;
}
