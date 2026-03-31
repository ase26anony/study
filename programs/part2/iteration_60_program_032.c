/* auto_inc_dec_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barriers to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Test functions marked noinline to preserve boundaries */
__attribute__((noinline))
int test_char_ptr_increment(volatile char *data, int n) {
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
int test_short_ptr_offset(volatile short *data, int n) {
    short *ptr = (short *)data;
    int sum = 0;
    
    /* Access with offset then increment */
    for (int i = 0; i < n; i++) {
        sum += *(ptr + 0);  /* Force XEXP(x, 0) extraction */
        ptr += 1;  /* Increment by stride */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_int_mixed_index_ptr(volatile int *data, int n) {
    int *ptr = (int *)data;
    int sum = 0;
    
    /* Mixed indexing and pointer arithmetic */
    for (int i = 0; i < n; i++) {
        sum += ptr[0];      /* Array indexing form */
        ptr = ptr + 1;      /* Pointer arithmetic form */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_longlong_complex_addr(volatile long long *data, int n) {
    long long *ptr1 = (long long *)data;
    long long *ptr2 = ptr1;
    int sum = 0;
    
    /* Complex addressing expression */
    for (int i = 0; i < n; i++) {
        /* Create complex address: ptr1 + (ptr2 - ptr2) */
        sum += (int)*(ptr1 + (ptr2 - ptr2));
        ptr1++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_ptr_copy(volatile int *src, volatile int *dst, int n) {
    int *s = (int *)src;
    int *d = (int *)dst;
    
    /* Classic copy pattern for auto-inc recognition */
    for (int i = 0; i < n; i++) {
        *d = *s;
        s++;
        d++;
    }
    
    COMPILER_BARRIER();
    return (int)(d - dst);
}

__attribute__((noinline))
int test_ptr_fill(volatile int *data, int value, int n) {
    int *ptr = (int *)data;
    
    /* Fill pattern with write and increment */
    for (int i = 0; i < n; i++) {
        *ptr = value;
        ptr++;
    }
    
    COMPILER_BARRIER();
    return n;
}

/* Structure for pointer walking test */
struct MixedData {
    char c;
    int i;
    short s;
    long long ll;
};

__attribute__((noinline))
int test_struct_ptr_walk(volatile struct MixedData *data, int n) {
    struct MixedData *ptr = (struct MixedData *)data;
    int sum = 0;
    
    /* Access multiple structure members with pointer increment */
    for (int i = 0; i < n; i++) {
        sum += ptr->c + ptr->i + ptr->s + (int)ptr->ll;
        ptr++;  /* Increment after all accesses */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_double_ptr_arithmetic(volatile int *data, int n) {
    int *ptr = (int *)data;
    int sum = 0;
    
    /* Multiple accesses with same pointer between increments */
    for (int i = 0; i < n; i++) {
        sum += *ptr;    /* First access */
        sum += *(ptr);  /* Second access - same address */
        ptr++;          /* Single increment after multiple uses */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_volatile_ptr_chain(volatile int *data1, volatile int *data2, int n) {
    volatile int *p1 = data1;
    volatile int *p2 = data2;
    int sum = 0;
    
    /* Chain of volatile pointer operations */
    for (int i = 0; i < n; i++) {
        sum += *p1 + *p2;
        p1++;
        p2++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    int base_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (base_size < 10) base_size = 100;
    
    /* Allocate arrays of different types */
    char *char_array = malloc(base_size * sizeof(char));
    short *short_array = malloc(base_size * sizeof(short));
    int *int_array = malloc(base_size * sizeof(int));
    long long *ll_array = malloc(base_size * sizeof(long long));
    int *src_array = malloc(base_size * sizeof(int));
    int *dst_array = malloc(base_size * sizeof(int));
    struct MixedData *struct_array = malloc(base_size * sizeof(struct MixedData));
    
    /* Initialize arrays */
    for (int i = 0; i < base_size; i++) {
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        int_array[i] = i * 3;
        ll_array[i] = i * 1000LL;
        src_array[i] = i * 5;
        dst_array[i] = 0;
        struct_array[i].c = (char)i;
        struct_array[i].i = i * 10;
        struct_array[i].s = (short)(i * 20);
        struct_array[i].ll = i * 100LL;
    }
    
    int total_sum = 0;
    
    /* Run all test patterns */
    total_sum += test_char_ptr_increment((volatile char *)char_array, base_size);
    total_sum += test_short_ptr_offset((volatile short *)short_array, base_size);
    total_sum += test_int_mixed_index_ptr((volatile int *)int_array, base_size);
    total_sum += test_longlong_complex_addr((volatile long long *)ll_array, base_size/2);
    total_sum += test_ptr_copy((volatile int *)src_array, (volatile int *)dst_array, base_size);
    total_sum += test_ptr_fill((volatile int *)dst_array, 42, base_size);
    total_sum += test_struct_ptr_walk((volatile struct MixedData *)struct_array, base_size/4);
    total_sum += test_double_ptr_arithmetic((volatile int *)int_array, base_size);
    total_sum += test_volatile_ptr_chain((volatile int *)src_array, (volatile int *)dst_array, base_size);
    
    /* Verify some results */
    int verify_sum = 0;
    for (int i = 0; i < base_size; i++) {
        verify_sum += dst_array[i];
    }
    
    printf("Total checksum: %d (dst array sum: %d)\n", total_sum, verify_sum);
    
    /* Cleanup */
    free(char_array);
    free(short_array);
    free(int_array);
    free(ll_array);
    free(src_array);
    free(dst_array);
    free(struct_array);
    
    return 0;
}
