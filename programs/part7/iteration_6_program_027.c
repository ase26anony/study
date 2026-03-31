/* Test program to cover lines 282-290 in resource.cc (mark_referenced_resources) */
#include <stdint.h>
#include <stdlib.h>

/* Compile-time check for optimization */
#ifdef __OPTIMIZE__
#define OPTIMIZED 1
#else
#error "Compile with optimization enabled (-O2 or -O3)"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
};

/* Function 1: Generate ZERO_EXTRACT patterns through bit-field operations */
NOINLINE static unsigned int test_zero_extract(struct bitfield *bf) {
    unsigned int result = 0;
    
    /* Multiple bit-field extractions that may generate ZERO_EXTRACT */
    result = bf->field1;                    /* Simple extraction */
    result |= (bf->field2 << 4);            /* Shifted extraction */
    result |= ((bf->field3 >> 2) & 0x3F);   /* Shifted and masked extraction */
    
    /* Additional bit-field operations */
    volatile unsigned int temp = bf->field2;
    result ^= (temp & 0x0F);                /* Masked extraction from volatile */
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART patterns via inline assembly (x86) */
NOINLINE static unsigned int test_strict_low_part(unsigned int val) {
    unsigned char byte_val;
    unsigned short word_val;
    
#ifdef __x86_64__ || __i386__
    /* Byte operation that may generate STRICT_LOW_PART */
    asm volatile ("movb %1, %0" 
                  : "=q"(byte_val)          /* =q constraint for byte register */
                  : "r"((unsigned char)val)
                  : "cc");
    
    /* Word operation */
    asm volatile ("movw %1, %0"
                  : "=r"(word_val)          /* May generate STRICT_LOW_PART for 16-bit */
                  : "r"((unsigned short)val)
                  : "cc");
#else
    /* Fallback: operations on sub-parts that might generate similar patterns */
    byte_val = (unsigned char)val;
    word_val = (unsigned short)val;
    
    /* Force partial register updates */
    volatile unsigned int *p = &val;
    *((unsigned char *)p) = byte_val;       /* Byte store through pointer */
#endif
    
    return byte_val + word_val;
}

/* Function 3: Generate SUBREG patterns through type conversions */
NOINLINE static int test_subreg(int a, short b, char c) {
    int result = 0;
    
    /* Various type conversions that may generate SUBREG */
    short s = a;                    /* int -> short */
    char ch = b;                    /* short -> char */
    int i1 = s;                     /* short -> int */
    int i2 = ch;                    /* char -> int */
    
    /* Mixed-size operations */
    result = a + s;                 /* int + short */
    result += i1 * i2;              /* Mixed operations */
    
    /* Access halves of 64-bit value */
    long long ll = (long long)a * b;
    int lower = (int)ll;            /* Lower 32 bits */
    int upper = (int)(ll >> 32);    /* Upper 32 bits */
    
    result += lower + upper;
    
    /* Pointer-based type punning */
    union {
        int i;
        short s[2];
    } u;
    u.i = a;
    result += u.s[0] + u.s[1];      /* Access sub-parts through union */
    
    return result;
}

/* Function 4: Generate complex MEM_P patterns with addressing modes */
NOINLINE static int test_mem_operands(int *base, int index1, int index2) {
    int result = 0;
    
    /* Complex array indexing with variable offsets */
    int arr[10][10];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Multiple memory accesses with complex addressing */
    result += arr[index1][index2];                  /* 2D array access */
    result += *(base + index1 * 2 - index2);        /* Pointer arithmetic */
    result += arr[index2 % 10][index1 % 10];        /* Modulo in indices */
    
    /* Structure with multiple fields */
    struct {
        int a;
        int b;
        int c[5];
    } s = {0};
    
    s.a = index1;
    s.b = index2;
    for (int i = 0; i < 5; i++) {
        s.c[i] = i * index1;
        result += s.c[i];                           /* Structure field access */
    }
    
    /* Memory access through multiple levels of indirection */
    int **ptr_ptr = &base;
    result += **ptr_ptr;                            /* Double indirection */
    
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    struct bitfield bf = {1, 2, 3};
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    /* Loop to increase chance of RTL generation and resource marking */
    for (int i = 0; i < 100; i++) {
        /* Exercise each pattern function */
        result += test_zero_extract(&bf);
        result += test_strict_low_part(i);
        result += test_subreg(i, i * 2, i * 3);
        result += test_mem_operands(array, i % 10, (i * 7) % 10);
        
        /* Conditional to prevent loop optimization */
        if (result > 1000000) {
            result = result % 1000;
        }
    }
    
    /* Return predictable result */
    return result % 256;
}
