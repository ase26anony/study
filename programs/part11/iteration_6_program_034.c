/* Compile with: gcc -O3 -fschedule-insns -fno-strict-aliasing -fdump-rtl-all -c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 3;
    unsigned int e : 5;
} g_bfs = {0};

/* Helper function to force bit-field assignments */
__attribute__((noinline, noipa))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;          /* 4-bit field */
    s->b = (x >> 4) & 0xFF;  /* 8-bit field */
    s->c = y & 0xFFF;        /* 12-bit field */
    s->d = (y >> 12) & 0x7;  /* 3-bit field */
    s->e = (y >> 15) & 0x1F; /* 5-bit field */
    
    /* Additional complex pattern: nested bit-field in condition */
    if ((s->a ^ s->b) & 0x7) {
        s->c = s->d | s->e;
    }
}

/* Another helper to force SUBREG patterns with mixed-width operations */
__attribute__((noinline, noipa))
int mixed_width_ops(short *shorts, char *chars, int count) {
    int sum = 0;
    volatile int temp; /* volatile to prevent optimization */
    
    for (int i = 0; i < count; i++) {
        /* SUBREG patterns: mixing different widths */
        short s_val = (short)(chars[i] * 3);  /* char -> short */
        int i_val = shorts[i] * 2;            /* short -> int */
        
        /* Store with potential SUBREG */
        shorts[i] = s_val + (i_val & 0xFFFF); /* int -> short truncation */
        
        /* Load with sign extension (SUBREG patterns) */
        char c_temp = chars[i];
        int extended = (int)c_temp;           /* char -> int sign extend */
        
        /* Complex expression with mixed widths */
        temp = (extended << 8) | (s_val & 0xFF);
        sum += temp;
    }
    return sum;
}

/* Function to create complex addressing modes */
__attribute__((noinline, noipa))
int complex_addressing(int (*arr)[100], volatile int *idx1, volatile int *idx2) {
    int total = 0;
    
    /* Complex addressing with volatile indices */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Non-constant indices from volatile pointers */
            int x = *idx1 + i;
            int y = *idx2 + j;
            
            /* Bounds checking */
            if (x >= 100) x = 99;
            if (y >= 100) y = 99;
            
            /* Complex memory access pattern */
            int val = arr[x][y];
            
            /* Bit-field like operation on memory value */
            int masked = val & 0x3FF;  /* 10-bit mask - may generate ZERO_EXTRACT */
            
            /* Store back with modification */
            arr[x][y] = masked ^ (i * j);
            
            total += masked;
        }
    }
    return total;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* 1. Trigger bit-field patterns (ZERO_EXTRACT/STRICT_LOW_PART) */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, argc, argc * 2);
    
    /* 2. Mixed-width operations for SUBREG patterns */
    volatile short short_array[100];
    volatile char char_array[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        short_array[i] = (short)(i * 3);
        char_array[i] = (char)(i * 5);
    }
    
    result += mixed_width_ops((short*)short_array, (char*)char_array, 
                             argc > 100 ? 100 : argc + 10);
    
    /* 3. Complex addressing modes */
    int matrix[100][100];
    volatile int idx1 = 10, idx2 = 20;
    
    /* Initialize matrix */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    result += complex_addressing(matrix, &idx1, &idx2);
    
    /* 4. Additional volatile operations to force memory patterns */
    volatile struct {
        unsigned int f1 : 2;
        unsigned int f2 : 6;
        unsigned int f3 : 24;
    } local_bf = {0};
    
    /* Multiple volatile bit-field assignments in sequence */
    for (int i = 0; i < 5; i++) {
        local_bf.f1 = (argc + i) & 0x3;
        local_bf.f2 = (argc * i) & 0x3F;
        local_bf.f3 = (argc << i) & 0xFFFFFF;
        
        /* Read back and use */
        result += local_bf.f1 + local_bf.f2 + local_bf.f3;
    }
    
    /* 5. Inline assembly to increase register pressure and force resource tracking */
    asm volatile (
        "/* Clobber many registers to force reload */"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* 6. Pointer casting for SUBREG patterns */
    volatile int *int_ptr = (volatile int*)&short_array[0];
    volatile short *short_ptr = (volatile short*)int_ptr;
    
    /* Mixed-width access through pointers */
    *int_ptr = argc * 1000;
    *short_ptr = (short)(*int_ptr & 0xFFFF);  /* Truncation store */
    
    /* 7. Final computation to prevent dead code elimination */
    result += g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.d + g_bfs.e;
    result += short_array[0] + char_array[0];
    result += matrix[0][0] + matrix[99][99];
    
    printf("Result: %d\n", result);
    return result & 0xFF; /* Return non-zero to prevent optimization */
}
