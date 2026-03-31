#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barriers to prevent reordering
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
        ptr++;  // This should trigger auto-inc pattern
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_mixed_index_pointer(int *arr, int n) {
    int sum = 0;
    
    // Mixed indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        // Use both array indexing and pointer arithmetic
        int *ptr = &arr[i];
        sum += *ptr;
        
        // Create complex addressing expression
        int *ptr2 = ptr + 1;
        if (ptr2 < arr + n) {
            sum += *(ptr2 - 1);  // Complex address: ptr2 - 1
        }
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_copy(int *dst, int *src, int n) {
    int *d = dst;
    int *s = src;
    int *end = src + n;
    
    // Classic pointer copy pattern
    while (s < end) {
        *d = *s;
        d++;
        s++;  // Two pointers incrementing
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
long long test_multiple_types(char *carr, short *sarr, int *iarr, long long *llarr, int n) {
    long long total = 0;
    
    // Process char array with pointer
    char *cptr = carr;
    for (int i = 0; i < n; i++) {
        total += *cptr;
        cptr++;  // char pointer increment
    }
    
    // Process short array with pointer
    short *sptr = sarr;
    for (int i = 0; i < n; i++) {
        total += *sptr;
        sptr++;  // short pointer increment
    }
    
    // Process int array with pointer + offset
    int *iptr = iarr;
    for (int i = 0; i < n; i++) {
        // Use pointer with offset to create complex address
        total += *(iptr + 0);
        iptr += 1;  // int pointer increment
    }
    
    // Process long long array
    long long *llptr = llarr;
    for (int i = 0; i < n; i++) {
        total += *llptr;
        llptr++;  // long long pointer increment
    }
    
    COMPILER_BARRIER();
    return total;
}

// Structure for pointer walking
struct MixedData {
    int a;
    char b;
    short c;
    int d;
};

__attribute__((noinline))
int test_struct_pointer_walk(struct MixedData *arr, int n) {
    int sum = 0;
    struct MixedData *ptr = arr;
    
    // Walk through array of structures
    for (int i = 0; i < n; i++) {
        // Access multiple members
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
int test_volatile_pointers(volatile int *varr, int n) {
    int sum = 0;
    volatile int *vptr = varr;
    
    // Use volatile pointer to prevent optimization
    for (int i = 0; i < n; i++) {
        sum += *vptr;
        vptr++;  // Volatile pointer increment
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_complex_addressing(int *arr, int n) {
    int sum = 0;
    int *ptr1 = arr;
    int *ptr2 = arr + n/2;
    
    // Create complex addressing expressions
    for (int i = 0; i < n/2; i++) {
        // Expression: *(ptr1 + (ptr2 - ptr2)) simplifies to *ptr1
        // but requires analysis to reach XEXP(x, 0)
        sum += *(ptr1 + (ptr2 - ptr2));
        ptr1++;
        
        // Another complex expression
        if (i % 2 == 0) {
            sum += ptr1[-1];  // Negative offset
        }
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_fill(int *arr, int value, int n) {
    int *ptr = arr;
    int *end = arr + n;
    
    // Fill array with pointer write and increment
    while (ptr < end) {
        *ptr = value;
        ptr++;  // Write with pointer increment
    }
    
    COMPILER_BARRIER();
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    // Allocate arrays of different types
    int *int_arr1 = (int*)malloc(size * sizeof(int));
    int *int_arr2 = (int*)malloc(size * sizeof(int));
    char *char_arr = (char*)malloc(size * sizeof(char));
    short *short_arr = (short*)malloc(size * sizeof(short));
    long long *ll_arr = (long long*)malloc(size * sizeof(long long));
    struct MixedData *struct_arr = (struct MixedData*)malloc(size * sizeof(struct MixedData));
    volatile int *volatile_arr = (volatile int*)malloc(size * sizeof(int));
    
    // Initialize arrays
    for (int i = 0; i < size; i++) {
        int_arr1[i] = i;
        int_arr2[i] = 0;
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i % 32768);
        ll_arr[i] = i * 2LL;
        struct_arr[i].a = i;
        struct_arr[i].b = (char)(i % 256);
        struct_arr[i].c = (short)(i % 32768);
        struct_arr[i].d = i * 2;
        volatile_arr[i] = i % 100;
    }
    
    int checksum = 0;
    
    // Test 1: Simple pointer increment
    checksum += test_pointer_increment_sum(int_arr1, size);
    
    // Test 2: Mixed index and pointer
    checksum += test_mixed_index_pointer(int_arr1, size);
    
    // Test 3: Pointer copy
    test_pointer_copy(int_arr2, int_arr1, size);
    checksum += int_arr2[size/2];
    
    // Test 4: Multiple types
    checksum += (int)test_multiple_types(char_arr, short_arr, int_arr1, ll_arr, size);
    
    // Test 5: Structure pointer walk
    checksum += test_struct_pointer_walk(struct_arr, size);
    
    // Test 6: Volatile pointers
    checksum += test_volatile_pointers(volatile_arr, size);
    
    // Test 7: Complex addressing
    checksum += test_complex_addressing(int_arr1, size);
    
    // Test 8: Pointer fill
    test_pointer_fill(int_arr2, 42, size);
    checksum += int_arr2[size/2];
    
    printf("Final checksum: %d\n", checksum);
    
    // Cleanup
    free(int_arr1);
    free(int_arr2);
    free(char_arr);
    free(short_arr);
    free(ll_arr);
    free(struct_arr);
    free((void*)volatile_arr);
    
    return 0;
}
