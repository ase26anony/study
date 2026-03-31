/* test_auto_inc_dec.c - Target GCC's auto-increment/decrement pattern matcher */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to preserve function boundaries */
#define NOINLINE __attribute__((noinline))

/* Compiler barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Structure for pointer walking */
struct MixedData {
    char c;
    int i;
    short s;
    long long ll;
    char pad[3];
};

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_size = 0;
static volatile int g_volatile_seed = 12345;

/* Test 1: Basic pointer increment with mixed access patterns */
NOINLINE static long long test1_basic_pointer_inc(int size, int *array) {
    int *ptr = array;
    int *end = array + size;
    long long sum = 0;
    
    /* Simple pointer increment pattern */
    for (; ptr < end; ) {
        sum += *ptr;
        ptr++;  /* This should trigger auto-inc pattern */
    }
    
    /* Mixed pattern: array indexing and pointer arithmetic */
    ptr = array;
    for (int i = 0; i < size; i++) {
        sum += ptr[i];      /* Array indexing form */
        if (i % 2 == 0) {
            sum += *(ptr + i);  /* Pointer + offset form */
        }
    }
    
    return sum;
}

/* Test 2: Two-pointer copy with complex addressing */
NOINLINE static long long test2_pointer_copy(int size, int *src, int *dst) {
    int *s = src;
    int *d = dst;
    int *end = src + size;
    long long sum = 0;
    
    /* Classic *dst++ = *src++ pattern */
    for (; s < end; ) {
        *d = *s;
        sum += *d;
        s++;
        d++;
    }
    
    /* More complex: pointer with constant offset */
    s = src;
    d = dst;
    for (int i = 0; i < size; i += 2) {
        *(d + 1) = *(s + 1);  /* Offset addressing */
        sum += d[1];
        s += 2;
        d += 2;
    }
    
    return sum;
}

/* Test 3: Structure pointer walking */
NOINLINE static long long test3_struct_pointer(int size, struct MixedData *data) {
    struct MixedData *ptr = data;
    struct MixedData *end = data + size;
    long long sum = 0;
    
    /* Access multiple structure members with pointer increment */
    for (; ptr < end; ) {
        sum += ptr->c;
        sum += ptr->i;
        sum += ptr->s;
        sum += ptr->ll;
        ptr++;  /* Increment after all accesses */
    }
    
    /* Alternative: pointer arithmetic in the middle */
    ptr = data;
    for (int i = 0; i < size; i++) {
        sum += (ptr + i)->i;  /* Complex address expression */
        if (i % 3 == 0) {
            sum += ptr[i].ll;  /* Array indexing on struct pointer */
        }
    }
    
    return sum;
}

/* Test 4: Mixed pointer types and sizes */
NOINLINE static long long test4_mixed_types(int size, void *base) {
    char *cptr = (char *)base;
    short *sptr = (short *)base;
    int *iptr = (int *)base;
    long long *llptr = (long long *)base;
    
    long long sum = 0;
    int char_count = size * sizeof(struct MixedData);
    int short_count = char_count / sizeof(short);
    int int_count = char_count / sizeof(int);
    int ll_count = char_count / sizeof(long long);
    
    /* Different pointer types with increment */
    for (int i = 0; i < int_count; i++) {
        sum += *iptr;
        iptr++;  /* int pointer increment */
    }
    
    for (int i = 0; i < short_count; i++) {
        sum += *sptr;
        sptr++;  /* short pointer increment */
    }
    
    /* Volatile pointer to prevent optimization */
    volatile char *vcptr = cptr;
    for (int i = 0; i < char_count; i++) {
        sum += *vcptr;
        vcptr++;  /* volatile char pointer increment */
    }
    
    return sum;
}

