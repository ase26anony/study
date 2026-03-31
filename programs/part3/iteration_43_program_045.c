/* resource_patterns.c
 * 
 * This program is designed to generate specific RTL patterns that trigger
 * the uncovered lines in resource.cc (lines 282-290) during GCC compilation.
 * The patterns target ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM RTL expressions.
 * 
 * Compilation recommendations:
 *   gcc -O2 -m32 -fno-strict-aliasing -fdump-rtl-all resource_patterns.c
 *   gcc -O3 -funroll-loops -march=i686 -fno-strict-aliasing resource_patterns.c
 *   gcc -Os -m32 -dP -da resource_patterns.c
 */

#include <stddef.h>

/* Force functions to not be inlined to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* ========== Pattern A: ZERO_EXTRACT and MEM ========== */
NOINLINE static void pattern_a(volatile int *counter) {
    /* Struct with volatile bit-field to generate ZERO_EXTRACT */
    struct S {
        volatile unsigned int f1 : 5;
        volatile unsigned int f2 : 3;
        volatile unsigned int f3 : 8;
    } s;
    
    /* Array with complex indexing to generate MEM with addressing modes */
    static volatile int arr[16][16];
    
    /* Use counter to create non-constant indices */
    int i = *counter & 0xF;
    int j = (*counter >> 4) & 0xF;
    
    /* ZERO_EXTRACT: Assignment to volatile bit-field */
    s.f1 = i & 0x1F;
    s.f2 = j & 0x07;
    s.f3 = (i + j) & 0xFF;
    
    /* MEM: Complex array access with pointer arithmetic */
    volatile int *ptr = &arr[i][j];
    ptr += (i * j) & 0x7;
    *ptr = s.f1 + s.f2;
    
    /* More MEM patterns with different addressing */
    arr[(i + 1) & 0xF][(j - 1) & 0xF] = *ptr;
    arr[i][(j + *counter) & 0xF] = s.f3;
}

/* ========== Pattern B: STRICT_LOW_PART and SUBREG ========== */
NOINLINE static void pattern_b(volatile int *counter) {
    /* Use char/short types for STRICT_LOW_PART */
    volatile char c = *counter & 0xFF;
    volatile short s = *counter & 0xFFFF;
    
    /* STRICT_LOW_PART: Inline assembly modifying only low part */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)      /* =q constraint for byte-addressable register */
        : "0"(c)       /* Same register for input and output */
        : "cc"
    );
    
    /* Another STRICT_LOW_PART pattern with short */
    asm volatile (
        "subw $2, %0\n\t"
        : "=r"(s)      /* Could also use q for certain cases */
        : "0"(s)
        : "cc"
    );
    
    /* SUBREG: Type punning between different sized accesses */
    int i = *counter;
    
    /* Access int as short (SUBREG of larger register) */
    short *ps = (short*)&i;
    *ps = s;           /* Generates SUBREG for the short access */
    
    /* Access int as char */
    char *pc = (char*)&i;
    pc[1] = c;         /* Another SUBREG pattern */
    
    /* Mixed size operations to encourage SUBREG usage */
    long long ll = *counter;
    int *pi = (int*)&ll;
    pi[0] = i;         /* Access 64-bit as two 32-bit SUBREGs on 32-bit target */
    
    /* Force use of result */
    *counter = i + c + s;
}

/* ========== Pattern C: Mixed patterns with ternary operator ========== */
NOINLINE static void pattern_c(volatile int *counter, volatile int selector) {
    /* Struct with bit-fields at different positions */
    struct Mixed {
        volatile unsigned int a : 3;
        volatile unsigned int b : 7;
        volatile unsigned int c : 10;
        volatile unsigned int padding : 12;
    } m1, m2;
    
    /* Array for MEM patterns */
    static volatile int buffer[32];
    
    /* Initialize */
    m1.a = *counter & 0x7;
    m1.b = (*counter >> 3) & 0x7F;
    m1.c = (*counter >> 10) & 0x3FF;
    
    /* Ternary operator selecting different bit-field assignments */
    volatile struct Mixed *mp;
    if (selector & 1) {
        mp = &m1;
    } else {
        mp = &m2;
    }
    
    /* ZERO_EXTRACT through selected pointer */
    mp->b = (selector >> 1) & 0x7F;
    
    /* Complex MEM access with ternary in address calculation */
    int idx = (selector & 2) ? (*counter & 0x1F) : ((*counter >> 5) & 0x1F);
    
    /* MEM with scaled index */
    volatile int *addr = &buffer[idx * 2];
    *addr = mp->a + mp->b;
    
    /* More complex addressing with multiple indices */
    buffer[(*counter + selector) & 0x1F] = 
        buffer[(*counter - selector) & 0x1F] + 
        buffer[selector & 0x1F];
    
    /* SUBREG through type punning in ternary context */
    int temp = *counter;
    volatile short *sp = (selector & 4) ? (short*)&temp : (short*)buffer;
    sp[0] = mp->c & 0xFFFF;
}

/* ========== Main driver ========== */
int main(int argc, char **argv) {
    volatile int counter = 0;
    volatile int selector = 0;
    
    /* Use argc to bound loops for compilation analysis */
    int iterations = (argc > 1) ? 100 : 10;
    
    /* Initialize some volatile arrays */
    volatile int init_arr[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            init_arr[i][j] = i * j;
        }
    }
    
    /* Main loop to generate repeated RTL patterns */
    for (int i = 0; i < iterations; i++) {
        counter = i;
        selector = argc + i;
        
        /* Call each pattern function */
        pattern_a(&counter);
        pattern_b(&counter);
        pattern_c(&counter, selector);
        
        /* Complex MEM addressing in main loop */
        volatile int *ptr = (volatile int*)init_arr;
        ptr[(i * 3) % 100] = counter;
        
        /* Force use of all results to prevent elimination */
        selector += counter;
    }
    
    /* Final dummy operation */
    volatile int result = counter + selector;
    
    /* The program doesn't need correct runtime semantics,
     * but this prevents it from being optimized away entirely */
    return result != 0;
}
