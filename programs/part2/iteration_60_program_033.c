/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement pattern recognition
 * Targets uncovered lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to preserve function boundaries */
#define NOINLINE __attribute__((noinline))

/* Memory barrier to prevent reordering without breaking patterns */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Structure for pointer walking tests */
struct MixedData {
    char c;
    int i;
    short s;
    long long ll;
    char pad[3];
};

/* Volatile variables to prevent compile-time optimization */
static volatile int g_volatile_size = 100;
static volatile int g_volatile_init = 42;

/* Test 1: Simple pointer increment in loop - basic pattern */
NOINLINE static long long test1_simple_pointer_increment(int *array, int size) {
    volatile int *volatile_ptr = &size; /* Force non-constant */
    int loop_size = *volatile_ptr;
    long long sum = 0;
    int *ptr = array;
    
    /* Basic pattern: *ptr then ptr++ */
    for (int i = 0; i < loop_size; i++) {
        sum += *ptr;
        ptr++;  /* Adjacent increment - should trigger pattern */
    }
    
    COMPILER_BARRIER(); /* Outside loop to preserve pattern */
    return sum;
}

/* Test 2: Mixed array indexing and pointer arithmetic */
NOINLINE static long long test2_mixed_index_pointer(int *array, int size) {
    volatile int *volatile_ptr = &size;
    int loop_size = *volatile_ptr;
    long long sum = 0;
    int *ptr = array;
    
    /* Mix indexing and pointer arithmetic */
    for (int i = 0; i < loop_size; i++) {
        /* Access via pointer with offset */
        sum += *(ptr + 0);  /* XEXP should analyze this address */
        
        /* Also use array indexing on same pointer */
        if (i % 2 == 0) {
            sum += ptr[0];  /* Another form of same address */
        }
        
        ptr += 1;  /* Increment after accesses */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 3: Structure pointer walking with multiple members */
NOINLINE static long long test3_struct_pointer_walk(struct MixedData *array, int size) {
    volatile int *volatile_ptr = &size;
    int loop_size = *volatile_ptr;
    long long sum = 0;
    struct MixedData *sp = array;
    
    /* Access multiple structure members then increment pointer */
    for (int i = 0; i < loop_size; i++) {
        sum += sp->c;      /* char access */
        sum += sp->i;      /* int access */
        sum += sp->s;      /* short access */
        sum += sp->ll;     /* long long access */
        
        sp++;  /* Pointer increment after all accesses */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 4: Complex addressing with two pointers */
NOINLINE static long long test4_complex_addressing(int *array1, int *array2, int size) {
    volatile int *volatile_ptr = &size;
    int loop_size = *volatile_ptr;
    long long sum = 0;
    int *p1 = array1;
    int *p2 = array2;
    
    /* Complex addressing that might simplify to base+0 */
    for (int i = 0; i < loop_size; i++) {
        /* Expression that should become *(p1 + 0) after analysis */
        sum += *(p1 + (p2 - p2));  /* p2-p2 = 0, but not constant-folded immediately */
        
        /* Another complex form */
        sum += p1[p2 - p2];  /* Same as above but with index notation */
        
        p1 += 1;
        p2 += 1;  /* Both pointers increment */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 5: Different pointer types and access sizes */
NOINLINE static long long test5_mixed_types(
    char *carray, short *sarray, int *iarray, long long *llarray, int size) {
    
    volatile int *volatile_ptr = &size;
    int loop_size = *volatile_ptr;
    long long sum = 0;
    
    char *cp = carray;
    short *sp = sarray;
    int *ip = iarray;
    long long *llp = llarray;
    
    /* Process each array with its own pointer */
    for (int i = 0; i < loop_size; i++) {
        sum += *cp;   cp++;  /* char access and increment */
        sum += *sp;   sp++;  /* short access and increment */
        sum += *ip;   ip++;  /* int access and increment */
        sum += *llp;  llp++; /* long long access and increment */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 6: Copy between arrays with pointer increments */
NOINLINE static long long test6_pointer_copy(int *src, int *dst, int size) {
    volatile int *volatile_ptr = &size;
    int loop_size = *volatile_ptr;
    long long sum = 0;
    
    /* Classic *dst++ = *src++ pattern */
    for (int i = 0; i < loop_size; i++) {
        *dst = *src;  /* Memory write */
        sum += *dst;  /* Use value for checksum */
        dst++;        /* Increment after write */
        src++;        /* Increment after read */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 7: Volatile pointer to inhibit early optimization */
NOINLINE static long long test7_volatile_pointer(int *array, int size) {
    volatile int *volatile_ptr = &size;
    int loop_size = *volatile_ptr;
    long long sum = 0;
    
    /* Use volatile pointer for address calculation */
    volatile int *vptr = (volatile int *)array;
    
    for (int i = 0; i < loop_size; i++) {
        /* Access through volatile pointer */
        sum += *vptr;
        
        /* Increment volatile pointer - harder to optimize */
        vptr = vptr + 1;  /* This should still be recognized */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 8: Nested loops with pointer reset */
NOINLINE static long long test8_nested_loops(int *array, int size) {
    volatile int *volatile_ptr = &size;
    int outer_size = (*volatile_ptr) / 4;
    long long sum = 0;
    
    for (int j = 0; j < outer_size; j++) {
        int *ptr = array + j * 4;
        
        /* Inner loop with pointer increment */
        for (int i = 0; i < 4; i++) {
            sum += *ptr;
            ptr++;  /* Increment in inner loop */
        }
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    /* Use argc to make sizes non-constant at compile time */
    int base_size = g_volatile_size;
    if (argc > 1) {
        base_size = atoi(argv[1]);
        if (base_size <= 0) base_size = 100;
    }
    
    int test_size = base_size;
    int array_size = test_size * 2; /* Extra space for safety */
    
    /* Allocate and initialize arrays */
    int *int_array1 = (int *)malloc(array_size * sizeof(int));
    int *int_array2 = (int *)malloc(array_size * sizeof(int));
    char *char_array = (char *)malloc(array_size * sizeof(char));
    short *short_array = (short *)malloc(array_size * sizeof(short));
    long long *ll_array = (long long *)malloc(array_size * sizeof(long long));
    struct MixedData *struct_array = (struct MixedData *)malloc(array_size * sizeof(struct MixedData));
    
    /* Initialize with pattern */
    for (int i = 0; i < array_size; i++) {
        int val = (i * 3 + 7) % 256;
        int_array1[i] = val;
        int_array2[i] = val * 2;
        char_array[i] = (char)val;
        short_array[i] = (short)val;
        ll_array[i] = (long long)val * val;
        
        struct_array[i].c = (char)val;
        struct_array[i].i = val;
        struct_array[i].s = (short)val;
        struct_array[i].ll = (long long)val;
    }
    
    long long total_sum = 0;
    
    /* Run all tests */
    total_sum += test1_simple_pointer_increment(int_array1, test_size);
    total_sum += test2_mixed_index_pointer(int_array1, test_size);
    total_sum += test3_struct_pointer_walk(struct_array, test_size);
    total_sum += test4_complex_addressing(int_array1, int_array2, test_size);
    total_sum += test5_mixed_types(char_array, short_array, int_array1, ll_array, test_size);
    total_sum += test6_pointer_copy(int_array1, int_array2, test_size);
    total_sum += test7_volatile_pointer(int_array1, test_size);
    total_sum += test8_nested_loops(int_array1, test_size);
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %lld\n", total_sum);
    
    /* Cleanup */
    free(int_array1);
    free(int_array2);
    free(char_array);
    free(short_array);
    free(ll_array);
    free(struct_array);
    
    return (total_sum != 0) ? 0 : 1;
}
