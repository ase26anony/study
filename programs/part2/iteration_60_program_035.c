/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-increment/decrement pattern recognition
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to preserve function boundaries */
#define NOINLINE __attribute__((noinline))

/* Structure for complex access patterns */
struct MixedData {
    char c;
    int i;
    short s;
    long long ll;
    char pad[3];
};

/* Volatile variables to prevent compile-time optimization */
static volatile int g_volatile_size = 0;
static volatile int g_volatile_init = 1;

/* Test 1: Basic pointer increment with mixed access patterns */
NOINLINE static long long test1_basic_pointer_inc(int *array, int size) {
    volatile int *volatile_ptr = array; /* Volatile pointer to prevent simplification */
    int *ptr = (int *)volatile_ptr;
    long long sum = 0;
    
    /* Simple pointer increment pattern */
    for (int i = 0; i < size; i++) {
        sum += *ptr;    /* Memory access */
        ptr++;          /* Pointer increment - should trigger auto-inc pattern */
    }
    
    /* Mixed pattern: array indexing and pointer arithmetic */
    ptr = (int *)volatile_ptr;
    for (int i = 0; i < size; i++) {
        sum += ptr[i];  /* Array indexing form */
        if (i % 2 == 0) {
            ptr += 1;   /* Pointer arithmetic nearby */
        }
    }
    
    return sum;
}

/* Test 2: Complex addressing with pointer + offset */
NOINLINE static long long test2_complex_addressing(short *array, int size) {
    volatile short *volatile_base = array;
    short *base = (short *)volatile_base;
    short *ptr = base;
    long long sum = 0;
    
    /* Access with pointer + constant offset */
    for (int i = 0; i < size - 4; i++) {
        /* Multiple addressing forms that should analyze XEXP(x, 0) */
        sum += *(ptr + 2);      /* Pointer + constant offset */
        sum += ptr[3];          /* Array indexing */
        sum += *ptr;            /* Direct pointer dereference */
        ptr += 1;               /* Increment - adjacent to access */
    }
    
    /* Two-pointer arithmetic that could create base+index addressing */
    short *p1 = base;
    short *p2 = base + size/2;
    for (int i = 0; i < size/2; i++) {
        /* Complex expression that might simplify to base addressing */
        sum += p1[(p2 - p2) + i];  /* Should become p1[i] but requires analysis */
        p1 += 1;                    /* Increment after access */
    }
    
    return sum;
}

/* Test 3: Structure access with pointer walking */
NOINLINE static long long test3_struct_access(struct MixedData *array, int size) {
    volatile struct MixedData *volatile_base = array;
    struct MixedData *ptr = (struct MixedData *)volatile_base;
    long long sum = 0;
    
    /* Access structure members with pointer increment */
    for (int i = 0; i < size; i++) {
        sum += ptr->c;      /* char access */
        sum += ptr->i;      /* int access */
        sum += ptr->s;      /* short access */
        sum += ptr->ll;     /* long long access */
        ptr++;              /* Pointer increment - should trigger pattern */
    }
    
    /* Alternative: pointer arithmetic with stride */
    ptr = (struct MixedData *)volatile_base;
    for (int i = 0; i < size; i += 2) {
        sum += ptr->i;
        ptr += 2;           /* Stride of 2 structures */
    }
    
    return sum;
}

/* Test 4: Copy between arrays with dual pointers */
NOINLINE static long long test4_dual_pointer_copy(char *src, char *dst, int size) {
    volatile char *volatile_src = src;
    volatile char *volatile_dst = dst;
    char *s = (char *)volatile_src;
    char *d = (char *)volatile_dst;
    
    /* Classic *dst++ = *src++ pattern */
    for (int i = 0; i < size; i++) {
        *d = *s;    /* Memory write */
        d++;        /* Destination pointer increment */
        s++;        /* Source pointer increment */
    }
    
    /* Compute checksum */
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* Test 5: Mixed types and access sizes */
NOINLINE static long long test5_mixed_types(void *buffer, int size) {
    char *cptr = (char *)buffer;
    short *sptr = (short *)buffer;
    int *iptr = (int *)buffer;
    long long *llptr = (long long *)buffer;
    long long sum = 0;
    
    int elem_count = size / sizeof(long long);
    
    /* Access same memory with different type pointers */
    for (int i = 0; i < elem_count; i++) {
        sum += *cptr;   /* char access */
        cptr += sizeof(int);  /* Skip ahead */
        
        sum += *sptr;   /* short access */
        sptr += 1;
        
        sum += *iptr;   /* int access */
        iptr += 1;
        
        sum += *llptr;  /* long long access */
        llptr += 1;
    }
    
    return sum;
}

/* Test 6: Pointer arithmetic with volatile to inhibit optimization */
NOINLINE static long long test6_volatile_pointer(int *array, int size) {
    /* Volatile pointer prevents constant folding of address */
    volatile int *volatile_ptr = array;
    int *ptr;
    
    /* Force reload from volatile */
    asm volatile("" : "=r"(ptr) : "0"(volatile_ptr) : "memory");
    
    long long sum = 0;
    int *end = ptr + size;
    
    /* While loop with pointer comparison */
    while (ptr < end) {
        sum += *ptr;    /* Memory access */
        ptr++;          /* Pointer increment */
        
        /* Compiler barrier outside loop to prevent reordering but preserve pattern */
        if ((ptr - array) % 64 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    return sum;
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    int base_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (base_size < 10) base_size = 100;
    
    /* Add volatile component to prevent constant propagation */
    int size = base_size + g_volatile_size;
    
    /* Allocate and initialize arrays */
    int *int_array = malloc(size * sizeof(int));
    short *short_array = malloc(size * sizeof(short));
    char *char_array1 = malloc(size * sizeof(char));
    char *char_array2 = malloc(size * sizeof(char));
    struct MixedData *struct_array = malloc(size * sizeof(struct MixedData));
    void *mixed_buffer = malloc(size * sizeof(long long));
    
    if (!int_array || !short_array || !char_array1 || !char_array2 || 
        !struct_array || !mixed_buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        int_array[i] = i * g_volatile_init;
        short_array[i] = (short)(i * 3);
        char_array1[i] = (char)(i % 256);
        char_array2[i] = 0;
        
        struct_array[i].c = (char)i;
        struct_array[i].i = i * 2;
        struct_array[i].s = (short)(i * 4);
        struct_array[i].ll = (long long)i * i;
    }
    
    memset(mixed_buffer, 0xAA, size * sizeof(long long));
    
    long long total_sum = 0;
    
    /* Run all tests */
    total_sum += test1_basic_pointer_inc(int_array, size);
    total_sum += test2_complex_addressing(short_array, size);
    total_sum += test3_struct_access(struct_array, size);
    total_sum += test4_dual_pointer_copy(char_array1, char_array2, size);
    total_sum += test5_mixed_types(mixed_buffer, size);
    total_sum += test6_volatile_pointer(int_array, size);
    
    /* Final compiler barrier */
    asm volatile("" : : : "memory");
    
    printf("Result: %lld\n", total_sum);
    
    /* Cleanup */
    free(int_array);
    free(short_array);
    free(char_array1);
    free(char_array2);
    free(struct_array);
    free(mixed_buffer);
    
    return 0;
}
