#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SIZE 256
#define PATTERN_SIZE 16

/* Structure to test pointer post-increment with field access */
struct Data {
    int value;
    char id;
    float weight;
};

/* Function 1: Copy with post-increment in loop condition */
void copy_with_postinc(char *dest, const char *src, size_t n) {
    if (n == 0) return;
    
    /* Tight loop with post-increment in assignment */
    char *d = dest;
    const char *s = src;
    while (n-- > 0) {
        *d++ = *s++;  /* Should generate post-increment addressing */
    }
}

/* Function 2: Summation with mixed volatile/non-volatile */
int sum_with_postinc(volatile int *arr, int count) {
    int sum = 0;
    volatile int *p = arr;
    
    /* Loop with post-increment in update statement */
    for (int i = 0; i < count; i++) {
        sum += *p++;  /* Volatile pointer with post-increment */
    }
    
    return sum;
}

/* Function 3: Search with post-increment in condition */
int find_value(const int *arr, int size, int target) {
    const int *p = arr;
    const int *end = arr + size;
    
    /* Post-increment in loop condition */
    while (p < end && *p++ != target) {
        /* Empty body - increment happens in condition */
    }
    
    return (p - 1) - arr;  /* Return index of found element */
}

/* Function 4: Structure array processing */
float process_structs(struct Data *sarr, int count) {
    float total_weight = 0.0f;
    struct Data *ptr = sarr;
    
    /* Access structure field with pointer post-increment */
    for (int i = 0; i < count; i++) {
        total_weight += ptr->weight;  /* Field access */
        ptr++;  /* Post-increment in separate statement */
        
        /* Create basic block boundary */
        if (ptr->value < 0) {
            /* Another memory access in different basic block */
            volatile int *vptr = (volatile int *)&ptr->value;
            (*vptr)++;  /* Volatile access */
        }
    }
    
    return total_weight;
}

/* Function 5: Complex control flow with post-increment */
int complex_control_flow(volatile short *buffer, int size) {
    int result = 0;
    volatile short *p = buffer;
    
    for (int i = 0; i < size; i++) {
        /* Switch with fall-through cases */
        switch (i % 4) {
            case 0:
                result += *p++;  /* Post-increment in case 0 */
                break;
            case 1:
                /* Comma expression with post-increment */
                result += (int)(*p, p++, *p);  /* Access, increment, access new */
                break;
            case 2:
                /* Nested if with post-increment */
                if (*p > 100) {
                    result += *p++;
                } else {
                    p++;  /* Increment without use in else branch */
                }
                break;
            case 3:
                /* Fall-through with post-increment */
                result += *p++;
                /* Fall through to increment again */
                p++;
                break;
        }
    }
    
    return result;
}

/* Function 6: String processing with null terminator check */
int string_length(const char *str) {
    const char *p = str;
    
    /* Classic strlen-like loop with post-increment */
    while (*p++ != '\0') {
        /* Increment happens in condition */
    }
    
    return (int)(p - str - 1);
}

/* Function 7: Array reversal with post-increment/decrement */
void reverse_array(int *arr, int size) {
    int *start = arr;
    int *end = arr + size - 1;
    
    while (start < end) {
        /* Swap with post-increment/decrement */
        int temp = *start;
        *start++ = *end;  /* Post-increment */
        *end-- = temp;    /* Post-decrement */
    }
}

/* Function 8: Memory block comparison */
int compare_blocks(const void *a, const void *b, size_t n) {
    const unsigned char *pa = (const unsigned char *)a;
    const unsigned char *pb = (const unsigned char *)b;
    
    /* Compare with post-increment in loop */
    while (n-- > 0) {
        if (*pa++ != *pb++) {  /* Both pointers post-increment */
            return -1;
        }
    }
    
    return 0;
}

int main(void) {
    /* Test data arrays - mix volatile and non-volatile */
    volatile int volatile_arr[SIZE];
    int regular_arr[SIZE];
    char string_buf[SIZE];
    struct Data struct_arr[PATTERN_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        volatile_arr[i] = i * 2;
        regular_arr[i] = i * 3;
    }
    
    /* Initialize string */
    const char *test_string = "Hello, Auto-Inc/Dec!";
    strcpy(string_buf, test_string);
    
    /* Initialize structure array */
    for (int i = 0; i < PATTERN_SIZE; i++) {
        struct_arr[i].value = i;
        struct_arr[i].id = 'A' + (i % 26);
        struct_arr[i].weight = i * 1.5f;
    }
    
    /* Test 1: Copy with post-increment */
    char copy_buf[SIZE];
    copy_with_postinc(copy_buf, string_buf, strlen(test_string) + 1);
    printf("Copy test: %s\n", copy_buf);
    
    /* Test 2: Summation with volatile */
    int sum1 = sum_with_postinc(volatile_arr, SIZE);
    printf("Volatile sum: %d\n", sum1);
    
    /* Test 3: Search with post-increment */
    int index = find_value(regular_arr, SIZE, 150);
    printf("Found 150 at index: %d\n", index);
    
    /* Test 4: Structure processing */
    float weight_sum = process_structs(struct_arr, PATTERN_SIZE);
    printf("Total weight: %.2f\n", weight_sum);
    
    /* Test 5: Complex control flow */
    volatile short short_buffer[100];
    for (int i = 0; i < 100; i++) {
        short_buffer[i] = i * 10;
    }
    int complex_result = complex_control_flow(short_buffer, 100);
    printf("Complex result: %d\n", complex_result);
    
    /* Test 6: String length */
    int len = string_length(test_string);
    printf("String length: %d\n", len);
    
    /* Test 7: Array reversal */
    int rev_arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    reverse_array(rev_arr, 10);
    printf("Reversed array first element: %d\n", rev_arr[0]);
    
    /* Test 8: Memory comparison */
    int cmp_result = compare_blocks(string_buf, copy_buf, strlen(test_string));
    printf("Comparison result: %d\n", cmp_result);
    
    /* Additional tight loops likely to generate auto-inc RTL */
    
    /* Loop with pointer arithmetic and zero offset */
    {
        int *ptr = regular_arr;
        int local_sum = 0;
        
        /* Direct dereference with post-increment */
        for (int i = 0; i < 10; i++) {
            local_sum += *ptr++;  /* Base + zero offset */
        }
        printf("Direct deref sum: %d\n", local_sum);
    }
    
    /* Nested loop with inner post-increment */
    {
        int matrix[5][5];
        int *flat = &matrix[0][0];
        
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                *flat++ = i * 5 + j;  /* Post-increment in nested loop */
            }
        }
    }
    
    /* Comma expression test */
    {
        volatile int *vptr = volatile_arr;
        int temp;
        
        /* Comma expression sequencing access and increment */
        temp = (*vptr, vptr++, *vptr);
        printf("Comma expr test: %d\n", temp);
    }
    
    return 0;
}
