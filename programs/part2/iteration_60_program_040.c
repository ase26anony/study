#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barriers to prevent reordering but keep patterns intact
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve boundaries
__attribute__((noinline))
int test_char_pointer_increment(volatile char *data, int n) {
    char *ptr = (char *)data;
    int sum = 0;
    
    // Simple pointer increment pattern
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;  // Adjacent increment - should trigger auto-inc/dec
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_short_pointer_offset(volatile short *data, int n) {
    short *ptr = (short *)data;
    int sum = 0;
    
    // Pointer with offset access followed by increment
    for (int i = 0; i < n; i++) {
        // Complex addressing: *(ptr + 0) which becomes XEXP(x, 0)
        sum += *(ptr + 0);
        ptr += 1;  // Increment by element size
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_int_mixed_index_pointer(volatile int *data, int n) {
    int *ptr = (int *)data;
    int sum = 0;
    
    // Mixed indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        // Array indexing form
        sum += ptr[0];
        // Pointer increment - adjacent to access
        ptr = ptr + 1;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
long long test_longlong_pointer_chain(volatile long long *data, int n) {
    long long *ptr = (long long *)data;
    long long sum = 0;
    
    // Chain of accesses with pointer walking
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        // Multiple increments to create complex pattern
        ptr = ptr + 1;
    }
    
    COMPILER_BARRIER();
    return sum;
}

// Structure for pointer walking pattern
struct MixedData {
    int a;
    short b;
    char c;
    long long d;
};

__attribute__((noinline))
long long test_struct_pointer_walk(volatile struct MixedData *data, int n) {
    struct MixedData *ptr = (struct MixedData *)data;
    long long sum = 0;
    
    // Structure access with pointer increment
    for (int i = 0; i < n; i++) {
        // Access multiple members
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        // Pointer increment after last access
        ptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_dual_pointer_copy(volatile int *src, volatile int *dst, int n) {
    int *s = (int *)src;
    int *d = (int *)dst;
    
    // Classic copy pattern: *dst++ = *src++
    for (int i = 0; i < n; i++) {
        *d = *s;
        s++;
        d++;
    }
    
    COMPILER_BARRIER();
    return n;
}

__attribute__((noinline))
int test_pointer_arithmetic_complex(volatile int *data1, volatile int *data2, int n) {
    int *p1 = (int *)data1;
    int *p2 = (int *)data2;
    int sum = 0;
    
    // Complex addressing that might simplify to base+0
    for (int i = 0; i < n; i++) {
        // Expression: *(p1 + (p2 - p2)) which should become *(p1 + 0)
        sum += *(p1 + (p2 - p2));
        p1++;
        // Keep p2 moving too to prevent complete optimization
        p2 = p2 + 1;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_volatile_pointer_chain(volatile int *volatile data, int n) {
    volatile int *ptr = data;
    int sum = 0;
    
    // Volatile pointer prevents some optimizations
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        // The increment must stay adjacent
        ptr = ptr + 1;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_fill_pattern(volatile int *data, int value, int n) {
    int *ptr = (int *)data;
    
    // Fill with increment pattern
    for (int i = 0; i < n; i++) {
        *ptr = value;
        ptr++;
    }
    
    COMPILER_BARRIER();
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant at compile time
    int base_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (base_size < 10) base_size = 100;
    
    // Allocate arrays of different types
    char *char_array = malloc(base_size * sizeof(char));
    short *short_array = malloc(base_size * sizeof(short));
    int *int_array = malloc(base_size * sizeof(int));
    long long *ll_array = malloc(base_size * sizeof(long long));
    struct MixedData *struct_array = malloc(base_size * sizeof(struct MixedData));
    int *src_array = malloc(base_size * sizeof(int));
    int *dst_array = malloc(base_size * sizeof(int));
    
    // Initialize with pattern
    for (int i = 0; i < base_size; i++) {
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        int_array[i] = i * 3;
        ll_array[i] = i * 1000LL;
        struct_array[i].a = i;
        struct_array[i].b = (short)(i * 2);
        struct_array[i].c = (char)(i % 128);
        struct_array[i].d = i * 1000LL;
        src_array[i] = i * 7;
    }
    
    long long total_sum = 0;
    
    // Run all test patterns
    total_sum += test_char_pointer_increment((volatile char *)char_array, base_size);
    total_sum += test_short_pointer_offset((volatile short *)short_array, base_size);
    total_sum += test_int_mixed_index_pointer((volatile int *)int_array, base_size);
    total_sum += test_longlong_pointer_chain((volatile long long *)ll_array, base_size);
    total_sum += test_struct_pointer_walk((volatile struct MixedData *)struct_array, base_size);
    total_sum += test_dual_pointer_copy((volatile int *)src_array, (volatile int *)dst_array, base_size);
    total_sum += test_pointer_arithmetic_complex((volatile int *)int_array, (volatile int *)src_array, base_size);
    total_sum += test_volatile_pointer_chain((volatile int *)int_array, base_size);
    
    // Test fill pattern
    test_fill_pattern((volatile int *)dst_array, 0xABCD, base_size);
    
    // Verify fill worked
    for (int i = 0; i < base_size; i++) {
        total_sum += dst_array[i];
    }
    
    printf("Total checksum: %lld\n", total_sum);
    
    // Cleanup
    free(char_array);
    free(short_array);
    free(int_array);
    free(ll_array);
    free(struct_array);
    free(src_array);
    free(dst_array);
    
    return (total_sum > 0) ? 0 : 1;
}
