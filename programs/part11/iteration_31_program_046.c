/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int value : 12;
    unsigned int pad   : 16;
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    struct bitfield_pack bits;
    unsigned int raw;
    unsigned short halves[2];
};

/* Function with inline assembly to force specific register usage */
static inline unsigned long long manipulate_bits(unsigned long long x) {
    unsigned long long result;
    /* Inline asm that operates on low 32 bits, potentially creating STRICT_LOW_PART */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "andl $0xFFFFF, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" ((unsigned int)x)
        : "%eax"
    );
    return result;
}

/* Function to create complex memory addressing */
static int complex_indexing(volatile int *arr, int idx, int stride) {
    /* Complex addressing that might generate MEM with complex XEXP */
    return arr[idx * stride + 3] + arr[idx * stride - 2];
}

int main(int argc, char **argv) {
    volatile int i, j, limit;
    volatile unsigned long long ll_var = 0x123456789ABCDEF0ULL;
    volatile double dbl_var = 3.141592653589793;
    volatile int sum = 0;
    
    /* Use argc to make execution path data-dependent */
    limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (limit <= 0) limit = 50;
    
    /* Declare and initialize bit-field structure */
    volatile union mixed_access data;
    data.raw = 0;
    data.bits.flag1 = 1;
    data.bits.flag2 = 5;
    data.bits.value = 2047;
    
    /* Declare volatile array with non-trivial access pattern */
    volatile int array[256];
    for (i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Main loop with mixed operations */
    for (i = 0; i < limit; i++) {
        /* 1. Bit-field manipulations - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        unsigned int temp;
        
        /* Extract bit-field using mask and shift */
        temp = (data.raw >> 4) & 0xFFF;  /* Extract 12-bit field */
        
        /* Modify specific bits */
        data.bits.flag2 = (data.bits.flag2 + i) & 0x7;  /* 3-bit field */
        data.bits.value = temp ^ 0xAAA;  /* 12-bit field */
        
        /* 2. Multi-word operations - may generate SUBREG */
        ll_var = ll_var + ((unsigned long long)data.raw << 16);
        dbl_var = dbl_var * 1.01 + (double)i;
        
        /* Force 64-bit operation on 32-bit target by using union */
        union {
            unsigned long long ll;
            unsigned int parts[2];
        } split;
        split.ll = ll_var;
        
        /* Compare high and low parts - may generate SUBREG comparisons */
        if (split.parts[0] != split.parts[1]) {
            /* Swap halves using bit operations */
            ll_var = ((unsigned long long)split.parts[1] << 32) | split.parts[0];
        }
        
        /* 3. Complex memory addressing with bit-field derived index */
        j = (data.bits.value * i) % 256;
        
        /* Array access with complex addressing */
        array[j] = array[(j * 3 + 7) % 256] + complex_indexing((int*)array, j, 5);
        
        /* 4. Conditional based on bit-field parity */
        if (data.bits.flag1 ^ (i & 1)) {
            /* Use inline assembly for bit manipulation */
            ll_var = manipulate_bits(ll_var);
            
            /* Additional bit-field operation */
            data.bits.flag1 = !data.bits.flag1;
            
            /* Access misaligned data through pointer casting */
            unsigned char *byte_ptr = (unsigned char*)&ll_var;
            int misaligned_int = *((volatile unsigned int*)(byte_ptr + 1));
            sum += misaligned_int & 0xFF;
        }
        
        /* 5. Switch based on 3-bit field value */
        switch (data.bits.flag2) {
            case 0:
                array[j] += 1;
                break;
            case 1:
            case 2:
                ll_var >>= 1;
                break;
            case 3:
            case 4:
                dbl_var += 0.5;
                break;
            default:
                /* Force SUBREG by operating on halves of long long */
                split.parts[0] ^= split.parts[1];
                split.parts[1] += i;
                ll_var = split.ll;
                break;
        }
        
        /* Accumulate results to prevent optimization */
        sum += array[j] + (int)dbl_var + (int)(ll_var & 0xFFFFFFFF);
    }
    
    /* Final aggregation and output */
    sum += data.raw + (int)(ll_var >> 32);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", sum);
    
    return sum & 0xFF;
}

/* Additional function to create more RTL patterns */
void extra_patterns(void) {
    volatile struct {
        unsigned int a : 5;
        unsigned int b : 7;
        unsigned int c : 20;
    } s1, s2;
    
    volatile long long ll1, ll2;
    
    /* Multiple bit-field assignments in sequence */
    s1.a = 0x1F;
    s1.b = 0x7F;
    s1.c = 0xFFFFF;
    
    /* Copy with modification - may generate interesting RTL */
    s2 = s1;
    s2.b = (s2.b + 1) & 0x7F;
    
    /* Operations that might use ZERO_EXTRACT for bit-field comparison */
    if ((s1.a & 0x0F) == (s2.b & 0x0F)) {
        ll1 = 0x12345678;
    }
    
    /* 64-bit operations on 32-bit boundary */
    ll2 = ll1 * 3;
    ll1 = ll2 + (ll1 << 16);
    
    /* Access as bytes to force MEM with offset */
    volatile unsigned char *p = (volatile unsigned char*)&ll1;
    for (int i = 0; i < 8; i++) {
        p[i] ^= 0x55;
    }
}
