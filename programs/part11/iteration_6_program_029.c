/* Compile with: gcc -O3 -fschedule-insns -fno-strict-aliasing -fdump-rtl-all -c */
/* Target: Trigger mark_referenced_resources for SET_DEST with ZERO_EXTRACT/STRICT_LOW_PART/SUBREG -> MEM patterns */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;    /* Will generate ZERO_EXTRACT for 4-bit field */
    unsigned int b : 8;    /* 8-bit field */
    unsigned int c : 20;   /* 20-bit field */
    unsigned int pad : 32; /* Full 32-bit field for contrast */
} g_bfs = {0};

/* 2D array for complex addressing modes */
static int g_arr[100][100];

/* Non-inline function to force memory addressing */
__attribute__((noinline)) 
void modify_bitfields(volatile struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to same struct */
    s->a = x & 0xF;        /* ZERO_EXTRACT pattern */
    s->b = y & 0xFF;       /* Another ZERO_EXTRACT */
    s->c = (x + y) & 0xFFFFF;
    
    /* Force STRICT_LOW_PART through partial assignment */
    /* This may generate STRICT_LOW_PART for partial word store */
    *(volatile unsigned short*)((char*)s + sizeof(struct BitFieldStruct) - 2) = 
        (unsigned short)(x * y);
}

/* Another noinline function for mixed-width operations */
__attribute__((noinline))
int mixed_width_ops(int argc, char **argv) {
    volatile short vs;      /* For SUBREG operations */
    volatile char vc;
    int sum = 0;
    
    /* Mixed-width operations causing SUBREG patterns */
    for (int i = 0; i < argc; i++) {
        /* int -> short conversion generates SUBREG */
        vs = (short)(argv[i][0] * i);
        
        /* short -> int with sign extension */
        sum += (int)vs;
        
        /* char -> int */
        vc = (char)(sum & 0xFF);
        sum += (int)vc * 2;
        
        /* Pointer casting for partial access */
        unsigned int temp = *(unsigned int*)&vs;
        sum += (temp & 0xFFFF);  /* Access lower 16 bits */
    }
    
    return sum;
}

/* Function with complex array addressing */
__attribute__((noinline))
int complex_addressing(volatile int idx_i, volatile int idx_j) {
    int result = 0;
    
    /* Complex addressing with volatile indices */
    result = g_arr[idx_i % 100][idx_j % 100];
    
    /* Additional bit-field like operation on the result */
    result = (result & 0x00FF00FF) | ((result & 0xFF00FF00) >> 8);
    
    /* Mixed-width store back */
    g_arr[idx_i % 100][idx_j % 100] = (short)result;  /* SUBREG pattern */
    
    return result;
}

/* Function to create register pressure */
__attribute__((noinline))
void create_register_pressure(void) {
    /* Many local variables to force register allocation */
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    short s0, s1, s2, s3;
    char c0, c1, c2;
    
    /* Initialize with computations */
    r0 = 1; r1 = 2; r2 = 3; r3 = 4; r4 = 5;
    r5 = r0 + r1; r6 = r2 * r3; r7 = r4 - r1;
    r8 = r5 ^ r6; r9 = r7 | r8;
    
    /* Mixed-width assignments */
    s0 = (short)r0; s1 = (short)r1; s2 = (short)r2; s3 = (short)r3;
    c0 = (char)r4; c1 = (char)r5; c2 = (char)r6;
    
    /* Inline assembly to clobber registers and force reload */
    asm volatile("" 
                 : "=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3), "=r"(r4)
                 : "0"(r0), "1"(r1), "2"(r2), "3"(r3), "4"(r4)
                 : /* Clobber more registers to increase pressure */
                   "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12",
                   "memory");
    
    /* Use the variables to prevent optimization */
    g_arr[0][0] = r0 + s0 + c0;
}

int main(int argc, char **argv) {
    int checksum = 0;
    volatile int idx_i, idx_j;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            g_arr[i][j] = i * 100 + j;
        }
    }
    
    /* 1. Bit-field operations on volatile struct */
    modify_bitfields(&g_bfs, argc, argc * 2);
    
    /* Add bit-field values to checksum */
    checksum += g_bfs.a;
    checksum += g_bfs.b;
    checksum += g_bfs.c;
    
    /* 2. Mixed-width operations */
    checksum += mixed_width_ops(argc > 10 ? 10 : argc, argv);
    
    /* 3. Complex array addressing with volatile indices */
    idx_i = argc;
    idx_j = argc * 3;
    checksum += complex_addressing(idx_i, idx_j);
    
    /* 4. Create register pressure */
    create_register_pressure();
    
    /* 5. Additional mixed-type operations in main */
    volatile short local_short;
    volatile char local_char;
    volatile int local_int;
    
    for (int i = 0; i < (argc % 5 + 1); i++) {
        /* int -> short -> char chain generates SUBREGs */
        local_int = checksum + i;
        local_short = (short)local_int;          /* SUBREG pattern */
        local_char = (char)local_short;          /* Another SUBREG */
        local_int = local_char * 3;              /* Sign extension */
        
        /* Bit-field style operation using masking */
        local_int = (local_int & 0xF) | ((local_int & 0xF0) << 4);
        
        checksum += local_int;
    }
    
    /* 6. Memory access with bit-field extraction */
    volatile unsigned int *mem_ptr = (volatile unsigned int*)&g_bfs;
    unsigned int mem_val = *mem_ptr;
    
    /* Simulate ZERO_EXTRACT through masking */
    unsigned int extracted = (mem_val >> 4) & 0xF;  /* Like 4-bit ZERO_EXTRACT */
    checksum += extracted;
    
    /* Final inline assembly for additional register pressure */
    asm volatile("" ::: 
                 "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                 "r8", "r9", "r10", "r11", "r12",
                 "memory");
    
    printf("Checksum: %d\n", checksum);
    return checksum == 0 ? 0 : 1;
}
