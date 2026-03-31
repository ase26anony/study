/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-increment/decrement pattern recognition
 * Target: lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to preserve function boundaries */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_PTR(type) volatile type*

/* Compiler barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Test structures for pointer walking */
struct MixedData {
    char c;
    int i;
    short s;
    long long ll;
};

struct SimplePair {
    int a;
    int b;
};

/* Global volatile to prevent constant propagation */
volatile int g_volatile_size = 100;

/* Test 1: Basic pointer increment with mixed access patterns */
NOINLINE long long test_basic_pointer_inc(int* array, int size) {
    VOLATILE_PTR(int) vptr = array;
    int* ptr = (int*)vptr;
    long long sum = 0;
    
    /* Force non-constant loop bound */
    int n = size > 0 ? size : g_volatile_size;
    
    /* Pattern 1: Simple pointer increment */
    for (int i = 0; i < n; i++) {
        sum += *ptr;  /* Memory access via pointer */
        ptr++;        /* Increment immediately after */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 2: Pointer with offset (*(ptr + constant)) */
NOINLINE long long test_pointer_offset(short* array, int size) {
    VOLATILE_PTR(short) vptr = array;
    short* ptr = (short*)vptr;
    long long sum = 0;
    int n = size > 0 ? size : g_volatile_size;
    
    /* Access with offset pattern */
    for (int i = 0; i < n; i += 2) {
        /* Create complex addressing: ptr[1] is *(ptr + 1) */
        sum += ptr[0] + ptr[1];  /* Two offset accesses */
        ptr += 2;                /* Increment by stride */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 3: Structure pointer walking */
NOINLINE long long test_struct_pointer(struct SimplePair* array, int size) {
    VOLATILE_PTR(struct SimplePair) vptr = array;
    struct SimplePair* ptr = (struct SimplePair*)vptr;
    long long sum = 0;
    int n = size > 0 ? size : g_volatile_size;
    
    /* Access multiple structure members */
    for (int i = 0; i < n; i++) {
        sum += ptr->a + ptr->b;  /* Two member accesses */
        ptr++;                   /* Increment after last access */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 4: Mixed index and pointer forms */
NOINLINE long long test_mixed_forms(char* array, int size) {
    VOLATILE_PTR(char) vptr = array;
    char* ptr = (char*)vptr;
    long long sum = 0;
    int n = size > 0 ? size : g_volatile_size;
    
    /* Mix array indexing and pointer arithmetic */
    for (int i = 0; i < n; i++) {
        /* Create addressing: array[i] then ptr++ */
        sum += array[i];  /* Index form */
        sum += *ptr;      /* Pointer form */
        ptr++;            /* Increment pointer */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 5: Copy with dual moving pointers (dst++ = *src++) */
NOINLINE void test_dual_pointers(int* src, int* dst, int size) {
    VOLATILE_PTR(int) vsrc = src;
    VOLATILE_PTR(int) vdst = dst;
    int* s = (int*)vsrc;
    int* d = (int*)vdst;
    int n = size > 0 ? size : g_volatile_size;
    
    /* Classic copy pattern */
    for (int i = 0; i < n; i++) {
        *d = *s;  /* Read from src, write to dst */
        s++;      /* Increment source pointer */
        d++;      /* Increment destination pointer */
    }
    
    COMPILER_BARRIER();
}

/* Test 6: Complex addressing with multiple base registers */
NOINLINE long long test_complex_addressing(long long* array1, long long* array2, int size) {
    VOLATILE_PTR(long long) vptr1 = array1;
    VOLATILE_PTR(long long) vptr2 = array2;
    long long* p1 = (long long*)vptr1;
    long long* p2 = (long long*)vptr2;
    long long sum = 0;
    int n = size > 0 ? size : g_volatile_size;
    
    /* Create complex addressing expressions */
    for (int i = 0; i < n; i++) {
        /* p1[p2 - p2] simplifies to p1[0] but requires analysis */
        long long offset = (p2 - p2);  /* Should be 0, but not constant-folded */
        sum += p1[offset];  /* Complex address: *(p1 + offset) */
        p1++;               /* Increment base pointer */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 7: Mixed data types in structure */
NOINLINE long long test_mixed_struct(struct MixedData* array, int size) {
    VOLATILE_PTR(struct MixedData) vptr = array;
    struct MixedData* ptr = (struct MixedData*)vptr;
    long long sum = 0;
    int n = size > 0 ? size : g_volatile_size;
    
    /* Access all structure members with different sizes */
    for (int i = 0; i < n; i++) {
        sum += ptr->c;   /* char access */
        sum += ptr->i;   /* int access */
        sum += ptr->s;   /* short access */
        sum += ptr->ll;  /* long long access */
        ptr++;           /* Increment after all accesses */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 8: Fill array with pointer write */
NOINLINE void test_fill_array(int* array, int value, int size) {
    VOLATILE_PTR(int) vptr = array;
    int* ptr = (int*)vptr;
    int n = size > 0 ? size : g_volatile_size;
    
    /* Write pattern with pointer increment */
    for (int i = 0; i < n; i++) {
        *ptr = value;  /* Write via pointer */
        ptr++;         /* Increment after write */
    }
    
    COMPILER_BARRIER();
}

/* Main driver */
int main(int argc, char** argv) {
    /* Use argc to make sizes non-constant at compile time */
    int base_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (base_size <= 0) base_size = 100;
    
    /* Allocate arrays of different types */
    int size_int = base_size;
    int size_short = base_size * 2;
    int size_char = base_size * 4;
    int size_ll = base_size;
    int size_struct = base_size / 2;
    
    /* Allocate and initialize */
    int* int_array = (int*)malloc(size_int * sizeof(int));
    short* short_array = (short*)malloc(size_short * sizeof(short));
    char* char_array = (char*)malloc(size_char * sizeof(char));
    long long* ll_array = (long long*)malloc(size_ll * sizeof(long long));
    int* int_array2 = (int*)malloc(size_int * sizeof(int));
    struct SimplePair* pair_array = (struct SimplePair*)malloc(size_struct * sizeof(struct SimplePair));
    struct MixedData* mixed_array = (struct MixedData*)malloc(size_struct * sizeof(struct MixedData));
    
    /* Initialize with pattern */
    for (int i = 0; i < size_int; i++) {
        int_array[i] = i * 3 + 1;
        int_array2[i] = 0;
    }
    for (int i = 0; i < size_short; i++) {
        short_array[i] = (short)(i * 5);
    }
    for (int i = 0; i < size_char; i++) {
        char_array[i] = (char)(i % 256);
    }
    for (int i = 0; i < size_ll; i++) {
        ll_array[i] = i * 7LL;
    }
    for (int i = 0; i < size_struct; i++) {
        pair_array[i].a = i * 2;
        pair_array[i].b = i * 2 + 1;
        mixed_array[i].c = (char)i;
        mixed_array[i].i = i * 3;
        mixed_array[i].s = (short)(i * 4);
        mixed_array[i].ll = i * 5LL;
    }
    
    long long total_sum = 0;
    
    /* Run all tests to trigger different patterns */
    total_sum += test_basic_pointer_inc(int_array, size_int);
    total_sum += test_pointer_offset(short_array, size_short / 2);
    total_sum += test_struct_pointer(pair_array, size_struct);
    total_sum += test_mixed_forms(char_array, size_char);
    
    test_dual_pointers(int_array, int_array2, size_int);
    
    /* Verify copy worked */
    for (int i = 0; i < size_int; i++) {
        total_sum += int_array2[i];
    }
    
    total_sum += test_complex_addressing(ll_array, ll_array, size_ll);
    total_sum += test_mixed_struct(mixed_array, size_struct);
    
    test_fill_array(int_array2, 42, size_int);
    
    /* Verify fill worked */
    for (int i = 0; i < size_int; i++) {
        total_sum += int_array2[i];
    }
    
    printf("Total checksum: %lld\n", total_sum);
    
    /* Cleanup */
    free(int_array);
    free(short_array);
    free(char_array);
    free(ll_array);
    free(int_array2);
    free(pair_array);
    free(mixed_array);
    
    return (int)(total_sum & 0x7FFFFFFF);
}
