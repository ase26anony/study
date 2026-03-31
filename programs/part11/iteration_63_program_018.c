/* test_resource_patterns.c
 * Designed to generate RTL SET destinations with:
 * - ZERO_EXTRACT
 * - STRICT_LOW_PART  
 * - SUBREG
 * - Complex MEM patterns
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
    short c;
    long long d;
} global_struct;

/* ========== ZERO_EXTRACT patterns ========== */

/* Pattern 1: Using bitfields in unions */
__attribute__((noinline, noipa))
void test_zero_extract_bitfield(void) {
    union BitfieldUnion {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    u.full = volatile_seed;
    /* This assignment to bitfield may generate ZERO_EXTRACT destination */
    u.bits.mid = volatile_int & 0xFF;
    sink(u.full);
    
    /* Another pattern with different bitfield sizes */
    union {
        unsigned long long full64;
        struct {
            unsigned int lower32;
            unsigned int upper32;
        } parts;
        struct {
            unsigned short s0;
            unsigned short s1;
            unsigned short s2;
            unsigned short s3;
        } shorts;
    } u2;
    
    u2.full64 = volatile_ll;
    /* Assigning to specific short within larger type */
    u2.shorts.s2 = volatile_short;
    sink((int)u2.full64);
}

/* Pattern 2: Manual bitfield extraction with masking */
__attribute__((noinline, noipa))
void test_zero_extract_masking(void) {
    unsigned int value = volatile_seed;
    unsigned int mask = 0x0000FF00;
    unsigned int insert = (volatile_int & 0xFF) << 8;
    
    /* This pattern: value = (value & ~mask) | insert
       May generate ZERO_EXTRACT for the masked portion */
    value = (value & ~mask) | insert;
    
    /* More complex pattern with shifting */
    unsigned long long big = volatile_ll;
    unsigned int part = volatile_int;
    /* Insert 16-bit field at bits 24-39 */
    unsigned long long field_mask = 0xFFFF0000ULL;
    unsigned long long shifted = ((unsigned long long)part & 0xFFFF) << 24;
    big = (big & ~field_mask) | shifted;
    
    sink((int)value + (int)big);
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Pattern 1: Assigning smaller type to larger type */
__attribute__((noinline, noipa))
void test_strict_low_part_small_type(void) {
    int large = volatile_seed;
    short small = volatile_short;
    
    /* This may generate STRICT_LOW_PART destination */
    large = (large & ~0xFFFF) | (small & 0xFFFF);
    
    /* Using pointer cast */
    int val = volatile_int;
    short *ps = (short*)&val;
    *ps = volatile_short;  /* Only modifies low 16 bits */
    
    sink(large + val);
}

/* Pattern 2: Mixed-size operations in loops */
__attribute__((noinline, noipa))
void test_strict_low_part_loop(void) {
    int results[10];
    volatile int bound = volatile_index % 10 + 1;
    
    for (int i = 0; i < bound; i++) {
        int base = volatile_seed + i * 100;
        short increment = volatile_short;
        
        /* Only modify low 16 bits in each iteration */
        base = (base & ~0xFFFF) | ((base + increment) & 0xFFFF);
        results[i] = base;
    }
    
    int sum = 0;
    for (int i = 0; i < bound; i++) {
        sum += results[i];
    }
    sink(sum);
}

/* ========== SUBREG patterns ========== */

/* Pattern 1: Type punning through pointers */
__attribute__((noinline, noipa))
void test_subreg_type_punning(void) {
    long long big_value = volatile_ll;
    
    /* Access different views of the same memory */
    int *as_int = (int*)&big_value;
    short *as_short = (short*)&big_value;
    
    /* These assignments may generate SUBREG destinations */
    as_int[1] = volatile_int;      /* Modify upper 32 bits */
    as_short[2] = volatile_short;  /* Modify middle 16 bits */
    
    /* Array with sub-word access */
    int array[4] = {0};
    volatile int idx = volatile_index % 4;
    short *ps = (short*)&array[idx];
    *ps = volatile_short;
    
    sink((int)big_value + array[0]);
}

/* Pattern 2: Structure member access with casts */
__attribute__((noinline, noipa))
void test_subreg_struct_access(void) {
    struct MixedTypes {
        char c;
        short s;
        int i;
        long long ll;
    } data;
    
    data.ll = volatile_ll;
    
    /* Access sub-parts through different pointer types */
    short *s_ptr = (short*)&data.i;
    *s_ptr = volatile_short;  /* May use SUBREG to access part of 'i' */
    
    /* Access part of long long */
    int *ll_part = (int*)&data.ll;
    ll_part[volatile_index & 1] = volatile_int;
    
    sink(data.i + (int)data.ll);
}

/* ========== Complex MEM patterns ========== */

/* Pattern 1: Computed addresses with pointer arithmetic */
__attribute__((noinline, noipa))
void test_mem_computed_address(void) {
    int *ptr;
    
    /* Complex addressing mode 1: global + variable offset */
    volatile int offset = volatile_index % 50;
    ptr = &global_array[offset + 10];
    *ptr = volatile_int;
    
    /* Complex addressing mode 2: structure with variable field */
    struct ComplexStruct *cs = &global_struct;
    ptr = &cs->b[volatile_index % 4];
    *ptr = volatile_seed;
    
    /* Complex addressing mode 3: pointer arithmetic with scaling */
    int *base = global_array;
    ptr = base + (volatile_index * 3) % 100;
    *ptr = volatile_int * 2;
    
    sink(global_array[0] + global_struct.b[0]);
}

/* Pattern 2: Nested addressing in loops */
__attribute__((noinline, noipa))
void test_mem_nested_addressing(void) {
    int local_array[50];
    volatile int outer_bound = (volatile_index % 5) + 1;
    volatile int inner_bound = (volatile_seed % 10) + 1;
    
    for (int i = 0; i < outer_bound; i++) {
        int *row_ptr = local_array + (i * 10);
        for (int j = 0; j < inner_bound; j++) {
            /* Complex addressing: base + (i*10) + j */
            row_ptr[j] = volatile_int + i * 100 + j;
        }
    }
    
    /* Another pattern: pointer chasing */
    int *chain[5];
    chain[0] = &local_array[0];
    for (int i = 1; i < 5; i++) {
        chain[i] = chain[i-1] + (volatile_index % 3) + 1;
        *chain[i] = volatile_int + i;
    }
    
    int sum = 0;
    for (int i = 0; i < 50; i++) {
        sum += local_array[i];
    }
    sink(sum);
}

/* Pattern 3: Memory with pre/post increment patterns */
__attribute__((noinline, noipa))
void test_mem_inc_dec(void) {
    int buffer[20];
    int *p = buffer;
    volatile int count = volatile_index % 15 + 1;
    
    /* Generate various memory addressing modes */
    for (int i = 0; i < count; i++) {
        p[i] = volatile_int + i;           /* Indexed */
        *(p + i + 5) = volatile_short;     /* Pointer + offset */
    }
    
    /* Different stride */
    int *q = buffer;
    for (int i = 0; i < count; i += 2) {
        q[0] = volatile_seed;
        q = q + 3;  /* Change pointer */
    }
    
    sink(buffer[0] + buffer[10]);
}

/* ========== Combined test function ========== */

__attribute__((noinline, noipa))
int run_all_tests(int seed) {
    volatile_seed = seed;
    volatile_index = seed * 3 + 1;
    volatile_short = (short)(seed ^ 0xABCD);
    volatile_int = seed * 7919;  /* Prime multiplier */
    volatile_ll = (long long)seed * 1000000007LL;
    
    int result = 0;
    
    /* Call all test functions */
    test_zero_extract_bitfield();
    test_zero_extract_masking();
    result += volatile_seed;
    
    test_strict_low_part_small_type();
    test_strict_low_part_loop();
    result += volatile_int;
    
    test_subreg_type_punning();
    test_subreg_struct_access();
    result += volatile_short;
    
    test_mem_computed_address();
    test_mem_nested_addressing();
    test_mem_inc_dec();
    result += volatile_index;
    
    /* Use global variables to prevent dead store elimination */
    result += global_array[0] + global_struct.a;
    
    return result;
}

/* ========== Main function ========== */

int main(int argc, char *argv[]) {
    /* Initialize with some entropy */
    int seed;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL) ^ getpid();
    }
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * i;
    }
    global_struct.a = 42;
    global_struct.c = 100;
    global_struct.d = 0x123456789ABCDEF0LL;
    
    /* Run tests multiple times with different seeds */
    int total_result = 0;
    for (int i = 0; i < 3; i++) {
        total_result += run_all_tests(seed + i * 1000);
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", total_result);
    
    return 0;
}

/* Dummy definitions to satisfy external references */
void sink(int x) {
    /* Empty - just to prevent optimization */
    asm volatile("" : : "r"(x));
}

void use(void* p) {
    /* Empty - just to prevent optimization */
    asm volatile("" : : "r"(p));
}
