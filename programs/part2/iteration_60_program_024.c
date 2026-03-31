#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barriers to prevent reordering but keep pointer arithmetic intact
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve function boundaries
__attribute__((noinline))
int test_pointer_increment_sum(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + n;
    
    // Simple pointer increment pattern - should trigger auto-inc-dec
    while (ptr < end) {
        sum += *ptr;
        ptr++;  // This increment should be adjacent to the access
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_mixed_index_pointer(int *arr, int n) {
    int sum = 0;
    volatile int *vptr = arr;  // Volatile to prevent simplification
    int *ptr = (int*)vptr;
    
    // Mixed forms: array indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        sum += arr[i];      // Array index form
        sum += *ptr;        // Pointer dereference form
        ptr = &arr[i] + 1;  // Complex address calculation
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
        *d = *s;
        d++;
        s++;
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
long long test_multiple_sizes(char *carr, short *sarr, int *iarr, long long *llarr, int n) {
    long long total = 0;
    
    // Process different types with pointer arithmetic
    char *cptr = carr;
    short *sptr = sarr;
    int *iptr = iarr;
    long long *llptr = llarr;
    
    for (int i = 0; i < n; i++) {
        total += *cptr;
        cptr++;  // char pointer increment
        
        total += *sptr;
        sptr++;  // short pointer increment
        
        total += *iptr;
        iptr++;  // int pointer increment
        
        total += *llptr;
        llptr++;  // long long pointer increment
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_complex_addressing(int *arr, int n) {
    int sum = 0;
    volatile int *vbase = arr;  // Volatile base pointer
    int *base = (int*)vbase;
    int *ptr1 = base;
    int *ptr2 = base + n/2;
    
    // Complex addressing that might require analysis
    for (int i = 0; i < n/2; i++) {
        // Expression: *(ptr1 + (ptr2 - ptr2)) simplifies to *ptr1
        // but requires address calculation analysis
        sum += *(ptr1 + (ptr2 - ptr2));
        ptr1++;
        
        // Another complex form
        sum += ptr2[-i];  // Negative index
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_structure_access(void *data, int n) {
    struct Element {
        int a;
        int b;
        int c;
    };
    
    struct Element *arr = (struct Element*)data;
    struct Element *ptr = arr;
    int sum = 0;
    
    // Structure access with pointer walking
    for (int i = 0; i < n; i++) {
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        ptr++;  // Increment after all accesses
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_pointer_arithmetic_with_offset(int *arr, int n, int stride) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + n;
    
    // Pointer with constant offset increment
    while (ptr < end) {
        sum += *ptr;
        ptr += stride;  // Non-unit stride
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_nested_pointer_ops(int *arr, int n) {
    int sum = 0;
    int **pptr = &arr;  // Pointer to pointer
    
    for (int i = 0; i < n; i++) {
        sum += *(*pptr + i);  // Dereference pointer to pointer with offset
    }
    
    // Also test direct pointer increment
    int *ptr = arr;
    for (int i = 0; i < n; i++) {
        sum += ptr[i];
        // No explicit increment - rely on indexing
    }
    
    COMPILER_BARRIER();
    return sum;
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant at compile time
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 10;
    if (n > 1000) n = 1000;
    
    // Allocate arrays of different types
    int *int_arr = malloc(n * sizeof(int));
    int *int_arr2 = malloc(n * sizeof(int));
    char *char_arr = malloc(n * sizeof(char));
    short *short_arr = malloc(n * sizeof(short));
    long long *ll_arr = malloc(n * sizeof(long long));
    struct Element {
        int a, b, c;
    } *struct_arr = malloc(n * sizeof(struct Element));
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        int_arr[i] = i;
        int_arr2[i] = n - i;
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i * 2);
        ll_arr[i] = (long long)i * 1000;
        struct_arr[i].a = i;
        struct_arr[i].b = i * 2;
        struct_arr[i].c = i * 3;
    }
    
    int checksum = 0;
    
    // Run all test patterns
    checksum += test_pointer_increment_sum(int_arr, n);
    
    test_pointer_copy(int_arr2, int_arr, n);
    checksum += test_pointer_increment_sum(int_arr2, n);
    
    checksum += test_mixed_index_pointer(int_arr, n);
    
    long long multi_size_result = test_multiple_sizes(char_arr, short_arr, int_arr, ll_arr, n);
    checksum += (int)multi_size_result;
    
    checksum += test_complex_addressing(int_arr, n);
    
    checksum += test_structure_access(struct_arr, n);
    
    checksum += test_pointer_arithmetic_with_offset(int_arr, n, 2);
    
    checksum += test_nested_pointer_ops(int_arr, n);
    
    printf("Final checksum: %d\n", checksum);
    
    // Cleanup
    free(int_arr);
    free(int_arr2);
    free(char_arr);
    free(short_arr);
    free(ll_arr);
    free(struct_arr);
    
    return 0;
}
