/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_temp;

/* ===== ZERO_EXTRACT patterns ===== */
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment - should generate ZERO_EXTRACT */
    struct bitfield {
        unsigned int field1 : 3;
        unsigned int field2 : 5;
        unsigned int field3 : 8;
    };
    volatile struct bitfield bf;
    
    /* Multiple bit-field assignments */
    bf.field1 = 5;      /* Should generate ZERO_EXTRACT for 3-bit field */
    bf.field2 = 31;     /* Should generate ZERO_EXTRACT for 5-bit field */
    bf.field3 = 127;    /* Should generate ZERO_EXTRACT for 8-bit field */
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(bf));
}

NOINLINE void test_zero_extract_complex(void) {
    /* More complex bit-field scenario */
    struct packed {
        unsigned short a : 4;
        unsigned short b : 7;
        unsigned short c : 5;
    } __attribute__((packed));
    
    volatile struct packed p;
    int i;
    
    /* Loop to create multiple ZERO_EXTRACT patterns */
    for (i = 0; i < 4; i++) {
        p.a = i & 0xF;
        p.b = (i * 7) & 0x7F;
        p.c = (i * 3) & 0x1F;
    }
    
    asm volatile("" : : "r"(p));
}

/* ===== STRICT_LOW_PART patterns ===== */
NOINLINE void test_strict_low_part(void) {
    /* Mixed-size assignments - common source of STRICT_LOW_PART */
    volatile short src16 = 0x1234;
    volatile char src8 = 0x56;
    volatile int dest32;
    
    /* short -> int assignment might use STRICT_LOW_PART */
    dest32 = src16;     /* Potential STRICT_LOW_PART for lower 16 bits */
    
    /* char -> int assignment */
    dest32 = src8;      /* Another potential STRICT_LOW_PART */
    
    /* Sequence of partial writes */
    volatile long long dest64;
    dest32 = src16;     /* Re-use for multiple patterns */
    
    /* Use inline asm to force partial register writes on x86 */
    #ifdef __i386__
    asm volatile("movw %1, %%ax\n\t"
                 "movl %%eax, %0"
                 : "=m"(dest32)
                 : "r"(src16)
                 : "eax");
    #endif
    
    asm volatile("" : : "r"(dest32), "r"(dest64));
}

/* Compile with -m32 for better STRICT_LOW_PART generation */
NOINLINE __attribute__((optimize("O1"))) 
void test_strict_low_part_m32(void) {
    /* Optimized for 32-bit target */
    volatile char c = 100;
    volatile short s = 2000;
    volatile int i;
    
    i = c;  /* char to int - good candidate */
    i = s;  /* short to int - another candidate */
    
    /* Chain of operations */
    i = (c << 8) | s;
    
    asm volatile("" : : "r"(i));
}

/* ===== SUBREG patterns ===== */
NOINLINE void test_subreg(void) {
    /* Vector types often generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v8hi c;
    
    /* Vector assignment */
    a = b;  /* May involve SUBREG operations */
    
    /* Type punning through union */
    union {
        v4si vec;
        int arr[4];
    } u;
    
    u.vec = a;
    int lane = u.arr[g_index & 3];  /* SUBREG for lane extraction */
    
    /* Mixed vector operations */
    c = __builtin_convertvector(a, v8hi);  /* Type conversion */
    
    /* Complex number assignment */
    _Complex float cf1 = 1.0f + 2.0fi;
    _Complex float cf2;
    cf2 = cf1;  /* May use SUBREG */
    
    asm volatile("" : : "r"(lane), "r"(c), "r"(cf2));
}

/* ===== MEM_P patterns ===== */
NOINLINE void test_mem_dest(void) {
    /* Complex memory destinations */
    int array[16];
    int *ptr;
    
    /* Store with complex address calculation */
    array[g_index] = g_value;  /* MEM with index */
    
    /* Pointer arithmetic store */
    ptr = &array[8];
    ptr[g_index - 2] = g_value * 2;
    
    /* Struct member store through pointer */
    struct point {
        int x;
        int y;
        int z;
    } points[4];
    
    points[g_index & 3].x = g_value;
    points[g_index & 3].y = g_value + 1;
    
    /* Multi-dimensional array */
    int matrix[4][4];
    int i = g_index & 3;
    int j = (g_index + 1) & 3;
    matrix[i][j] = g_value;
    
    asm volatile("" : : "m"(array), "m"(points), "m"(matrix));
}

NOINLINE void test_mem_complex_addr(void) {
    /* Even more complex address expressions */
    int buffer[32];
    volatile int idx1 = g_index;
    volatile int idx2 = g_index * 2;
    
    /* Multiple index calculations */
    buffer[idx1 + idx2] = 100;
    buffer[(idx1 * 3) % 32] = 200;
    
    /* Pointer with offset */
    int *p = buffer + 16;
    p[-idx1] = 300;
    
    /* Struct with array */
    struct data {
        int header;
        int payload[8];
        int footer;
    } d;
    
    d.payload[idx1 & 7] = 400;
    
    asm volatile("" : : "m"(buffer), "m"(d));
}

/* ===== Combined test ===== */
NOINLINE void test_combined(void) {
    /* Mix all patterns in one function */
    volatile struct {
        unsigned int bf1 : 4;
        unsigned int bf2 : 12;
    } s;
    
    volatile short vs;
    volatile int vi;
    int arr[8];
    
    /* ZERO_EXTRACT */
    s.bf1 = 7;
    s.bf2 = 4095;
    
    /* STRICT_LOW_PART candidate */
    vs = 255;
    vi = vs;
    
    /* MEM_P with SUBREG-like addressing */
    arr[vi & 7] = s.bf1 + s.bf2;
    
    /* Additional MEM_P */
    *(int*)((char*)arr + 4) = vi;
    
    asm volatile("" : : "r"(s), "r"(vi), "m"(arr));
}

/* Main driver */
int main(void) {
    int i;
    
    /* Call test functions multiple times with different conditions */
    for (i = 0; i < 10; i++) {
        g_index = i;
        g_value = i * 10;
        
        test_zero_extract();
        test_zero_extract_complex();
        test_strict_low_part();
        test_subreg();
        test_mem_dest();
        test_mem_complex_addr();
        test_combined();
        
        #ifdef __i386__
        test_strict_low_part_m32();
        #endif
    }
    
    return 0;
}
