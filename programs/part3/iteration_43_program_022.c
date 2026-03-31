/* gcc -O2 -m32 -fno-strict-aliasing -funroll-loops -fdump-rtl-all -o coverage_test coverage_test.c */

#include <stdint.h>
#include <stdlib.h>

/* Function A: ZERO_EXTRACT and MEM patterns */
static void __attribute__((noinline))
pattern_a(volatile int *arr, int idx1, int idx2)
{
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 3;
        volatile unsigned int field3 : 8;
    } bit_struct;
    
    /* MEM pattern with complex addressing */
    volatile int val = arr[idx1 * 8 + idx2];
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    bit_struct.field1 = val & 0x1F;
    bit_struct.field2 = (val >> 5) & 0x7;
    bit_struct.field3 = (val >> 8) & 0xFF;
    
    /* More MEM patterns with pointer arithmetic */
    volatile int *ptr = arr + idx1;
    volatile int v1 = ptr[idx2];
    volatile int v2 = ptr[idx2 * 2];
    
    /* Use results to prevent elimination */
    asm volatile("" : : "r"(v1), "r"(v2));
}

/* Function B: STRICT_LOW_PART and SUBREG patterns */
static void __attribute__((noinline))
pattern_b(volatile short *ps, volatile char *pc)
{
    /* STRICT_LOW_PART pattern using inline assembly */
    unsigned short var16;
    unsigned char var8;
    
    /* Initialize with volatile reads */
    var16 = *ps;
    var8 = *pc;
    
    /* STRICT_LOW_PART: modify only low byte of 16-bit register */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(var16)
        : "0"(var16)
        : "cc"
    );
    
    /* Another STRICT_LOW_PART pattern with different operation */
    asm volatile (
        "orb $0x0F, %0\n\t"
        : "=q"(var8)
        : "0"(var8)
        : "cc"
    );
    
    /* SUBREG pattern: type punning between different sizes */
    int32_t i32;
    int16_t *p16 = (int16_t*)&i32;
    int8_t *p8 = (int8_t*)&i32;
    
    /* Mixed-size accesses generating SUBREG RTL */
    i32 = 0x12345678;
    *p16 = 0xABCD;      /* SUBREG access to lower 16 bits */
    p8[2] = 0xEF;       /* SUBREG access to third byte */
    
    /* More SUBREG: access 64-bit value as 32-bit parts on 32-bit target */
    int64_t i64 = 0;
    int32_t *p32 = (int32_t*)&i64;
    p32[0] = var16;     /* SUBREG for low 32 bits */
    p32[1] = var8;      /* SUBREG for high 32 bits */
    
    /* Use results */
    asm volatile("" : : "r"(i32), "r"(i64));
}

/* Function C: Complex expression mixing patterns */
static void __attribute__((noinline))
pattern_c(volatile int *arr1, volatile int *arr2, int cond)
{
    /* Complex addressing mode for MEM */
    volatile int *select = cond ? arr1 : arr2;
    
    /* Nested array access with multiple indices */
    int matrix[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* MEM with complex addressing */
            matrix[i][j] = select[i * j];
        }
    }
    
    /* Bit-field in union for potential ZERO_EXTRACT */
    union {
        struct {
            volatile unsigned int a : 4;
            volatile unsigned int b : 4;
            volatile unsigned int c : 4;
        } bits;
        volatile unsigned int full;
    } u;
    
    /* Conditional ZERO_EXTRACT assignment */
    u.full = cond ? 0xAAA : 0x555;
    u.bits.a = matrix[0][0] & 0xF;
    u.bits.b = matrix[1][1] & 0xF;
    u.bits.c = matrix[2][2] & 0xF;
    
    /* Pointer chasing creating MEM chains */
    volatile int **pp = (volatile int**)&select;
    volatile int *p = *pp;
    volatile int v = p[cond];
    
    /* Use result */
    asm volatile("" : : "r"(u.full), "r"(v));
}

/* Helper to create more SUBREG patterns */
static void __attribute__((noinline))
subreg_helper(volatile int *ptr)
{
    /* Access int as chars (SUBREG patterns) */
    volatile char *cptr = (volatile char*)ptr;
    cptr[0] = 1;
    cptr[1] = 2;
    cptr[2] = 3;
    cptr[3] = 4;
    
    /* Access int as short (more SUBREG) */
    volatile short *sptr = (volatile short*)ptr;
    sptr[0] = 0x1234;
    sptr[1] = 0x5678;
}

int main(int argc, char **argv)
{
    volatile int iteration_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (iteration_limit <= 0) iteration_limit = 100;
    
    /* Volatile data to prevent optimization */
    volatile int arr1[64];
    volatile int arr2[64];
    volatile short sarr[64];
    volatile char carr[64];
    
    /* Initialize with some values */
    for (int i = 0; i < 64; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        sarr[i] = i & 0xFFFF;
        carr[i] = i & 0xFF;
    }
    
    volatile int sum = 0;
    
    /* Main loop to generate repeated RTL patterns */
    for (volatile int iter = 0; iter < iteration_limit; iter++) {
        int idx1 = iter % 8;
        int idx2 = (iter * 3) % 8;
        int cond = iter & 1;
        
        /* Call pattern functions */
        pattern_a((int*)arr1, idx1, idx2);
        pattern_b((short*)sarr + idx1, (char*)carr + idx2);
        pattern_c((int*)arr1, (int*)arr2, cond);
        subreg_helper((int*)arr1 + idx1);
        
        /* Dummy operation to prevent elimination */
        sum += arr1[idx1] + arr2[idx2];
    }
    
    /* Prevent dead code elimination of sum */
    asm volatile("" : : "r"(sum));
    
    return 0;
}
