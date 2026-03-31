#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barriers to prevent reordering but keep patterns intact
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve boundaries
__attribute__((noinline))
int test_pointer_increment_sum(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + n;
    
    // Simple pointer increment pattern
    while (ptr < end) {
        sum += *ptr;
        ptr++;  // Adjacent increment - should trigger auto-inc-dec
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_mixed_index_pointer(int *arr, int n) {
    int sum = 0;
    volatile int *vptr = arr;  // Volatile to prevent simplification
    
    // Mixed indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        // Access via pointer with offset
        sum += *(vptr + i);
        
        // Also use array syntax with pointer
        int *temp = (int*)vptr;
        sum += temp[i];
        
        // Force complex address expression
        sum += *(temp + (i - i) + 1);
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_copy(int *dst, int *src, int n) {
    int *d = dst;
    int *s = src;
    int *end = src + n;
    
    // Classic copy pattern with post-increment
    while (s < end) {
        *d++ = *s++;  // Should trigger auto-inc-dec for both pointers
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
long long test_multiple_sizes(char *carr, short *sarr, int *iarr, long long *llarr, int n) {
    long long total = 0;
    
    // Process char array with pointer
    char *cptr = carr;
    for (int i = 0; i < n; i++) {
        total += *cptr;
        cptr += 1;  // Char pointer increment
    }
    
    // Process short array with pointer
    short *sptr = sarr;
    for (int i = 0; i < n; i++) {
        total += *sptr;
        sptr += 1;  // Short pointer increment
    }
    
    // Process int array with pointer and offset
    int *iptr = iarr;
    for (int i = 0; i < n; i++) {
        total += *(iptr + 0);  // Force address analysis
        iptr = iptr + 1;
    }
    
    // Process long long array with stride
    long long *llptr = llarr;
    for (int i = 0; i < n; i++) {
        total += *llptr;
        llptr++;  // Long long pointer increment
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_structure_access(struct Data {
    int a;
    int b;
    char c;
    short d;
} *data, int n) {
    int sum = 0;
    struct Data *ptr = data;
    
    // Walk through structure array with pointer
    for (int i = 0; i < n; i++) {
        // Access multiple members
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        
        // Increment after all accesses
        ptr++;  // Should trigger auto-inc-dec for structure pointer
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_complex_address(int *base1, int *base2, int n) {
    int sum = 0;
    volatile int *volatile_ptr1 = base1;
    volatile int *volatile_ptr2 = base2;
    
    // Complex addressing that might simplify to base + 0
    for (int i = 0; i < n; i++) {
        // Expression: *(p1 + p2 - p2) should simplify to *p1
        // but requires address analysis
        int *temp1 = (int*)volatile_ptr1;
        int *temp2 = (int*)volatile_ptr2;
        sum += *(temp1 + (temp2 - temp2));
        
        // Move pointers in complex way
        volatile_ptr1 = volatile_ptr1 + 1;
        volatile_ptr2 = volatile_ptr2 + 1;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_pointer_arithmetic_loop(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    // Loop with pointer arithmetic that creates MEM address
    for (int i = 0; i < n; i++) {
        // Use pointer with constant offset
        sum += ptr[0];
        sum += ptr[1];
        
        // Increment by 2 to skip one element
        ptr = ptr + 2;
    }
    
    COMPILER_BARRIER();
    return sum;
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    // Allocate arrays of different types
    int *int_arr1 = malloc(size * sizeof(int));
    int *int_arr2 = malloc(size * sizeof(int));
    char *char_arr = malloc(size * sizeof(char));
    short *short_arr = malloc(size * sizeof(short));
    long long *llong_arr = malloc(size * sizeof(long long));
    struct Data *struct_arr = malloc(size * sizeof(struct Data));
    
    // Initialize arrays
    for (int i = 0; i < size; i++) {
        int_arr1[i] = i;
        int_arr2[i] = size - i;
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i * 2);
        llong_arr[i] = (long long)i * 1000;
        struct_arr[i].a = i;
        struct_arr[i].b = i * 2;
        struct_arr[i].c = (char)(i % 128);
        struct_arr[i].d = (short)(i * 3);
    }
    
    int result = 0;
    
    // Test 1: Simple pointer increment
    result += test_pointer_increment_sum(int_arr1, size);
    
    // Test 2: Mixed index and pointer
    result += test_mixed_index_pointer(int_arr1, size);
    
    // Test 3: Pointer copy
    test_pointer_copy(int_arr2, int_arr1, size);
    result += test_pointer_increment_sum(int_arr2, size);
    
    // Test 4: Multiple sizes
    result += test_multiple_sizes(char_arr, short_arr, int_arr1, llong_arr, size);
    
    // Test 5: Structure access
    result += test_structure_access(struct_arr, size);
    
    // Test 6: Complex addressing
    result += test_complex_address(int_arr1, int_arr2, size);
    
    // Test 7: Pointer arithmetic with stride
    result += test_pointer_arithmetic_loop(int_arr1, size / 2);
    
    printf("Result: %d\n", result);
    
    // Cleanup
    free(int_arr1);
    free(int_arr2);
    free(char_arr);
    free(short_arr);
    free(llong_arr);
    free(struct_arr);
    
    return 0;
}
