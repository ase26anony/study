/* test_resource.c - Program to exercise specific RTL patterns in GCC's resource tracking */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 0;
volatile int g_value = 42;

/* ========== ZERO_EXTRACT patterns ========== */
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment - likely generates ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int field1 : 3;
        volatile unsigned int field2 : 5;
        volatile unsigned int field3 : 8;
    } bfs;
    
    /* Multiple bit-field assignments */
    bfs.field1 = 5;           /* Should generate ZERO_EXTRACT for 3-bit field */
    bfs.field2 = g_value & 0x1F;  /* 5-bit field with dynamic value */
    bfs.field3 = 127;         /* 8-bit field */
    
    /* Volatile bit-field in union */
    union {
        volatile unsigned int all;
        struct {
            volatile unsigned int a : 4;
            volatile unsigned int b : 4;
            volatile unsigned int c : 4;
        } parts;
    } u;
    
    u.parts.a = 3;
    u.parts.b = 7;
    u.parts.c = 15;
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(bfs.field1), "r"(u.all));
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE void test_strict_low_part(int x) {
    /* Mixed-size integer assignments - common source of STRICT_LOW_PART */
    volatile short src_short = x & 0xFFFF;
    volatile char src_char = x & 0xFF;
    
    int dest_int;
    long dest_long;
    
    /* These assignments may generate STRICT_LOW_PART on x86 */
    dest_int = src_short;     /* 16-bit to 32-bit */
    dest_long = src_char;     /* 8-bit to 64-bit */
    
    /* Pointer casting with smaller types */
    volatile int *ptr_int = &dest_int;
    volatile short *ptr_short = (volatile short *)ptr_int;
    *ptr_short = src_short;   /* Writing 16-bit to what might be 32-bit aligned */
    
    /* Use the results */
    asm volatile("" : : "r"(dest_int), "r"(dest_long), "r"(*ptr_short));
}

/* ========== SUBREG patterns ========== */
NOINLINE void test_subreg(void) {
    /* GCC vector types - often use SUBREG for lane access */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si vec_int = {1, 2, 3, 4};
    v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Vector lane extraction - may use SUBREG */
    int lane0 = vec_int[0];
    float lane1 = vec_float[1];
    
    /* Type punning through union - can generate SUBREG */
    union {
        float f;
        int i;
    } pun;
    
    pun.f = 3.14159f;
    int int_bits = pun.i;     /* Bit-cast: float->int */
    
    /* Complex number assignment */
    _Complex float cf1 = 1.0f + 2.0fi;
    _Complex float cf2 = cf1; /* Complex copy may use SUBREG */
    
    asm volatile("" : : "r"(lane0), "r"(lane1), "r"(int_bits), "r"(cf2));
}

/* ========== MEM_P with complex addressing ========== */
NOINLINE void test_mem_complex_address(int *base, int offset) {
    /* Various complex memory addressing modes */
    int array[100];
    
    /* Array with variable index */
    array[g_index] = g_value;
    
    /* Pointer arithmetic */
    *(base + offset) = 123;
    
    /* Struct pointer dereference */
    struct point {
        int x;
        int y;
        int z;
    } pt;
    
    struct point *pt_ptr = &pt;
    pt_ptr->x = offset;       /* Field access through pointer */
    pt_ptr->y = g_value;
    
    /* Multi-dimensional array */
    int matrix[10][10];
    matrix[offset % 10][g_index % 10] = 456;
    
    /* Use asm to ensure stores aren't optimized away */
    asm volatile("" : : "m"(array[0]), "m"(*(base + offset)), 
                  "m"(pt_ptr->x), "m"(matrix[0][0]));
}

/* ========== Combined test with all patterns ========== */
NOINLINE __attribute__((optimize("O2"))) 
void test_combined(int x, int *ptr) {
    /* Mix all patterns in one function with O2 optimization */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned bits : 10;
    } bf;
    bf.bits = x & 0x3FF;
    
    /* STRICT_LOW_PART via mixed-size assignment */
    volatile char c = x & 0xFF;
    int i = c;  /* char to int */
    
    /* SUBREG via vector */
    typedef short v8hi __attribute__((vector_size(16)));
    v8hi v = {1,2,3,4,5,6,7,8};
    short s = v[3];
    
    /* MEM with complex address */
    ptr[x % 16] = s + i;
    
    /* Another MEM with pointer arithmetic */
    int *q = ptr + (x & 7);
    *q = bf.bits;
    
    asm volatile("" : : "r"(bf.bits), "r"(i), "r"(s), "m"(*ptr), "m"(*q));
}

/* Helper to create complex address expressions */
NOINLINE int* get_pointer(int *base, int idx1, int idx2) {
    /* Complex pointer computation */
    return base + (idx1 * idx2 + g_index) % 32;
}

/* Main driver */
int main(void) {
    int data[100];
    int i;
    
    /* Initialize data */
    for (i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* Run tests multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part(i);
        test_subreg();
        test_mem_complex_address(data, i);
        
        /* Get pointer with complex computation */
        int *complex_ptr = get_pointer(data, i, i+1);
        test_combined(i, complex_ptr);
    }
    
    return 0;
}