/* Test 5: Complex addressing with multiple base registers */
NOINLINE static long long test5_complex_addressing(int size, int *arr1, int *arr2) {
    int *p1 = arr1;
    int *p2 = arr2;
    long long sum = 0;
    
    /* Expression that could be seen as base-plus-index */
    for (int i = 0; i < size; i++) {
        /* Complex address: *(p1 + (p2 - p2) + i) simplifies but requires analysis */
        sum += *(p1 + i);
        sum += p2[i];
        
        /* Force address computation */
        if (i % 4 == 0) {
            sum += *(p1 + (p2 - p2 + i));  /* p2 - p2 = 0, but not obvious early */
        }
    }
    
    /* Pointer walking with stride */
    p1 = arr1;
    int stride = 3;
    for (int i = 0; i < size; i += stride) {
        sum += *p1;
        p1 += stride;  /* Non-unity increment */
    }
    
    return sum;
}

/* Test 6: Pointer increment in nested loops */
NOINLINE static long long test6_nested_loops(int size, int *array) {
    int *ptr = array;
    long long sum = 0;
    int chunk = 16;
    
    for (int outer = 0; outer < size; outer += chunk) {
        int *chunk_end = ptr + (chunk < size - outer ? chunk : size - outer);
        
        /* Inner loop with pointer increment */
        for (; ptr < chunk_end; ) {
            sum += *ptr;
            ptr++;  /* Increment in inner loop */
        }
        
        COMPILER_BARRIER(); /* Prevent reordering across chunks */
    }
    
    return sum;
}

/* Test 7: Write pattern with pointer increment */
NOINLINE static long long test7_write_pattern(int size, int *array) {
    int *ptr = array;
    int *end = array + size;
    long long sum = 0;
    volatile int write_value = g_volatile_seed;
    
    /* Fill array with incrementing pointer */
    for (; ptr < end; ) {
        *ptr = write_value++;
        ptr++;  /* Write with post-increment */
    }
    
    /* Verify the writes */
    ptr = array;
    for (int i = 0; i < size; i++) {
        sum += ptr[i];
    }
    
    return sum;
}

/* Main driver */
int main(int argc, char **argv) {
    /* Use argc to make size non-constant */
    int base_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (base_size < 10) base_size = 100;
    
    /* Make size volatile to prevent constant propagation */
    g_volatile_size = base_size;
    int size = g_volatile_size;
    
    /* Allocate arrays of different types */
    int *int_array1 = (int *)malloc(size * sizeof(int));
    int *int_array2 = (int *)malloc(size * sizeof(int));
    struct MixedData *struct_array = (struct MixedData *)malloc(size * sizeof(struct MixedData));
    char *char_buffer = (char *)malloc(size * sizeof(struct MixedData));
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        int_array1[i] = i * 3 + 1;
        int_array2[i] = i * 5 + 2;
        struct_array[i].c = i & 0xFF;
        struct_array[i].i = i * 7;
        struct_array[i].s = i * 11;
        struct_array[i].ll = (long long)i * 13;
    }
    memset(char_buffer, 0xAA, size * sizeof(struct MixedData));
    
    long long total_sum = 0;
    
    /* Run all tests */
    COMPILER_BARRIER();
    total_sum += test1_basic_pointer_inc(size, int_array1);
    COMPILER_BARRIER();
    total_sum += test2_pointer_copy(size, int_array1, int_array2);
    COMPILER_BARRIER();
    total_sum += test3_struct_pointer(size, struct_array);
    COMPILER_BARRIER();
    total_sum += test4_mixed_types(size, char_buffer);
    COMPILER_BARRIER();
    total_sum += test5_complex_addressing(size, int_array1, int_array2);
    COMPILER_BARRIER();
    total_sum += test6_nested_loops(size, int_array1);
    COMPILER_BARRIER();
    total_sum += test7_write_pattern(size, int_array2);
    COMPILER_BARRIER();
    
    /* Print result to ensure side effects are observable */
    printf("Checksum: %lld\n", total_sum);
    
    /* Cleanup */
    free(int_array1);
    free(int_array2);
    free(struct_array);
    free(char_buffer);
    
    return (total_sum != 0) ? 0 : 1;
}
