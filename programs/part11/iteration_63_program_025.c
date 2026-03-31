/* test_resource_marking.c - Generate RTL patterns for resource.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_long(long);
extern void use_ptr(void*);
extern void sink(int);

/* Volatile variables to prevent constant propagation */
volatile int volatile_seed;
volatile int volatile_idx;
volatile short volatile_short;
volatile int volatile_int;
volatile long volatile_long;

/* Global variables for memory patterns */
int global_array[100];
struct ComplexStruct {
    int a;
    int b;
    long c;
    short d;
} global_struct;

/* ===== Pattern 1: ZERO_EXTRACT destination ===== */
__attribute__((noinline, noipa))
void test_zero_extract(int seed) {
    /* Using bitfields in unions to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    /* Volatile operations to prevent optimization */
    u.full = seed;
    u.bits.mid = volatile_short & 0xFF;  /* Should generate ZERO_EXTRACT */
    u.bits.high = (volatile_int >> 16) & 0xFFFF;  /* Another ZERO_EXTRACT */
    
    /* Force usage */
    use_int(u.full);
    
    /* Alternative: bitwise operations that mask specific bits */
    unsigned int val = seed;
    unsigned int mask = 0xFF00;
    val = (val & ~mask) | ((volatile_int & 0xFF) << 8);  /* ZERO_EXTRACT pattern */
    sink(val);
}

/* ===== Pattern 2: STRICT_LOW_PART destination ===== */
__attribute__((noinline, noipa))
void test_strict_low_part(int seed) {
    /* Assigning to short portion of int */
    int value = seed;
    short s = volatile_short;
    
    /* This should generate STRICT_LOW_PART */
    *(short*)&value = s;  /* Type punning for low part assignment */
    use_int(value);
    
    /* Alternative: using bitwise operations */
    long long big_val = volatile_long;
    int *p = (int*)&big_val;
    *p = volatile_int;  /* Modifies low part of long long */
    
    /* In a loop to increase chances */
    for (volatile int i = 0; i < (volatile_seed & 3); i++) {
        int temp = i * 100;
        *(short*)&temp = volatile_short;  /* Another STRICT_LOW_PART */
        sink(temp);
    }
}

/* ===== Pattern 3: SUBREG destination ===== */
__attribute__((noinline, noipa))
void test_subreg(int seed) {
    /* Type punning with different sizes */
    long long big_array[4];
    int *int_ptr = (int*)&big_array[volatile_idx & 3];
    
    /* SUBREG pattern: accessing part of larger object */
    *int_ptr = volatile_int;  /* Should generate SUBREG */
    use_long(big_array[0]);
    
    /* Array with sub-word access */
    int array[10];
    short *short_ptr = (short*)array;
    
    /* Complex addressing with volatile index */
    int idx = volatile_idx % 5;
    short_ptr[idx * 2 + 1] = volatile_short;  /* SUBREG in memory access */
    
    /* Structure field access */
    struct Nested {
        long a;
        int b;
        short c;
    } nested;
    
    int *b_ptr = &nested.b;
    *b_ptr = volatile_int;  /* Could generate SUBREG depending on alignment */
    
    /* Use results */
    sink(array[0]);
    sink(nested.b);
}

/* ===== Pattern 4: Complex MEM destination ===== */
__attribute__((noinline, noipa))
void test_complex_mem(int seed) {
    /* Complex addressing modes */
    int *ptr = &global_array[volatile_idx % 50];
    
    /* Pointer arithmetic */
    ptr += (volatile_seed & 7) - 3;
    *ptr = volatile_int;  /* MEM with complex address */
    
    /* Structure with offset */
    struct ComplexStruct local_struct;
    int *field_ptr = &local_struct.a;
    field_ptr += (volatile_idx & 1);  /* Select a or b field */
    *field_ptr = volatile_int;  /* MEM with structure offset */
    
    /* Two-dimensional access */
    int matrix[10][10];
    int (*row_ptr)[10] = &matrix[volatile_idx % 10];
    (*row_ptr)[volatile_seed % 10] = volatile_int;  /* MEM with 2D addressing */
    
    /* Pointer to pointer */
    int **pptr = (int**)&global_array;
    *((int*)((char*)pptr + (volatile_idx * sizeof(int)))) = volatile_int;
    
    /* Use results */
    sink(global_array[0]);
    sink(local_struct.a);
    sink(matrix[0][0]);
}

/* ===== Pattern 5: Combined patterns in loops ===== */
__attribute__((noinline, noipa))
void test_combined_patterns(int seed) {
    int result = 0;
    
    /* Loop with multiple patterns */
    for (int i = 0; i < (volatile_seed & 7) + 1; i++) {
        /* Mix different patterns */
        if (i & 1) {
            /* ZERO_EXTRACT pattern */
            unsigned int val = seed + i;
            val = (val & ~0xFFFF0000) | ((volatile_int & 0xFFFF) << 16);
            result ^= val;
        } else {
            /* STRICT_LOW_PART pattern */
            int temp = result;
            *(short*)&temp = volatile_short + i;
            result = temp;
        }
        
        /* SUBREG pattern every 3 iterations */
        if (i % 3 == 0) {
            long long big = volatile_long;
            int *half = (int*)&big + (i & 1);
            *half = volatile_int + i;
            result += (int)big;
        }
        
        /* Complex MEM pattern */
        global_array[(seed + i) % 50] = result;
    }
    
    sink(result);
}

/* ===== Main test driver ===== */
int main(int argc, char *argv[]) {
    /* Initialize volatile variables */
    volatile_seed = argc > 1 ? atoi(argv[1]) : 12345;
    volatile_idx = volatile_seed % 100;
    volatile_short = (short)(volatile_seed * 37);
    volatile_int = volatile_seed * 7919;
    volatile_long = (long)volatile_seed * 999983;
    
    /* Initialize globals */
    memset(global_array, 0, sizeof(global_array));
    global_struct.a = volatile_seed;
    global_struct.b = volatile_int;
    
    printf("Starting resource pattern tests...\n");
    
    /* Run all pattern tests */
    test_zero_extract(volatile_seed);
    test_strict_low_part(volatile_seed);
    test_subreg(volatile_seed);
    test_complex_mem(volatile_seed);
    test_combined_patterns(volatile_seed);
    
    /* Create checksum of results */
    int checksum = 0;
    for (int i = 0; i < 50; i++) {
        checksum ^= global_array[i];
    }
    checksum ^= global_struct.a;
    checksum ^= global_struct.b;
    
    printf("Checksum: %d\n", checksum);
    printf("Tests completed.\n");
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy definitions to satisfy linker */
void use_int(int x) { volatile int dummy = x; (void)dummy; }
void use_long(long x) { volatile long dummy = x; (void)dummy; }
void use_ptr(void* x) { volatile void* dummy = x; (void)dummy; }
void sink(int x) { volatile int dummy = x; (void)dummy; }
