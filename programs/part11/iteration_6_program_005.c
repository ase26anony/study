/* Compile with: gcc -O2 -fdump-rtl-reload -fschedule-insns -fno-strict-aliasing coverage.c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to force ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} g_bfs = {0};

/* 2D array for complex addressing modes */
static int arr[100][100];

/* Non-inline function to force memory addressing */
__attribute__((noinline)) 
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments - may generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFF;
    s->d = (x ^ y) & 0xFF;
}

/* Another noinline function for mixed-width operations */
__attribute__((noinline))
int mixed_width_ops(volatile short *shorts, volatile char *chars, int count) {
    int sum = 0;
    for (int i = 0; i < count; i++) {
        /* SUBREG patterns from mixed-width operations */
        int temp = shorts[i];          /* Load short, may use SUBREG */
        temp += (int)chars[i];         /* Char to int conversion */
        shorts[i] = (short)(temp & 0xFFFF); /* Store back to short - SUBREG dest */
        sum += temp;
    }
    return sum;
}

/* Function with complex addressing and register pressure */
__attribute__((noinline))
int complex_addressing(int idx1, int idx2, int mask) {
    volatile int vi1 = idx1;
    volatile int vi2 = idx2;
    
    /* Complex memory access with volatile indices */
    int val = arr[vi1 % 100][vi2 % 100];
    
    /* Bit-field like operation that may generate ZERO_EXTRACT */
    val = (val & mask) | ((~val) & ~mask);
    
    /* More mixed-width operations */
    short s_val = (short)(val & 0xFFFF);
    char c_val = (char)(val & 0xFF);
    
    return (int)s_val + (int)c_val;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* 1. Bit-field operations on global volatile struct */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, argc, argc * 2);
    
    /* 2. Mixed-width local arrays */
    volatile short short_arr[50];
    volatile char char_arr[50];
    
    /* Initialize arrays */
    for (int i = 0; i < 50; i++) {
        short_arr[i] = (short)(i * argc);
        char_arr[i] = (char)(i + argc);
    }
    
    /* Perform mixed-width operations */
    result += mixed_width_ops(short_arr, char_arr, argc % 50);
    
    /* 3. Complex addressing with 2D array */
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    result += complex_addressing(argc, argc * 3, 0x00FF00FF);
    
    /* 4. Additional mixed-type operations to increase SUBREG usage */
    {
        volatile int vi = argc;
        volatile short vs;
        volatile char vc;
        
        /* Multiple SUBREG patterns */
        vs = (short)vi;                    /* int to short - SUBREG */
        vc = (char)(vi >> 8);              /* int to char - SUBREG */
        vi = (int)vs + (int)vc;            /* short/char to int */
        
        /* Bit-field extraction that may use ZERO_EXTRACT */
        int extracted = (vi >> 4) & 0xF;   /* Like bit-field read */
        result += extracted;
    }
    
    /* 5. Inline assembly to increase register pressure and force reload */
    asm volatile (
        "/* Clobber multiple registers to force spilling */"
        :
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* 6. More complex pattern: struct with bit-fields in loop */
    {
        volatile struct NestedBF {
            struct {
                unsigned int x : 3;
                unsigned int y : 5;
            } inner;
            unsigned int z : 24;
        } nbf;
        
        for (int i = 0; i < 10; i++) {
            nbf.inner.x = (argc + i) & 0x7;
            nbf.inner.y = (argc * i) & 0x1F;
            nbf.z = (argc << 8) & 0xFFFFFF;
            result += nbf.inner.x + nbf.inner.y;
        }
    }
    
    /* 7. Pointer casting for SUBREG patterns */
    {
        volatile int *int_ptr = &result;
        volatile short *short_ptr = (volatile short *)int_ptr;
        volatile char *char_ptr = (volatile char *)int_ptr;
        
        *short_ptr = (short)(*int_ptr & 0xFFFF);      /* SUBREG store */
        *char_ptr = (char)(*int_ptr & 0xFF);          /* SUBREG store */
        
        /* Read back through different widths */
        result = (int)*short_ptr + (int)*char_ptr;
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", result + g_bfs.a + g_bfs.b);
    
    return 0;
}
