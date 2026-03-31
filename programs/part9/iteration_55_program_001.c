/* test_resource.c - Coverage for GCC resource.cc mark_referenced_resources */
/* Compile with: gcc -O2 -fprofile-arcs -ftest-coverage -m32 -fdump-rtl-all -c test_resource.c */
/* Also try: gcc -O1 -m32 -fprofile-arcs -ftest-coverage test_resource.c -o test && ./test */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile short g_short = 0x1234;
volatile char g_char = 0x56;

/* ==================== ZERO_EXTRACT patterns ==================== */
/* Bit-field assignments often generate ZERO_EXTRACT in RTL */
struct bitfield_struct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
};

NOINLINE void test_zero_extract(void) {
    volatile struct bitfield_struct bfs;
    
    /* These assignments should generate ZERO_EXTRACT as SET_DEST */
    bfs.field1 = 5;          /* 3-bit field */
    bfs.field2 = 20;         /* 5-bit field */
    bfs.field3 = 255;        /* 8-bit field */
    
    /* Mix with computation to prevent optimization */
    bfs.field1 = g_value & 0x7;
    bfs.field2 = (g_value >> 3) & 0x1F;
    
    /* Force use of bitfield */
    asm volatile("" : : "r"(bfs));
}

/* Another ZERO_EXTRACT variant with union */
union mixed_bf {
    struct {
        unsigned int low : 4;
        unsigned int high : 12;
    } bits;
    uint16_t word;
};

NOINLINE void test_zero_extract_union(void) {
    volatile union mixed_bf u;
    u.bits.low = 0xF;
    u.bits.high = 0xABC;
    
    /* Complex expression as source */
    u.bits.low = (g_value ^ g_index) & 0xF;
    
    asm volatile("" : : "r"(u.word));
}

/* ==================== STRICT_LOW_PART patterns ==================== */
/* Partial register writes, especially on 32-bit x86 */
NOINLINE void test_strict_low_part(void) {
    volatile int dest;
    volatile short src = g_short;
    volatile char csrc = g_char;
    
    /* short -> int assignment may generate STRICT_LOW_PART */
    dest = src;              /* SET_DEST might be (strict_low_part (subreg:SI (reg:HI))) */
    
    /* char -> int assignment */
    dest = csrc;
    
    /* More complex case with computation */
    dest = (src << 4) | (csrc & 0xF);
    
    /* Use inline assembly that modifies partial register */
    asm volatile(
        "movw %w1, %0\n\t"   /* 16-bit move to 32-bit dest (x86) */
        : "=r"(dest)
        : "r"(src)
    );
    
    asm volatile("" : : "r"(dest));
}

/* STRICT_LOW_PART with different integer types */
NOINLINE void test_mixed_sizes(void) {
    volatile long long big = 0x123456789ABCDEF0LL;
    volatile int medium;
    volatile short small;
    
    /* int -> long long (partial write to lower 32 bits) */
    medium = g_value;
    big = medium;           /* May generate STRICT_LOW_PART for lower 32 bits */
    
    /* short -> int */
    small = g_short;
    medium = small;
    
    /* char -> short */
    volatile char c = g_char;
    small = c;
    
    asm volatile("" : : "r"(big), "r"(medium), "r"(small));
}

/* ==================== SUBREG patterns ==================== */
/* GCC vector extensions often generate SUBREG */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    volatile v4si a = {1, 2, 3, 4};
    volatile v4si b = {5, 6, 7, 8};
    volatile int lane;
    
    /* Vector lane extraction - generates SUBREG */
    lane = a[0];            /* SET_DEST is (subreg:SI (reg:V4SI)) */
    lane = b[g_index & 3];
    
    /* Vector assignment */
    a = b;                  /* May involve SUBREG for parts */
    
    /* Type punning through union */
    union {
        v4si vec;
        int arr[4];
    } u;
    u.vec = a;
    lane = u.arr[1];        /* Memory access with potential SUBREG */
    
    /* Mixed vector sizes */
    v8hi vshort = {1, 2, 3, 4, 5, 6, 7, 8};
    v4si vint;
    
    /* Cast between vector types */
    vint = (v4si)vshort;    /* Likely generates SUBREG */
    
    asm volatile("" : : "r"(lane), "r"(a), "r"(vint));
}

