#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SIZE 256
#define PATTERN_SIZE 32

/* Structure to test pointer post-increment with field access */
struct Data {
    int value;
    int count;
    char id;
};

/* Function 1: Copy with post-increment in loop condition */
void copy_with_postinc(char *dest, const char *src) {
    // Classic strcpy-like loop with post-increment
    while ((*dest++ = *src++) != '\0') {
        // Empty body - all work in condition
    }
}

/* Function 2: Summation with mixed volatile/non-volatile */
int sum_with_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *p = arr;
    volatile int *end = arr + n;
    
    // Post-increment in loop update
    for (; p < end; sum += *p++) {
        // Control flow inside loop
        if (sum > 1000) {
            // Post-increment in conditional path
            volatile int *q = p;
            sum += *q++;
            p = q;
        }
    }
    return sum;
}

/* Function 3: Search with post-increment in while condition */
int* find_value(int *arr, int n, int target) {
    int *p = arr;
    int *end = arr + n;
    
    // Post-increment in while condition
    while (p < end && *p++ != target) {
        // Nested control flow
        if (p - arr > n/2) {
            // Another post-increment in if body
            int temp = *p++;
            if (temp == target * 2) {
                return p - 1;
            }
        }
    }
    return (p <= end && *(p-1) == target) ? p-1 : NULL;
}

/* Function 4: Structure array processing */
void process_structs(struct Data *sarr, int n) {
    struct Data *ptr = sarr;
    struct Data *end = sarr + n;
    
    // Post-increment accessing structure fields
    for (; ptr < end; ptr++) {
        // Multiple post-increments in same basic block
        int val1 = ptr->value;
        char id1 = ptr->id;
        
        // Comma expression with post-increment
        int val2 = (ptr->count, ptr->value, ptr++->value);
        ptr--; // Reset for consistency
        
        // In conditional
        if (val1 > 100) {
            struct Data *temp = ptr;
            int x = temp++->value;
            ptr = temp;
        }
    }
}

/* Function 5: Byte buffer copy with switch */
void copy_buffers(volatile char *dest, volatile char *src, int mode) {
    volatile char *d = dest;
    volatile char *s = src;
    int i = 0;
    
    switch (mode) {
        case 0:
            // Post-increment in switch case
            while (i < SIZE) {
                *d++ = *s++;
                i++;
            }
            break;
            
        case 1:
            // Post-increment in for loop
            for (i = 0; i < SIZE; i++) {
                char c = *s++;
                *d++ = c;
            }
            break;
            
        case 2:
            // Do-while with post-increment
            do {
                *d = *s;
                d++;
                s++;
                i++;
            } while (i < SIZE);
            break;
            
        default:
            // Comma expression sequence
            while (i < SIZE) {
                (*d = *s, d++, s++, i++);
            }
    }
}

/* Function 6: Mixed pointer types in complex expression */
int complex_postinc(volatile int *varr, int *arr, int n) {
    int result = 0;
    volatile int *vp = varr;
    int *p = arr;
    
    // Multiple post-increments in same expression
    for (int i = 0; i < n; i++) {
        // Access with zero offset (arr[0] pattern)
        result += *vp++ + *p++;
        
        // Nested loop with post-increment
        for (int j = 0; j < 2; j++) {
            volatile int *inner = vp - 1;
            result += *inner++;
        }
    }
    return result;
}

/* Function 7: String operations with post-increment */
int string_ops(const char *str) {
    const char *p = str;
    int len = 0;
    
    // Classic strlen-like loop
    while (*p++ != '\0') {
        len++;
    }
    
    // Reset and process
    p = str;
    int sum = 0;
    while (*p) {
        // Post-increment in conditional expression
        if (*p++ == 'A') {
            sum += 10;
        }
    }
    
    return len + sum;
}

int main() {
    // Test data - mix volatile and non-volatile
    volatile int vbuffer[SIZE];
    int buffer[SIZE];
    volatile char vstr[PATTERN_SIZE];
    char str[PATTERN_SIZE];
    struct Data sdata[16];
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        vbuffer[i] = i % 100;
        buffer[i] = i % 100;
    }
    
    for (int i = 0; i < PATTERN_SIZE - 1; i++) {
        vstr[i] = 'A' + (i % 26);
        str[i] = 'A' + (i % 26);
    }
    vstr[PATTERN_SIZE - 1] = '\0';
    str[PATTERN_SIZE - 1] = '\0';
    
    for (int i = 0; i < 16; i++) {
        sdata[i].value = i * 10;
        sdata[i].count = i;
        sdata[i].id = 'A' + i;
    }
    
    // Test 1: String copy with post-increment
    char dest[PATTERN_SIZE];
    copy_with_postinc(dest, str);
    printf("Copy test: %s\n", dest);
    
    // Test 2: Summation with volatile
    int sum1 = sum_with_postinc(vbuffer, SIZE);
    printf("Sum with volatile: %d\n", sum1);
    
    // Test 3: Search with post-increment
    int *found = find_value(buffer, SIZE, 50);
    printf("Search test: %s\n", found ? "Found" : "Not found");
    
    // Test 4: Structure processing
    process_structs(sdata, 16);
    printf("Struct processed\n");
    
    // Test 5: Buffer copy with switch
    volatile char vdest[SIZE];
    copy_buffers(vdest, vstr, 0);
    printf("Buffer copy done\n");
    
    // Test 6: Complex mixed pointers
    int result = complex_postinc(vbuffer, buffer, 64);
    printf("Complex result: %d\n", result);
    
    // Test 7: String operations
    int str_result = string_ops(str);
    printf("String ops result: %d\n", str_result);
    
    // Additional tight loops likely to generate auto-inc
    // Byte copy loop
    {
        volatile char *src = vstr;
        volatile char *dst = (volatile char *)vdest;
        int cnt = PATTERN_SIZE;
        while (cnt--) {
            *dst++ = *src++;
        }
    }
    
    // Integer array processing with zero offset
    {
        volatile int *p = vbuffer;
        volatile int *end = vbuffer + SIZE;
        int checksum = 0;
        while (p < end) {
            // Direct dereference with zero offset
            checksum += *p;
            p++;  // Post-increment separate but adjacent
        }
        printf("Checksum: %d\n", checksum);
    }
    
    // Nested loop with inner post-increment
    {
        int matrix[8][8];
        for (int i = 0; i < 8; i++) {
            int *row = matrix[i];
            for (int j = 0; j < 8; j++) {
                *row++ = i * 8 + j;  // Post-increment in inner loop
            }
        }
        printf("Matrix filled\n");
    }
    
    return 0;
}
