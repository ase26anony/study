/* test_resource_coverage.c
 * Generates RTL patterns to cover specific SET destination cases in GCC's resource.cc
 * Compile with: gcc -O2 -fno-strict-aliasing -c test.c -o test.o
 * Or for 32-bit: gcc -O2 -m32 -fno-strict-aliasing -c test.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_long(long);
extern void use_ptr(void*);
extern void sink(int);

/* Volatile variables to prevent constant propagation */
static volatile int volatile_seed;
static volatile short volatile_short;
static volatile int volatile_int;
static volatile long volatile_long;
static volatile int volatile_idx;

/* Global variables for memory patterns */
int global_array[100];
struct ComplexStruct {
    int a;
    long b;
    short c;
    int d;
} global_struct;

/* ========== Pattern 1: ZERO_EXTRACT destination ========== */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int seed) {
    /* Using union with bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    u.full = seed;
    
    /* Store into specific bitfield - may generate ZERO_EXTRACT destination */
    u.bits.mid = (volatile_short & 0xFF);
    u.bits.high = (volatile_int & 0xFFFF);
    
    /* Complex bitfield manipulation in loop */
    for (int i = 0; i < (seed & 3); i++) {
        u.bits.low = (u.bits.low + i) & 0xFF;
    }
    
    use_int(u.full);
}

__attribute__((noinline, noipa))
void test_zero_extract_2(volatile int seed) {
    /* Alternative: manual bitfield using bitwise operations */
    unsigned int value = seed;
    
    /* This pattern: store into masked portion of variable */
    for (int i = 0; i < (volatile_idx & 3); i++) {
        /* May generate SET with ZERO_EXTRACT destination */
        value = (value & ~(0xFF << 8)) | 
                ((volatile_int + i) & 0xFF) << 8;
    }
    
    use_int(value);
}

/* ========== Pattern 2: STRICT_LOW_PART destination ========== */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile int seed) {
    int large_val = seed * 100;
    short small_val = volatile_short;
    
    /* Assign short to low part of int - may generate STRICT_LOW_PART */
    /* Using pointer cast to force low-part assignment */
    *(short*)&large_val = small_val;
    
    /* Alternative: arithmetic approach */
    int another = seed;
    another = (another & ~0xFFFF) | (small_val & 0xFFFF);
    
    use_int(large_val + another);
}

__attribute__((noinline, noipa))
void test_strict_low_part_2(volatile int seed) {
    long long big = seed;
    big = (big << 32) | seed;
    
    /* Store int into low part of long long */
    int *low_part = (int*)&big;
    *low_part = volatile_int;
    
    /* Store short into even lower part */
    short *very_low = (short*)&big;
    very_low[1] = volatile_short;  /* Middle of low part */
    
    use_long(big);
}

/* ========== Pattern 3: SUBREG destination ========== */
__attribute__((noinline, noipa))
void test_subreg(volatile int seed) {
    /* Array access with type punning */
    int array[4] = {seed, seed+1, seed+2, seed+3};
    
    /* Access sub-word parts via different pointer types */
    short *short_view = (short*)array;
    
    for (int i = 0; i < (volatile_idx & 7); i++) {
        /* Store through short pointer into int array - may use SUBREG */
        short_view[i] = volatile_short + i;
    }
    
    /* Structure with mixed types */
    struct Mixed {
        long a;
        int b;
        short c;
        char d;
    } mix;
    
    mix.a = seed;
    /* Access parts of structure with different types */
    int *b_ptr = (int*)&mix.a;  /* Points to first 4 bytes of long */
    *b_ptr = volatile_int;
    
    use_int(array[0] + mix.b);
}

__attribute__((noinline, noipa))
void test_subreg_2(volatile int seed) {
    /* Long long accessed as two ints */
    long long big_value = (long long)seed << 32 | (seed & 0xFFFFFFFF);
    
    /* Type-punning to access halves */
    int *halves = (int*)&big_value;
    
    if (volatile_int & 1) {
        halves[0] = volatile_int;  /* Low half */
    } else {
        halves[1] = volatile_int;  /* High half */
    }
    
    /* Union for sub-register access */
    union {
        double d;
        unsigned int parts[2];
    } u;
    
    u.d = seed * 3.14;
    u.parts[0] = volatile_int;  /* Modify low 32 bits of double */
    
    use_long(big_value + u.parts[1]);
}

