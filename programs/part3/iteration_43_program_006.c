/* resource_patterns.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stddef.h>

/* Pattern 1: ZERO_EXTRACT + MEM */
static void __attribute__((noinline)) 
pattern_zero_extract_mem(volatile int *counter) 
{
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct S { 
        volatile unsigned int f1:5;
        volatile unsigned int f2:7;
        volatile unsigned int f3:4;
    } s;
    
    /* Array with complex addressing for MEM */
    volatile int arr[16][8];
    volatile int *ptr;
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    s.f1 = (*counter) & 0x1F;
    s.f2 = ((*counter) >> 5) & 0x7F;
    s.f3 = ((*counter) >> 12) & 0x0F;
    
    /* MEM pattern with complex addressing */
    ptr = &arr[(*counter) & 0xF][((*counter) >> 4) & 0x7];
    *ptr = s.f1 + s.f2 + s.f3;
    
    /* Additional MEM with pointer arithmetic */
    volatile int *p = (volatile int*)&s;
    p[0] = arr[1][2] + arr[3][4];  /* Type punning through pointer */
}

/* Pattern 2: STRICT_LOW_PART + SUBREG */
static void __attribute__((noinline))
pattern_strict_low_part_subreg(volatile int *counter)
{
    volatile short hs = *counter & 0xFFFF;
    volatile char c = *counter & 0xFF;
    volatile int i = *counter;
    
    /* STRICT_LOW_PART pattern via inline assembly */
    /* Modify only low byte of a register */
    asm volatile (
        "addb %1, %0\n\t"
        : "=q"(c)          /* =q constraint for byte-addressable register */
        : "q"((char)(*counter)), "0"(c)
        : "cc"
    );
    
    /* Another STRICT_LOW_PART pattern */
    asm volatile (
        "incb %0\n\t"
        : "=q"(c)
        : "0"(c)
        : "cc"
    );
    
    /* SUBREG pattern: type punning between different sizes */
    /* Access int as short */
    volatile short *ps = (volatile short*)&i;
    ps[0] = hs + c;        /* Generates SUBREG for partial access */
    ps[1] = hs - c;        /* Another SUBREG access */
    
    /* More SUBREG: mixed-size operations */
    volatile long long ll = (long long)i * 1000;
    volatile int *pi = (volatile int*)&ll;
    i = pi[0] + pi[1];     /* Access 64-bit as two 32-bit parts */
    
    /* SUBREG through pointer casting */
    volatile char *pc = (volatile char*)&hs;
    for (int j = 0; j < 2; j++) {
        pc[j] = (c + j) & 0xFF;  /* Byte-wise access to short */
    }
}

/* Pattern 3: Mixed patterns with ternary operator */
static void __attribute__((noinline))
pattern_mixed_complex(volatile int *counter, volatile int *result)
{
    /* Struct with bit-fields at different positions */
    struct BitFields {
        volatile unsigned int a:3;
        volatile unsigned int b:9;
        volatile unsigned int c:13;
        volatile unsigned int d:7;
    } bf;
    
    /* 2D array for complex MEM addressing */
    volatile int matrix[8][12];
    volatile int *select_ptr;
    
    /* Initialize bit-fields */
    bf.a = (*counter >> 0) & 0x7;
    bf.b = (*counter >> 3) & 0x1FF;
    bf.c = (*counter >> 12) & 0x1FFF;
    bf.d = (*counter >> 25) & 0x7F;
    
    /* Complex expression with ternary selecting address */
    /* This may generate interesting RTL combinations */
    int idx1 = (*counter) & 0x7;
    int idx2 = ((*counter) >> 3) & 0xB;
    
    /* Ternary selects between different array elements */
    select_ptr = (idx1 > idx2) ? 
                 &matrix[idx1][idx2] : 
                 &matrix[idx2 % 8][idx1 % 12];
    
    /* Assignment that could involve multiple RTL transformations */
    *select_ptr = (bf.a << bf.b) | (bf.c & bf.d);
    
    /* Additional MEM with bit-field combination */
    volatile int *bf_ptr = (volatile int*)&bf;
    *result += bf_ptr[0] + *select_ptr;
    
    /* Nested ternary with bit-field reference */
    volatile int temp = (bf.a > 3) ? 
                       ((bf.b & 0xF) << 4) : 
                       ((bf.c & 0xFF) | (bf.d << 8));
    
    /* Complex addressing with multiple indices */
    matrix[(temp >> 4) & 0x7][temp & 0x7] = bf.a + bf.b + bf.c + bf.d;
}

/* Pattern 4: Loop-based pattern generation */
static void __attribute__((noinline))
pattern_loop_based(volatile int iterations)
{
    /* Mixed-size array for SUBREG operations */
    volatile char bytes[64];
    volatile short words[32];
    volatile int dwords[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        bytes[i] = (iterations + i) & 0xFF;
    }
    
    /* SUBREG pattern: access int array as short */
    volatile short *wptr = (volatile short*)dwords;
    for (int i = 0; i < 32; i++) {
        words[i] = wptr[i] + bytes[i*2];  /* Mixed-size access */
    }
    
    /* MEM pattern with complex index calculation */
    for (int i = 0; i < 16; i++) {
        int idx = (i * iterations) & 0xF;
        dwords[idx] = words[i*2] + words[i*2+1] + bytes[i*4];
    }
    
    /* Bit-field operations in loop (potential ZERO_EXTRACT) */
    struct LoopBF {
        volatile unsigned int field:6;
    } lbf;
    
    for (int i = 0; i < 8; i++) {
        lbf.field = (dwords[i] >> (i*3)) & 0x3F;  /* Bit-field assignment */
        bytes[i*8] = lbf.field & 0xFF;
    }
}

/* Main function that drives all patterns */
int main(int argc, char **argv) 
{
    volatile int counter = 0;
    volatile int result = 0;
    
    /* Use argc to bound loops for compilation safety */
    int max_iterations = (argc > 1) ? 10 : 5;
    
    /* Initialize some volatile arrays */
    volatile int init_arr[20];
    for (int i = 0; i < 20; i++) {
        init_arr[i] = i * i;
    }
    
    /* Main loop calling pattern functions */
    for (counter = 0; counter < max_iterations; counter++) {
        /* Call each pattern function */
        pattern_zero_extract_mem(&counter);
        pattern_strict_low_part_subreg(&counter);
        pattern_mixed_complex(&counter, &result);
        pattern_loop_based(counter);
        
        /* Additional volatile operations to prevent optimization */
        result += init_arr[counter % 20];
        
        /* Complex addressing mode */
        volatile int *ptr = &init_arr[(counter * 7) % 20];
        *ptr = result & 0xFFFF;
        
        /* Inline assembly with STRICT_LOW_PART potential */
        volatile char c = counter & 0xFF;
        asm volatile (
            "subb %1, %0\n\t"
            : "=q"(c)
            : "q"((char)(counter >> 2)), "0"(c)
            : "cc"
        );
        result += c;
    }
    
    /* Final dummy use of result to prevent dead code elimination */
    volatile int final = result;
    
    /* The program doesn't need to run correctly, just compile */
    /* Return 0 to satisfy compiler */
    return 0;
}
