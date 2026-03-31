/* Compile with: gcc -O3 -fschedule-insns -fno-strict-aliasing -fdump-rtl-all -c */
/* Target: Trigger mark_referenced_resources for SET_DEST with ZERO_EXTRACT/STRICT_LOW_PART/SUBREG -> MEM patterns */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile struct with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;    /* Will generate ZERO_EXTRACT for 4-bit field */
    unsigned int b : 8;    /* 8-bit field */
    unsigned int c : 20;   /* 20-bit field */
    unsigned int pad : 32; /* Full 32-bit field for contrast */
} g_bfs = {0};

/* Force memory addressing with complex indices */
volatile int g_idx1 = 0;
volatile int g_idx2 = 0;

/* Non-inline function to prevent optimization of bit-field accesses */
__attribute__((noinline, optimize("O0")))
void modify_bitfields(struct BitFieldStruct *s, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* These assignments should generate STRICT_LOW_PART or ZERO_EXTRACT
           when storing to memory locations */
        s->a = (i & 0xF);           /* 4-bit field */
        s->b = (i * 3) & 0xFF;      /* 8-bit field */
        s->c = (i * 5) & 0xFFFFF;   /* 20-bit field */
        s->pad = i;                 /* Full 32-bit for comparison */
        
        /* Mix with memory barrier to force separate instructions */
        asm volatile("" ::: "memory");
    }
}

/* Another noinline function to create SUBREG patterns */
__attribute__((noinline))
void mixed_width_operations(short *shorts, int *ints, char *chars, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate SUBREG patterns by mixing widths */
        shorts[i] = ints[i] & 0xFFFF;          /* Truncation to 16-bit */
        chars[i] = (ints[i] >> 8) & 0xFF;      /* Truncation to 8-bit */
        
        /* Sign extension patterns that might create SUBREG */
        ints[i] = (int)shorts[i] * (int)chars[i];  /* Promote to int */
        
        /* Complex expression with partial registers */
        ints[i] = ((ints[i] & 0xFF00) >> 8) | ((ints[i] & 0xFF) << 8);
    }
}

/* Function with 2D array and complex addressing */
__attribute__((noinline))
int complex_addressing(int arr[100][100], volatile int idx1, volatile int idx2) {
    int sum = 0;
    
    /* Complex addressing with volatile indices */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* This should generate complex MEM addresses */
            sum += arr[idx1 + i][idx2 + j];
            
            /* Bit-field like operation on the loaded value */
            sum = (sum & 0x7F) + ((sum >> 7) & 0x7F);
        }
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* 1. Trigger ZERO_EXTRACT/STRICT_LOW_PART patterns */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, argc > 1 ? atoi(argv[1]) : 10);
    
    /* 2. Create SUBREG patterns with mixed-width operations */
    volatile short short_array[100];
    volatile int int_array[100];
    volatile char char_array[100];
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i * argc;  /* Make value argc-dependent */
    }
    
    mixed_width_operations((short*)short_array, (int*)int_array, 
                          (char*)char_array, 100);
    
    /* 3. Complex 2D array addressing */
    int arr_2d[100][100];
    
    /* Initialize 2D array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr_2d[i][j] = i * 100 + j;
        }
    }
    
    /* Use volatile indices to prevent constant propagation */
    g_idx1 = argc % 50;
    g_idx2 = (argc * 2) % 50;
    
    result += complex_addressing(arr_2d, g_idx1, g_idx2);
    
    /* 4. Additional mixed operations to increase register pressure */
    {
        volatile int temp;
        volatile short stemp;
        volatile char ctemp;
        
        /* Operations that should generate SUBREG in SET_DEST */
        for (int i = 0; i < 50; i++) {
            temp = int_array[i];
            stemp = (short)(temp >> 16);  /* High 16 bits to short */
            ctemp = (char)(temp & 0xFF);  /* Low 8 bits to char */
            
            /* Recombine with different widths */
            int_array[i] = (int)stemp << 8 | (int)ctemp;
        }
    }
    
    /* 5. Inline assembly to clobber registers and force reload */
    asm volatile(
        "/* Clobber multiple registers to increase pressure */\n\t"
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
    );
    
    /* 6. Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        result += int_array[i];
        result += short_array[i];
        result += char_array[i];
    }
    
    result += g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.pad;
    
    printf("Result: %d\n", result);
    
    return result != 0;
}
