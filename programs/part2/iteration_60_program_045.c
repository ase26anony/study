/* test_auto_inc_dec.c - Target auto-inc-dec.cc lines 1352-1358 */

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
    char pad[4];
};

/* Test 1: Simple pointer increment in loop */
NOINLINE long long test_pointer_increment(int *array, int n) {
    volatile int *volatile_ptr = array; /* Volatile to prevent constant folding */
    int *ptr = (int *)volatile_ptr;
    long long sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access via pointer with potential offset */
        sum += *ptr;
        /* Increment immediately after access - target pattern */
        ptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 2: Mixed array indexing and pointer arithmetic */
NOINLINE long long test_mixed_access(short *array, int n) {
    volatile short *volatile_base = array;
    short *base = (short *)volatile_base;
    short *ptr = base;
    long long sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing: base[i] and ptr access */
        sum += base[i] + *ptr;
        /* Pointer increment adjacent to access */
        ptr++;
        
        /* Additional offset access */
        if (i % 2 == 0) {
            sum += *(ptr + 1);  /* Offset from current ptr */
        }
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 3: Structure pointer walking */
NOINLINE long long test_struct_walk(struct MixedData *array, int n) {
    volatile struct MixedData *volatile_base = array;
    struct MixedData *ptr = (struct MixedData *)volatile_base;
    long long sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access multiple structure members */
        sum += ptr->c + ptr->i + ptr->s;
        /* Critical: increment after last member access */
        ptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 4: Dual pointer copy with complex addressing */
NOINLINE long long test_dual_pointers(char *src, char *dst, int n) {
    volatile char *volatile_src = src;
    volatile char *volatile_dst = dst;
    char *s = (char *)volatile_src;
    char *d = (char *)volatile_dst;
    long long sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Classic *dst++ = *src++ pattern */
        *d = *s;
        sum += *d;
        
        /* Increment both pointers - GCC should recognize this */
        d++;
        s++;
        
        /* Additional offset access to create complex addressing */
        if (i % 3 == 0) {
            sum += *(s - 1) + *(d - 1);
        }
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 5: Multiple pointer types with different sizes */
NOINLINE long long test_mixed_types(void *buffer, int n) {
    volatile char *volatile_buf = (volatile char *)buffer;
    char *cptr = (char *)volatile_buf;
    short *sptr = (short *)(cptr + 64);
    int *iptr = (int *)(cptr + 128);
    long long *llptr = (long long *)(cptr + 256);
    
    long long sum = 0;
    
    /* Process each type with pointer arithmetic */
    for (int i = 0; i < n && i < 16; i++) {
        sum += *cptr;
        cptr++;  /* char pointer increment */
        
        sum += *sptr;
        sptr++;  /* short pointer increment */
        
        sum += *iptr;
        iptr++;  /* int pointer increment */
        
        sum += *llptr;
        llptr++;  /* long long pointer increment */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 6: Complex pointer arithmetic that may simplify */
NOINLINE long long test_complex_address(int *array1, int *array2, int n) {
    volatile int *volatile_p1 = array1;
    volatile int *volatile_p2 = array2;
    int *p1 = (int *)volatile_p1;
    int *p2 = (int *)volatile_p2;
    long long sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing expression: p1 + (p2 - p2) */
        int *addr = p1 + (p2 - p2);  /* Should simplify to just p1 */
        sum += *addr;
        
        /* Multiple increments to create pattern */
        p1++;
        if (i % 2 == 0) {
            p2++;  /* p2 changes, making (p2 - p2) not trivially constant */
        }
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 7: Pointer with stride (not 1) */
NOINLINE long long test_pointer_stride(int *array, int n, int stride) {
    volatile int *volatile_base = array;
    int *ptr = (int *)volatile_base;
    long long sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        /* Stride may be recognized as constant offset */
        ptr += stride;  /* Could be auto-increment with larger step */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Main driver */
int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    int base_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (base_size < 10) base_size = 100;
    
    /* Allocate and initialize arrays */
    int int_array[200];
    short short_array[200];
    char char_array_src[200], char_array_dst[200];
    struct MixedData struct_array[50];
    void *mixed_buffer = malloc(512);
    
    /* Initialize with pattern */
    for (int i = 0; i < 200; i++) {
        int_array[i] = i * 3 + 1;
        short_array[i] = i * 2;
        char_array_src[i] = i % 128;
        if (i < 50) {
            struct_array[i].c = i;
            struct_array[i].i = i * 100;
            struct_array[i].s = i * 2;
            struct_array[i].ll = (long long)i * 1000;
        }
    }
    
    memset(mixed_buffer, 0xAA, 512);
    
    long long total = 0;
    
    /* Run all tests */
    total += test_pointer_increment(int_array, base_size % 150);
    total += test_mixed_access(short_array, base_size % 150);
    total += test_struct_walk(struct_array, base_size % 40);
    total += test_dual_pointers(char_array_src, char_array_dst, base_size % 150);
    total += test_mixed_types(mixed_buffer, base_size % 20);
    total += test_complex_address(int_array, int_array + 50, base_size % 100);
    total += test_pointer_stride(int_array, base_size % 100, 2);
    
    printf("Checksum: %lld\n", total);
    
    free(mixed_buffer);
    return (total > 0) ? 0 : 1;
}
