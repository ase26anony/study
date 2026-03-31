/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-increment/decrement pattern recognition
 * Targeting uncovered lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent reordering without breaking patterns */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Test functions marked noinline to preserve boundaries */
__attribute__((noinline, optimize("no-unroll-loops")))
static int test_pointer_loop_sum(int *arr, volatile int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    /* Pattern: *ptr then ptr++ - should trigger auto-inc recognition */
    while (p < end) {
        sum += *p;      /* Memory access via pointer */
        p++;            /* Pointer increment adjacent to access */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
static int test_mixed_index_pointer(int *arr, volatile int n) {
    int sum = 0;
    int *p = arr;
    
    /* Mixed pattern: array indexing and pointer arithmetic */
    for (int i = 0; i < n; i++) {
        sum += arr[i];      /* Array index access */
        sum += *(p + i);    /* Pointer with offset */
        p = arr;            /* Reset pointer - creates complex flow */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
static void test_pointer_copy(int *dst, int *src, volatile int n) {
    int *d = dst;
    int *s = src;
    int *end = src + n;
    
    /* Classic *dst++ = *src++ pattern */
    while (s < end) {
        *d = *s;    /* Memory access via two pointers */
        d++;        /* Both pointers incremented */
        s++;
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline, optimize("no-unroll-loops")))
static long long test_multiple_sizes(volatile char *cptr, volatile short *sptr, 
                                     volatile int *iptr, volatile long long *llptr, 
                                     volatile int n) {
    long long total = 0;
    volatile char *cp = cptr;
    volatile short *sp = sptr;
    volatile int *ip = iptr;
    volatile long long *lp = llptr;
    
    /* Different access sizes with pointer increments */
    for (int i = 0; i < n; i++) {
        total += *cp;   /* char access */
        cp++;
        
        total += *sp;   /* short access */
        sp++;
        
        total += *ip;   /* int access */
        ip++;
        
        total += *lp;   /* long long access */
        lp++;
    }
    
    COMPILER_BARRIER();
    return total;
}

/* Structure for pointer walking pattern */
struct MixedData {
    int a;
    char b;
    short c;
    int d;
};

__attribute__((noinline, optimize("no-unroll-loops")))
static int test_struct_pointer_walk(struct MixedData *arr, volatile int n) {
    int sum = 0;
    struct MixedData *p = arr;
    
    /* Pointer walking through struct array with multiple member accesses */
    for (int i = 0; i < n; i++) {
        sum += p->a;    /* First member access */
        sum += p->b;    /* Second member */
        sum += p->c;    /* Third member */
        sum += p->d;    /* Fourth member - increment should be adjacent to this */
        p++;            /* Pointer increment after last member access */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
static int test_complex_address_expressions(int *base, volatile int *offset_ptr, volatile int n) {
    int sum = 0;
    volatile int *volatile_ptr = base;
    
    /* Complex addressing: *(ptr + offset - offset) pattern */
    for (int i = 0; i < n; i++) {
        /* This creates XEXP(x, 0) that needs analysis */
        sum += *(volatile_ptr + *offset_ptr - *offset_ptr);
        
        /* Also try pointer with constant offset */
        sum += *(base + i);
        
        /* And pointer array access */
        sum += volatile_ptr[*offset_ptr];
        
        volatile_ptr = base;  /* Reset to create flow complexity */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
static int test_double_pointer_arithmetic(int *arr1, int *arr2, volatile int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    /* Two pointers with arithmetic that could be seen as base+index */
    for (int i = 0; i < n; i++) {
        /* p1[p2 - p2] simplifies but requires analysis */
        sum += p1[p2 - p2];
        
        /* *(p1 + (p2 - arr2)) - more complex */
        sum += *(p1 + (p2 - arr2));
        
        p1++;
        if (i % 2) p2++;  /* Conditional increment for complexity */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static int test_pointer_stride_access(int *arr, volatile int stride, volatile int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n * stride;
    
    /* Pointer with stride increment */
    while (p < end) {
        sum += *p;
        p += stride;  /* Non-unit stride increment */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    volatile int test_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (test_size < 10) test_size = 100;
    
    /* Allocate and initialize arrays of different types */
    int *int_arr1 = (int*)malloc(test_size * sizeof(int));
    int *int_arr2 = (int*)malloc(test_size * sizeof(int));
    char *char_arr = (char*)malloc(test_size * sizeof(char));
    short *short_arr = (short*)malloc(test_size * sizeof(short));
    long long *ll_arr = (long long*)malloc(test_size * sizeof(long long));
    struct MixedData *struct_arr = (struct MixedData*)malloc(test_size * sizeof(struct MixedData));
    
    /* Initialize with pattern */
    for (int i = 0; i < test_size; i++) {
        int_arr1[i] = i * 3 + 1;
        int_arr2[i] = i * 5 + 2;
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i * 7);
        ll_arr[i] = (long long)i * 11;
        struct_arr[i].a = i;
        struct_arr[i].b = (char)(i % 128);
        struct_arr[i].c = (short)(i * 3);
        struct_arr[i].d = i * 2 + 1;
    }
    
    int checksum = 0;
    volatile int offset = 0;
    
    /* Run all test patterns */
    checksum += test_pointer_loop_sum(int_arr1, test_size);
    
    test_pointer_copy(int_arr2, int_arr1, test_size / 2);
    checksum += int_arr2[test_size / 4];
    
    checksum += test_mixed_index_pointer(int_arr1, test_size);
    
    checksum += (int)test_multiple_sizes(char_arr, short_arr, int_arr1, ll_arr, test_size / 4);
    
    checksum += test_struct_pointer_walk(struct_arr, test_size);
    
    checksum += test_complex_address_expressions(int_arr1, &offset, test_size);
    
    checksum += test_double_pointer_arithmetic(int_arr1, int_arr2, test_size);
    
    checksum += test_pointer_stride_access(int_arr1, 2, test_size / 2);
    
    /* Final compiler barrier */
    COMPILER_BARRIER();
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(int_arr1);
    free(int_arr2);
    free(char_arr);
    free(short_arr);
    free(ll_arr);
    free(struct_arr);
    
    return 0;
}
