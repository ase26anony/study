#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SIZE 100
#define PATTERN_SIZE 20

/* Structure for testing pointer post-increment with field access */
struct Data {
    int value;
    int count;
    char id;
};

/* Function 1: Copy with post-increment in loop condition */
void copy_with_postinc(char *dest, const char *src) {
    while ((*dest++ = *src++) != '\0') {
        /* Empty body - post-increment in condition */
    }
}

/* Function 2: Summation with post-increment in update statement */
int sum_array_postinc(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    const int *end = arr + n;
    
    /* Post-increment in loop update */
    for (; p < end; sum += *p++) {
        /* Complex control flow inside */
        if (sum > 1000) {
            /* Another post-increment in conditional path */
            volatile int *vp = (volatile int *)p;
            int temp = *vp;
            vp++;
            p = (const int *)vp;
            continue;
        }
    }
    return sum;
}

/* Function 3: Search with post-increment in while condition */
int find_value_postinc(const int *arr, int n, int target) {
    const int *p = arr;
    const int *end = arr + n;
    
    /* Post-increment in while condition */
    while (p < end && *p++ != target) {
        /* Nested control flow */
        switch (target % 3) {
            case 0:
                /* Fall-through case with post-increment */
                if (*(p - 1) == target - 1) {
                    volatile int *vp = (volatile int *)(p - 1);
                    int check = *vp;
                    vp++;
                    /* Comma expression with post-increment */
                    return (check, p = (const int *)vp, -1);
                }
                /* FALLTHROUGH */
            case 1:
                /* Post-increment in switch case */
                if (p < end - 1) {
                    int temp = *p;
                    p++;
                    if (temp == target) return (p - arr - 1);
                }
                break;
            default:
                break;
        }
    }
    return (p <= end && *(p-1) == target) ? (p - arr - 1) : -1;
}

/* Function 4: Structure array processing with post-increment */
int process_structs_postinc(struct Data *data, int n) {
    int total = 0;
    struct Data *ptr = data;
    struct Data *end = data + n;
    
    /* Mixed volatile and non-volatile in same loop */
    volatile struct Data *vptr = (volatile struct Data *)data;
    
    while (ptr < end) {
        /* Access through non-volatile pointer */
        total += ptr->value;
        ptr++;
        
        /* Access through volatile pointer with post-increment */
        if (vptr < (volatile struct Data *)end) {
            /* Comma expression: access, increment, use result */
            int val = (vptr->value, vptr++, val);
            total += val;
        }
        
        /* Nested loop with post-increment */
        for (int i = 0; i < ptr->count && i < 5; i++) {
            volatile char *cptr = &ptr->id;
            char c = *cptr;
            cptr++;
            total += (int)c;
        }
    }
    return total;
}

/* Function 5: Byte buffer processing with multiple post-increment patterns */
void process_buffer_postinc(volatile uint8_t *buf, int size) {
    volatile uint8_t *p = buf;
    volatile uint8_t *end = buf + size;
    
    /* Multiple basic blocks with post-increment */
    if (size > 10) {
        /* Taken path with post-increment */
        uint8_t first = *p++;
        uint8_t second = *p++;
        
        /* Loop with post-increment in condition */
        while (p < end - 2 && *p++ != 0xFF) {
            /* Inner loop with pointer arithmetic */
            volatile uint8_t *inner = p;
            for (int i = 0; i < 3 && inner < end; i++) {
                uint8_t val = *inner;
                inner++;
                if (val == 0xAA) break;
            }
            p = inner;
        }
    } else {
        /* Not-taken path also with post-increment */
        while (p < end) {
            uint8_t val = *p;
            p++;
            if (val == 0) break;
        }
    }
}

/* Function 6: Array zeroing with post-increment */
void zero_array_postinc(int *arr, int n) {
    int *p = arr;
    int *end = arr + n;
    
    /* Simple post-increment in loop */
    while (p < end) {
        *p++ = 0;
    }
}

/* Function 7: Mixed qualifiers in complex expression */
int mixed_qualifiers_postinc(volatile int *varr, int *arr, int n) {
    int result = 0;
    volatile int *vp = varr;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Access volatile, increment, access non-volatile */
        int vval = *vp;
        vp++;
        
        int val = *p;
        p++;
        
        /* Comma expression combining both */
        result += (vval, val, vval + val);
        
        /* Conditional with post-increment */
        if (i % 2 == 0) {
            volatile int *temp_vp = vp;
            int temp = *temp_vp;
            temp_vp++;
            result += temp;
        } else {
            int *temp_p = p;
            int temp = *temp_p;
            temp_p++;
            result -= temp;
        }
    }
    return result;
}

int main() {
    /* Test data */
    char source[] = "Test string for post-increment copying";
    char destination[SIZE];
    
    int int_array[SIZE];
    volatile int volatile_array[SIZE];
    
    struct Data struct_array[PATTERN_SIZE];
    
    volatile uint8_t buffer[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i;
        volatile_array[i] = i * 2;
        buffer[i] = (uint8_t)(i % 256);
    }
    
    for (int i = 0; i < PATTERN_SIZE; i++) {
        struct_array[i].value = i * 3;
        struct_array[i].count = i % 5;
        struct_array[i].id = 'A' + (i % 26);
    }
    
    /* Test 1: String copy with post-increment */
    copy_with_postinc(destination, source);
    printf("Copy test: %s\n", destination);
    
    /* Test 2: Array summation with post-increment */
    int sum = sum_array_postinc(int_array, SIZE);
    printf("Sum of array: %d\n", sum);
    
    /* Test 3: Search with post-increment */
    int search_result = find_value_postinc(int_array, SIZE, 42);
    printf("Search for 42: found at index %d\n", search_result);
    
    /* Test 4: Structure processing with post-increment */
    int struct_total = process_structs_postinc(struct_array, PATTERN_SIZE);
    printf("Structure total: %d\n", struct_total);
    
    /* Test 5: Buffer processing with volatile post-increment */
    process_buffer_postinc(buffer, SIZE);
    printf("Buffer processed\n");
    
    /* Test 6: Array zeroing */
    int temp_array[SIZE];
    for (int i = 0; i < SIZE; i++) temp_array[i] = i;
    zero_array_postinc(temp_array, SIZE);
    printf("Array zeroed (first element: %d)\n", temp_array[0]);
    
    /* Test 7: Mixed qualifiers */
    int mixed_result = mixed_qualifiers_postinc(volatile_array, int_array, SIZE/2);
    printf("Mixed qualifiers result: %d\n", mixed_result);
    
    /* Additional complex pattern: nested loops with post-increment */
    {
        volatile int *vp = volatile_array;
        int *p = int_array;
        
        for (int i = 0; i < 10; i++) {
            /* Comma expression with post-increment */
            int a = (*vp, vp++, *vp);
            int b = (*p, p++, *p);
            
            /* Switch with post-increment in cases */
            switch (i % 3) {
                case 0:
                    vp++;
                    break;
                case 1:
                    p++;
                    break;
                case 2:
                    vp++;
                    p++;
                    break;
            }
        }
    }
    
    return 0;
}
