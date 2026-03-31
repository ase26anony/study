/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 0;
volatile int g_value = 42;
volatile int g_temp = 0;

/* ===== ZERO_EXTRACT patterns ===== */
/* Bit-field assignments often generate ZERO_EXTRACT in RTL */
struct bitfield_struct {
    unsigned int field3 : 3;
    unsigned int field5 : 5;
    unsigned int field8 : 8;
    volatile unsigned int padding : 16;
};

NOINLINE void test_zero_extract(void) {
    volatile struct bitfield_struct bfs;
    
    /* These assignments should generate ZERO_EXTRACT as SET_DEST */
    bfs.field3 = 5;      /* ZERO_EXTRACT for 3-bit field */
    bfs.field5 = 0x1F;   /* ZERO_EXTRACT for 5-bit field */
    bfs.field8 = 0xFF;   /* ZERO_EXTRACT for 8-bit field */
    
    /* Use the values to prevent dead code elimination */
    g_temp = bfs.field3 + bfs.field5 + bfs.field8;
}

/* ===== STRICT_LOW_PART patterns ===== */
/* Partial register writes, especially on 32-bit x86 */
NOINLINE void test_strict_low_part(void) {
    volatile short src16 = 0x1234;
    volatile char src8 = 0x56;
    volatile int dest32 = 0;
    
    /* These assignments may generate STRICT_LOW_PART */
    dest32 = src16;      /* short -> int, preserving only low 16 bits */
    
    /* Char to int with sign extension - may use STRICT_LOW_PART */
    int dest32_2 = src8;
    
    /* Use inline assembly to force partial register write on x86 */
    asm volatile (
        "movw %1, %%ax\n\t"
        "movl %%eax, %0"
        : "=r" (dest32)
        : "r" (src16)
        : "%eax"
    );
    
    g_temp = dest32 + dest32_2;
}

/* ===== SUBREG patterns ===== */
/* Type conversions and vector operations generate SUBREG */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v8hi vec_short = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector operations often involve SUBREG */
    vec_a = vec_a + vec_b;
    
    /* Extract element from vector - generates SUBREG */
    int element = vec_a[2];
    
    /* Type punning through union - may generate SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14f;
    int int_from_float = pun.i;
    
    /* Vector cast - generates SUBREG */
    v8hi converted = (v8hi)vec_a;
    
    g_temp = element + int_from_float + converted[0];
}

/* ===== MEM_P patterns ===== */
/* Complex memory addresses generate MEM with address expressions */
int global_array[100];

NOINLINE void test_mem_p(int index, int value) {
    /* Complex addressing modes */
    global_array[index * 2 + 1] = value;           /* MEM with arithmetic */
    global_array[global_array[index]] = index;     /* MEM with memory load in address */
    
    /* Pointer arithmetic */
    int *ptr = &global_array[10];
    ptr[index] = value * 2;
    
    /* Struct with pointer */
    struct {
        int data[20];
        int *next;
    } s;
    s.data[index % 20] = value;
    
    /* Multi-dimensional array */
    int matrix[10][10];
    matrix[index/10][index%10] = value;
    
    /* Use asm to mark memory as clobbered */
    asm volatile ("" : : "m" (global_array[0]), "m" (matrix[0][0]));
}

/* ===== Combined test function ===== */
/* Mix all patterns in one function to increase coverage chance */
NOINLINE __attribute__((optimize("O2"))) 
void test_combined(volatile int param) {
    /* ZERO_EXTRACT */
    struct {
        unsigned int bf1 : 4;
        unsigned int bf2 : 12;
    } s;
    s.bf1 = param & 0xF;
    s.bf2 = (param >> 4) & 0xFFF;
    
    /* STRICT_LOW_PART */
    short src = param;
    int dest = src;
    
    /* SUBREG */
    v4si v1 = {param, param+1, param+2, param+3};
    int elem = v1[param % 4];
    
    /* MEM_P with complex address */
    int idx = (param * 37) % 100;
    global_array[idx] = elem + dest + s.bf1;
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r" (dest), "r" (elem), "m" (global_array[idx]));
}

/* ===== Main driver ===== */
int main(void) {
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i;
    }
    
    /* Test each pattern multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_mem_p(i, i * 10);
        test_combined(i);
    }
    
    /* Verify something was written */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += global_array[i];
    }
    
    return sum > 0 ? 0 : 1;
}
