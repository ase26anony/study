/* auto_inc_dec_test.c - Target coverage for auto-inc-dec.cc lines 1352-1358 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to preserve function boundaries */
#define NOINLINE __attribute__((noinline))

/* Structure for pointer walking tests */
struct MixedData {
    char c;
    int i;
    short s;
    long long ll;
    char pad[3];
};

/* Volatile variables to prevent compile-time optimization */
static volatile int g_volatile_size = 0;
static volatile int g_volatile_init = 42;

/* Test 1: Basic pointer increment in loop - should trigger simple pattern */
NOINLINE static long test1_basic_pointer_increment(int *array, int size) {
    volatile int *volatile_ptr = &g_volatile_init;
    int *ptr = array;
    int *end = array + size;
    long sum = 0;
    
    /* Memory barrier before loop */
    asm volatile ("" : : : "memory");
    
    /* Pattern: *ptr access followed by ptr++ */
    while (ptr < end) {
        sum += *ptr;    /* Memory access */
        ptr++;          /* Pointer increment - should be adjacent */
    }
    
    /* Memory barrier after loop */
    asm volatile ("" : : : "memory");
    
    /* Use volatile to prevent dead code elimination */
    sum += *volatile_ptr;
    return sum;
}

/* Test 2: Mixed array indexing and pointer arithmetic */
NOINLINE static long test2_mixed_index_pointer(int *array, int size) {
    volatile int *volatile_ptr = &g_volatile_init;
    int *ptr = array;
    long sum = 0;
    int i;
    
    asm volatile ("" : : : "memory");
    
    /* Mixed forms: array[i] and ptr++ in same loop */
    for (i = 0; i < size; i++) {
        /* Access via array indexing */
        sum += array[i];
        
        /* Also access via pointer with offset */
        sum += *(ptr + i);
        
        /* Pointer increment - complex pattern for XEXP analysis */
        if (i % 2 == 0) {
            sum += *ptr;
            ptr++;  /* Increment in conditional block */
        }
    }
    
    asm volatile ("" : : : "memory");
    sum += *volatile_ptr;
    return sum;
}

/* Test 3: Structure pointer walking with multiple members */
NOINLINE static long long test3_struct_pointer_walk(struct MixedData *data, int size) {
    volatile int *volatile_ptr = &g_volatile_init;
    struct MixedData *ptr = data;
    struct MixedData *end = data + size;
    long long sum = 0;
    
    asm volatile ("" : : : "memory");
    
    /* Access multiple structure members then increment pointer */
    while (ptr < end) {
        sum += ptr->c;      /* char access */
        sum += ptr->i;      /* int access */
        sum += ptr->s;      /* short access */
        sum += ptr->ll;     /* long long access */
        
        ptr++;  /* Pointer increment after all accesses */
    }
    
    asm volatile ("" : : : "memory");
    sum += *volatile_ptr;
    return sum;
}

/* Test 4: Multiple pointer types with different access sizes */
NOINLINE static long test4_mixed_pointer_types(char *cptr, short *sptr, 
                                               int *iptr, long long *llptr, 
                                               int size) {
    volatile int *volatile_ptr = &g_volatile_init;
    char *cp = cptr;
    short *sp = sptr;
    int *ip = iptr;
    long long *lp = llptr;
    long sum = 0;
    int i;
    
    asm volatile ("" : : : "memory");
    
    /* Different pointer types with their own increments */
    for (i = 0; i < size; i++) {
        /* Char pointer with offset */
        sum += *(cp + i);
        
        /* Short pointer with post-increment */
        sum += *sp;
        sp++;
        
        /* Int pointer with complex addressing */
        sum += ip[i & 1];
        
        /* Long long pointer with conditional increment */
        if (i % 3 == 0) {
            sum += (long)*lp;
            lp++;
        }
    }
    
    asm volatile ("" : : : "memory");
    sum += *volatile_ptr;
    return sum;
}

