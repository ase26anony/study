/* auto-inc-dec-test.c
 * 
 * This program is designed to generate RTL patterns that trigger the
 * auto-increment/decrement optimization in GCC's RTL passes, specifically
 * targeting the uncovered lines in auto-inc-dec.cc (lines 1352-1358).
 * 
 * The code uses post-increment/decrement pointer arithmetic in various
 * contexts to create opportunities for auto-inc-dec addressing modes.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* ========== Function 1: Copy with post-increment in loop ========== */
/* This creates a tight byte-copy loop that should generate 
 * post-increment memory operations in RTL */
void copy_with_postinc(char *dest, const char *src, size_t n) {
    /* Classic K&R style copy - likely to generate auto-inc patterns */
    while (n-- > 0) {
        *dest++ = *src++;
    }
}

/* ========== Function 2: Summation with mixed volatile/non-volatile ========== */
int sum_with_postinc(volatile int *arr, int count) {
    int sum = 0;
    volatile int *p = arr;
    
    /* Loop with post-increment in the update expression */
    for (int i = 0; i < count; i++) {
        sum += *p++;
    }
    
    /* Another loop with pointer arithmetic in condition */
    int *non_volatile_ptr = (int*)arr;
    int verify_sum = 0;
    while (count-- > 0) {
        verify_sum += *non_volatile_ptr++;
    }
    
    /* Use comma expression to sequence access and increment */
    p = arr;
    int temp_sum = 0;
    for (int i = 0; i < 5; i++) {
        temp_sum += (temp = *p, p++, temp);  /* comma expression */
    }
    
    return sum + verify_sum + temp_sum;
}

/* ========== Function 3: String operations with post-increment ========== */
size_t strlen_with_postinc(const char *str) {
    const char *p = str;
    while (*p++ != '\0') {
        /* Empty body - post-increment in condition */
    }
    return p - str - 1;
}

int strcmp_with_postinc(const char *s1, const char *s2) {
    /* Compare with post-increment in loop */
    while (*s1 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }
    
    /* Post-increment in return expression */
    return (*(const unsigned char *)s1 - *(const unsigned char *)s2);
}

/* ========== Function 4: Structure access with pointer increment ========== */
struct Data {
    int value;
    volatile int timestamp;
    char id[8];
};

int process_struct_array(struct Data *arr, int n) {
    int total = 0;
    struct Data *ptr = arr;
    
    /* Access structure fields with post-increment */
    for (int i = 0; i < n; i++) {
        total += ptr->value;  /* Base address with zero offset */
        ptr->timestamp = i;   /* Volatile access */
        ptr++;  /* Post-increment after access */
    }
    
    /* Nested loop with inner post-increment */
    ptr = arr;
    for (int i = 0; i < n; i++) {
        /* Access with zero offset followed by increment */
        int val = ptr->value;
        ptr++;
        
        /* Inner loop on id array */
        char *id_ptr = ptr->id;
        while (*id_ptr++ != '\0') {
            /* Process ID characters */
        }
    }
    
    return total;
}

/* ========== Function 5: Complex control flow with auto-inc ========== */
int search_with_postinc(volatile int *array, int size, int target) {
    volatile int *p = array;
    int found = -1;
    
    /* Post-increment in loop with conditional break */
    for (int i = 0; i < size; i++) {
        if (*p++ == target) {  /* Post-increment in condition */
            found = i;
            break;
        }
    }
    
    /* Switch statement with post-increment */
    p = array;
    int category = 0;
    switch (target % 3) {
        case 0:
            category = *p++;  /* Post-increment in switch case */
            break;
        case 1:
            p++;  /* Increment pointer */
            category = *p;   /* Access with zero offset */
            break;
        default:
            category = (temp = *p, p++, temp);  /* Comma expression */
            break;
    }
    
    return found + category;
}

/* ========== Function 6: Array initialization with post-inc ========== */
void init_array_with_postinc(int *arr, int value, int count) {
    int *p = arr;
    
    /* Different loop styles to generate various RTL patterns */
    
    /* Style 1: While loop with post-increment */
    int n = count;
    while (n-- > 0) {
        *p++ = value;
    }
    
    /* Style 2: For loop with pointer comparison */
    p = arr;
    int *end = arr + count;
    while (p < end) {
        *p++ = value + 1;
    }
    
    /* Style 3: Do-while with post-increment */
    p = arr;
    int i = 0;
    do {
        *p++ = value + i++;
    } while (i < count);
}

/* ========== Main function ========== */
int main(void) {
    /* Test data - mix volatile and non-volatile */
    volatile int volatile_array[100];
    int regular_array[100];
    char string_buffer[256];
    struct Data struct_array[20];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    for (int i = 0; i < 20; i++) {
        struct_array[i].value = i * 10;
        struct_array[i].timestamp = 0;
        snprintf(struct_array[i].id, sizeof(struct_array[i].id), "ID%d", i);
    }
    
    strcpy(string_buffer, "Test string for pointer operations");
    
    /* Execute functions to generate RTL patterns */
    
    /* 1. Copy operations */
    char dest[256];
    copy_with_postinc(dest, string_buffer, strlen(string_buffer) + 1);
    printf("Copy result: %s\n", dest);
    
    /* 2. Summation with mixed qualifiers */
    int sum1 = sum_with_postinc(volatile_array, 50);
    printf("Sum 1: %d\n", sum1);
    
    int sum2 = sum_with_postinc((volatile int*)regular_array, 50);
    printf("Sum 2: %d\n", sum2);
    
    /* 3. String operations */
    size_t len = strlen_with_postinc(string_buffer);
    printf("String length: %zu\n", len);
    
    int cmp = strcmp_with_postinc(string_buffer, "Test string");
    printf("String compare: %d\n", cmp);
    
    /* 4. Structure processing */
    int struct_total = process_struct_array(struct_array, 20);
    printf("Structure total: %d\n", struct_total);
    
    /* 5. Search with complex control flow */
    int found = search_with_postinc(volatile_array, 100, 42);
    printf("Search result: %d\n", found);
    
    /* 6. Array initialization */
    init_array_with_postinc(regular_array, 999, 100);
    printf("Array initialized\n");
    
    /* Verify some results */
    int verify_sum = 0;
    int *p = regular_array;
    for (int i = 0; i < 100; i++) {
        verify_sum += *p++;
    }
    printf("Verification sum: %d\n", verify_sum);
    
    return 0;
}
