/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields of various widths to induce ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int flag : 1;      /* 1-bit field */
    unsigned int mode : 3;      /* 3-bit field */
    unsigned int value : 12;    /* 12-bit field */
    unsigned int pad : 16;      /* padding to 32 bits */
} __attribute__((packed));

/* Union for accessing the same memory as bit-fields or raw integer */
union bitfield_union {
    struct bitfield_struct bits;
    volatile unsigned int raw;
};

/* Function to force complex addressing modes */
static int complex_index(int *arr, int i, int stride, int offset) {
    return arr[i * stride + offset];
}

/* Function with inline assembly to mimic STRICT_LOW_PART operations */
static long long asm_low_part(long long x) {
    long long result;
    /* Operate on low 32 bits, high bits implicitly preserved/constrained */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl $0x1234, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (result)
        : "m" (x)
        : "%eax", "memory"
    );
    return result;
}

int main(int argc, char *argv[]) {
    volatile union bitfield_union bf;
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[256];
    volatile int limit = (argc > 1) ? atoi(argv[1]) : 100;
    int i, temp;
    unsigned int sum = 0;
    
    /* Initialize array with pattern */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Initialize bit-field structure */
    bf.bits.flag = 1;
    bf.bits.mode = 5;
    bf.bits.value = 2047;
    
    /* Main loop with mixed operations to generate complex RTL */
    for (i = 0; i < limit; i++) {
        /* 1. Bit-field operations that may generate ZERO_EXTRACT/STRICT_LOW_PART */
        unsigned int extracted;
        
        /* Extract specific bit ranges using masks and shifts */
        extracted = (bf.raw >> 4) & 0xFFF;  /* Extract 12-bit field with shift */
        
        /* Modify bit-fields individually */
        bf.bits.flag ^= 1;  /* Toggle 1-bit field */
        bf.bits.mode = (bf.bits.mode + 1) & 0x7;  /* 3-bit field with wrap-around */
        
        /* Combined bit-field assignment - may generate STRICT_LOW_PART */
        bf.bits.value = (extracted + i) & 0xFFF;
        
        /* 2. Multi-word operations to generate SUBREG expressions */
        /* Operations on long long (64-bit) on 32-bit targets */
        ll_var = ll_var + 0x100000001LL;
        ll_var = ll_var ^ 0x0F0F0F0F0F0F0F0FLL;
        
        /* Access high and low parts separately */
        unsigned int low_part = (unsigned int)(ll_var & 0xFFFFFFFF);
        unsigned int high_part = (unsigned int)(ll_var >> 32);
        
        /* Double operations that may be split */
        dbl_var = dbl_var * 1.01;
        if (dbl_var > 100.0) {
            dbl_var = dbl_var / 2.0;
        }
        
        /* 3. Complex memory addressing with bit-field derived index */
        int idx = (bf.bits.value * 3 + bf.bits.mode * 7) & 0xFF;
        
        /* Read-modify-write with complex addressing */
        array[idx] = array[idx] + (bf.bits.flag ? 1 : -1);
        
        /* Access with stride and offset */
        temp = complex_index((int*)array, bf.bits.mode, 16, bf.bits.flag);
        
        /* 4. Control flow based on bit-field and multi-word comparisons */
        if (bf.bits.flag) {
            /* Branch 1: Focus on low part operations */
            ll_var = asm_low_part(ll_var);
            
            /* Additional bit-field manipulation */
            bf.bits.value = (bf.bits.value * 3) & 0xFFF;
        } else {
            /* Branch 2: Focus on high part and memory operations */
            if (high_part > low_part) {
                /* Swap high and low parts using SUBREG-like operations */
                ll_var = ((long long)low_part << 32) | high_part;
            }
            
            /* Complex array update */
            int idx2 = (i * 13 + bf.bits.value) & 0xFF;
            array[idx2] = array[idx2] ^ temp;
        }
        
        /* Conditional based on parity of bit-field */
        switch (bf.bits.mode & 0x3) {
            case 0:
                ll_var = ll_var << 1;
                break;
            case 1:
                ll_var = ll_var >> 1;
                break;
            case 2:
                ll_var = ~ll_var;
                break;
            case 3:
                ll_var = ll_var + ((long long)bf.bits.value << 20);
                break;
        }
        
        /* Accumulate results to prevent optimization */
        sum += bf.bits.value + (low_part & 0xFFF) + (array[i & 0xFF] & 0xFF);
    }
    
    /* Final aggregation and output */
    unsigned int final_result = sum + (unsigned int)(ll_var & 0xFFFFFFFF) + 
                               (unsigned int)(bf.raw & 0xFFFF);
    
    printf("Result: %u\n", final_result);
    
    /* Also return from main to ensure all code paths are considered */
    return (final_result > 1000000) ? 0 : 1;
}

/* Additional function to create more RTL patterns during compilation */
static void extra_patterns(void) {
    volatile struct {
        unsigned short a : 4;
        unsigned short b : 4;
        unsigned short c : 4;
        unsigned short d : 4;
    } packed;
    
    volatile long long ll1, ll2, ll3;
    volatile int *ptr;
    volatile int buffer[64];
    
    /* More bit-field manipulations */
    packed.a = 3;
    packed.b = packed.a + 1;
    packed.c = (packed.b << 1) | packed.a;
    packed.d = ~packed.c & 0xF;
    
    /* Multi-register operations with casting */
    ll1 = 0x1111222233334444LL;
    ll2 = 0x5555666677778888LL;
    ll3 = ll1 + ll2;
    ll3 = ll3 * 2;
    
    /* Pointer arithmetic with type punning */
    ptr = buffer;
    *(volatile unsigned short*)((char*)ptr + 1) = 0xABCD;
    
    /* Misaligned access that might generate complex MEM expressions */
    int misaligned = *(volatile int*)((char*)buffer + 2);
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(packed), "r"(ll3), "r"(misaligned));
}
