/* resource_patterns.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stddef.h>

/* Prevent optimization and force resource tracking */
static volatile int volatile_counter = 0;
static volatile int volatile_sum = 0;

/* ========== PATTERN 1: ZERO_EXTRACT + MEM ========== */
struct BitfieldStruct {
    volatile unsigned int field1 : 5;
    volatile unsigned int field2 : 7;
    volatile unsigned int field3 : 3;
    volatile unsigned int padding : 17;
};

static void __attribute__((noinline)) 
pattern_zero_extract_mem(struct BitfieldStruct *s, int idx) {
    /* Complex addressing with MEM */
    volatile int *arr = (volatile int*)s;
    
    /* Multiple MEM accesses with addressing modes */
    volatile int temp = arr[idx] + arr[idx + 1] * 2;
    
    /* ZERO_EXTRACT through volatile bit-field assignment */
    s->field1 = (temp & 0x1F);      /* 5 bits */
    s->field2 = ((temp >> 5) & 0x7F); /* 7 bits */
    s->field3 = ((temp >> 12) & 0x7); /* 3 bits */
    
    /* More MEM with pointer arithmetic */
    volatile int *ptr = &arr[idx];
    ptr += (temp & 0x3);
    volatile_counter += *ptr;
}

/* ========== PATTERN 2: STRICT_LOW_PART + SUBREG ========== */
static void __attribute__((noinline))
pattern_strict_low_part_subreg(int base) {
    volatile char c = (char)(base & 0xFF);
    volatile short s = (short)(base & 0xFFFF);
    volatile int i = base;
    
    /* STRICT_LOW_PART via inline assembly modifying byte part */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c) 
        : "0"(c) 
        : "cc"
    );
    
    /* Another STRICT_LOW_PART for 16-bit part */
    asm volatile (
        "addw $1, %0\n\t"
        : "=r"(s)
        : "0"(s)
        : "cc"
    );
    
    /* SUBREG through type punning */
    short *ps = (short*)&i;
    *ps = (short)(s + 1);           /* SUBREG access to lower 16 bits */
    
    /* More SUBREG with char access */
    char *pc = (char*)&i;
    pc[2] = (char)(c + 2);          /* SUBREG access to byte */
    
    /* Mixed-size accesses causing SUBREG operations */
    i = (i & 0xFFFF0000) | ((int)s << 8) | c;
    
    volatile_sum += i;
}

/* ========== PATTERN 3: Complex mixed patterns ========== */
static void __attribute__((noinline))
pattern_complex_mixed(int idx1, int idx2) {
    /* Create array with volatile elements for MEM patterns */
    volatile int matrix[4][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    
    /* Complex MEM addressing with multiple indices */
    volatile int val = matrix[idx1 & 0x3][idx2 & 0x3];
    val += matrix[(idx1 + 1) & 0x3][(idx2 + 1) & 0x3];
    val *= matrix[(idx1 + 2) & 0x3][(idx2 + 2) & 0x3];
    
    /* Bit-field struct for ZERO_EXTRACT */
    struct {
        volatile unsigned int low : 4;
        volatile unsigned int mid : 8;
        volatile unsigned int high : 4;
    } bf;
    
    /* Ternary selecting different addressing modes */
    volatile int *select_ptr = (val & 1) ? 
                               &matrix[0][0] : 
                               &matrix[2][2];
    
    /* Assignment involving multiple transformations */
    bf.low = (*select_ptr & 0xF);
    bf.mid = ((val >> 4) & 0xFF);
    bf.high = ((val >> 12) & 0xF);
    
    /* More SUBREG through type casting */
    unsigned short *usp = (unsigned short*)&val;
    usp[0] = (unsigned short)bf.low;
    usp[1] = (unsigned short)bf.mid;
    
    volatile_counter += val + bf.low + bf.mid + bf.high;
}

/* ========== PATTERN 4: Loop-based pattern generation ========== */
static void __attribute__((noinline))
pattern_loop_based(int iterations) {
    struct BitfieldStruct bf_array[4];
    volatile int temp_array[8];
    
    /* Initialize arrays */
    for (int i = 0; i < 4; i++) {
        bf_array[i].field1 = i;
        bf_array[i].field2 = i * 2;
        bf_array[i].field3 = i * 3;
    }
    
    for (int i = 0; i < 8; i++) {
        temp_array[i] = i * 10;
    }
    
    /* Loop with mixed patterns */
    for (volatile int i = 0; i < iterations; i++) {
        int idx = i & 0x3;
        
        /* MEM with complex addressing */
        volatile int *ptr1 = &temp_array[idx];
        volatile int *ptr2 = &temp_array[idx + 1];
        volatile int val = *ptr1 + *ptr2 * 2;
        
        /* ZERO_EXTRACT assignment */
        bf_array[idx].field1 = (val & 0x1F);
        bf_array[idx].field2 = ((val >> 5) & 0x7F);
        
        /* SUBREG access */
        short *sp = (short*)&val;
        sp[1] = (short)(bf_array[idx].field1 * 2);
        
        /* STRICT_LOW_PART for byte */
        volatile char c = (char)val;
        asm volatile (
            "subb $1, %0\n\t"
            : "=q"(c)
            : "0"(c)
            : "cc"
        );
        
        volatile_sum += val + c;
    }
}

/* ========== MAIN FUNCTION ========== */
int main(int argc, char *argv[]) {
    /* Use argc to bound loops for compilation safety */
    int iterations = (argc > 1) ? 3 : 2;
    
    /* Initialize data structures */
    struct BitfieldStruct bs = {0};
    volatile int array_data[16];
    
    for (int i = 0; i < 16; i++) {
        array_data[i] = i * i;
    }
    
    /* Main loop calling pattern functions */
    for (volatile int outer = 0; outer < iterations; outer++) {
        /* Pattern 1: ZERO_EXTRACT + MEM */
        pattern_zero_extract_mem(&bs, outer & 0xF);
        
        /* Pattern 2: STRICT_LOW_PART + SUBREG */
        pattern_strict_low_part_subreg(outer * 100 + array_data[outer & 0xF]);
        
        /* Pattern 3: Complex mixed patterns */
        pattern_complex_mixed(outer & 0x3, (outer >> 2) & 0x3);
        
        /* Pattern 4: Loop-based patterns */
        pattern_loop_based(2);
        
        /* Prevent dead code elimination */
        volatile_sum += bs.field1 + bs.field2 + bs.field3;
        volatile_counter++;
    }
    
    /* Final dummy operation */
    asm volatile ("" : : "r"(volatile_sum), "r"(volatile_counter));
    
    return 0;
}
