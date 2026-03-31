/* Target: resource.cc lines 282-290 */
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
__attribute__((noinline, optimize("O0")))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFF;
    s->d = (y >> 12) & 0xFF;
    
    /* Force memory barrier */
    asm volatile("" ::: "memory");
}

/* Another noinline function to force SUBREG patterns */
__attribute__((noinline, optimize("O0")))
int mixed_width_ops(short *shorts, char *chars, int count) {
    int sum = 0;
    volatile int temp; /* Prevent optimization */
    
    for (int i = 0; i < count; i++) {
        /* Generate SUBREG patterns through mixed-width operations */
        temp = shorts[i];          /* short -> int: potential SUBREG */
        temp = temp * chars[i];    /* char promotion: more SUBREG */
        
        /* Cast back to short causing SUBREG as SET_DEST */
        shorts[i] = (short)(temp & 0xFFFF);
        
        /* Complex expression with memory */
        sum += (int)shorts[i] + (int)chars[i];
    }
    return sum;
}

/* Function with complex addressing modes */
__attribute__((noinline, optimize("O0")))
int complex_addressing(int arr[][100], volatile int *idx1, volatile int *idx2) {
    /* Volatile indices prevent constant propagation */
    int i = *idx1 % 100;
    int j = *idx2 % 100;
    
    /* Complex memory access with bitwise operation */
    int val = arr[i][j];
    
    /* Bit-field like operation on memory value */
    val = (val & 0xFF00FF00) | ((val & 0x00FF00FF) << 8);
    
    /* Store back - may generate ZERO_EXTRACT for partial write */
    arr[i][j] = val;
    
    return val;
}

/* Main function creating all required patterns */
int main(int argc, char *argv[]) {
    /* 1. Bit-field operations on volatile struct */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, argc, argc * 2);
    
    /* 2. Mixed-width operations with SUBREG patterns */
    volatile short short_arr[100];
    volatile char char_arr[100];
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 100; i++) {
        short_arr[i] = (short)(i * argc);
        char_arr[i] = (char)(i + argc);
    }
    
    int sum1 = mixed_width_ops((short*)short_arr, (char*)char_arr, 
                               argc % 50 + 10);
    
    /* 3. Complex addressing with 2D array */
    int arr[100][100];
    volatile int idx1 = argc;
    volatile int idx2 = argc * 3;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * 100 + j + argc;
        }
    }
    
    int val = complex_addressing(arr, &idx1, &idx2);
    
    /* 4. More register pressure with inline assembly */
    asm volatile(
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "add r2, r0, r1\n\t"
        : 
        : "r" (sum1), "r" (val)
        : "r0", "r1", "r2", "memory"
    );
    
    /* 5. Additional mixed-type operations to increase SUBREG usage */
    volatile int int_var = argc;
    volatile short short_var;
    volatile char char_var;
    
    /* Generate SUBREG patterns through assignments */
    short_var = (short)int_var;          /* int -> short: SUBREG as SET_DEST */
    char_var = (char)(int_var >> 8);     /* Another SUBREG pattern */
    
    /* Bit-field extraction that may generate ZERO_EXTRACT */
    int extracted = (int_var >> 4) & 0xF;  /* Like bit-field read */
    
    /* 6. Complex expression combining everything */
    int result = g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.d
                 + sum1 + val + short_var + char_var + extracted;
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* More register pressure */
    volatile int pressure[20];
    for (int i = 0; i < 20; i++) {
        pressure[i] = i * result;
        asm volatile("" : "+r" (pressure[i]) : : "memory");
    }
    
    return result != 0 ? 0 : 1;
}

/* Additional function to create more patterns during compilation */
__attribute__((noinline, optimize("O0")))
void extra_patterns(void) {
    /* Structure with packed bit-fields */
    struct {
        volatile unsigned int f1 : 3;
        volatile unsigned int f2 : 5;
        volatile unsigned int f3 : 10;
        volatile unsigned int f4 : 14;
    } local_bf;
    
    /* Multiple assignments to different bit-fields */
    local_bf.f1 = 1;
    local_bf.f2 = 2;
    local_bf.f3 = 3;
    local_bf.f4 = 4;
    
    /* Mixed-width memory access */
    volatile int* ptr = (volatile int*)&local_bf;
    volatile short* sptr = (volatile short*)ptr;
    volatile char* cptr = (volatile char*)ptr;
    
    /* Generate various SUBREG patterns */
    *sptr = (short)(*ptr >> 16);
    *cptr = (char)(*ptr >> 24);
    
    /* Force memory barrier */
    asm volatile("" ::: "memory");
}
