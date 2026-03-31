#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 100
#define BUFFER_SIZE 50

/* Structure for testing pointer post-increment with field access */
struct Data {
    int value;
    char id;
    float weight;
};

/* Function 1: Copy with post-increment in loop condition */
void copy_with_postinc(char *dest, const char *src) {
    while ((*dest++ = *src++) != '\0') {
        /* Empty body - post-increment in condition */
    }
}

/* Function 2: Summation with post-increment in update statement */
int sum_array(const int *arr, int size) {
    int sum = 0;
    const int *p = arr;
    const int *end = arr + size;
    
    /* Post-increment in loop update */
    for (; p < end; sum += *p++) {
        /* Complex control flow inside */
        if (sum > 1000) {
            /* Post-increment in conditional path */
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
int find_value(volatile int *arr, int size, int target) {
    volatile int *p = arr;
    volatile int *end = arr + size;
    
    /* Post-increment in while condition */
    while (p < end && *p++ != target) {
        /* Nested control flow */
        switch (target) {
            case 0:
                /* Fall-through case with post-increment */
                if (*(p - 1) == -1) {
                    volatile int *temp = p;
                    int val = *temp;
                    temp++;
                    p = temp;
                }
                /* Fall through */
            case 1:
                /* Post-increment in comma expression */
                (val = *(p - 1), p++, val);
                break;
            default:
                break;
        }
    }
    return (p - 1) - arr;
}

/* Function 4: Structure array processing with post-increment */
float process_structures(struct Data *sptr, int count) {
    float total_weight = 0.0f;
    struct Data *end = sptr + count;
    
    /* Post-increment accessing structure fields */
    while (sptr < end) {
        total_weight += sptr->weight;
        
        /* Post-increment in comma expression */
        int val = sptr->value;
        sptr++;
        
        /* Complex control flow */
        if (val < 0) {
            volatile struct Data *vs = (volatile struct Data *)sptr;
            float w = vs->weight;
            vs++;
            sptr = (struct Data *)vs;
        }
    }
    return total_weight;
}

/* Function 5: Mixed volatile/non-volatile pointers */
void mixed_pointers_operation(volatile int *varr, int *arr, int size) {
    volatile int *vp = varr;
    int *p = arr;
    int *end = arr + size;
    
    /* Loop with post-increment on both pointer types */
    for (int i = 0; i < size; i++) {
        /* Access volatile, increment after */
        int temp = *vp;
        vp++;
        
        /* Access non-volatile, increment after */
        *p = temp * 2;
        p++;
        
        /* Alternative: post-increment in array access */
        if (i % 2 == 0) {
            arr[i] = varr[0];  /* Offset zero access */
        }
    }
}

/* Function 6: String operations with post-increment */
int string_length(const char *str) {
    const char *p = str;
    
    /* Tight loop with post-increment */
    while (*p++ != '\0') {
        /* Empty - post-increment in condition */
    }
    
    return p - str - 1;
}

/* Function 7: Byte buffer copy with post-increment */
void copy_buffer(volatile char *dest, const char *src, int size) {
    volatile char *d = dest;
    const char *s = src;
    const char *end = src + size;
    
    /* Multiple basic blocks with post-increment */
    if (size > 0) {
        /* First element special case */
        *d++ = *s++;
        
        /* Loop for rest */
        while (s < end) {
            /* Nested if inside loop */
            if (*s != 0) {
                *d = *s;
                d++;
                s++;
            } else {
                /* Alternative post-increment pattern */
                char c = *s;
                s++;
                *d = c;
                d++;
            }
        }
    }
}

int main() {
    /* Test data arrays - mix volatile and non-volatile */
    volatile int vdata[ARRAY_SIZE];
    int data[ARRAY_SIZE];
    char buffer[BUFFER_SIZE];
    volatile char vbuffer[BUFFER_SIZE];
    struct Data structures[20];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        vdata[i] = i % 10;
        data[i] = i;
    }
    
    for (int i = 0; i < BUFFER_SIZE - 1; i++) {
        buffer[i] = 'A' + (i % 26);
        vbuffer[i] = 'a' + (i % 26);
    }
    buffer[BUFFER_SIZE - 1] = '\0';
    vbuffer[BUFFER_SIZE - 1] = '\0';
    
    /* Initialize structures */
    for (int i = 0; i < 20; i++) {
        structures[i].value = i - 10;
        structures[i].id = 'A' + i;
        structures[i].weight = i * 1.5f;
    }
    
    /* Test 1: String copy with post-increment */
    char dest[BUFFER_SIZE];
    copy_with_postinc(dest, buffer);
    printf("Copy test: %s\n", dest);
    
    /* Test 2: Array sum with post-increment */
    int sum = sum_array(data, ARRAY_SIZE);
    printf("Sum test: %d\n", sum);
    
    /* Test 3: Search with volatile and post-increment */
    int index = find_value(vdata, ARRAY_SIZE, 5);
    printf("Search test: found at index %d\n", index);
    
    /* Test 4: Structure processing */
    float total_weight = process_structures(structures, 20);
    printf("Structure test: total weight = %.2f\n", total_weight);
    
    /* Test 5: Mixed pointers */
    mixed_pointers_operation(vdata, data, ARRAY_SIZE);
    
    /* Test 6: String length */
    int len = string_length(buffer);
    printf("String length: %d\n", len);
    
    /* Test 7: Buffer copy */
    copy_buffer(vbuffer, buffer, BUFFER_SIZE - 1);
    
    /* Verify results */
    printf("Verification:\n");
    printf("  dest[0] = %c\n", dest[0]);
    printf("  data[10] = %d\n", data[10]);
    printf("  vbuffer[5] = %c\n", (char)vbuffer[5]);
    
    return 0;
}
