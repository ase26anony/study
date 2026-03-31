/* test_resource_cc.c - Generate RTL patterns for uncovered lines in resource.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_long(long);
extern void use_ptr(void*);
extern void sink(int);

/* Volatile globals to force memory accesses */
volatile int g_volatile_int = 0;
volatile short g_volatile_short = 0;
volatile char g_volatile_char = 0;

/* ========== Pattern 1: ZERO_EXTRACT destination ========== */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int seed) {
    /* Use union with bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    u.full = seed;
    /* Store into a specific bitfield - may generate ZERO_EXTRACT destination */
    u.bits.mid = g_volatile_char & 0xFF;
    
    /* Another approach: explicit bitfield assignment */
    struct {
        unsigned int a: 4;
        unsigned int b: 12;
        unsigned int c: 16;
    } s;
    s.a = seed & 0xF;
    s.b = (seed >> 4) & 0xFFF;
    s.c = g_volatile_short & 0xFFFF;
    
    /* Use result to prevent elimination */
    use_int(u.full + s.a + s.b + s.c);
}

/* ========== Pattern 2: STRICT_LOW_PART destination ========== */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile int seed) {
    int val = seed;
    short s = g_volatile_short;
    
    /* Assign short to low part of int - may generate STRICT_LOW_PART */
    *(short*)&val = s;
    
    /* Alternative: explicit masking */
    long long big = seed * 100LL;
    int low_part = g_volatile_int;
    big = (big & ~0xFFFFFFFFLL) | (low_part & 0xFFFFFFFFLL);
    
    /* Use pointer to short to modify low part */
    int another = seed * 2;
    short* ps = (short*)&another;
    ps[0] = g_volatile_char;  /* Modify low 16 bits */
    
    use_long(big + another + val);
}

/* ========== Pattern 3: SUBREG destination ========== */
__attribute__((noinline, noipa))
void test_subreg(volatile int seed) {
    /* Array access with type punning */
    int array[4] = {seed, seed+1, seed+2, seed+3};
    volatile int idx = seed % 3;
    
    /* Access as short - may generate SUBREG */
    short* ps = (short*)&array[idx];
    *ps = g_volatile_short;
    
    /* Long long to int assignment */
    long long big_val = seed * 1000LL;
    int* p_int = (int*)&big_val;
    p_int[0] = g_volatile_int;  /* Modify first 32 bits */
    p_int[1] = g_volatile_int + 1;  /* Modify second 32 bits */
    
    /* Structure with different sized members */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    m.ll = seed;
    *(int*)&m.ll = g_volatile_int;  /* Type punning within struct */
    
    use_ptr(array);
    use_int(m.i + *ps);
}

/* ========== Pattern 4: Complex MEM destination ========== */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int seed) {
    /* Structure with pointer arithmetic */
    struct point {
        int x;
        int y;
        int z;
    } pt = {seed, seed+1, seed+2};
    
    volatile int offset = seed % 3;
    int* ptr = &pt.x + offset;
    *ptr = g_volatile_int;  /* Complex address calculation */
    
    /* Array with variable index */
    int matrix[3][3] = {{0}};
    volatile int row = seed % 3;
    volatile int col = (seed >> 2) % 3;
    matrix[row][col] = g_volatile_int;  /* 2D array access */
    
    /* Pointer chain */
    int a = seed;
    int b = seed + 10;
    int c = seed + 20;
    int* ptrs[3] = {&a, &b, &c};
    volatile int idx = seed % 3;
    *ptrs[idx] = g_volatile_int;  /* Indirect memory access */
    
    /* Global with offset */
    extern int global_array[];
    int* global_ptr = &global_array[seed % 100];
    *global_ptr = g_volatile_int;
    
    use_int(a + b + c + matrix[0][0]);
}

/* ========== Combined test with control flow ========== */
__attribute__((noinline, noipa))
int test_combined(volatile int seed) {
    int result = 0;
    
    /* Loop with volatile bound */
    for (int i = 0; i < (seed & 0x3); i++) {
        /* Conditional based on volatile */
        if (g_volatile_int & (1 << i)) {
            test_zero_extract(seed + i);
            result += 1;
        } else {
            test_strict_low_part(seed - i);
            result += 2;
        }
        
        /* Nested condition */
        switch (i) {
            case 0:
                test_subreg(seed * i);
                result += 3;
                break;
            case 1:
                test_complex_mem(seed + 100);
                result += 4;
                break;
            default:
                /* Mix patterns */
                int temp = seed;
                *(short*)&temp = g_volatile_short;  /* STRICT_LOW_PART */
                result += temp;
                break;
        }
    }
    
    return result;
}

/* External array declaration */
int global_array[100];

int main(int argc, char** argv) {
    /* Use argv for volatile seed */
    volatile int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = (int)(long)main;  /* Use address as seed */
    }
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * i;
    }
    
    /* Run individual pattern tests */
    test_zero_extract(seed);
    test_strict_low_part(seed + 1);
    test_subreg(seed + 2);
    test_complex_mem(seed + 3);
    
    /* Run combined test with control flow */
    int final_result = test_combined(seed);
    
    /* Use sink to prevent optimization */
    sink(final_result);
    
    /* Print result to ensure code isn't dead */
    printf("Result: %d\n", final_result);
    
    return final_result & 0xFF;
}

/* Dummy definitions to satisfy linker */
void use_int(int x) { g_volatile_int = x; }
void use_long(long x) { g_volatile_int = (int)x; }
void use_ptr(void* p) { g_volatile_int = (int)(long)p; }
void sink(int x) { g_volatile_int = x; }
