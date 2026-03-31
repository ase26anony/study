/* test_resource_marking.c - Generate RTL patterns for resource.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_long(long);
extern void use_ptr(void*);
extern void sink(int);

/* Volatile helpers */
static volatile int vol_int = 0;
static volatile short vol_short = 0;
static volatile char vol_char = 0;

/* Global variables for memory patterns */
int global_array[16];
struct compound {
    int a;
    long b;
    short c[4];
} global_struct;

/* ========== Pattern 1: ZERO_EXTRACT destination ========== */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int seed) {
    /* Using bitfields to encourage ZERO_EXTRACT */
    union bitfield_union {
        unsigned int full;
        struct {
            unsigned int low8 : 8;
            unsigned int mid8 : 8;
            unsigned int high16 : 16;
        } parts;
    } u;
    
    /* Volatile source to prevent constant propagation */
    volatile unsigned int source = seed & 0xFF;
    
    /* This assignment to bitfield may generate ZERO_EXTRACT */
    u.parts.mid8 = source;
    
    /* Complex control flow to keep it alive */
    for (int i = 0; i < (seed & 3); i++) {
        u.parts.low8 = source + i;
        if (seed & (1 << i)) {
            u.parts.high16 = source * i;
        }
    }
    
    sink(u.full);
}

/* Alternative ZERO_EXTRACT via bitwise operations */
__attribute__((noinline, noipa))
void test_zero_extract_bitwise(volatile int seed) {
    unsigned int dest = 0x12345678;
    unsigned int src = seed;
    
    /* Masking assignment that may become ZERO_EXTRACT */
    dest = (dest & ~0x0000FF00) | ((src & 0xFF) << 8);
    
    /* Nested conditionals */
    if (seed & 1) {
        dest = (dest & ~0x00FF0000) | ((src & 0xF0) << 12);
    }
    
    sink(dest);
}

/* ========== Pattern 2: STRICT_LOW_PART destination ========== */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile int seed) {
    long long big_val = 0x123456789ABCDEF0LL;
    short s = seed & 0xFFFF;
    
    /* Assigning short to part of long long */
    *(short*)&big_val = s;  /* May generate STRICT_LOW_PART */
    
    /* With arithmetic */
    for (int i = 0; i < (seed & 7); i++) {
        *(short*)((char*)&big_val + 2) = s + i;
    }
    
    sink((int)big_val);
}

/* Using integer type conversion */
__attribute__((noinline, noipa))
void test_strict_low_part_int(volatile int seed) {
    int dest = 0x87654321;
    short src = seed;
    
    /* Preserve high bits, set low bits */
    dest = (dest & ~0xFFFF) | (src & 0xFFFF);
    
    /* Multiple assignments in loop */
    for (volatile int i = 0; i < 2; i++) {
        dest = (dest & ~0xFFFF0000) | ((src + i) << 16);
    }
    
    sink(dest);
}

/* ========== Pattern 3: SUBREG destination ========== */
__attribute__((noinline, noipa))
void test_subreg(volatile int seed) {
    long long big_array[4] = {0};
    volatile int idx = seed & 3;
    
    /* Access sub-word of long long */
    int* p_int = (int*)&big_array[idx];
    *p_int = seed;  /* May generate SUBREG */
    
    /* Multiple type punning */
    short* p_short = (short*)&big_array[1];
    for (int i = 0; i < 2; i++) {
        p_short[i] = seed + i;
    }
    
    sink((int)big_array[0]);
}

/* SUBREG through structure */
__attribute__((noinline, noipa))
void test_subreg_struct(volatile int seed) {
    struct mixed {
        long long a;
        int b;
        short c;
    } m;
    
    m.a = seed;
    
    /* Access different sized members */
    short* ps = &m.c;
    *ps = seed & 0x7FFF;
    
    /* Pointer arithmetic */
    char* pc = (char*)&m;
    pc[3] = seed & 0xFF;
    
    sink(m.b);
}

/* ========== Pattern 4: Complex MEM destinations ========== */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int seed) {
    volatile int offset = seed & 7;
    
    /* Memory with computed address */
    int* ptr = &global_array[offset * 2];
    *ptr = seed;  /* Complex addressing mode */
    
    /* More complex addressing */
    ptr = &global_array[(seed & 3) + ((seed >> 2) & 3)];
    *ptr = seed * 2;
    
    sink(global_array[0]);
}

/* MEM with structure and pointer arithmetic */
__attribute__((noinline, noipa))
void test_complex_mem_struct(volatile int seed) {
    struct compound local_struct;
    volatile int idx = seed & 3;
    
    /* Complex structure addressing */
    int* p = &local_struct.c[idx];
    *p = seed;
    
    /* Pointer arithmetic with casting */
    char* base = (char*)&local_struct;
    int* p2 = (int*)(base + 4 + (seed & 12));
    *p2 = seed * 3;
    
    sink(local_struct.a);
}

/* MEM with global and index */
__attribute__((noinline, noipa))
void test_complex_mem_global(volatile int seed) {
    extern int extern_global;
    static int static_array[8];
    
    /* Multiple memory destinations with different addressing */
    int* addr1 = &extern_global + (seed & 3);
    *addr1 = seed;
    
    int* addr2 = static_array + ((seed >> 2) & 7);
    *addr2 = seed + 1;
    
    /* Indirect through pointer array */
    int* ptr_array[4];
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &static_array[i];
    }
    *ptr_array[seed & 3] = seed + 2;
    
    sink(static_array[0]);
}

/* ========== Combined test with all patterns ========== */
__attribute__((noinline, noipa))
int test_all_patterns(volatile int seed) {
    int result = 0;
    
    /* Mix different patterns */
    test_zero_extract(seed);
    result += seed & 1;
    
    test_strict_low_part(seed + 1);
    result += (seed >> 1) & 1;
    
    test_subreg(seed + 2);
    result += (seed >> 2) & 1;
    
    test_complex_mem(seed + 3);
    result += (seed >> 3) & 1;
    
    /* Additional variations */
    test_zero_extract_bitwise(seed + 4);
    test_strict_low_part_int(seed + 5);
    test_subreg_struct(seed + 6);
    test_complex_mem_struct(seed + 7);
    test_complex_mem_global(seed + 8);
    
    return result;
}

/* ========== Main driver ========== */
int main(int argc, char** argv) {
    /* Use argument or time as volatile seed */
    volatile int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    /* Initialize globals */
    for (int i = 0; i < 16; i++) {
        global_array[i] = i;
    }
    
    /* Run tests multiple times with different seeds */
    int total = 0;
    for (volatile int i = 0; i < 3; i++) {
        int r = test_all_patterns(seed + i);
        total += r;
        
        /* Vary control flow */
        if (seed & (1 << i)) {
            test_zero_extract(seed * i);
        } else {
            test_strict_low_part(seed / (i + 1));
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total & 1;
}

/* Dummy definitions to satisfy linker */
void use_int(int x) { vol_int = x; }
void use_long(long x) { vol_int = (int)x; }
void use_ptr(void* x) { if (x) vol_int = 1; }
void sink(int x) { vol_int = x; }

/* External global */
int extern_global = 0;
