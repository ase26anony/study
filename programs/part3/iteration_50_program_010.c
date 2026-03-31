/* auto-inc-dec-test.c
 * 
 * This program is designed to trigger the specific uncovered lines in
 * auto-inc-dec.cc (lines 1352-1358) by creating patterns that encourage
 * the compiler to use auto-increment/decrement addressing modes.
 * 
 * Compilation recommendations:
 *   gcc -O2 -fno-inline -fno-strict-aliasing -fdump-rtl-auto_inc_dec auto-inc-dec-test.c -lm
 *   gcc -O3 -funroll-loops -fno-omit-frame-pointer -fno-schedule-insns auto-inc-dec-test.c -lm
 *   gcc -O1 -da -fdump-rtl-all -fno-expensive-optimizations auto-inc-dec-test.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Mixed data types for alignment testing */
struct Heterogeneous {
    char c;
    short s;
    int i;
    long l;
    float f;
    double d;
};

/* Structure for array traversal */
struct S {
    int a;
    float b;
    double c;
};

/* Helper function marked noinline to create separate optimization context */
__attribute__((noinline)) 
static void process_with_pointers(int* restrict arr_int, float* restrict arr_float, 
                                  struct S* restrict arr_struct, int n) {
    register int* p = arr_int;
    register float* q = arr_float;
    register struct S* r = arr_struct;
    
    /* Pattern 1: Multiple base registers with zero offset */
    for (int i = 0; i < n; i++) {
        /* These should generate [reg + 0] addressing */
        *p = *p * 2;
        *q = *q * 1.5f;
        r->a = r->a + 1;
        
        /* Post-increment operations */
        p++;
        q++;
        r++;
    }
    
    /* Reset pointers and use post-decrement */
    p = &arr_int[n-1];
    q = &arr_float[n-1];
    
    while (p >= arr_int) {
        *p = *p / 2;
        *q = *q / 1.5f;
        p--;  /* Post-decrement */
        q--;  /* Post-decrement */
    }
}

/* Another helper with different patterns */
__attribute__((noinline))
static void mixed_index_pointer_traversal(char* data, int size) {
    /* Mix pointer and index arithmetic */
    char* base = data;
    int idx = 0;
    
    for (idx = 0; idx < size; idx++) {
        /* ptr = &base + idx pattern */
        char* ptr = base + idx;
        *ptr = (char)(*ptr + idx);
    }
    
    /* Now traverse with pure pointer arithmetic */
    char* p = data;
    char* end = data + size;
    while (p < end) {
        *p = *p ^ 0x55;  /* XOR with pattern */
        p++;  /* Should trigger auto-inc opportunity */
    }
}

int main(void) {
    /* Declare arrays of different types and sizes */
    int arr_i[100];
    float arr_f[100];
    double arr_d[100];
    struct S arr_s[50];
    struct Heterogeneous arr_h[25];
    
    /* Initialize with random values */
    srand(42);
    for (int i = 0; i < 100; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
    }
    
    for (int i = 0; i < 50; i++) {
        arr_s[i].a = rand() % 1000;
        arr_s[i].b = (float)(rand() % 1000) / 10.0f;
        arr_s[i].c = (double)(rand() % 1000) / 10.0;
    }
    
    for (int i = 0; i < 25; i++) {
        arr_h[i].c = (char)(rand() % 256);
        arr_h[i].s = (short)(rand() % 1000);
        arr_h[i].i = rand() % 1000;
        arr_h[i].l = rand() % 1000;
        arr_h[i].f = (float)(rand() % 1000) / 10.0f;
        arr_h[i].d = (double)(rand() % 1000) / 10.0;
    }
    
    long long checksum = 0;
    
    /* Pattern 1: for loop with int* and post-increment */
    #pragma GCC unroll 4
    for (register int* p = arr_i; p < &arr_i[100]; ) {
        checksum += *p++;
    }
    
    /* Compiler barrier via math function */
    double barrier = sin(checksum % 100);
    (void)barrier;
    
    /* Pattern 2: while loop with float* and post-decrement */
    float* q = &arr_f[99];
    while (q >= arr_f) {
        checksum += (long long)(*q--);
    }
    
    /* Another barrier */
    barrier = cos(checksum % 100);
    (void)barrier;
    
    /* Pattern 3: Nested loops with struct access and pointer increment */
    for (int outer = 0; outer < 2; outer++) {
        register struct S* ptr = arr_s;
        for (int i = 0; i < 50; i++) {
            /* Access struct fields - should create base + offset patterns */
            checksum += ptr->a;
            checksum += (long long)ptr->b;
            ptr++;  /* Post-increment */
        }
    }
    
    /* Pattern 4: Mixed integer index and pointer arithmetic */
    int* base_ptr = arr_i;
    for (int idx = 0; idx < 100; idx++) {
        /* Compound expression with index modification */
        checksum += arr_i[idx++];  /* Post-increment in array index */
        if (idx < 100) {
            checksum += *(base_ptr + idx);  /* Pointer arithmetic */
        }
    }
    
    /* Pattern 5: Complex loop with pre-decrement */
    int index = 99;
    while (index > 0) {
        /* Pre-decrement in array access */
        checksum += arr_i[--index];
        
        /* Insert inline assembly as compiler barrier */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Pattern 6: Heterogeneous struct traversal */
    struct Heterogeneous* h_ptr = arr_h;
    for (int i = 0; i < 25; i++) {
        /* Access different sized members - tests offset calculations */
        checksum += h_ptr->c;
        checksum += h_ptr->s;
        checksum += h_ptr->i;
        checksum += h_ptr->l;
        checksum += (long long)h_ptr->f;
        checksum += (long long)h_ptr->d;
        h_ptr++;
    }
    
    /* Call helper functions that create additional contexts */
    process_with_pointers(arr_i, arr_f, arr_s, 50);
    
    /* Process char array with mixed patterns */
    char char_data[200];
    for (int i = 0; i < 200; i++) {
        char_data[i] = (char)(i % 256);
    }
    mixed_index_pointer_traversal(char_data, 200);
    
    /* Final checksum computation to prevent dead code elimination */
    for (int i = 0; i < 200; i++) {
        checksum += char_data[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}
