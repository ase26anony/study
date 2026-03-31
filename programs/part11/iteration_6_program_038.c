/* Target: resource.cc lines 282-290 coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} g_bfs = {0};

/* Force memory addressing modes with non-inline function */
__attribute__((noinline)) 
void modify_bitfields(volatile struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate SET with ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;          /* Should generate ZERO_EXTRACT */
    s->b = (x >> 4) & 0xFF;  /* Another bit-field store */
    s->c = y & 0xFFF;        /* More bit-field operations */
    s->d = (y >> 12) & 0xFF; /* Mixed-width bit-field */
}

/* Another noinline function to force SUBREG patterns */
__attribute__((noinline))
int mixed_width_ops(short *shorts, char *chars, int count) {
    int sum = 0;
    volatile int temp; /* Volatile to prevent optimization */
    
    for (int i = 0; i < count; i++) {
        /* Operations that should generate SUBREG RTL */
        temp = shorts[i];          /* short to int - potential SUBREG */
        temp += (int)chars[i];     /* char to int - potential SUBREG */
        shorts[i] = temp & 0xFFFF; /* int to short - potential SUBREG as SET_DEST */
        sum += temp;
    }
    return sum;
}

/* Complex addressing with 2D array */
__attribute__((noinline))
int complex_addressing(int arr[][100], volatile int *idx1, volatile int *idx2) {
    /* Volatile indices prevent constant propagation */
    int i = *idx1 % 100;
    int j = *idx2 % 100;
    
    /* Complex memory access that may generate MEM with complex address */
    int val = arr[i][j];
    
    /* Bitwise operation that could be represented as ZERO_EXTRACT */
    val = (val & 0x00FF00FF) | ((val & 0xFF00FF00) >> 8);
    
    /* Store back - might generate SET with complex MEM destination */
    arr[i][j] = val;
    
    return val;
}

int main(int argc, char *argv[]) {
    /* Prevent optimization of main */
    __attribute__((optimize("O0"))) 
    int result = 0;
    
    /* 1. Bit-field operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
    modify_bitfields(&g_bfs, argc, argc * 2);
    
    /* 2. Mixed-width operations to generate SUBREG patterns */
    volatile short short_array[100];
    volatile char char_array[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        short_array[i] = (short)(i * 3);
        char_array[i] = (char)(i * 5);
    }
    
    /* Force SUBREG generation with mixed-width operations */
    result += mixed_width_ops((short *)short_array, (char *)char_array, 
                             argc > 1 ? atoi(argv[1]) % 100 : 50);
    
    /* 3. Complex addressing with 2D array */
    int arr[100][100];
    volatile int idx1 = argc;
    volatile int idx2 = argc * 2;
    
    /* Initialize 2D array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    result += complex_addressing(arr, &idx1, &idx2);
    
    /* 4. More mixed-type operations to increase SUBREG generation */
    volatile int int_var = argc;
    volatile short short_var;
    volatile char char_var;
    
    /* Multiple assignments that should generate SUBREG as SET_DEST */
    short_var = int_var & 0xFFFF;          /* int to short */
    char_var = int_var & 0xFF;             /* int to char */
    int_var = short_var + char_var;        /* short/char to int */
    short_var = int_var >> 8;              /* Another int to short */
    
    /* 5. Inline assembly to increase register pressure and force reload */
    asm volatile ("" 
                  : 
                  : "r"(int_var), "r"(short_var), "r"(char_var)
                  : "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    
    /* 6. Additional bit-field operations in a loop */
    for (int i = 0; i < (argc % 10); i++) {
        g_bfs.a = (g_bfs.a + 1) & 0xF;
        g_bfs.b = (g_bfs.b * 3) & 0xFF;
        g_bfs.c = (g_bfs.c ^ i) & 0xFFF;
        
        /* Mixed operation to potentially generate SUBREG */
        short_var = g_bfs.c & 0xFFFF;
    }
    
    /* 7. Pointer casting for partial memory access (potential SUBREG) */
    volatile int *int_ptr = &int_var;
    volatile short *short_ptr = (volatile short *)int_ptr;
    volatile char *char_ptr = (volatile char *)int_ptr;
    
    *short_ptr = (*short_ptr + 1) & 0x7FFF;  /* May generate SUBREG SET_DEST */
    *char_ptr = (*char_ptr - 1) & 0x7F;      /* Another partial store */
    
    /* 8. Compute checksum to prevent dead code elimination */
    result += g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.d;
    result += int_var + short_var + char_var;
    result += *short_ptr + *char_ptr;
    
    /* Final inline assembly with clobber to ensure resource tracking */
    asm volatile (""
                  :
                  : "r"(result)
                  : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
                    "r8", "r9", "r10", "r11", "r12", "memory");
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
