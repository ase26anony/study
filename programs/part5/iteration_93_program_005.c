#include <stdio.h>
#include <string.h>

/* Structure for testing pointer post-increment with field access */
struct Data {
    int value;
    int count;
    char id;
};

/* Function 1: Copy with post-increment - tight loop */
void copy_with_postinc(char *dest, const char *src, int n) {
    // Basic post-increment copy loop
    while (n-- > 0) {
        *dest++ = *src++;
    }
}

/* Function 2: Summation with mixed volatile/non-volatile */
int sum_with_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *p = arr;
    
    // Post-increment in loop condition
    while (n-- > 0) {
        sum += *p++;
    }
    return sum;
}

/* Function 3: Search with post-increment in condition */
int find_with_postinc(const char *str, char target) {
    const char *p = str;
    
    // Post-increment in while condition
    while (*p != '\0' && *p++ != target) {
        // Empty body - increment happens in condition
    }
    
    return (*(p - 1) == target) ? (int)(p - str - 1) : -1;
}

/* Function 4: Structure array processing */
void process_structs(struct Data *sptr, int count) {
    // Post-increment on structure pointer
    for (int i = 0; i < count; i++) {
        // Access field then increment pointer
        int val = sptr->value;
        sptr->count = val * 2;
        sptr++;  // Post-increment equivalent in separate statement
        
        // Alternative: comma expression simulating post-increment
        // (access = sptr->value, sptr++, access)
    }
}

/* Function 5: Complex control flow with post-increment */
int complex_postinc(volatile int *data, int *results, int n) {
    volatile int *vptr = data;
    int *rptr = results;
    int processed = 0;
    
    for (int i = 0; i < n; i++) {
        // Post-increment in if condition
        if (*vptr++ > 0) {
            // Post-increment in assignment
            *rptr++ = *(vptr - 1) * 2;
            processed++;
        } else {
            // Different path with post-increment
            *rptr++ = 0;
            vptr++;  // Skip next element
        }
        
        // Nested switch with post-increment
        switch (i % 3) {
            case 0:
                // Post-increment in case body
                *rptr++ = *vptr++;
                break;
            case 1:
                // Fall through with post-increment
                *rptr = *vptr++;
                rptr++;
                break;
            case 2:
                // Comma expression with post-increment
                *rptr = (*vptr++, *(vptr - 1));
                rptr++;
                vptr++;
                break;
        }
    }
    
    return processed;
}

/* Function 6: String operations with post-increment */
int string_ops(char *buffer) {
    char *ptr = buffer;
    int len = 0;
    
    // Multiple post-increment patterns
    while ((*ptr++ = getchar()) != '\n' && len++ < 99) {
        // Loop with post-increment in condition
    }
    
    // Null terminate
    *(ptr - 1) = '\0';
    
    // Search with post-increment
    char *found = buffer;
    while (*found && *found++ != 'x') {
        // Search loop
    }
    
    return len;
}

/* Function 7: Array processing with zero offset */
void array_zero_offset(int *arr, int n) {
    int *ptr = arr;
    
    // Direct dereference with post-increment
    for (int i = 0; i < n; i++) {
        // Equivalent to arr[0] with ptr increment
        int val = *ptr;
        ptr++;
        
        // Process val
        arr[i] = val * 3;
    }
}

/* Function 8: Mixed pointer types in expression */
void mixed_pointers(volatile short *vptr, short *regptr, int n) {
    // Both volatile and non-volatile in same computation
    for (int i = 0; i < n; i++) {
        short temp = *vptr++;  // Volatile post-increment
        *regptr++ = temp + 5;  // Non-volatile post-increment
    }
}

int main() {
    // Test data
    char src[50] = "Test string for post-increment operations";
    char dest[50] = {0};
    volatile int varr[20];
    int regarr[20];
    struct Data structs[10];
    
    // Initialize volatile array
    for (int i = 0; i < 20; i++) {
        varr[i] = i * 2;
        regarr[i] = 0;
    }
    
    // Initialize structures
    for (int i = 0; i < 10; i++) {
        structs[i].value = i;
        structs[i].count = 0;
        structs[i].id = 'A' + i;
    }
    
    printf("Starting auto-inc/dec pattern tests...\n");
    
    // Test 1: Basic copy with post-increment
    copy_with_postinc(dest, src, strlen(src) + 1);
    printf("Copy test: %s\n", dest);
    
    // Test 2: Summation with volatile
    int sum = sum_with_postinc(varr, 20);
    printf("Sum test: %d\n", sum);
    
    // Test 3: Search with post-increment
    int pos = find_with_postinc(src, 'i');
    printf("Search test: found 'i' at position %d\n", pos);
    
    // Test 4: Structure processing
    process_structs(structs, 10);
    printf("Structure test: first count = %d\n", structs[0].count);
    
    // Test 5: Complex control flow
    int processed = complex_postinc(varr, regarr, 10);
    printf("Complex test: processed %d positive values\n", processed);
    
    // Test 6: Array with zero offset
    array_zero_offset(regarr, 10);
    printf("Zero offset test: regarr[0] = %d\n", regarr[0]);
    
    // Test 7: Mixed pointers
    volatile short vsarr[10];
    short sarr[10];
    for (int i = 0; i < 10; i++) {
        vsarr[i] = i * 10;
    }
    mixed_pointers(vsarr, sarr, 10);
    printf("Mixed pointer test: sarr[0] = %d\n", sarr[0]);
    
    // Test 8: String operations
    printf("Enter a short string for test: ");
    fflush(stdout);
    int len = string_ops(dest);
    printf("String test: entered %d chars: %s\n", len, dest);
    
    // Verification
    int verify_sum = 0;
    for (int i = 0; i < 20; i++) {
        verify_sum += varr[i];
    }
    printf("Verification sum: %d (should match earlier sum)\n", verify_sum);
    
    return 0;
}
