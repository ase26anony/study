/* Test program to trigger uncovered lines in resource.cc */
#include <stdio.h>
#include <stdint.h>

/* Function to prevent optimization */
static int use_result(int sum) {
    volatile int sink = sum;
    return sink;
}

int main(void) {
    int checksum = 0;
    
    /* 1. Bitfield Store Test - targeting ZERO_EXTRACT/STRICT_LOW_PART */
    {
        /* Volatile bitfield struct to force ZERO_EXTRACT */
        volatile struct {
            unsigned int field1 : 4;
            unsigned int field2 : 8;
            unsigned int field3 : 12;
        } bitfields = {0};
        
        /* Initialize some variables */
        unsigned int a = 0xABCD;
        unsigned int b = 0x1234;
        unsigned int c = 0x5678;
        
        /* Complex bitfield assignments that may generate ZERO_EXTRACT */
        bitfields.field1 = (a & 0xF) + (b & 0xF);          /* 4-bit field */
        bitfields.field2 = ((a >> 4) & 0xFF) ^ (c & 0xFF); /* 8-bit field */
        bitfields.field3 = (b >> 4) | (c >> 8);            /* 12-bit field */
        
        /* Read back and accumulate to checksum */
        checksum += bitfields.field1;
        checksum += bitfields.field2;
        checksum += bitfields.field3;
    }
    
    /* 2. Sub-word Type Store Test - targeting SUBREG */
    {
        /* Volatile short to force store instruction */
        volatile short vs1, vs2;
        
        /* Register variables to encourage register operations */
        register int r1 = 0x12345678;
        register int r2 = 0x9ABCDEF0;
        
        /* Explicit narrowing casts that may generate SUBREG */
        vs1 = (short)(r1 + 0x100);          /* Cast from int to short */
        vs2 = (short)((r1 & 0xFFFF) + (r2 & 0xFFFF));
        
        /* Arithmetic with implicit narrowing */
        volatile char vc;
        char c1 = 100;
        char c2 = 50;
        vc = c1 + c2;                       /* May overflow and truncate */
        
        checksum += vs1 + vs2 + vc;
    }
    
    /* 3. Complex Memory Store Test - targeting MEM_P with complex addressing */
    {
        /* Local arrays with restrict to prevent aliasing assumptions */
        int arr1[100];
        int arr2[50][10];
        
        /* Initialize with some values */
        for (int i = 0; i < 100; i++) {
            arr1[i] = i * 3;
        }
        
        /* Complex addressing patterns */
        for (int i = 0; i < 10; i++) {
            /* Multi-dimensional array with non-linear index */
            int idx = i * 7 + 3;
            if (idx < 50) {
                /* Complex address: base + stride * i + offset */
                arr2[idx][i] = arr1[i * 5] + i;
                
                /* Pointer arithmetic with multiple offsets */
                int *ptr = arr1 + i * 3 + 2;
                *ptr = arr2[idx][i] * 2;
            }
        }
        
        /* Compute checksum from array elements */
        for (int i = 0; i < 10; i++) {
            checksum += arr1[i * 3];
            if (i < 5) {
                checksum += arr2[i * 2][i];
            }
        }
    }
    
    /* 4. Combined Test - mixing patterns */
    {
        /* Struct with bitfield and array */
        struct combined {
            volatile unsigned int flags : 16;
            short data[20];
        } comb;
        
        /* Initialize */
        for (int i = 0; i < 20; i++) {
            comb.data[i] = i * 2;
        }
        
        /* Combined operations */
        register int temp = 0x87654321;
        
        /* Bitfield assignment (ZERO_EXTRACT potential) */
        comb.flags = (temp & 0xFFFF) ^ 0x1234;
        
        /* Array assignment with narrowing cast (SUBREG potential) */
        int idx = 5 * 2 + 3;  /* Complex index */
        comb.data[idx] = (short)(temp >> 16);
        
        /* Complex memory access through pointer */
        short *ptr = &comb.data[0];
        ptr[idx + 1] = (short)(temp & 0xFFFF);
        
        checksum += comb.flags;
        checksum += comb.data[idx];
        checksum += comb.data[idx + 1];
    }
    
    /* 5. Inline Assembly for direct RTL influence */
    {
        int array[10] = {0};
        int index = 3;
        
        /* Inline asm with complex memory addressing */
        asm volatile (
            "# Force complex memory operand\n"
            : "=m" (array[index * 2 + 1])  /* Complex addressing */
            :
            : "memory"
        );
        
        /* Another asm with bitfield-like constraint */
        struct {
            volatile unsigned int low : 8;
            volatile unsigned int high : 8;
        } bits = {0};
        
        unsigned int value = 0xA5A5;
        
        /* This might generate ZERO_EXTRACT */
        asm volatile (
            ""
            : "=r" (value)
            : "0" (value)
        );
        
        bits.low = value & 0xFF;
        bits.high = (value >> 8) & 0xFF;
        
        checksum += array[7] + bits.low + bits.high;
    }
    
    /* 6. Additional patterns using builtins */
    {
        volatile unsigned char bytes[4] = {0x11, 0x22, 0x33, 0x44};
        unsigned int word;
        
        /* Load byte and extend - may involve SUBREG */
        word = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
        
        /* Bit manipulation builtins on sub-word data */
        unsigned char b = 0x5A;
        int parity = __builtin_parity(b);      /* May extract bits */
        int popcount = __builtin_popcount(b);  /* May extract bits */
        
        /* Store with masking - potential ZERO_EXTRACT */
        bytes[2] = (word >> 16) & 0x7F;        /* Store only 7 bits */
        
        checksum += word + parity + popcount + bytes[2];
    }
    
    /* Final result computation and output */
    checksum = use_result(checksum);
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