/* ========== Pattern 4: Complex MEM destinations ========== */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int seed) {
    /* Complex addressing modes */
    int *ptr = &global_array[volatile_idx];
    
    /* Store with index computation */
    for (int i = 0; i < (seed & 7); i++) {
        ptr[i * 2] = volatile_int + i;
    }
    
    /* Structure member access with offset */
    struct ComplexStruct local_struct;
    int *member_ptr = &local_struct.a + (volatile_idx & 3);
    *member_ptr = volatile_int;
    
    /* Pointer arithmetic with multiple steps */
    int *complex_addr = &global_array[0] + 
                       (volatile_int & 31) * 
                       ((volatile_short & 3) + 1);
    *complex_addr = seed;
    
    use_int(global_array[0] + local_struct.a);
}

__attribute__((noinline, noipa))
void test_complex_mem_2(volatile int seed) {
    /* Multi-dimensional array with volatile index */
    int matrix[10][10];
    
    for (int i = 0; i < (volatile_idx & 3); i++) {
        for (int j = 0; j < (volatile_int & 3); j++) {
            /* Complex address calculation */
            matrix[i + (seed & 1)][j + (seed & 1)] = 
                volatile_int + i * 10 + j;
        }
    }
    
    /* Linked-list style access */
    struct Node {
        int value;
        struct Node *next;
    } nodes[5];
    
    for (int i = 0; i < 4; i++) {
        nodes[i].next = &nodes[i + 1];
        nodes[i].value = seed + i;
    }
    
    struct Node *current = &nodes[volatile_idx & 3];
    current->value = volatile_int;
    
    /* Compute address through pointer chain */
    int *indirect = &(current->next->value);
    *indirect = volatile_int;
    
    use_int(matrix[0][0] + nodes[0].value);
}

/* ========== Combined test with all patterns ========== */
__attribute__((noinline, noipa))
int run_all_tests(volatile int seed) {
    int result = 0;
    
    /* Initialize volatile variables */
    volatile_seed = seed;
    volatile_short = (short)(seed ^ (seed >> 16));
    volatile_int = seed * 1103515245 + 12345;
    volatile_long = (long)seed * 6364136223846793005LL + 1;
    volatile_idx = (seed & 0xFF) + 1;
    
    /* Run all pattern tests */
    test_zero_extract(seed);
    test_zero_extract_2(seed + 1);
    
    test_strict_low_part(seed + 2);
    test_strict_low_part_2(seed + 3);
    
    test_subreg(seed + 4);
    test_subreg_2(seed + 5);
    
    test_complex_mem(seed + 6);
    test_complex_mem_2(seed + 7);
    
    /* Aggregate results from globals */
    result = global_array[0] + global_struct.a;
    
    /* Additional mixed pattern in main test */
    union {
        long long combined;
        struct {
            int low;
            int high;
        } parts;
    } final;
    
    final.parts.low = volatile_int;
    final.parts.high = volatile_int >> 16;
    
    /* Force SUBREG access */
    short *short_ptr = (short*)&final;
    for (int i = 0; i < 4; i++) {
        short_ptr[i] = (volatile_short + i) & 0xFFFF;
    }
    
    result ^= (final.combined & 0xFFFFFFFF);
    
    return result;
}

/* External function declarations (never defined) */
void use_int(int x) { sink(x); }
void use_long(long x) { sink((int)x); }
void use_ptr(void* x) { sink((int)(long)x); }
void sink(int x) {
    /* Empty in real use, but prevent optimization */
    volatile int dummy = x;
    (void)dummy;
}

int main(int argc, char **argv) {
    /* Use command line argument or time as seed */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i + seed;
    }
    
    global_struct.a = seed;
    global_struct.b = seed * 2;
    global_struct.c = (short)seed;
    global_struct.d = seed + 100;
    
    /* Run tests multiple times with different seeds */
    int total_result = 0;
    for (int i = 0; i < 3; i++) {
        total_result += run_all_tests(seed + i * 1000);
    }
    
    /* Print result to prevent complete optimization */
    printf("Result: %d\n", total_result);
    
    return total_result & 0xFF;
}
