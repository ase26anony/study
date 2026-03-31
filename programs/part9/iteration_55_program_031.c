/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_temp = 0;

/* ===== ZERO_EXTRACT patterns ===== */
/* Bit-field assignments often generate ZERO_EXTRACT in RTL */
struct bitfield_struct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
};

/* Use volatile to prevent optimization */
volatile struct bitfield_struct g_bf;

NOINLINE void test_zero_extract(int a, int b, int c) {
    /* Multiple bit-field assignments to increase coverage */
    struct bitfield_struct local_bf;
    volatile struct bitfield_struct *volatile_ptr = &g_bf;
    
    /* Direct bit-field assignment - should generate ZERO_EXTRACT */
    local_bf.field1 = a & 0x7;          /* ZERO_EXTRACT target */
    local_bf.field2 = b & 0x1F;         /* ZERO_EXTRACT target */
    local_bf.field3 = c & 0xFF;         /* ZERO_EXTRACT target */
    
    /* Volatile bit-field assignment */
    volatile_ptr->field4 = (a + b) & 0xFFFF;  /* ZERO_EXTRACT target */
    
    /* Nested bit-field in struct */
    struct {
        struct bitfield_struct inner;
        int padding;
    } nested;
    nested.inner.field2 = b & 0x1F;     /* ZERO_EXTRACT target */
    
    /* Force use of variables */
    g_temp = local_bf.field1 + volatile_ptr->field4 + nested.inner.field2;
}

/* ===== STRICT_LOW_PART patterns ===== */
/* Partial register writes, especially on 32-bit x86 */
NOINLINE void test_strict_low_part(short s_val, char c_val, int i_val) {
    volatile int dest1, dest2, dest3;
    
    /* short to int assignment - may generate STRICT_LOW_PART */
    dest1 = s_val;                      /* Potential STRICT_LOW_PART target */
    
    /* char to long assignment */
    long l_dest;
    l_dest = c_val;                     /* Potential STRICT_LOW_PART target */
    
    /* Multiple partial writes */
    int combined = 0;
    combined = (combined & ~0xFF) | (c_val & 0xFF);  /* STRICT_LOW_PART target */
    
    /* Use inline assembly hint for partial register */
    asm volatile ("# Partial register hint" : "+r"(i_val));
    
    /* Mixed-size operations */
    dest2 = (dest1 & 0xFFFF0000) | (s_val & 0xFFFF);  /* STRICT_LOW_PART target */
    
    /* Force use */
    g_temp = dest1 + dest2 + (int)l_dest + combined;
}

/* ===== SUBREG patterns ===== */
/* Type conversions and vector operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE void test_subreg(float f_val, double d_val, int i_val) {
    volatile v4si vec_int;
    volatile v4sf vec_float;
    v8hi vec_short;
    
    /* Vector initialization */
    vec_int = (v4si){i_val, i_val+1, i_val+2, i_val+3};
    vec_float = (v4sf){f_val, f_val*2, f_val*3, f_val*4};
    
    /* Vector element extraction - generates SUBREG */
    int elem0 = vec_int[0];             /* SUBREG target */
    float elem1 = vec_float[1];         /* SUBREG target */
    
    /* Type punning through unions - generates SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = f_val;
    int int_bits = pun.i;               /* SUBREG target */
    
    /* Vector truncation */
    vec_short = __builtin_convertvector(vec_int, v8hi);  /* SUBREG operations */
    
    /* Complex number operations (C99) */
    #ifdef __STDC_IEC_559_COMPLEX__
    double _Complex cplx = d_val + d_val*1.0i;
    double real_part = __real__ cplx;   /* SUBREG target */
    #endif
    
    /* Force use */
    g_temp = elem0 + (int)elem1 + int_bits + vec_short[0];
}

/* ===== MEM_P patterns ===== */
/* Complex memory addresses and stores */
NOINLINE void test_mem_p(int *base, int offset, int value) {
    volatile int array[64];
    volatile int *ptr;
    
    /* Array with variable index - complex MEM address */
    int idx = (offset * g_index) & 63;
    array[idx] = value;                 /* MEM_P target with complex address */
    
    /* Pointer arithmetic with multiple operations */
    ptr = base + (offset & 0xF);
    ptr[g_index] = value * 2;           /* MEM_P target */
    
    /* Struct with pointer chasing */
    struct node {
        int data;
        struct node *next;
    };
    volatile struct node nodes[4];
    
    nodes[offset & 3].data = value;     /* MEM_P target */
    nodes[offset & 3].next = (struct node*)&nodes[(offset + 1) & 3];
    
    /* Multi-dimensional array */
    volatile int matrix[8][8];
    int i = (offset >> 2) & 7;
    int j = offset & 7;
    matrix[i][j] = value + i - j;       /* MEM_P target */
    
    /* Force use and prevent dead store elimination */
    asm volatile ("" : : "r"(array[0]), "r"(ptr[0]), "r"(nodes[0].data), "r"(matrix[0][0]));
}

/* ===== Combined test function ===== */
/* Mix all patterns in one function for maximum coverage */
NOINLINE __attribute__((optimize("O2"))) 
void test_combined(int seed) {
    /* ZERO_EXTRACT patterns */
    struct {
        unsigned int bf1 : 4;
        unsigned int bf2 : 12;
    } local_combined;
    local_combined.bf1 = seed & 0xF;    /* ZERO_EXTRACT target */
    local_combined.bf2 = (seed >> 4) & 0xFFF;  /* ZERO_EXTRACT target */
    
    /* STRICT_LOW_PART patterns */
    short short_val = seed & 0x7FFF;
    int int_val = seed;
    int partial = (int_val & ~0xFFFF) | (short_val & 0xFFFF);  /* STRICT_LOW_PART target */
    
    /* SUBREG patterns */
    typedef float v2f __attribute__((vector_size(8)));
    v2f vec2 = {seed * 0.5f, seed * 1.5f};
    float f_elem = vec2[seed & 1];      /* SUBREG target */
    
    /* MEM_P patterns */
    volatile int mem_array[32];
    int mem_idx = (seed * 17) & 31;
    mem_array[mem_idx] = partial + (int)f_elem;  /* MEM_P target */
    
    /* Complex control flow to prevent optimization */
    for (int i = 0; i < (seed & 3); i++) {
        mem_array[(mem_idx + i) & 31] += local_combined.bf1 + local_combined.bf2;
    }
    
    g_temp = partial + (int)f_elem + mem_array[0];
}

/* ===== Main driver ===== */
int main(int argc, char **argv) {
    int base_seed = argc > 1 ? atoi(argv[1]) : 12345;
    
    /* Test each pattern multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        int seed = base_seed + i * 1001;
        
        test_zero_extract(seed & 0xFF, (seed >> 8) & 0xFF, (seed >> 16) & 0xFF);
        test_strict_low_part(seed & 0x7FFF, seed & 0x7F, seed);
        test_subreg(seed * 0.1f, seed * 0.01, seed);
        
        int array[100];
        for (int j = 0; j < 100; j++) array[j] = seed + j;
        test_mem_p(array, seed & 0x3F, seed);
        
        test_combined(seed);
    }
    
    /* Final validation to prevent dead code elimination */
    asm volatile ("" : : "r"(g_temp));
    
    return g_temp != 0;
}
