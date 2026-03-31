/* test_resource_patterns.c
 * Designed to trigger uncovered lines in GCC's resource.cc
 * Specifically targeting SET destinations with:
 * 1. ZERO_EXTRACT
 * 2. STRICT_LOW_PART  
 * 3. SUBREG
 * 4. Complex MEM patterns
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
static volatile int volatile_index;
static volatile short volatile_short;
static volatile int volatile_int;
static volatile long volatile_long;

/* Global variables for memory patterns */
int global_array[100];
struct compound {
    int a;
    long b;
    short c[4];
} global_struct;

/* Pattern 1: ZERO_EXTRACT destination */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int seed) {
    /* Using bitfields to encourage ZERO_EXTRACT */
    union bitfield_union {
        unsigned int full;
        struct {
            unsigned int low16 : 16;
            unsigned int high16 : 16;
        } parts;
    } u;
    
    /* Volatile operations to prevent optimization */
    u.full = seed;
    volatile_short = (short)(seed * 3);
    
    /* This should generate ZERO_EXTRACT when storing into bitfield */
    u.parts.low16 = volatile_short;
    u.parts.high16 = (unsigned short)(volatile_short ^ 0x55AA);
    
    /* Use the result */
    use_int(u.full);
    
    /* Another pattern: explicit bit masking */
    unsigned int mask = 0xFF00FF00;
    unsigned int val = seed;
    unsigned int insert = volatile_int & 0x00FF00FF;
    
    /* Store into masked portion - may generate ZERO_EXTRACT */
    val = (val & mask) | insert;
    use_int(val);
}

/* Pattern 2: STRICT_LOW_PART destination */
__attribute__((noinline, noipa)) 
void test_strict_low_part(volatile int seed) {
    /* Assigning short to int with masking */
    int destination = seed;
    short source = (short)(seed ^ 0x1234);
    
    /* This may generate STRICT_LOW_PART */
    destination = (destination & ~0xFFFF) | (source & 0xFFFF);
    use_int(destination);
    
    /* Pointer casting approach */
    long big_val = volatile_long;
    int* int_ptr = (int*)&big_val;
    
    /* Store into low part of long */
    *int_ptr = volatile_int;
    use_long(big_val);
    
    /* Using smaller type assignment */
    struct mixed_types {
        long long big;
        int medium;
        short small;
    } mt;
    
    mt.big = volatile_long;
    mt.medium = volatile_int;
    
    /* This assignment might use STRICT_LOW_PART */
    mt.small = volatile_short;
    use_int(mt.medium);
}

/* Pattern 3: SUBREG destination */
__attribute__((noinline, noipa))
void test_subreg(volatile int seed) {
    /* Type punning with different sizes */
    long long big_buffer[2];
    volatile_index = seed % 4;
    
    /* Access sub-parts through pointers */
    int* int_view = (int*)big_buffer;
    short* short_view = (short*)big_buffer;
    
    /* These assignments may generate SUBREG */
    int_view[volatile_index] = volatile_int;
    short_view[volatile_index * 2] = volatile_short;
    
    use_ptr(big_buffer);
    
    /* Array with sub-word access */
    int array[10];
    for (int i = 0; i < volatile_index && i < 10; i++) {
        /* Cast to different type for access */
        char* byte_ptr = (char*)&array[i];
        byte_ptr[1] = (char)(seed + i);  /* Middle byte */
    }
    
    /* Structure with mixed types */
    struct subreg_test {
        int a;
        short b;
        char c;
    } st;
    
    st.a = volatile_int;
    /* Access through different pointer type */
    *(short*)((char*)&st + 2) = volatile_short;
    
    use_int(st.a);
}

/* Pattern 4: Complex MEM destination */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int seed) {
    /* Complex addressing modes */
    int* ptr = &global_array[0];
    volatile_index = seed % 50;
    
    /* Memory store with index computation */
    ptr[volatile_index * 2] = volatile_int;
    ptr[volatile_index + 10] = volatile_int ^ 0xAA;
    
    /* Structure member with offset */
    struct compound* sp = &global_struct;
    sp->c[volatile_index % 4] = volatile_short;
    
    /* Pointer arithmetic */
    int* computed = ptr + (volatile_index * 3) / 2;
    *computed = seed;
    
    /* Nested addressing */
    global_struct.a = volatile_int;
    ((short*)&global_struct.b)[1] = volatile_short;
    
    /* Loop with memory stores */
    for (int i = 0; i < (volatile_index & 3); i++) {
        global_array[i * 10] = seed + i;
        sp->c[i] = (short)(seed - i);
    }
    
    use_ptr(ptr);
}

/* Combined test with control flow */
__attribute__((noinline, noipa))
int test_combined(volatile int seed) {
    int result = 0;
    
    /* Conditional execution based on volatile */
    if (seed & 1) {
        test_zero_extract(seed);
        result ^= 1;
    }
    
    if (seed & 2) {
        test_strict_low_part(seed + 1);
        result ^= 2;
    }
    
    /* Loop to increase RTL complexity */
    for (int i = 0; i < (seed & 3); i++) {
        test_subreg(seed + i);
        result += i;
    }
    
    if (seed & 4) {
        test_complex_mem(seed * 3);
        result ^= 4;
    }
    
    return result;
}

/* External function definitions to satisfy linker */
void use_int(int x) { sink(x); }
void use_long(long x) { sink((int)x); }
void use_ptr(void* x) { sink((int)(long)x); }

int main(int argc, char** argv) {
    /* Initialize volatile seed from command line or time */
    if (argc > 1) {
        volatile_seed = atoi(argv[1]);
    } else {
        volatile_seed = time(NULL);
    }
    
    /* Initialize other volatile variables */
    volatile_int = volatile_seed ^ 0x55AA55AA;
    volatile_short = (short)(volatile_seed * 13);
    volatile_long = (long)volatile_seed * 1001;
    volatile_index = (volatile_seed >> 3) & 0xFF;
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i + volatile_seed;
    }
    
    global_struct.a = volatile_int;
    global_struct.b = volatile_long;
    for (int i = 0; i < 4; i++) {
        global_struct.c[i] = (short)(volatile_short + i);
    }
    
    /* Run tests multiple times with different seeds */
    int final_result = 0;
    
    for (int iteration = 0; iteration < 3; iteration++) {
        int test_seed = volatile_seed + iteration * 1000;
        final_result ^= test_combined(test_seed);
        
        /* Modify volatiles between iterations */
        volatile_int += 12345;
        volatile_short ^= 0x1234;
    }
    
    /* Print result to prevent complete optimization */
    printf("Result: %d\n", final_result);
    
    return final_result & 0xFF;
}

/* Dummy sink function to prevent dead code elimination */
void sink(int value) {
    /* Use inline asm to prevent optimization */
    __asm__ volatile ("" : : "r"(value));
}
