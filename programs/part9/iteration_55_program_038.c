/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 0;
volatile int g_value = 42;
volatile int g_condition = 1;

/* Structure with bit-fields for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field3 : 3;    /* Will generate ZERO_EXTRACT */
    volatile unsigned int field5 : 5;
    volatile unsigned int field8 : 8;
    volatile unsigned int padding : 16;
};

/* Vector types for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Global arrays for MEM operations */
int global_array[256];
struct bitfield_struct global_bf;

/* Function 1: Focus on ZERO_EXTRACT and MEM_P */
NOINLINE __attribute__((optimize("O2")))
void test_zero_extract_and_mem(int idx, int val) {
    struct bitfield_struct local_bf;
    int *ptr;
    
    /* ZERO_EXTRACT: Bit-field assignment */
    local_bf.field3 = val & 0x7;      /* Should generate ZERO_EXTRACT in SET_DEST */
    local_bf.field5 = (val >> 3) & 0x1F;
    local_bf.field8 = (val >> 8) & 0xFF;
    
    /* MEM_P: Complex memory store with pointer arithmetic */
    ptr = &global_array[idx];
    ptr[0] = val;                     /* Simple MEM */
    ptr[idx % 128] = val * 2;         /* More complex MEM with index */
    
    /* MEM_P with even more complex address calculation */
    if (g_condition) {
        global_array[(idx * 17) & 255] = local_bf.field8;
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(local_bf.field3), "r"(ptr[0]));
}

/* Function 2: Focus on STRICT_LOW_PART and SUBREG */
NOINLINE __attribute__((optimize("O1")))
void test_strict_low_part_and_subreg(short s_val, char c_val, long l_val) {
    int dest_int;
    long dest_long;
    short dest_short;
    v4si vec_a, vec_b;
    v8hi vec_short;
    
    /* STRICT_LOW_PART: Partial register writes */
    dest_int = s_val;                 /* short to int - may generate STRICT_LOW_PART */
    dest_long = c_val;                /* char to long - may generate STRICT_LOW_PART */
    
    /* More explicit partial register access */
    dest_short = c_val;               /* char to short */
    dest_int = dest_short;            /* short to int - chain of partial writes */
    
    /* SUBREG: Vector operations */
    vec_a = (v4si){1, 2, 3, 4};
    vec_b = (v4si){5, 6, 7, 8};
    
    /* Vector lane extraction - generates SUBREG */
    int lane0 = vec_a[0];             /* SUBREG extraction from vector */
    int lane2 = vec_a[2];
    
    /* Vector store to memory - MEM with SUBREG address */
    int temp_array[4];
    memcpy(temp_array, &vec_a, sizeof(vec_a));
    
    /* Type punning through union for SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14f;
    dest_int = pun.i;                 /* May involve SUBREG for type conversion */
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(dest_int), "r"(lane0), "r"(temp_array[0]));
}

/* Function 3: Mixed patterns with control flow */
NOINLINE __attribute__((optimize("O2")))
void test_mixed_patterns(int iter, int base) {
    volatile struct bitfield_struct mixed_bf;
    int temp_storage[8];
    int i;
    
    /* Loop to create multiple basic blocks */
    for (i = 0; i < iter && i < 8; i++) {
        /* ZERO_EXTRACT in loop */
        mixed_bf.field3 = (base + i) & 0x7;
        
        /* MEM_P with varying address */
        temp_storage[i] = mixed_bf.field3 * i;
        
        /* Conditional STRICT_LOW_PART-like behavior */
        if (i & 1) {
            short partial = (base + i) & 0xFFFF;
            temp_storage[i] = partial;  /* short to int assignment */
        }
    }
    
    /* SUBREG through vector operations */
    if (iter > 4) {
        v4si vec = {0};
        for (i = 0; i < 4; i++) {
            vec[i] = temp_storage[i];   /* Vector element set - may use SUBREG */
        }
        
        /* Store vector to memory - complex MEM address */
        int* aligned_ptr = (int*)(((uintptr_t)temp_storage + 15) & ~15);
        if (aligned_ptr < temp_storage + 8) {
            memcpy(aligned_ptr, &vec, sizeof(vec));
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(temp_storage[0]), "r"(mixed_bf.field3));
}

/* Helper function to create complex address expressions */
NOINLINE int* get_complex_ptr(int idx) {
    if (g_condition) {
        return &global_array[(idx * 13 + 7) & 255];
    } else {
        return &global_array[(idx * 29 + 11) & 255];
    }
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    int i;
    
    /* Initialize global data */
    memset(global_array, 0, sizeof(global_array));
    memset(&global_bf, 0, sizeof(global_bf));
    
    /* Test with various inputs to hit different code paths */
    for (i = 0; i < 10; i++) {
        test_zero_extract_and_mem(i * 3, i * 7 + 1);
        test_strict_low_part_and_subreg(i * 100, i * 10 + 5, i * 1000L + 123);
        test_mixed_patterns(i + 1, i * 11);
        
        /* Additional MEM_P test with complex pointer */
        int* complex_ptr = get_complex_ptr(i);
        *complex_ptr = i * i;          /* MEM store with complex address */
        
        /* Global bit-field assignment */
        global_bf.field8 = i & 0xFF;   /* ZERO_EXTRACT on global */
    }
    
    /* Final validation to prevent optimization */
    int sum = 0;
    for (i = 0; i < 256; i++) {
        sum += global_array[i];
    }
    
    asm volatile("" : : "r"(sum), "r"(global_bf.field3));
    
    return sum > 0 ? 0 : 1;
}
