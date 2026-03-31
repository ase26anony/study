/* test_auto_inc_dec.c - Target auto-inc-dec.cc lines 1352-1358 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barriers to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Test functions marked noinline to preserve boundaries */
__attribute__((noinline, noipa))
static int test_pointer_loop_sum(int *array, volatile int n) {
    int sum = 0;
    int *ptr = array;
    int *end = array + n;
    
    /* Simple pointer increment pattern */
    while (ptr < end) {
        sum += *ptr;
        ptr++;  /* This should trigger auto-inc-dec pattern */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline, noipa))
static int test_pointer_with_offset(int *array, volatile int n) {
    int sum = 0;
    int *ptr = array;
    
    /* Access with offset then increment */
    for (int i = 0; i < n; i++) {
        sum += *(ptr + 0);  /* Force XEXP(x, 0) extraction */
        ptr += 1;           /* Increment after access */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline, noipa))
static void test_pointer_copy(int *dst, int *src, volatile int n) {
    int *d = dst;
    int *s = src;
    int *end = src + n;
    
    /* Classic copy pattern that should trigger auto-inc */
    while (s < end) {
        *d = *s;
        d++;
        s++;  /* Two pointers incrementing */
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline, noipa))
static int test_mixed_index_pointer(int *array, volatile int n) {
    int sum = 0;
    int *ptr = array;
    
    /* Mix array indexing and pointer arithmetic */
    for (int i = 0; i < n; i++) {
        sum += ptr[0];      /* Array notation */
        ptr = ptr + 1;      /* Pointer arithmetic - different form */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline, noipa))
static int test_complex_address(int *array, volatile int n) {
    int sum = 0;
    volatile int *vptr = array;  /* Volatile pointer to prevent optimization */
    int *ptr = (int *)vptr;
    
    /* Complex address calculation that should still simplify */
    for (int i = 0; i < n; i++) {
        sum += *(ptr + (i - i));  /* Complex but simplifies to *(ptr + 0) */
        ptr++;  /* Increment after access */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline, noipa))
static int test_multiple_base_registers(int *arr1, int *arr2, volatile int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    /* Use two pointers in address calculation */
    for (int i = 0; i < n; i++) {
        sum += *(p1 + (p2 - p2));  /* p2 - p2 = 0, but requires analysis */
        p1++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Structure for pointer walking */
struct Data {
    int a;
    int b;
    short c;
    char d;
};

__attribute__((noinline, noipa))
static int test_struct_pointer_walk(struct Data *array, volatile int n) {
    int sum = 0;
    struct Data *ptr = array;
    
    /* Walk through struct array with pointer */
    for (int i = 0; i < n; i++) {
        sum += ptr->a + ptr->b;  /* Multiple member accesses */
        ptr++;  /* Increment after last access */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test different access sizes */
__attribute__((noinline, noipa))
static int test_mixed_sizes(volatile int n) {
    char *cptr;
    short *sptr;
    int *iptr;
    long long *llptr;
    
    /* Allocate arrays */
    char carr[256];
    short sarr[128];
    int iarr[64];
    long long llarr[32];
    
    /* Initialize */
    for (int i = 0; i < 256; i++) carr[i] = i % 128;
    for (int i = 0; i < 128; i++) sarr[i] = i;
    for (int i = 0; i < 64; i++) iarr[i] = i * 2;
    for (int i = 0; i < 32; i++) llarr[i] = i * 1000LL;
    
    int sum = 0;
    
    /* Char pointer loop */
    cptr = carr;
    for (int i = 0; i < n && i < 256; i++) {
        sum += *cptr;
        cptr++;
    }
    
    /* Short pointer loop */
    sptr = sarr;
    for (int i = 0; i < n && i < 128; i++) {
        sum += *sptr;
        sptr++;
    }
    
    /* Int pointer loop */
    iptr = iarr;
    for (int i = 0; i < n && i < 64; i++) {
        sum += *iptr;
        iptr++;
    }
    
    /* Long long pointer loop */
    llptr = llarr;
    for (int i = 0; i < n && i < 32; i++) {
        sum += (int)*llptr;
        llptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline, noipa))
static int test_pointer_fill(int *array, volatile int n, int value) {
    int *ptr = array;
    int *end = array + n;
    
    /* Fill pattern with pointer write */
    while (ptr < end) {
        *ptr = value;
        ptr++;  /* Increment after write */
    }
    
    /* Verify by reading back */
    int sum = 0;
    ptr = array;
    while (ptr < end) {
        sum += *ptr;
        ptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    volatile int test_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (test_size <= 0) test_size = 100;
    if (test_size > 1000) test_size = 1000;
    
    /* Allocate test arrays */
    int *array1 = (int *)malloc(test_size * sizeof(int));
    int *array2 = (int *)malloc(test_size * sizeof(int));
    struct Data *struct_array = (struct Data *)malloc(test_size * sizeof(struct Data));
    
    if (!array1 || !array2 || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < test_size; i++) {
        array1[i] = i * 3 + 1;
        array2[i] = 0;
        struct_array[i].a = i;
        struct_array[i].b = i * 2;
        struct_array[i].c = (short)i;
        struct_array[i].d = (char)(i % 128);
    }
    
    int total_sum = 0;
    
    /* Run all tests */
    total_sum += test_pointer_loop_sum(array1, test_size);
    total_sum += test_pointer_with_offset(array1, test_size);
    
    test_pointer_copy(array2, array1, test_size);
    total_sum += test_pointer_loop_sum(array2, test_size);
    
    total_sum += test_mixed_index_pointer(array1, test_size);
    total_sum += test_complex_address(array1, test_size);
    total_sum += test_multiple_base_registers(array1, array2, test_size);
    total_sum += test_struct_pointer_walk(struct_array, test_size);
    total_sum += test_mixed_sizes(test_size / 4);
    total_sum += test_pointer_fill(array2, test_size, 42);
    
    /* Clean up */
    free(array1);
    free(array2);
    free(struct_array);
    
    printf("Total checksum: %d\n", total_sum);
    return 0;
}
