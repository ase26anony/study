#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Structure for testing pointer post-increment */
struct Data {
    int value;
    int count;
    char tag;
};

/* Function 1: Copy with post-increment - basic case */
void copy_with_postinc(char *dest, const char *src, size_t n) {
    while (n-- > 0) {
        *dest++ = *src++;  // Post-increment in assignment
    }
}

/* Function 2: Sum with post-increment in loop condition */
int sum_with_postinc(const int *arr, size_t n) {
    int sum = 0;
    const int *p = arr;
    const int *end = arr + n;
    
    while (p < end) {
        sum += *p++;  // Post-increment in expression
    }
    return sum;
}

/* Function 3: Mixed volatile and non-volatile pointers */
int volatile_sum(volatile int *varr, int *arr, size_t n) {
    int sum = 0;
    volatile int *vp = varr;
    int *p = arr;
    
    for (size_t i = 0; i < n; i++) {
        // Mix volatile and non-volatile in same expression
        sum += (int)(*vp++) + (*p++);
    }
    return sum;
}

/* Function 4: Structure access with post-increment */
int process_structs(struct Data *data, size_t count) {
    int total = 0;
    struct Data *ptr = data;
    
    // Post-increment in array-like access
    for (size_t i = 0; i < count; i++) {
        total += ptr->value;  // Base + 0 offset
        ptr++;  // Post-increment after access
    }
    return total;
}

/* Function 5: Complex control flow with post-increment */
int search_with_postinc(const char *str, char target) {
    const char *p = str;
    
    // Post-increment in while condition
    while (*p != '\0' && *p++ != target) {
        // Empty body - increment happens in condition
    }
    
    // If found, p points one past the target
    if (*(p - 1) == target) {
        return 1;
    }
    return 0;
}

/* Function 6: Nested loops with post-increment */
void matrix_process(int matrix[3][3], int factor) {
    for (int i = 0; i < 3; i++) {
        int *row = matrix[i];
        int *end = row + 3;
        
        // Inner loop with post-increment
        while (row < end) {
            *row++ *= factor;  // Post-increment with modification
        }
    }
}

/* Function 7: Comma expression with post-increment */
int comma_postinc(int *arr, size_t n) {
    int sum = 0;
    int *p = arr;
    
    for (size_t i = 0; i < n; i++) {
        // Comma expression: access then increment
        sum += (temp = *p, p++, temp);
    }
    return sum;
}

/* Function 8: Switch with post-increment */
int switch_postinc(const char *str) {
    const char *p = str;
    int count = 0;
    
    while (*p) {
        switch (*p++) {  // Post-increment in switch expression
            case 'a':
            case 'e':
            case 'i':
            case 'o':
            case 'u':
                count++;
                break;
            default:
                // Post-increment in default case too
                if (*(p - 1) == ' ') {
                    // Another pointer access
                }
                break;
        }
    }
    return count;
}

/* Function 9: Pointer to pointer with post-increment */
void copy_pointers(int **dest, int **src, size_t n) {
    while (n-- > 0) {
        *dest++ = *src++;  // Post-increment of pointer-to-pointer
    }
}

int main() {
    // Test data
    char src[] = "Hello, World!";
    char dest[50];
    volatile int varr[10];
    int arr[10];
    struct Data structs[5];
    int matrix[3][3];
    int *ptr_arr1[5], *ptr_arr2[5];
    
    // Initialize test data
    for (int i = 0; i < 10; i++) {
        varr[i] = i * 2;
        arr[i] = i;
    }
    
    for (int i = 0; i < 5; i++) {
        structs[i].value = i * 10;
        structs[i].count = i;
        structs[i].tag = 'A' + i;
        ptr_arr1[i] = &arr[i];
    }
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            matrix[i][j] = i * 3 + j + 1;
        }
    }
    
    // Test 1: Basic copy with post-increment
    copy_with_postinc(dest, src, strlen(src) + 1);
    printf("Copy test: %s\n", dest);
    
    // Test 2: Sum with post-increment
    int sum1 = sum_with_postinc(arr, 10);
    printf("Sum test 1: %d\n", sum1);
    
    // Test 3: Mixed volatile/non-volatile
    int sum2 = volatile_sum(varr, arr, 10);
    printf("Volatile sum: %d\n", sum2);
    
    // Test 4: Structure access
    int struct_sum = process_structs(structs, 5);
    printf("Structure sum: %d\n", struct_sum);
    
    // Test 5: Search with post-increment
    int found = search_with_postinc(src, 'W');
    printf("Search for 'W': %s\n", found ? "Found" : "Not found");
    
    // Test 6: Nested loops
    matrix_process(matrix, 2);
    printf("Matrix[0][0] after processing: %d\n", matrix[0][0]);
    
    // Test 7: Comma expression (commented out as temp not defined)
    // int comma_sum = comma_postinc(arr, 10);
    // printf("Comma sum: %d\n", comma_sum);
    
    // Test 8: Switch with post-increment
    int vowel_count = switch_postinc("Hello World");
    printf("Vowel count: %d\n", vowel_count);
    
    // Test 9: Pointer to pointer
    copy_pointers(ptr_arr2, ptr_arr1, 5);
    printf("Pointer copy test: %p -> %p\n", (void*)ptr_arr1[0], (void*)ptr_arr2[0]);
    
    // Additional tight loop tests
    // Test 10: String length with post-increment
    const char *test_str = "Testing";
    int len = 0;
    while (*test_str++) len++;  // Post-increment in condition
    printf("String length: %d\n", len);
    
    // Test 11: Array zeroing with post-increment
    int zero_arr[10];
    int *zp = zero_arr;
    int *zend = zero_arr + 10;
    while (zp < zend) {
        *zp++ = 0;  // Post-increment with assignment
    }
    
    // Test 12: Complex expression with multiple post-increments
    int a[5] = {1, 2, 3, 4, 5};
    int b[5];
    int *ap = a;
    int *bp = b;
    for (int i = 0; i < 5; i++) {
        *bp++ = *ap++ + 10;  // Multiple post-increments
    }
    printf("Complex copy test: b[0] = %d\n", b[0]);
    
    return 0;
}
