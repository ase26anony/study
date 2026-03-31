/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass (resource.cc lines 282-290). It creates
   operations that generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG,
   and complex MEM expressions in the RTL representation. */

#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization and create control flow */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* Global variables for memory operations */
unsigned int g_bitfield_source = 0xDEADBEEF;
int g_array[256];
long long g_large_var = 0x123456789ABCDEF0LL;

/* Structs for bit-field and union operations */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
} g_bitfield;

union subreg_union {
    int32_t full;
    int16_t halves[2];
    int8_t bytes[4];
} g_union;

/* 1. Generate ZERO_EXTRACT RTL patterns */
int zero_extract_pattern(volatile unsigned int *p) {
    /* Multiple bit-field extractions that may generate ZERO_EXTRACT */
    int result = 0;
    
    /* Extract bits 8-15 */
    result += (*p >> 8) & 0xFF;
    
    /* Extract bits 16-23 using different shift */
    result += (*p >> 16) & 0xFF;
    
    /* Extract bits 0-7 with mask */
    result += (*p & 0xFF);
    
    /* Complex extraction: bits 4-19 */
    result += ((*p >> 4) & 0xFFFF);
    
    return result;
}

/* 2. Generate STRICT_LOW_PART RTL patterns */
void strict_low_part_pattern(volatile unsigned int *p, unsigned char v) {
    /* Operations that write only low parts */
    
    /* Write to low byte only */
    *p = (*p & ~0xFF) | v;
    
    /* Write to low 16 bits */
    unsigned short s = v * 257;
    *p = (*p & ~0xFFFF) | s;
    
    /* Conditional low-part write */
    if (g_volatile_flag) {
        *p = (*p & ~0xFF) | (v + 1);
    }
}

/* 3. Generate SUBREG RTL patterns */
int subreg_pattern(void) {
    int result = 0;
    
    /* Access parts through union (likely generates SUBREG) */
    g_union.full = 0x12345678;
    result += g_union.halves[0];  /* Access low 16 bits */
    result += g_union.halves[1];  /* Access high 16 bits */
    
    /* Cast between pointer types */
    long long ll = g_large_var;
    result += *(int32_t*)&ll;     /* Access low 32 bits */
    result += *((int32_t*)&ll + 1); /* Access high 32 bits */
    
    /* Mixed-size operations */
    int32_t x = 0xABCD1234;
    int16_t y = *(int16_t*)&x;    /* Access low 16 bits */
    result += y;
    
    return result;
}

/* 4. Generate complex MEM operand patterns */
int complex_mem_pattern(int *base, int idx1, int idx2, int idx3) {
    /* Multiple complex addressing modes */
    int sum = 0;
    
    /* Array with scaled index */
    sum += base[idx1 * 4];
    
    /* Array with multiple indices */
    sum += base[idx1 + idx2 * 2];
    
    /* Nested addressing */
    sum += base[(idx1 + idx2) * 3 + idx3];
    
    /* Struct-like access pattern */
    struct block {
        int data[8];
        int extra;
    } *bp = (struct block*)base;
    sum += bp->data[idx1 & 7];
    
    return sum;
}

/* 5. Combined function with control flow */
int combined_patterns(void) {
    int result = 0;
    volatile unsigned int *volatile_ptr = &g_bitfield_source;
    
    /* Loop with volatile condition */
    for (int i = 0; i < 3 && g_volatile_flag; i++) {
        /* Mix different patterns */
        result += zero_extract_pattern(volatile_ptr);
        
        strict_low_part_pattern(volatile_ptr, i + 1);
        
        result += subreg_pattern();
        
        /* Complex memory access */
        result += complex_mem_pattern(g_array, i, i*2, i*3);
        
        g_volatile_counter++;
    }
    
    return result;
}

/* Helper functions to increase pass activity */
void helper1(void) {
    /* Emphasize bit-field operations */
    struct bitfield_struct local_bf;
    local_bf.low8 = g_volatile_counter & 0xFF;
    local_bf.mid16 = (g_volatile_counter >> 8) & 0xFFFF;
    g_bitfield = local_bf;
}

void helper2(void) {
    /* Emphasize subreg operations */
    union subreg_union local_union;
    local_union.full = g_volatile_counter;
    local_union.halves[1] = local_union.halves[0] + 1;
    g_union = local_union;
}

void helper3(void) {
    /* Emphasize memory addressing */
    for (int i = 0; i < 10; i++) {
        g_array[(i * 17) & 0xFF] = i + g_volatile_counter;
    }
}

/* Main function with observable side effects */
int main(void) {
    int final_result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3;
    }
    
    /* Call helper functions in non-trivial order */
    for (int iter = 0; iter < 5; iter++) {
        helper1();
        helper2();
        helper3();
        
        /* Main combined patterns */
        final_result += combined_patterns();
        
        /* Modify volatile to change control flow */
        g_volatile_flag = (iter % 2) ? 1 : 0;
        g_volatile_counter += iter;
    }
    
    /* Compute checksum to ensure all operations contribute */
    unsigned int checksum = final_result;
    checksum += g_bitfield_source;
    checksum += g_union.full;
    for (int i = 0; i < 16; i++) {
        checksum += g_array[i * 16];
    }
    
    /* Print result to prevent optimization */
    printf("Result: %u (0x%08x)\n", checksum, checksum);
    
    return (checksum & 0xFF);
}
