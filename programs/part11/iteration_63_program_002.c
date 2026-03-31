/* test_resource_patterns.c
 * Designed to generate RTL SET destinations with:
 * 1. ZERO_EXTRACT (bitfield assignments)
 * 2. STRICT_LOW_PART (low-part assignments)
 * 3. SUBREG (sub-register accesses)
 * 4. Complex MEM_P (memory addressing modes)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use_int(int x);
extern void use_long(long x);
extern void use_ptr(void *p);

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
    short c;
    char d;
} global_struct;

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Pattern 1: ZERO_EXTRACT destinations (bitfield operations) */
NOINLINE void test_zero_extract(int seed) {
    /* Union with bitfields - explicit ZERO_EXTRACT pattern */
    union {
        unsigned int full;
        struct {
            unsigned int low : 8;
            unsigned int mid : 8;
            unsigned int high : 16;
        } bits;
    } u;
    
    /* Volatile source to prevent optimization */
    volatile unsigned int src = seed;
    
    /* Multiple bitfield assignments that may generate ZERO_EXTRACT */
    u.bits.low = src & 0xFF;
    u.bits.mid = (src >> 8) & 0xFF;
    u.bits.high = (src >> 16) & 0xFFFF;
    
    /* Another pattern: masking operations */
    unsigned int val = 0;
    val = (val & ~0xFF) | (src & 0xFF);           /* Low 8 bits */
    val = (val & ~(0xFF << 8)) | ((src & 0xFF) << 8); /* Next 8 bits */
    
    /* Use results to keep them live */
    use_int(u.full);
    use_int(val);
}

/* Pattern 2: STRICT_LOW_PART destinations (low-part assignments) */
NOINLINE void test_strict_low_part(int seed) {
    /* Assigning to short part of int */
    int dest_int = seed;
    short src_short = (short)(seed ^ 0x1234);
    
    /* Pattern that may generate STRICT_LOW_PART */
    dest_int = (dest_int & ~0xFFFF) | (src_short & 0xFFFF);
    
    /* Pointer casting pattern */
    long long_val = seed * 3L;
    int *int_ptr = (int *)&long_val;
    *int_ptr = src_short;  /* Assigning int to part of long */
    
    /* Another pattern with explicit type punning */
    struct {
        int whole;
    } container;
    container.whole = seed;
    *(short *)&container.whole = src_short;
    
    use_long(long_val);
    use_int(container.whole);
}

/* Pattern 3: SUBREG destinations (sub-register accesses) */
NOINLINE void test_subreg(int seed) {
    /* Array with sub-word access */
    int array[4] = {seed, seed + 1, seed + 2, seed + 3};
    volatile int idx = volatile_index & 3;
    
    /* Access via different type pointer - may generate SUBREG */
    short *short_ptr = (short *)&array[idx];
    *short_ptr = (short)seed;
    
    /* Type punning between different sizes */
    long long big_value = seed * 1000LL;
    int *int_ptr = (int *)&big_value;
    int_ptr[0] = seed;      /* First 32 bits */
    int_ptr[1] = seed + 1;  /* Next 32 bits (on 64-bit) */
    
    /* Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long l;
    } m;
    
    m.i = seed;
    m.s = (short)seed;
    m.c = (char)seed;
    
    /* Pointer arithmetic with different types */
    char *byte_ptr = (char *)array;
    byte_ptr[idx * sizeof(int) + 1] = (char)seed;
    
    use_ptr(array);
    use_long(big_value);
    use_int(m.i);
}

/* Pattern 4: Complex MEM_P destinations (memory addressing) */
NOINLINE void test_complex_mem(int seed) {
    volatile int offset = volatile_index % 50;
    
    /* Complex addressing modes */
    int *ptr1 = &global_array[offset * 2];
    *ptr1 = seed;
    
    /* Pointer arithmetic with structure */
    struct compound *sptr = &global_struct;
    int *member_ptr = &sptr->a + offset;
    *member_ptr = seed;
    
    /* Computed address with multiple operations */
    int *ptr2 = global_array + (offset * 3) / 2;
    ptr2[0] = seed;
    ptr2[1] = seed + 1;
    
    /* Nested addressing */
    int **pptr = (int **)&ptr1;
    **pptr = seed * 2;
    
    /* Array of pointers */
    static int *ptr_array[10];
    for (int i = 0; i < 10; i++) {
        ptr_array[i] = &global_array[i * 10];
    }
    
    /* Use volatile index for unpredictable access */
    int idx = volatile_index % 10;
    *ptr_array[idx] = seed + idx;
    
    /* Structure with array */
    struct with_array {
        int data[20];
        int count;
    } wa;
    
    int *data_ptr = wa.data + (offset % 15);
    *data_ptr = seed;
    
    use_ptr(ptr1);
    use_ptr(&wa);
}

/* Combined test with control flow */
NOINLINE int test_combined(int seed) {
    int result = 0;
    
    /* Loop with volatile bound to create control flow */
    volatile int loop_bound = (seed % 10) + 1;
    
    for (int i = 0; i < loop_bound; i++) {
        /* Conditional based on volatile */
        if (volatile_int & (1 << i)) {
            test_zero_extract(seed + i);
            result += 1;
        } else {
            test_strict_low_part(seed - i);
            result += 2;
        }
        
        /* Nested condition */
        switch (i % 3) {
            case 0:
                test_subreg(seed * i);
                result += 3;
                break;
            case 1:
                test_complex_mem(seed + i * 100);
                result += 4;
                break;
            default:
                /* Mix patterns */
                test_zero_extract(seed);
                test_subreg(seed);
                result += 5;
        }
    }
    
    return result;
}

/* Main driver with volatile initialization */
int main(int argc, char **argv) {
    /* Initialize volatile variables */
    volatile_seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    volatile_index = volatile_seed % 100;
    volatile_short = (short)(volatile_seed ^ 0xABCD);
    volatile_int = volatile_seed * 3;
    volatile_long = volatile_seed * 1000L;
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i;
    }
    global_struct.a = 1;
    global_struct.b = 2;
    global_struct.c = 3;
    global_struct.d = 4;
    
    /* Run individual pattern tests */
    test_zero_extract(volatile_seed);
    test_strict_low_part(volatile_seed + 1);
    test_subreg(volatile_seed + 2);
    test_complex_mem(volatile_seed + 3);
    
    /* Run combined test with control flow */
    int final_result = test_combined(volatile_seed);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    return final_result & 0xFF;
}

/* Dummy definitions for external functions */
void use_int(int x) {
    volatile static int sink;
    sink = x;
}

void use_long(long x) {
    volatile static long sink;
    sink = x;
}

void use_ptr(void *p) {
    volatile static void *sink;
    sink = p;
}
