/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields of varying widths to induce ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag : 1;
    unsigned int mode : 3;
    unsigned int data : 12;
    unsigned int count : 8;
    unsigned int pad : 8;
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    struct bitfield_pack bits;
    unsigned int word;
    unsigned short halves[2];
    unsigned char bytes[4];
};

/* Function to force complex addressing modes */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Function with inline assembly to simulate STRICT_LOW_PART operations */
static long long asm_lowpart_op(long long a, long long b) {
    long long result;
    /* Inline asm that operates on low 32 bits, potentially creating STRICT_LOW_PART */
    __asm__ volatile (
        "addl %k1, %k0\n\t"          /* Add low 32 bits */
        "adcl $0, %H0"               /* Propagate carry to high 32 bits */
        : "+r" (result)
        : "r" (b), "0" (a)
        : "cc"
    );
    return result;
}

int main(int argc, char *argv[]) {
    volatile struct bitfield_pack bf = {0};
    volatile union mixed_access u = {0};
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[256];
    volatile int i, j, temp;
    volatile int limit = (argc > 1) ? atoi(argv[1]) : 100;
    int result = 0;
    
    /* Initialize array with pattern */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Main loop with mixed operations */
    for (i = 0; i < limit; i++) {
        /* 1. Bit-field operations to trigger ZERO_EXTRACT/STRICT_LOW_PART */
        bf.flag = i & 1;
        bf.mode = (i >> 1) & 0x7;
        bf.data = (bf.data + i) & 0xFFF;
        bf.count = (bf.count + 1) & 0xFF;
        
        /* Access through union with type punning */
        u.bits = bf;
        temp = u.word;
        
        /* Manual bit extraction (may generate ZERO_EXTRACT) */
        unsigned int extracted = (temp >> 4) & 0xFFF;  /* Extract 12-bit field */
        
        /* 2. Multi-word operations to trigger SUBREG generation */
        /* Operations on long long (64-bit) on 32-bit targets */
        ll_var = ll_var + (long long)(extracted * 3);
        ll_var = ll_var | ((long long)i << 32);
        ll_var = ll_var ^ 0xAAAAAAAAAAAAAAAALL;
        
        /* Double operations (may use multiple registers) */
        dbl_var = dbl_var * 1.01 + (double)(i & 0xF);
        
        /* 3. Complex memory addressing with bit-field derived index */
        j = complex_index(i, bf.mode + 1, bf.data & 0x3F);
        if (j >= 0 && j < 256) {
            /* Read-modify-write with bit manipulation */
            array[j] = (array[j] + (temp & 0xFF)) | ((extracted << 8) & 0xFF00);
            
            /* Another potential ZERO_EXTRACT from memory */
            int masked = array[j] & 0xF0F;
            array[(j + 128) % 256] = masked | ((i << 4) & 0xF0);
        }
        
        /* 4. Control flow based on bit-field and multi-word results */
        if (bf.flag) {
            /* Check high vs low word of long long */
            unsigned int low = (unsigned int)(ll_var & 0xFFFFFFFF);
            unsigned int high = (unsigned int)(ll_var >> 32);
            
            if (high > low) {
                /* Use inline assembly for low-part operation */
                ll_var = asm_lowpart_op(ll_var, (long long)(high - low));
                
                /* More bit-field manipulation */
                bf.data = (bf.data ^ low) & 0xFFF;
            } else {
                /* Different memory access pattern */
                int idx = (low % 64) * 4;
                if (idx < 256) {
                    array[idx] = array[idx] - (high & 0xFF);
                }
            }
            
            /* Switch based on mode field */
            switch (bf.mode) {
                case 0:
                    dbl_var = dbl_var / 2.0;
                    break;
                case 1:
                    ll_var = ll_var << 1;
                    break;
                case 2:
                    bf.count = (bf.count * 3) & 0xFF;
                    break;
                default:
                    /* Complex array update */
                    for (temp = 0; temp < 4; temp++) {
                        int pos = (i * 4 + temp) % 256;
                        array[pos] = array[pos] ^ (bf.data << temp);
                    }
                    break;
            }
        }
        
        /* Prevent optimization of loop */
        __asm__ volatile ("" : : "r"(&bf), "r"(&ll_var), "r"(array) : "memory");
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (i = 0; i < 256; i++) {
        result += array[i];
    }
    result += bf.data;
    result += bf.count;
    result += (int)(ll_var & 0xFFFFFFFF);
    result += (int)(dbl_var * 1000);
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