/* SUBREG with float/int bitcasting */
NOINLINE void test_subreg_float(void) {
    volatile float f = 3.14159f;
    volatile int i;
    
    /* Type punning - generates SUBREG */
    i = *(volatile int*)&f;  /* SET_DEST is (subreg:SI (reg:SF)) */
    
    /* Double to long long */
    volatile double d = 2.71828;
    volatile long long ll;
    ll = *(volatile long long*)&d;
    
    asm volatile("" : : "r"(i), "r"(ll));
}

/* ==================== MEM_P patterns ==================== */
/* Complex memory addresses for MEM as SET_DEST */
int global_array[100];
struct point {
    int x, y, z;
};
struct point global_points[10];

NOINLINE void test_mem_complex_address(void) {
    volatile int index = g_index;
    volatile int value = g_value;
    
    /* Array with variable index - MEM with complex address */
    global_array[index * 2 + 5] = value;  /* SET_DEST is (mem (plus (mult ...))) */
    
    /* Struct member through pointer */
    struct point *p = &global_points[index & 7];
    p->x = value;
    p->y = value * 2;
    p->z = index;
    
    /* Pointer arithmetic */
    int *ptr = global_array + index * 3;
    ptr[0] = value;
    ptr[1] = value + 1;
    ptr[-1] = value - 1;
    
    /* Multi-dimensional array */
    int matrix[10][10];
    matrix[index][index + 1] = value;
    
    /* Memory with side effects in address */
    global_array[g_index++] = value;  /* Address has side effect */
    
    asm volatile("" : : "r"(index), "r"(value), "r"(p));
}

/* MEM with even more complex addressing */
NOINLINE void test_mem_nested(struct point *points, int n) {
    for (int i = 0; i < n; i++) {
        /* Complex addressing with struct and array */
        points[i].x = g_value + i;
        points[i].y = g_index * i;
        points[i].z = points[(i + 1) % n].x;
        
        /* Conditional store */
        if (i & 1) {
            global_array[i * 2] = points[i].x;
        } else {
            global_array[i * 2 + 1] = points[i].y;
        }
    }
}

/* ==================== Combined test ==================== */
/* Function that mixes all patterns */
NOINLINE USED void test_combined(void) {
    /* ZERO_EXTRACT */
    volatile struct bitfield_struct bfs;
    bfs.field1 = g_value & 0x7;
    
    /* STRICT_LOW_PART */
    volatile int dest;
    volatile short src = g_short;
    dest = src;
    
    /* SUBREG */
    typedef int v2si __attribute__((vector_size(8)));
    volatile v2si v = {1, 2};
    volatile int lane = v[0];
    
    /* MEM_P */
    global_array[g_index * 3] = dest + lane;
    
    /* Use all results */
    asm volatile("" : : "r"(bfs), "r"(dest), "r"(lane));
}

/* ==================== Main driver ==================== */
int main(void) {
    /* Initialize globals */
    memset(global_array, 0, sizeof(global_array));
    memset(global_points, 0, sizeof(global_points));
    
    /* Call all test functions multiple times with different "inputs" */
    for (int i = 0; i < 10; i++) {
        g_index = (i * 7) % 50;
        g_value = i * 100 + 42;
        g_short = i * 1000;
        g_char = i * 10;
        
        test_zero_extract();
        test_zero_extract_union();
        test_strict_low_part();
        test_mixed_sizes();
        test_subreg();
        test_subreg_float();
        test_mem_complex_address();
        test_mem_nested(global_points, 5);
        test_combined();
    }
    
    return 0;
}
