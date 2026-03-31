/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdint.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    volatile unsigned int a : 3;    /* 3-bit field */
    volatile unsigned int b : 12;   /* 12-bit field */
    volatile unsigned int c : 1;    /* 1-bit field */
    volatile unsigned int d : 8;    /* 8-bit field */
    volatile unsigned int e : 8;    /* 8-bit field */
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    volatile struct bitfield_pack bits;
    volatile uint32_t word;
    volatile uint16_t halfwords[2];
    volatile uint8_t bytes[4];
};

/* Function to force complex addressing modes */
static inline int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Function with inline assembly to mimic STRICT_LOW_PART behavior */
static uint32_t asm_low_part(uint64_t val) {
    uint32_t result;
    /* Assembly that operates on low 32 bits, implicitly constraining high bits */
    __asm__ volatile (
        "movl %1, %0\n\t"
        "addl $0x1234, %0"
        : "=r" (result)
        : "r" ((uint32_t)val)
        : "cc"
    );
    return result;
}

int main(int argc, char *argv[]) {
    volatile union mixed_access data;
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[256];
    volatile int i, limit;
    int result = 0;
    
    /* Initialize array with pattern */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Use argc to control loop iterations (prevents optimization) */
    limit = (argc > 1) ? 100 : 50;
    
    /* Main loop with mixed operations */
    for (i = 0; i < limit; i++) {
        /* 1. Bit-field operations (ZERO_EXTRACT/STRICT_LOW_PART) */
        data.bits.a = (i & 0x7);                    /* 3-bit field */
        data.bits.b = (i * 17) & 0xFFF;             /* 12-bit field */
        data.bits.c = (i & 0x1);                    /* 1-bit field */
        
        /* Extract and combine bit-fields using masks and shifts */
        uint32_t temp = (data.bits.b << 3) | data.bits.a;
        data.bits.d = (temp >> 4) & 0xFF;           /* 8-bit field */
        data.bits.e = (temp >> 12) & 0xFF;          /* 8-bit field */
        
        /* 2. Multi-word operations (SUBREG generation) */
        /* Operations on long long (64-bit) on 32-bit targets */
        ll_var = ll_var + (data.word * 0x10001LL);
        ll_var = ll_var ^ (0xF0F0F0F0F0F0F0F0LL >> data.bits.a);
        
        /* Double operations that may use multiple registers */
        dbl_var = dbl_var * 1.01 + (double)(i & 0xF);
        
        /* 3. Complex memory addressing with bit-field derived index */
        int idx = complex_index(i, 7, data.bits.d);
        if (idx >= 0 && idx < 256) {
            /* Read-modify-write with bit manipulation */
            array[idx] = (array[idx] & ~0xFF) | data.bits.e;
            array[idx] ^= (1 << data.bits.a);
        }
        
        /* 4. Control flow based on bit-field and multi-word results */
        if (data.bits.c) {  /* Branch based on 1-bit field */
            /* Use inline assembly for low-part operation */
            uint32_t low_result = asm_low_part(ll_var);
            data.halfwords[0] = low_result & 0xFFFF;
            
            /* Additional array access with different stride */
            int idx2 = complex_index(i, 11, data.bits.b & 0x3F);
            if (idx2 >= 0 && idx2 < 256) {
                array[idx2] += low_result;
            }
        } else {
            /* Compare high vs low parts of long long */
            uint32_t low_part = (uint32_t)ll_var;
            uint32_t high_part = (uint32_t)(ll_var >> 32);
            
            if (low_part > high_part) {
                /* Swap bytes in the word */
                data.word = ((data.word & 0xFF) << 24) |
                           ((data.word & 0xFF00) << 8) |
                           ((data.word >> 8) & 0xFF00) |
                           ((data.word >> 24) & 0xFF);
            }
            
            /* Access array with pointer arithmetic */
            volatile int *ptr = &array[complex_index(i, 5, 2)];
            *ptr = (*ptr * 3) / 2;
        }
        
        /* 5. Additional SUBREG patterns through type punning */
        /* Cast between types of different sizes */
        if (i & 0x4) {
            float float_temp = (float)dbl_var;
            uint32_t int_temp;
            *(volatile float *)&int_temp = float_temp;  /* Type punning */
            data.word ^= int_temp;
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (i = 0; i < 256; i++) {
        result += array[i];
    }
    result += data.word;
    result += (int)(ll_var & 0xFFFFFFFF);
    result += (int)(ll_var >> 32);
    result += (int)(dbl_var * 1000);
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}

/* Additional function to force RTL generation during compilation */
void __attribute__((noinline)) extra_patterns(void) {
    volatile struct {
        unsigned int x : 5;
        unsigned int y : 10;
        unsigned int z : 17;
    } s;
    
    volatile uint64_t big_val = 0;
    
    /* Pattern that might generate ZERO_EXTRACT for sign extension */
    for (int j = 0; j < 10; j++) {
        s.x = j;
        s.y = (j * 37) & 0x3FF;
        s.z = (s.y << 7) | s.x;
        
        /* Operation that might use STRICT_LOW_PART */
        big_val = (big_val << 12) | (s.z & 0xFFF);
        
        /* Memory access with displacement */
        volatile char buffer[64];
        int offset = (s.x * 3 + s.y) & 0x3F;
        buffer[offset] = (big_val >> (j * 4)) & 0xFF;
    }
}