/* Test 5: Copy with two moving pointers (src++ and dst++) */
NOINLINE static long test5_pointer_copy(int *dst, int *src, int size) {
    volatile int *volatile_ptr = &g_volatile_init;
    int *d = dst;
    int *s = src;
    int *end = src + size;
    long sum = 0;
    
    asm volatile ("" : : : "memory");
    
    /* Classic copy pattern: *dst++ = *src++ */
    while (s < end) {
        *d = *s;    /* Memory write */
        sum += *d;  /* Memory read */
        d++;        /* Destination pointer increment */
        s++;        /* Source pointer increment */
    }
    
    asm volatile ("" : : : "memory");
    sum += *volatile_ptr;
    return sum;
}

/* Test 6: Complex addressing with multiple base registers */
NOINLINE static long test6_complex_addressing(int *array1, int *array2, int size) {
    volatile int *volatile_ptr = &g_volatile_init;
    int *p1 = array1;
    int *p2 = array2;
    long sum = 0;
    int i;
    
    asm volatile ("" : : : "memory");
    
    /* Complex addressing that might simplify to base+index */
    for (i = 0; i < size; i++) {
        /* Expression: *(p1 + p2 - p2) which simplifies to *p1 */
        sum += *(p1 + (p2 - p2));
        
        /* Array indexing with pointer difference */
        sum += p1[p2 - array2];
        
        /* Pointer arithmetic that creates non-trivial XEXP */
        sum += *(p1 + 4);  /* Constant offset */
        
        /* Increment one pointer */
        p1++;
    }
    
    asm volatile ("" : : : "memory");
    sum += *volatile_ptr;
    return sum;
}

/* Test 7: Fill array with pointer write and increment */
NOINLINE static long test7_pointer_fill(int *array, int size, int value) {
    volatile int *volatile_ptr = &g_volatile_init;
    int *ptr = array;
    int *end = array + size;
    long sum = 0;
    
    asm volatile ("" : : : "memory");
    
    /* Fill pattern: *ptr++ = value */
    while (ptr < end) {
        *ptr = value;   /* Memory write */
        sum += *ptr;    /* Memory read */
        ptr++;          /* Pointer increment */
        value++;        /* Change value to prevent optimization */
    }
    
    asm volatile ("" : : : "memory");
    sum += *volatile_ptr;
    return sum;
}

/* Main driver that calls all tests */
int main(int argc, char **argv) {
    /* Use argc for non-constant sizes to prevent unrolling */
    int base_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (base_size < 10) base_size = 100;
    
    /* Different sizes to test various patterns */
    int size1 = base_size;
    int size2 = base_size / 2;
    int size3 = base_size / 4;
    
    /* Allocate arrays of different types */
    int *int_array1 = (int*)malloc(size1 * sizeof(int));
    int *int_array2 = (int*)malloc(size1 * sizeof(int));
    char *char_array = (char*)malloc(size1 * sizeof(char));
    short *short_array = (short*)malloc(size1 * sizeof(short));
    long long *ll_array = (long long*)malloc(size1 * sizeof(long long));
    struct MixedData *struct_array = (struct MixedData*)malloc(size3 * sizeof(struct MixedData));
    
    if (!int_array1 || !int_array2 || !char_array || !short_array || 
        !ll_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < size1; i++) {
        int_array1[i] = i * 3 + 1;
        int_array2[i] = i * 5 + 2;
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 7);
        ll_array[i] = (long long)i * 11;
    }
    
    for (int i = 0; i < size3; i++) {
        struct_array[i].c = (char)(i % 128);
        struct_array[i].i = i * 13;
        struct_array[i].s = (short)(i * 17);
        struct_array[i].ll = (long long)i * 19;
    }
    
    long total = 0;
    
    /* Run all tests in sequence */
    total += test1_basic_pointer_increment(int_array1, size1);
    total += test2_mixed_index_pointer(int_array2, size2);
    total += test3_struct_pointer_walk(struct_array, size3);
    total += test4_mixed_pointer_types(char_array, short_array, 
                                       int_array1, ll_array, size2);
    total += test5_pointer_copy(int_array2, int_array1, size2);
    total += test6_complex_addressing(int_array1, int_array2, size2);
    total += test7_pointer_fill(int_array1, size2, g_volatile_init);
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %ld\n", total);
    
    /* Cleanup */
    free(int_array1);
    free(int_array2);
    free(char_array);
    free(short_array);
    free(ll_array);
    free(struct_array);
    
    return 0;
}
