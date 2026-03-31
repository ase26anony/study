#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barriers to prevent reordering but keep pointer arithmetic intact
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve function boundaries
__attribute__((noinline))
int test_pointer_increment_sum(int *array, int n) {
    volatile int *volatile_ptr = array; // Volatile to prevent constant folding
    int *ptr = (int *)volatile_ptr;
    int sum = 0;
    
    // Pattern: *ptr followed by ptr++ in same iteration
    for (int i = 0; i < n; i++) {
        sum += *ptr;  // Memory access via pointer
        ptr++;        // Pointer increment immediately after
    }
    
    COMPILER_BARRIER(); // Outside loop to prevent reordering
    return sum;
}

__attribute__((noinline))
int test_pointer_with_offset(int *array, int n) {
    volatile int *volatile_base = array;
    int *base = (int *)volatile_base;
    int sum = 0;
    
    // Pattern: *(ptr + constant) with ptr increment
    for (int i = 0; i < n; i++) {
        // Complex addressing: base + 0 (simplifies but requires analysis)
        sum += *(base + 0);
        base += 1;  // Increment by stride
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_mixed_index_pointer(int *array, int n) {
    volatile int *volatile_ptr = array;
    int *ptr = (int *)volatile_ptr;
    int sum = 0;
    
    // Mixed forms: array indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // Array indexing form
        ptr = ptr + 1;  // Pointer arithmetic form
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_two_pointers_copy(int *src, int *dst, int n) {
    volatile int *volatile_src = src;
    volatile int *volatile_dst = dst;
    int *s = (int *)volatile_src;
    int *d = (int *)volatile_dst;
    
    // Classic *dst++ = *src++ pattern
    for (int i = 0; i < n; i++) {
        *d = *s;  // Memory access via both pointers
        d++;      // Both pointers incremented
        s++;
    }
    
    COMPILER_BARRIER();
    
    // Verify copy by summing destination
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += dst[i];
    }
    return sum;
}

__attribute__((noinline))
int test_multiple_base_registers(int *array1, int *array2, int n) {
    volatile int *volatile_p1 = array1;
    volatile int *volatile_p2 = array2;
    int *p1 = (int *)volatile_p1;
    int *p2 = (int *)volatile_p2;
    int sum = 0;
    
    // Complex addressing that could be seen as base-plus-index
    for (int i = 0; i < n; i++) {
        // Expression: p1 + (p2 - p2) which simplifies to p1
        // but requires analysis to recognize as simple base
        sum += p1[p2 - p2];
        p1++;  // Increment one pointer
        // Keep p2 unchanged to create asymmetry
    }
    
    COMPILER_BARRIER();
    return sum;
}

// Structure for pointer walking test
struct Data {
    int a;
    int b;
    char c;
    short d;
};

__attribute__((noinline))
int test_structure_pointer_walk(struct Data *data, int n) {
    volatile struct Data *volatile_ptr = data;
    struct Data *ptr = (struct Data *)volatile_ptr;
    int sum = 0;
    
    // Access multiple structure members with pointer increment
    for (int i = 0; i < n; i++) {
        sum += ptr->a + ptr->b;  // Multiple member accesses
        sum += ptr->c + ptr->d;
        ptr++;  // Pointer increment after last access
    }
    
    COMPILER_BARRIER();
    return sum;
}

// Different memory access sizes
__attribute__((noinline))
int test_mixed_sizes(char *carr, short *sarr, int *iarr, long long *llarr, int n) {
    volatile char *volatile_cptr = carr;
    volatile short *volatile_sptr = sarr;
    volatile int *volatile_iptr = iarr;
    volatile long long *volatile_llptr = llarr;
    
    char *cptr = (char *)volatile_cptr;
    short *sptr = (short *)volatile_sptr;
    int *iptr = (int *)volatile_iptr;
    long long *llptr = (long long *)volatile_llptr;
    
    int sum = 0;
    
    // Process each array with pointer increment
    for (int i = 0; i < n; i++) {
        sum += *cptr;
        cptr++;
    }
    
    for (int i = 0; i < n; i++) {
        sum += *sptr;
        sptr++;
    }
    
    for (int i = 0; i < n; i++) {
        sum += *iptr;
        iptr++;
    }
    
    for (int i = 0; i < n; i++) {
        sum += (int)*llptr;  // Truncate for checksum
        llptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_pointer_fill(int *array, int n, int value) {
    volatile int *volatile_ptr = array;
    int *ptr = (int *)volatile_ptr;
    
    // Pattern: *ptr = value; ptr++;
    for (int i = 0; i < n; i++) {
        *ptr = value;  // Write via pointer
        ptr++;         // Increment after write
    }
    
    COMPILER_BARRIER();
    
    // Verify fill by summing
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += array[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant at compile time
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 10;
    if (n > 1000) n = 1000;
    
    // Allocate and initialize arrays of different types
    int *int_array1 = (int *)malloc(n * sizeof(int));
    int *int_array2 = (int *)malloc(n * sizeof(int));
    char *char_array = (char *)malloc(n * sizeof(char));
    short *short_array = (short *)malloc(n * sizeof(short));
    long long *ll_array = (long long *)malloc(n * sizeof(long long));
    struct Data *struct_array = (struct Data *)malloc(n * sizeof(struct Data));
    
    // Initialize with pattern
    for (int i = 0; i < n; i++) {
        int_array1[i] = i % 100;
        int_array2[i] = 0;
        char_array[i] = (char)(i % 128);
        short_array[i] = (short)(i % 1000);
        ll_array[i] = i;
        struct_array[i].a = i;
        struct_array[i].b = i * 2;
        struct_array[i].c = (char)(i % 128);
        struct_array[i].d = (short)(i % 1000);
    }
    
    int checksum = 0;
    
    // Run all test patterns
    checksum += test_pointer_increment_sum(int_array1, n);
    checksum += test_pointer_with_offset(int_array1, n);
    checksum += test_mixed_index_pointer(int_array1, n);
    checksum += test_two_pointers_copy(int_array1, int_array2, n);
    checksum += test_multiple_base_registers(int_array1, int_array2, n);
    checksum += test_structure_pointer_walk(struct_array, n);
    checksum += test_mixed_sizes(char_array, short_array, int_array1, ll_array, n);
    checksum += test_pointer_fill(int_array2, n, 42);
    
    // Final compiler barrier
    COMPILER_BARRIER();
    
    printf("Final checksum: %d\n", checksum);
    
    // Cleanup
    free(int_array1);
    free(int_array2);
    free(char_array);
    free(short_array);
    free(ll_array);
    free(struct_array);
    
    return 0;
}
