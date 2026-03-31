#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barrier to prevent reordering
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
        ptr++;  // This should trigger auto-inc-dec pattern
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_mixed_index_pointer(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    // Mixed indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        sum += arr[i];      // Array index access
        sum += *ptr;        // Pointer dereference
        ptr++;              // Pointer increment - complex pattern
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_copy(int *dst, int *src, int n) {
    int *d = dst;
    int *s = src;
    int *end = src + n;
    
    // Classic copy pattern with two moving pointers
    while (s < end) {
        *d = *s;
        d++;
        s++;  // Two auto-increment opportunities
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
int test_pointer_with_offset(short *arr, int n) {
    int sum = 0;
    short *ptr = arr;
    
    // Pointer with constant offset
    for (int i = 0; i < n; i++) {
        sum += *(ptr + 1);  // Offset access
        sum += *ptr;        // Direct access
        ptr += 2;           // Stride increment
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
long long test_multiple_sizes(char *carr, short *sarr, int *iarr, long long *llarr, int n) {
    long long total = 0;
    
    // Different pointer types with increments
    char *cp = carr;
    short *sp = sarr;
    int *ip = iarr;
    long long *llp = llarr;
    
    for (int i = 0; i < n; i++) {
        total += *cp;
        cp++;  // char pointer increment
        
        total += *sp;
        sp++;  // short pointer increment
        
        total += *ip;
        ip++;  // int pointer increment
        
        total += *llp;
        llp++;  // long long pointer increment
    }
    
    COMPILER_BARRIER();
    return total;
}

// Structure for pointer walking
struct Data {
    int a;
    int b;
    short c;
    char d;
};

__attribute__((noinline))
int test_struct_pointer_walk(struct Data *arr, int n) {
    int sum = 0;
    struct Data *ptr = arr;
    
    // Walk through array of structures
    for (int i = 0; i < n; i++) {
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        ptr++;  // Structure pointer increment
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_complex_address_expression(int *base1, int *base2, int n) {
    int sum = 0;
    volatile int *volatile_ptr = base1;  // Volatile to prevent simplification
    
    // Complex address expression that might simplify to base + 0
    for (int i = 0; i < n; i++) {
        // This creates address expression: *(base1 + (base2 - base2))
        // Should simplify to *base1 but requires analysis
        sum += *(volatile_ptr + (base2 - base2));
        volatile_ptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_pointer_arithmetic_chain(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    // Chain of pointer arithmetic operations
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr = ptr + 1;  // Alternative form of increment
        sum += *(ptr - 1);  // Access with negative offset
        ptr = ptr + 0;  // No-op that creates address expression
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_fill_with_increment(int *arr, int value, int n) {
    int *ptr = arr;
    int *end = arr + n;
    
    // Fill pattern with write and increment
    while (ptr < end) {
        *ptr = value;
        ptr++;  // Write with auto-increment
    }
    
    COMPILER_BARRIER();
}

// Main test driver
int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant at compile time
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 100;
    
    // Allocate arrays of different types
    int *int_arr1 = (int*)malloc(n * sizeof(int));
    int *int_arr2 = (int*)malloc(n * sizeof(int));
    short *short_arr = (short*)malloc(n * sizeof(short));
    char *char_arr = (char*)malloc(n * sizeof(char));
    long long *ll_arr = (long long*)malloc(n * sizeof(long long));
    struct Data *struct_arr = (struct Data*)malloc(n * sizeof(struct Data));
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        int_arr1[i] = i;
        int_arr2[i] = i * 2;
        short_arr[i] = (short)(i % 1000);
        char_arr[i] = (char)(i % 128);
        ll_arr[i] = i * 1000LL;
        struct_arr[i].a = i;
        struct_arr[i].b = i * 2;
        struct_arr[i].c = (short)(i % 500);
        struct_arr[i].d = (char)(i % 64);
    }
    
    int result = 0;
    
    // Run all test patterns
    result += test_pointer_increment_sum(int_arr1, n);
    result += test_mixed_index_pointer(int_arr1, n);
    
    test_pointer_copy(int_arr2, int_arr1, n);
    result += test_pointer_increment_sum(int_arr2, n);
    
    result += test_pointer_with_offset(short_arr, n / 2);
    result += test_multiple_sizes(char_arr, short_arr, int_arr1, ll_arr, n);
    result += test_struct_pointer_walk(struct_arr, n);
    result += test_complex_address_expression(int_arr1, int_arr2, n);
    result += test_pointer_arithmetic_chain(int_arr1, n);
    
    test_fill_with_increment(int_arr1, 42, n);
    result += test_pointer_increment_sum(int_arr1, n);
    
    // Cleanup
    free(int_arr1);
    free(int_arr2);
    free(short_arr);
    free(char_arr);
    free(ll_arr);
    free(struct_arr);
    
    printf("Result: %d\n", result);
    return 0;
}
