/* test_auto_inc_dec.c - Target coverage for auto-inc-dec.cc lines 1352-1358 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent reordering without breaking patterns */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Test functions marked noinline to preserve boundaries */
__attribute__((noinline))
static int test_char_pointer_sum(volatile int size, char *data) {
    char *ptr = data;
    char *end = data + size;
    int sum = 0;
    
    /* Simple pointer increment pattern */
    while (ptr < end) {
        sum += *ptr;
        ptr++;  /* This should trigger auto-inc-dec pattern */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static int test_short_pointer_offset(volatile int size, short *data) {
    short *ptr = data;
    int sum = 0;
    
    /* Pointer with offset access */
    for (int i = 0; i < size; i++) {
        /* Complex addressing: *(ptr + 0) but ptr changes each iteration */
        sum += *(ptr + 0);
        ptr += 1;  /* Adjacent increment */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static void test_int_copy_pattern(volatile int size, int *src, int *dst) {
    int *s = src;
    int *d = dst;
    int *end = src + size;
    
    /* Classic copy pattern with post-increment */
    while (s < end) {
        *d = *s;
        s++;
        d++;  /* Two pointers incrementing - complex pattern */
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
static long long test_mixed_index_pointer(volatile int size, int *data) {
    long long sum = 0;
    int *ptr = data;
    
    /* Mixed indexing and pointer arithmetic */
    for (int i = 0; i < size; i++) {
        /* Access via pointer dereference */
        sum += *ptr;
        
        /* Also access via index on same pointer */
        if (i % 2 == 0) {
            sum += ptr[0];  /* Same as *ptr but different syntax */
        }
        
        ptr++;  /* Increment after mixed accesses */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Structure for pointer walking */
struct TwoInts {
    int a;
    int b;
};

__attribute__((noinline))
static int test_struct_pointer_walk(volatile int size, struct TwoInts *data) {
    struct TwoInts *ptr = data;
    int sum = 0;
    
    /* Walk structure array with pointer */
    for (int i = 0; i < size; i++) {
        /* Access multiple structure members */
        sum += ptr->a;
        sum += ptr->b;
        
        /* Increment pointer after last member access */
        ptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static int test_double_pointer_arithmetic(volatile int size, int *data) {
    int *p1 = data;
    int *p2 = data + size/2;
    int sum = 0;
    
    /* Complex pointer arithmetic that might create base+index addressing */
    for (int i = 0; i < size/2; i++) {
        /* Expression that could be seen as base-plus-index */
        sum += *(p1 + (p2 - p2));  /* Should simplify to *p1 */
        p1++;
        
        /* Another complex expression */
        sum += p1[p2 - p2];  /* Should be same as *p1 */
        p1++;  /* Second increment in same loop */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static int test_volatile_pointer(volatile int size, int *base_data) {
    /* Volatile pointer to prevent optimization of address calculation */
    volatile int *vptr = base_data;
    int sum = 0;
    
    /* Loop with volatile pointer increment */
    for (int i = 0; i < size; i++) {
        sum += *vptr;
        vptr++;  /* Volatile pointer increment */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static long long test_long_long_access(volatile int size, long long *data) {
    long long *ptr = data;
    long long sum = 0;
    
    /* 64-bit accesses with pointer increment */
    for (int i = 0; i < size; i++) {
        sum += *ptr;
        ptr++;  /* DImode pointer increment */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static void test_pointer_fill(volatile int size, int *data, int value) {
    int *ptr = data;
    int *end = data + size;
    
    /* Fill pattern with write and increment */
    while (ptr < end) {
        *ptr = value;
        ptr++;  /* Post-increment after write */
    }
    
    COMPILER_BARRIER();
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Use argc to make sizes non-constant at compile time */
    volatile int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    int checksum = 0;
    
    /* Allocate and initialize arrays of different types */
    char *char_array = malloc(size * sizeof(char));
    short *short_array = malloc(size * sizeof(short));
    int *int_array = malloc(size * sizeof(int));
    long long *ll_array = malloc(size * sizeof(long long));
    struct TwoInts *struct_array = malloc(size * sizeof(struct TwoInts));
    int *src_array = malloc(size * sizeof(int));
    int *dst_array = malloc(size * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        int_array[i] = i * 3;
        ll_array[i] = i * 5LL;
        struct_array[i].a = i;
        struct_array[i].b = i * 2;
        src_array[i] = i * 7;
    }
    
    COMPILER_BARRIER();
    
    /* Run all test patterns */
    checksum += test_char_pointer_sum(size, char_array);
    checksum += test_short_pointer_offset(size, short_array);
    
    test_int_copy_pattern(size, src_array, dst_array);
    for (int i = 0; i < size; i++) {
        checksum += dst_array[i];
    }
    
    checksum += test_mixed_index_pointer(size, int_array);
    checksum += test_struct_pointer_walk(size, struct_array);
    checksum += test_double_pointer_arithmetic(size, int_array);
    checksum += test_volatile_pointer(size, int_array);
    
    long long ll_sum = test_long_long_access(size / 2, ll_array);
    checksum += (int)(ll_sum & 0xFFFFFFFF) + (int)(ll_sum >> 32);
    
    test_pointer_fill(size, dst_array, 42);
    for (int i = 0; i < size; i++) {
        checksum += dst_array[i];
    }
    
    COMPILER_BARRIER();
    
    /* Cleanup */
    free(char_array);
    free(short_array);
    free(int_array);
    free(ll_array);
    free(struct_array);
    free(src_array);
    free(dst_array);
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
