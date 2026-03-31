/* test_auto_inc_dec.c - Target GCC's auto-increment/decrement pattern recognition */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barriers to prevent unwanted optimizations */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Test functions marked noinline to preserve boundaries */
__attribute__((noinline))
static int test_char_ptr_increment(volatile char *data, int n) {
    char *ptr = (char *)data;
    int sum = 0;
    
    /* Simple pointer increment pattern */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;  /* This should trigger auto-inc pattern */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static int test_short_ptr_complex_offset(volatile short *data, int n, int offset) {
    short *ptr = (short *)data;
    int sum = 0;
    
    /* Complex addressing: ptr + offset with post-increment */
    for (int i = 0; i < n; i++) {
        /* Access with offset, then increment */
        sum += *(ptr + offset);
        ptr++;  /* Adjacent increment */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static long long test_int_ptr_mixed_indexing(volatile int *data, int n) {
    int *ptr = (int *)data;
    long long sum = 0;
    
    /* Mixed indexing and pointer arithmetic */
    for (int i = 0; i < n; i++) {
        /* Array indexing form */
        sum += ptr[i];
        /* Pointer arithmetic form in same iteration */
        ptr += 1;  /* This increment should be analyzed with the previous access */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static long long test_longlong_ptr_stride(volatile long long *data, int n, int stride) {
    long long *ptr = (long long *)data;
    long long sum = 0;
    
    /* Pointer with stride increment */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr += stride;  /* Non-unit increment */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Structure for pointer walking */
struct MixedData {
    int a;
    short b;
    char c;
    long long d;
};

__attribute__((noinline))
static long long test_struct_ptr_walk(volatile struct MixedData *data, int n) {
    struct MixedData *ptr = (struct MixedData *)data;
    long long sum = 0;
    
    /* Walk through struct array with pointer */
    for (int i = 0; i < n; i++) {
        /* Multiple member accesses */
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        /* Increment after all accesses */
        ptr++;  /* Should trigger pattern recognition */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static void test_dual_ptr_copy(volatile int *src, volatile int *dst, int n) {
    int *s = (int *)src;
    int *d = (int *)dst;
    
    /* Classic dual-pointer copy pattern */
    for (int i = 0; i < n; i++) {
        *d = *s;
        s++;  /* Both increments should be candidates */
        d++;
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
static int test_pointer_arithmetic_chain(volatile int *data1, volatile int *data2, int n) {
    int *p1 = (int *)data1;
    int *p2 = (int *)data2;
    int sum = 0;
    
    /* Complex pointer arithmetic that may create interesting address expressions */
    for (int i = 0; i < n; i++) {
        /* Expression: *(p1 + (p2 - p2)) simplifies but requires analysis */
        sum += *(p1 + (p2 - p2));
        p1++;  /* Increment after complex address calculation */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static int test_volatile_ptr_with_offset(volatile int *volatile vptr, int n) {
    volatile int *ptr = vptr;
    int sum = 0;
    
    /* Volatile pointer prevents some optimizations */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;  /* Increment volatile pointer */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static void test_fill_with_ptr(volatile int *data, int value, int n) {
    int *ptr = (int *)data;
    
    /* Write pattern with pointer increment */
    for (int i = 0; i < n; i++) {
        *ptr = value;
        ptr++;  /* Post-increment after write */
    }
    
    COMPILER_BARRIER();
}

/* Main driver that creates various patterns */
int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    if (size > 10000) size = 10000;  /* Reasonable limit */
    
    /* Allocate arrays of different types */
    char *char_array = malloc(size * sizeof(char));
    short *short_array = malloc(size * sizeof(short));
    int *int_array = malloc(size * sizeof(int));
    long long *ll_array = malloc(size * sizeof(long long));
    struct MixedData *struct_array = malloc(size * sizeof(struct MixedData));
    int *src_array = malloc(size * sizeof(int));
    int *dst_array = malloc(size * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        int_array[i] = i * 3;
        ll_array[i] = i * 5LL;
        struct_array[i].a = i;
        struct_array[i].b = (short)(i * 2);
        struct_array[i].c = (char)(i % 128);
        struct_array[i].d = i * 7LL;
        src_array[i] = i * 11;
    }
    
    long long total = 0;
    
    /* Execute all test patterns */
    total += test_char_ptr_increment((volatile char *)char_array, size);
    total += test_short_ptr_complex_offset((volatile short *)short_array, size, 0);
    total += test_int_ptr_mixed_indexing((volatile int *)int_array, size);
    total += test_longlong_ptr_stride((volatile long long *)ll_array, size, 1);
    total += test_struct_ptr_walk((volatile struct MixedData *)struct_array, size);
    
    test_dual_ptr_copy((volatile int *)src_array, (volatile int *)dst_array, size);
    
    /* Verify copy worked */
    for (int i = 0; i < size; i++) {
        total += dst_array[i];
    }
    
    total += test_pointer_arithmetic_chain((volatile int *)int_array, 
                                          (volatile int *)src_array, size);
    
    total += test_volatile_ptr_with_offset((volatile int *)int_array, size);
    
    test_fill_with_ptr((volatile int *)dst_array, 42, size);
    
    /* Verify fill worked */
    for (int i = 0; i < size; i++) {
        total += dst_array[i];
    }
    
    printf("Total checksum: %lld\n", total);
    
    /* Cleanup */
    free(char_array);
    free(short_array);
    free(int_array);
    free(ll_array);
    free(struct_array);
    free(src_array);
    free(dst_array);
    
    return (total > 0) ? 0 : 1;
}
