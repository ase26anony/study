/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_condition = 1;

/* ==================== ZERO_EXTRACT patterns ==================== */
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment - should generate ZERO_EXTRACT in SET_DEST */
    struct {
        volatile unsigned int field1 : 3;
        volatile unsigned int field2 : 5;
        volatile unsigned int field3 : 8;
    } bit_struct;
    
    bit_struct.field1 = 5;      /* ZERO_EXTRACT for 3-bit field */
    bit_struct.field2 = 17;     /* ZERO_EXTRACT for 5-bit field */
    bit_struct.field3 = 123;    /* ZERO_EXTRACT for 8-bit field */
    
    /* Volatile bit-field in union */
    union {
        volatile unsigned int full;
        struct {
            volatile unsigned int low : 16;
            volatile unsigned int high : 16;
        } parts;
    } bit_union;
    
    bit_union.parts.low = 0xABCD;   /* ZERO_EXTRACT for 16-bit field */
    bit_union.parts.high = 0x1234;  /* ZERO_EXTRACT for 16-bit field */
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(bit_struct.field1), "r"(bit_union.full));
}

/* ==================== STRICT_LOW_PART patterns ==================== */
NOINLINE void test_strict_low_part(int x) {
    /* Mixed-size integer assignments - may generate STRICT_LOW_PART */
    volatile short src_short = x & 0xFFFF;
    volatile char src_char = x & 0xFF;
    
    int dest_int;
    long dest_long;
    
    /* These assignments might use STRICT_LOW_PART for partial register writes */
    dest_int = src_short;      /* Possible STRICT_LOW_PART for 16->32 bit */
    dest_long = src_char;      /* Possible STRICT_LOW_PART for 8->64 bit */
    
    /* Use inline assembly to force partial register writes on x86 */
    #ifdef __i386__
    asm volatile(
        "movw %1, %%ax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(dest_int)
        : "r"(src_short)
        : "eax"
    );
    #endif
    
    /* Prevent optimization */
    asm volatile("" : : "r"(dest_int), "r"(dest_long));
}

/* ==================== SUBREG patterns ==================== */
NOINLINE void test_subreg(void) {
    /* GCC vector types - often generate SUBREG operations */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    
    /* Vector operations that might use SUBREG */
    v4si vec_c = vec_a + vec_b;
    
    /* Extract lane - creates SUBREG */
    int lane0 = vec_c[0];      /* SUBREG extraction */
    int lane2 = vec_c[2];      /* SUBREG extraction */
    
    /* Type punning through union - creates SUBREG */
    union {
        float f;
        int i;
    } pun;
    
    pun.f = 3.14159f;
    int int_bits = pun.i;      /* SUBREG for type conversion */
    
    /* Complex number assignment */
    _Complex float c1 = 1.0f + 2.0fi;
    _Complex float c2 = c1;    /* May use SUBREG */
    
    /* Prevent dead code */
    asm volatile("" : : "r"(lane0), "r"(int_bits), "r"(c2));
}

/* ==================== MEM_P patterns ==================== */
NOINLINE void test_mem_dest(int index, int value) {
    /* Array with complex indexing - creates MEM with address expression */
    int array[100];
    
    /* Store with non-constant index - MEM destination */
    array[index] = value;                     /* MEM with index addressing */
    array[index * 2 + 1] = value * 2;         /* MEM with complex address */
    array[g_index] = g_value;                 /* MEM with global variable */
    
    /* Pointer arithmetic for complex addresses */
    int *ptr = &array[50];
    ptr[index] = value;                       /* MEM with pointer+offset */
    *(ptr + index * 3) = value * 3;           /* MEM with scaled offset */
    
    /* Struct with pointer access */
    struct {
        int a;
        int b;
        int c[10];
    } mystruct;
    
    mystruct.a = value;                       /* MEM with struct field */
    mystruct.c[index % 10] = value;           /* MEM with array in struct */
    
    /* Multi-dimensional array */
    int matrix[10][10];
    matrix[index % 10][index / 10] = value;   /* MEM with 2D indexing */
    
    /* Prevent optimization */
    asm volatile("" : : "m"(array[0]), "m"(mystruct), "m"(matrix[0][0]));
}

/* ==================== Combined test function ==================== */
NOINLINE __attribute__((optimize("O2"))) 
void combined_test(int x) {
    /* Mix all patterns in one function with optimization */
    
    /* ZERO_EXTRACT pattern */
    struct {
        volatile unsigned bits : 4;
    } bf;
    bf.bits = x & 0xF;
    
    /* STRICT_LOW_PART pattern */
    short s = x;
    int i = s;  /* Potential STRICT_LOW_PART */
    
    /* SUBREG pattern with vectors */
    typedef float v4f __attribute__((vector_size(16)));
    v4f v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    float f = v1[0];  /* SUBREG extraction */
    
    /* MEM_P pattern with complex addressing */
    int arr[20];
    for (int j = 0; j < 10; j++) {
        arr[j * 2] = x + j;  /* MEM with loop-varying index */
    }
    
    /* Control flow to prevent single basic block */
    if (x > 0) {
        arr[0] = i;
    } else {
        arr[1] = (int)f;
    }
    
    asm volatile("" : : "r"(bf.bits), "r"(i), "r"(f), "m"(arr[0]));
}

/* ==================== Helper for complex addresses ==================== */
NOINLINE int* get_pointer(int *base, int offset) {
    /* Function returning pointer with condition - creates complex address exprs */
    if (g_condition) {
        return base + offset * 2;
    } else {
        return base + offset;
    }
}

/* ==================== Main test driver ==================== */
int main(void) {
    /* Test with various inputs to ensure different code paths */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part(i);
        test_subreg();
        test_mem_dest(i, i * 10);
        combined_test(i);
        
        /* Additional test with pointer helper */
        int buffer[100];
        int *ptr = get_pointer(buffer, i);
        *ptr = i;  /* MEM with function-derived address */
    }
    
    return 0;
}
