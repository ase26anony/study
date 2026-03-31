/* test_resource_patterns.c
 * Designed to generate RTL SET destinations with:
 * 1. ZERO_EXTRACT destinations (bitfield assignments)
 * 2. STRICT_LOW_PART destinations (low-part assignments)
 * 3. SUBREG destinations (sub-register accesses)
 * 4. Complex MEM destinations (non-trivial addressing modes)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void sink(int);
extern void use(void*);

/* Volatile variables to prevent constant propagation */
static volatile int volatile_seed;
static volatile int volatile_index;
static volatile short volatile_short;
static volatile int volatile_int;
static volatile long long volatile_ll;

/* Global variables for memory patterns */
int global_array[100];
struct ComplexStruct {
    int a;
    int b[4];
    long long c;
} global_struct;

/* Prevent inlining and IPA */
__attribute__((noinline, noipa))
void test_zero_extract_pattern(int seed) {
    /* Pattern 1: ZERO_EXTRACT destination via bitfield union */
    union BitfieldUnion {
        unsigned int full;
        struct {
            unsigned int low : 8;
            unsigned int mid : 8;
            unsigned int high : 16;
        } parts;
    } u;
    
    u.full = seed;
    /* This assignment to bitfield may generate ZERO_EXTRACT destination */
    u.parts.mid = (volatile_short & 0xFF);
    
    /* Force the value to be used */
    sink(u.full);
    
    /* Another ZERO_EXTRACT pattern using masking */
    unsigned int mask = 0xFF00;
    unsigned int val = seed;
    /* Store into masked portion - may become ZERO_EXTRACT */
    val = (val & ~mask) | ((volatile_int << 8) & mask);
    sink(val);
}

__attribute__((noinline, noipa))
void test_strict_low_part_pattern(int seed) {
    /* Pattern 2: STRICT_LOW_PART destination via short-to-int assignment */
    int large_val = seed;
    
    /* Assign to low part only - may generate STRICT_LOW_PART */
    *(short*)&large_val = volatile_short;
    sink(large_val);
    
    /* Another pattern: explicit masking of low bits */
    long long big_val = volatile_ll;
    int* low_part = (int*)&big_val;
    /* This modifies only low 32 bits of big_val */
    *low_part = volatile_int;
    sink(big_val);
    
    /* Using small type assignment to larger type */
    char c = volatile_seed & 0xFF;
    int i = seed;
    /* Modify only low 8 bits */
    i = (i & ~0xFF) | (c & 0xFF);
    sink(i);
}

__attribute__((noinline, noipa))
void test_subreg_pattern(int seed) {
    /* Pattern 3: SUBREG destination via type punning */
    long long big_array[2] = {seed, seed * 2};
    
    /* Access sub-register of larger object */
    int* sub_ptr = (int*)&big_array[volatile_index & 1];
    *sub_ptr = volatile_int;
    sink(big_array[0]);
    
    /* Array with sub-word access */
    int array[4] = {seed, seed + 1, seed + 2, seed + 3};
    short* short_ptr = (short*)array;
    /* Access half-word subreg */
    short_ptr[volatile_index & 3] = volatile_short;
    sink(array[volatile_index & 3]);
    
    /* Structure with mixed types */
    struct Mixed {
        long long a;
        int b;
        short c;
    } mix;
    mix.a = seed;
    /* Access subreg of structure member */
    short* c_ptr = (short*)&mix.b;
    *c_ptr = volatile_short;
    sink(mix.b);
}

__attribute__((noinline, noipa))
void test_complex_mem_pattern(int seed) {
    /* Pattern 4: Complex MEM destination with non-trivial addressing */
    
    /* Array with computed index */
    int* ptr = &global_array[volatile_index % 100];
    *ptr = volatile_int;
    sink(*ptr);
    
    /* Structure with computed member access */
    int* struct_ptr = &global_struct.b[volatile_index & 3];
    *struct_ptr = volatile_int;
    sink(*struct_ptr);
    
    /* Pointer arithmetic with scaling */
    char* byte_ptr = (char*)global_array;
    int* aligned_ptr = (int*)(byte_ptr + (volatile_index * 4) % 400);
    *aligned_ptr = volatile_int;
    sink(*aligned_ptr);
    
    /* Two-dimensional access pattern */
    int matrix[10][10];
    for (int i = 0; i < (volatile_seed & 3); i++) {
        for (int j = 0; j < (volatile_seed & 3); j++) {
            /* Complex addressing: matrix[i][j] */
            matrix[i][j] = volatile_int + i + j;
        }
    }
    sink(matrix[0][0]);
}

__attribute__((noinline, noipa))
void test_combined_patterns(int seed) {
    /* Combine multiple patterns in control flow */
    
    if (seed & 1) {
        /* Bitfield in conditional */
        union {
            unsigned int val;
            struct {
                unsigned int a : 4;
                unsigned int b : 4;
                unsigned int c : 24;
            } bits;
        } u;
        u.val = seed;
        u.bits.b = volatile_short & 0xF;
        sink(u.val);
    }
    
    /* Loop with mixed patterns */
    for (int i = 0; i < (volatile_seed & 7); i++) {
        /* Alternate between patterns */
        switch (i & 3) {
            case 0: {
                /* SUBREG in loop */
                long long x = seed + i;
                *(int*)&x = volatile_int + i;
                sink(x);
                break;
            }
            case 1: {
                /* STRICT_LOW_PART in loop */
                int y = seed;
                *(short*)&y = volatile_short + i;
                sink(y);
                break;
            }
            case 2: {
                /* Complex MEM in loop */
                global_array[i] = volatile_int * i;
                sink(global_array[i]);
                break;
            }
            case 3: {
                /* ZERO_EXTRACT in loop */
                unsigned int z = seed;
                unsigned int mask = 0xF0F0;
                z = (z & ~mask) | ((volatile_int << 4) & mask);
                sink(z);
                break;
            }
        }
    }
}

int main(int argc, char** argv) {
    /* Initialize volatile variables */
    volatile_seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    volatile_index = volatile_seed % 50;
    volatile_short = volatile_seed & 0xFFFF;
    volatile_int = volatile_seed;
    volatile_ll = (long long)volatile_seed * volatile_seed;
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i + volatile_seed;
    }
    global_struct.a = volatile_seed;
    for (int i = 0; i < 4; i++) {
        global_struct.b[i] = volatile_seed + i;
    }
    global_struct.c = volatile_ll;
    
    /* Run all pattern tests */
    test_zero_extract_pattern(volatile_seed);
    test_strict_low_part_pattern(volatile_seed + 1);
    test_subreg_pattern(volatile_seed + 2);
    test_complex_mem_pattern(volatile_seed + 3);
    test_combined_patterns(volatile_seed + 4);
    
    /* Create checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += global_array[i];
    }
    checksum += global_struct.a;
    checksum += (int)global_struct.c;
    
    printf("Result: %d\n", checksum);
    return 0;
}

/* Dummy sink function definition to satisfy linker */
void sink(int x) {
    /* Empty - just to prevent optimization */
}

void use(void* p) {
    /* Empty - just to prevent optimization */
}
