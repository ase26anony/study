#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SIZE 100
#define PATTERN_SIZE 20

/* Structure to test pointer post-increment with field access */
struct Data {
    int value;
    char id;
    float weight;
};

/* Function using post-increment in array indexing */
int sum_array_postinc(const int* arr, int n) {
    int sum = 0;
    const int* p = arr;
    const int* end = arr + n;
    
    /* Basic post-increment in loop - should generate auto-inc RTL */
    while (p < end) {
        sum += *p++;  /* Post-increment access */
    }
    
    return sum;
}

/* Function using post-increment with volatile */
int sum_volatile_postinc(volatile int* arr, int n) {
    int sum = 0;
    volatile int* p = arr;
    volatile int* end = arr + n;
    
    /* Mix of volatile and control flow */
    if (n > 0) {
        sum += *p++;  /* Post-increment on volatile */
    }
    
    for (; p < end; ) {
        /* Comma expression with post-increment */
        int val = (*p, p++, val);
        sum += val;
    }
    
    return sum;
}

/* String copy with post-increment (classic example) */
void copy_string_postinc(char* dest, const char* src) {
    /* Tight loop likely to generate auto-inc addressing */
    while ((*dest++ = *src++) != '\0') {
        /* Empty body - all work in condition */
    }
}

/* Function with nested loops and post-increment */
int find_pattern(const char* buffer, int size, char pattern) {
    const char* p = buffer;
    const char* end = buffer + size;
    int count = 0;
    
    /* Outer loop */
    for (; p < end; ) {
        /* Inner search with post-increment */
        const char* q = p;
        while (q < end && *q++ != pattern) {
            /* Search loop */
        }
        
        /* Post-increment in if statement */
        if (p < end - 1) {
            char current = *p++;
            if (current == pattern) {
                count++;
            }
        } else {
            p++;
        }
    }
    
    return count;
}

/* Function using structure pointers with post-increment */
float sum_struct_weights(struct Data* data, int n) {
    float total = 0.0f;
    struct Data* ptr = data;
    struct Data* end = data + n;
    
    /* Switch with fall-through cases */
    switch (n) {
        case 0:
            return 0.0f;
        case 1:
            total += ptr->weight;
            ptr++;
            break;
        default:
            /* Loop with structure field access and post-increment */
            while (ptr < end) {
                total += ptr->weight;  /* Access field */
                ptr++;  /* Post-increment after access */
            }
    }
    
    return total;
}

/* Function with multiple basic blocks and post-increment */
int process_buffer(volatile char* buf, int size, char sentinel) {
    volatile char* p = buf;
    int processed = 0;
    
    /* Complex control flow with post-increment */
    while (p < buf + size) {
        if (*p == sentinel) {
            /* Taken path with post-increment */
            char val = *p++;
            processed += (int)val;
            
            /* Nested if */
            if (val > 64) {
                p++;  /* Skip next */
            }
        } else {
            /* Not-taken path with post-increment */
            p++;
        }
        
        /* Another post-increment in loop body */
        if (processed % 2 == 0) {
            volatile char* temp = p;
            char dummy = *temp;
            p = temp + 1;  /* Simulated post-increment */
        }
    }
    
    return processed;
}

/* Function with array[0] access and pointer increment */
int sum_first_elements(int* arrays[], int num_arrays) {
    int sum = 0;
    
    for (int i = 0; i < num_arrays; i++) {
        /* Access with zero offset: arrays[i][0] */
        sum += arrays[i][0];
        
        /* Post-increment equivalent */
        int* ptr = arrays[i];
        sum += *ptr;  /* Same as ptr[0] */
        ptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Main function that exercises all patterns */
int main() {
    /* Non-volatile arrays */
    int array[SIZE];
    char buffer[SIZE];
    struct Data struct_array[PATTERN_SIZE];
    
    /* Volatile arrays */
    volatile int volatile_array[SIZE];
    volatile char volatile_buffer[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i;
        buffer[i] = 'A' + (i % 26);
        volatile_array[i] = i * 2;
        volatile_buffer[i] = 'a' + (i % 26);
    }
    
    for (int i = 0; i < PATTERN_SIZE; i++) {
        struct_array[i].value = i;
        struct_array[i].id = 'A' + i;
        struct_array[i].weight = i * 1.5f;
    }
    
    /* Test 1: Basic post-increment summation */
    int sum1 = sum_array_postinc(array, SIZE);
    printf("Sum of array (post-inc): %d\n", sum1);
    
    /* Test 2: Volatile array with post-increment */
    int sum2 = sum_volatile_postinc(volatile_array, SIZE);
    printf("Sum of volatile array: %d\n", sum2);
    
    /* Test 3: String copy with post-increment */
    char dest[SIZE];
    copy_string_postinc(dest, buffer);
    printf("Copied string: %s\n", dest);
    
    /* Test 4: Pattern finding with post-increment */
    int count = find_pattern(buffer, SIZE, 'C');
    printf("Found 'C' %d times\n", count);
    
    /* Test 5: Structure access with post-increment */
    float weight_sum = sum_struct_weights(struct_array, PATTERN_SIZE);
    printf("Total weight: %.2f\n", weight_sum);
    
    /* Test 6: Complex control flow with volatile */
    int processed = process_buffer(volatile_buffer, SIZE, 'c');
    printf("Processed volatile buffer: %d\n", processed);
    
    /* Test 7: Array of pointers with zero offset access */
    int* ptr_array[5];
    for (int i = 0; i < 5; i++) {
        ptr_array[i] = &array[i * 10];
    }
    int first_sum = sum_first_elements(ptr_array, 5);
    printf("Sum of first elements: %d\n", first_sum);
    
    /* Additional tight loops likely to generate auto-inc RTL */
    
    /* Byte-wise copy with post-increment */
    {
        volatile char* src = volatile_buffer;
        char local_buf[SIZE];
        char* dst = local_buf;
        volatile char* src_end = src + SIZE;
        
        while (src < src_end) {
            *dst++ = *src++;  /* Both post-increments */
        }
    }
    
    /* Search loop with post-increment */
    {
        const char* search_ptr = buffer;
        int found_at = -1;
        int index = 0;
        
        while (*search_ptr != '\0' && index < SIZE) {
            if (*search_ptr++ == 'Z') {  /* Post-increment in condition */
                found_at = index;
                break;
            }
            index++;
        }
        
        if (found_at >= 0) {
            printf("Found 'Z' at position %d\n", found_at);
        }
    }
    
    /* Loop with multiple post-increment operations */
    {
        int* p1 = array;
        int* p2 = &array[SIZE/2];
        int result = 0;
        
        for (int i = 0; i < SIZE/2; i++) {
            /* Multiple memory accesses with post-increment */
            result += *p1++ + *p2++;  /* Both post-increments */
        }
        
        printf("Combined sum: %d\n", result);
    }
    
    return 0;
}
