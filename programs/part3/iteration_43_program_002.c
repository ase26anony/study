/* gcc -O2 -m32 -fno-strict-aliasing -funroll-loops -fdump-rtl-all -o coverage_test coverage_test.c */

#include <stdint.h>
#include <string.h>

/* Function A: ZERO_EXTRACT and MEM patterns */
static void __attribute__((noinline)) 
pattern_zero_extract_mem(volatile int *counter) {
    /* Struct with volatile bit-fields for ZERO_EXTRACT */
    struct S { 
        volatile unsigned int f1:5;
        volatile unsigned int f2:7;
        volatile unsigned int f3:9;
    } s;
    
    /* Array with complex addressing for MEM */
    static volatile int arr[32][32];
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    s.f1 = (*counter) & 0x1F;
    s.f2 = (*counter >> 5) & 0x7F;
    s.f3 = (*counter >> 12) & 0x1FF;
    
    /* MEM pattern with complex addressing */
    int idx = *counter;
    volatile int v = arr[(idx * 3) % 32][(idx * 7) % 32];
    
    /* Combine: use bit-field value in memory access */
    arr[s.f1][s.f2] = s.f3 + v;
}

/* Function B: STRICT_LOW_PART and SUBREG patterns */
static void __attribute__((noinline))
pattern_strict_low_part_subreg(volatile int *counter) {
    volatile short hs = *counter & 0xFFFF;
    volatile char c = *counter & 0xFF;
    int i = *counter;
    
    /* STRICT_LOW_PART pattern via inline asm */
    /* Modify only low byte of a register */
    asm volatile (
        "addb $1, %0\n\t"
        "subb $2, %0"
        : "=q"(c)    /* =q constraint for byte-addressable register */
        : "0"(c)     /* matching input constraint */
        : "cc"
    );
    
    /* SUBREG pattern: type punning with different sizes */
    /* Cast pointer to larger type to pointer to smaller type */
    int *pi = &i;
    short *ps = (short *)pi;
    char *pc = (char *)pi;
    
    /* Mixed-size accesses generating SUBREG RTL */
    *ps = hs;          /* Write short to int (SUBREG) */
    pc[1] = c;         /* Write char to int (SUBREG) */
    
    /* More SUBREG: access parts of larger type */
    struct Packed {
        int a;
        short b;
        char c;
    } p;
    
    p.a = i;
    p.b = *ps;
    p.c = *pc;
    
    /* Use result to prevent elimination */
    *counter = p.a + p.b + p.c;
}

/* Function C: Complex mixed patterns */
static void __attribute__((noinline))
pattern_mixed_complex(volatile int *counter, volatile int *sum) {
    /* Array with volatile elements for MEM patterns */
    static volatile int matrix[16][16];
    
    /* Struct with bit-fields for ZERO_EXTRACT */
    struct BF {
        volatile unsigned int a:3;
        volatile unsigned int b:4;
        volatile unsigned int c:5;
    } bf;
    
    /* Initialize bit-fields */
    bf.a = (*counter) & 0x7;
    bf.b = (*counter >> 3) & 0xF;
    bf.c = (*counter >> 7) & 0x1F;
    
    /* Complex addressing with ternary operator */
    volatile int *ptr;
    int idx1 = *counter & 0xF;
    int idx2 = (*counter >> 4) & 0xF;
    
    /* Ternary selecting different addressing modes */
    ptr = (bf.a > 3) ? &matrix[idx1][idx2] : &matrix[idx2][idx1];
    
    /* MEM access with pointer arithmetic */
    volatile int val = *(ptr + bf.b);
    
    /* Type punning for SUBREG */
    int temp = val;
    short *sptr = (short *)&temp;
    
    /* Inline asm for STRICT_LOW_PART on the short */
    asm volatile (
        "incw %0"
        : "=q"(*sptr)
        : "0"(*sptr)
        : "cc"
    );
    
    /* ZERO_EXTRACT assignment */
    bf.c = temp & 0x1F;
    
    /* Update sum to prevent elimination */
    *sum += bf.a + bf.b + bf.c + val;
}

/* Helper with pointer arithmetic for MEM patterns */
static void __attribute__((noinline))
mem_pattern_helper(volatile int *base, int offset1, int offset2) {
    /* Complex pointer arithmetic */
    volatile int *p1 = base + offset1;
    volatile int *p2 = p1 + offset2;
    volatile int *p3 = p2 - offset1;
    
    /* Chain of MEM accesses */
    volatile int v1 = *p1;
    volatile int v2 = *p2;
    volatile int v3 = *p3;
    
    /* Use values to prevent elimination */
    *base = v1 + v2 + v3;
}

int main(int argc, char **argv) {
    volatile int counter = 0;
    volatile int sum = 0;
    
    /* Use argc to bound loops for analysis */
    int iterations = (argc > 1) ? 10 : 5;
    
    /* Initialize some data */
    volatile int data[64];
    for (int i = 0; i < 64; i++) {
        data[i] = i * 3;
    }
    
    /* Main loop triggering all patterns */
    for (int i = 0; i < iterations; i++) {
        /* Update volatile counter */
        counter += i;
        
        /* Call pattern functions */
        pattern_zero_extract_mem(&counter);
        pattern_strict_low_part_subreg(&counter);
        pattern_mixed_complex(&counter, &sum);
        
        /* Additional MEM patterns with helper */
        mem_pattern_helper(&data[i & 0x3F], i, i * 2);
        
        /* Mixed-size operations for SUBREG */
        {
            long long big = counter;
            int *ip = (int *)&big;
            short *sp = (short *)&big;
            
            /* Accesses generating SUBREG */
            ip[0] = counter;
            sp[2] = (counter >> 16) & 0xFFFF;
            
            /* Use the result */
            sum += ip[0] + sp[2];
        }
        
        /* More bit-field operations for ZERO_EXTRACT */
        {
            struct {
                volatile unsigned int x:10;
                volatile unsigned int y:12;
                volatile unsigned int z:10;
            } bf2;
            
            bf2.x = counter & 0x3FF;
            bf2.y = (counter >> 10) & 0xFFF;
            bf2.z = (counter >> 22) & 0x3FF;
            
            sum += bf2.x + bf2.y + bf2.z;
        }
    }
    
    /* Final dummy use to prevent elimination */
    asm volatile ("" : : "r"(sum), "r"(counter));
    
    return 0;
}
