/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-increment/decrement pattern recognition
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

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
};

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_size = 0;

/* Test 1: Simple pointer increment in loop */
NOINLINE static long long test_simple_inc(int *array, int size) {
    volatile int *volatile_ptr = array; /* Volatile pointer to prevent optimization */
    int *ptr = (int *)volatile_ptr;
    long long sum = 0;
    
    /* Force address calculation to be non-trivial */
    int *base_ptr = ptr;
    
    for (int i = 0; i < size; i++) {
        /* Access via pointer with potential offset calculation */
        sum += *(base_ptr + i);
        /* This should create: MEM[(int *)volatile_ptr + i], want to match auto-inc */
    }
    
    /* Now use explicit pointer increment pattern */
    int *p = array;
    for (int i = 0; i < size; ) {
        sum += *p;  /* Memory access */
        p++;        /* Pointer increment - should trigger auto-inc pattern */
        i++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 2: Mixed array indexing and pointer arithmetic */
NOINLINE static long long test_mixed_indexing(short *array, int size) {
    long long sum = 0;
    volatile short *vol_ptr = array;
    short *ptr = (short *)vol_ptr;
    
    /* Complex addressing: ptr[size - i - 1] creates non-trivial address */
    for (int i = 0; i < size; i++) {
        sum += ptr[size - i - 1];  /* Complex index calculation */
    }
    
    /* Switch to pointer increment pattern */
    short *p = array;
    for (int i = 0; i < size; ) {
        sum += *p;  /* Access memory */
        p += 1;     /* Increment with constant - should match auto-inc */
        i++;
    }
    
    /* Another pattern: pointer with constant offset */
    short *q = array;
    for (int i = 0; i < size - 1; i++) {
        sum += *(q + 1);  /* Offset access */
        q++;              /* Pointer increment */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 3: Structure access with pointer walking */
NOINLINE static long long test_struct_access(struct MixedData *array, int size) {
    long long sum = 0;
    volatile struct MixedData *vol_struct = array;
    struct MixedData *sp = (struct MixedData *)vol_struct;
    
    /* Access multiple structure members with same base pointer */
    for (int i = 0; i < size; ) {
        sum += sp->c + sp->i + sp->s;  /* Multiple memory accesses */
        sp++;                           /* Pointer increment - should trigger pattern */
        i++;
    }
    
    /* Alternative: access with offset */
    struct MixedData *sp2 = array;
    for (int i = 0; i < size; i++) {
        /* Create complex address expression */
        sum += (sp2 + i)->ll;  /* Base plus index */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 4: Copy between arrays using two moving pointers */
NOINLINE static long long test_copy_pattern(char *src, char *dst, int size) {
    volatile char *v_src = src;
    volatile char *v_dst = dst;
    char *s = (char *)v_src;
    char *d = (char *)v_dst;
    
    /* Classic *dst++ = *src++ pattern */
    for (int i = 0; i < size; ) {
        *d = *s;    /* Memory read and write */
        d++;        /* Both pointers increment */
        s++;
        i++;
    }
    
    /* Mixed pattern with stride */
    s = src;
    d = dst;
    for (int i = 0; i < size - 3; i += 2) {
        *(d + 2) = *(s + 2);  /* Offset access */
        d += 2;               /* Pointer increment by constant */
        s += 2;
    }
    
    COMPILER_BARRIER();
    
    /* Compute checksum */
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test 5: Multiple pointer types and access sizes */
NOINLINE static long long test_mixed_types(void *buffer, int size) {
    char *cptr = (char *)buffer;
    short *sptr = (short *)buffer;
    int *iptr = (int *)buffer;
    long long *llptr = (long long *)buffer;
    
    long long sum = 0;
    int elem_count = size / 8;  /* Use largest type size */
    
    /* Access with different pointer types in sequence */
    for (int i = 0; i < elem_count; ) {
        sum += *cptr;   /* char access */
        cptr++;
        
        if (i < elem_count - 1) {
            sum += *sptr;  /* short access */
            sptr += 1;
        }
        
        if (i < elem_count - 2) {
            sum += *iptr;  /* int access */
            iptr++;
        }
        
        if (i < elem_count - 3) {
            sum += *llptr; /* long long access */
            llptr++;
        }
        
        i++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 6: Complex addressing with multiple base registers */
NOINLINE static long long test_complex_addressing(int *array1, int *array2, int size) {
    long long sum = 0;
    volatile int *v1 = array1;
    volatile int *v2 = array2;
    int *p1 = (int *)v1;
    int *p2 = (int *)v2;
    
    /* Create expression that might simplify to base + index */
    for (int i = 0; i < size; i++) {
        /* Complex address: p1 + (p2 - p2) + i */
        sum += *(p1 + (p2 - p2) + i);  /* Should simplify but require analysis */
    }
    
    /* Pointer arithmetic that could be auto-inc */
    int *ptr = array1;
    for (int i = 0; i < size; ) {
        sum += *ptr;
        ptr = ptr + 1;  /* Alternative form of increment */
        i++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Main function with non-constant loop bounds */
int main(int argc, char **argv) {
    /* Use argc to make sizes non-constant at compile time */
    int base_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (base_size < 10) base_size = 100;
    
    /* Make size volatile to prevent optimization */
    g_volatile_size = base_size;
    int size = g_volatile_size;
    
    /* Allocate and initialize arrays */
    int *int_array = (int *)malloc(size * sizeof(int));
    short *short_array = (short *)malloc(size * sizeof(short));
    char *char_array1 = (char *)malloc(size * sizeof(char));
    char *char_array2 = (char *)malloc(size * sizeof(char));
    struct MixedData *struct_array = (struct MixedData *)malloc(size * sizeof(struct MixedData));
    void *mixed_buffer = malloc(size * 8);
    
    if (!int_array || !short_array || !char_array1 || !char_array2 || 
        !struct_array || !mixed_buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        int_array[i] = i * 3 + 1;
        short_array[i] = (short)(i * 5 + 2);
        char_array1[i] = (char)(i % 256);
        char_array2[i] = 0;
        
        struct_array[i].c = (char)(i % 128);
        struct_array[i].i = i * 7 + 3;
        struct_array[i].s = (short)(i * 11 + 5);
        struct_array[i].ll = (long long)i * 13 + 7;
    }
    
    memset(mixed_buffer, 0xAA, size * 8);
    
    long long total_sum = 0;
    
    /* Run tests in sequence */
    total_sum += test_simple_inc(int_array, size);
    COMPILER_BARRIER();
    
    total_sum += test_mixed_indexing(short_array, size);
    COMPILER_BARRIER();
    
    total_sum += test_struct_access(struct_array, size);
    COMPILER_BARRIER();
    
    total_sum += test_copy_pattern(char_array1, char_array2, size);
    COMPILER_BARRIER();
    
    total_sum += test_mixed_types(mixed_buffer, size);
    COMPILER_BARRIER();
    
    total_sum += test_complex_addressing(int_array, int_array + size/2, size/2);
    COMPILER_BARRIER();
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %lld\n", total_sum);
    
    /* Cleanup */
    free(int_array);
    free(short_array);
    free(char_array1);
    free(char_array2);
    free(struct_array);
    free(mixed_buffer);
    
    return 0;
}
