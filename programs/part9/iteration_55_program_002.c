/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 0;
volatile int g_value = 42;
volatile int g_condition = 1;

/* ==================== ZERO_EXTRACT patterns ==================== */
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment - should generate ZERO_EXTRACT in RTL */
    struct {
        volatile unsigned int field1 : 3;
        volatile unsigned int field2 : 5;
        volatile unsigned int field3 : 8;
    } bit_struct;
    
    bit_struct.field1 = 5;      /* ZERO_EXTRACT destination */
    bit_struct.field2 = 31;     /* Another bit-field write */
    bit_struct.field3 = bit_struct.field1 + bit_struct.field2;
    
    /* Volatile bit-field in union */
    union {
        volatile unsigned int full;
        struct {
            volatile unsigned int low : 16;
            volatile unsigned int high : 16;
        } parts;
    } bit_union;
    
    bit_union.parts.low = 0xABCD;   /* ZERO_EXTRACT destination */
    bit_union.parts.high = 0x1234;  /* Another ZERO_EXTRACT */
    
    /* Force use of results */
    asm volatile("" : : "r"(bit_struct.field3), "r"(bit_union.full));
}

/* ==================== STRICT_LOW_PART patterns ==================== */
NOINLINE void test_strict_low_part(int x) {
    /* Mixed-size assignments - common source of STRICT_LOW_PART */
    short src16 = x & 0xFFFF;
    int dest32 = 0;
    
    /* This assignment may generate STRICT_LOW_PART on x86 */
    dest32 = src16;                 /* Potential STRICT_LOW_PART destination */
    
    /* Char to int assignment */
    char c = x & 0xFF;
    int i = c;                      /* Another potential STRICT_LOW_PART */
    
    /* Through pointer with type punning */
    int32_t val32 = 0x12345678;
    int16_t *ptr16 = (int16_t*)&val32;
    *ptr16 = 0xABCD;               /* Partial write through pointer */
    
    /* Use results */
    asm volatile("" : : "r"(dest32), "r"(i), "r"(val32));
}

/* ==================== SUBREG patterns ==================== */
NOINLINE void test_subreg(float f, double d) {
    /* Type punning between float and int */
    float f1 = f * 2.0f;
    int i1;
    memcpy(&i1, &f1, sizeof(i1));  /* May generate SUBREG operations */
    
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = vec_a + vec_b;
    
    /* Extract lane - may use SUBREG */
    int lane0 = vec_c[0];          /* SUBREG destination */
    int lane1 = vec_c[1];
    
    /* Vector type conversion */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    v4si int_vec = __builtin_convertvector(short_vec, v4si);
    
    /* Use results */
    asm volatile("" : : "r"(i1), "r"(lane0), "r"(lane1), "r"(int_vec[0]));
}

/* ==================== MEM_P patterns ==================== */
NOINLINE void test_mem_dest(int *base, int offset) {
    int array[64];
    int *ptr;
    
    /* Complex address calculation for MEM destination */
    ptr = &array[g_index * 2 + offset];  /* Non-constant index */
    *ptr = g_value;                      /* MEM destination with complex address */
    
    /* Pointer arithmetic with multiple components */
    int *p = base + (g_condition ? 10 : 20);
    p[g_index] = 100;                    /* Another MEM with complex address */
    
    /* Struct with pointer chain */
    struct node {
        int value;
        struct node *next;
    } nodes[4];
    
    nodes[0].value = 1;
    nodes[1].value = nodes[0].value + 1;
    nodes[2].value = nodes[1].value * 2;
    
    /* Store through computed pointer */
    struct node *current = &nodes[g_index % 3];
    current->value = 999;                /* MEM destination through struct */
    
    /* Use results */
    asm volatile("" : : "r"(array[0]), "r"(p[0]), "r"(nodes[3].value));
}

/* ==================== Combined test function ==================== */
/* Use O2 optimization specifically for this function */
__attribute__((optimize("O2"))) 
NOINLINE void combined_test(int x, float f, int *ptr) {
    /* Mix all patterns in one function */
    
    /* ZERO_EXTRACT */
    struct {
        volatile unsigned int flags : 4;
        volatile unsigned int mode : 4;
    } settings;
    settings.flags = x & 0xF;
    settings.mode = (x >> 4) & 0xF;
    
    /* STRICT_LOW_PART */
    short s = x & 0x7FFF;
    int combined = s;
    
    /* SUBREG with vectors */
    typedef float v4f __attribute__((vector_size(16)));
    v4f v1 = {f, f+1, f+2, f+3};
    v4f v2 = v1 * v1;
    float first = v2[0];  /* SUBREG extract */
    
    /* MEM with complex address */
    int buffer[32];
    int idx = (x * 17) % 32;
    buffer[idx] = combined;  /* MEM destination */
    
    /* Pointer chain */
    int *p = ptr + idx;
    *p = first;
    
    /* Use everything */
    asm volatile("" : : "r"(settings.flags), "r"(combined), 
                 "r"(first), "r"(buffer[0]), "r"(*p));
}

/* ==================== Main driver ==================== */
int main(void) {
    int data[100];
    float fvalues[] = {1.0f, 2.5f, 3.14f, 0.0f};
    
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        data[i] = i * 3;
    }
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part(i);
        test_subreg(fvalues[i % 4], fvalues[i % 4] * 2.0);
        test_mem_dest(data, i);
        combined_test(i, fvalues[i % 4], data + i * 2);
    }
    
    return 0;
}
