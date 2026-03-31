/* Pattern-generating code for GCC resource.cc coverage testing */
#include <stddef.h>

/* Force no inlining to preserve RTL patterns */
#define NOINLINE __attribute__((noinline))

/* Function A: ZERO_EXTRACT and MEM patterns */
NOINLINE static void func_a(volatile int *counter) {
    /* Struct with volatile bit-fields for ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int f1 : 5;
        volatile unsigned int f2 : 3;
        volatile unsigned int f3 : 8;
    } bs;
    
    /* Array for MEM patterns with complex addressing */
    volatile int arr[16][16];
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    bs.f1 = (*counter) & 0x1F;
    bs.f2 = (*counter >> 5) & 0x7;
    bs.f3 = (*counter >> 8) & 0xFF;
    
    /* MEM pattern with complex addressing */
    int idx1 = (*counter) & 0xF;
    int idx2 = (*counter >> 4) & 0xF;
    
    /* Force MEM reference with pointer arithmetic */
    volatile int *ptr = &arr[idx1][idx2];
    *ptr = (*counter) * 2;
    
    /* Additional MEM with offset */
    volatile int val = arr[(idx1 + 1) & 0xF][(idx2 + 1) & 0xF];
    (void)val; /* Prevent unused warning */
}

/* Function B: STRICT_LOW_PART and SUBREG patterns */
NOINLINE static void func_b(volatile int *counter) {
    /* Use char/short types for STRICT_LOW_PART */
    volatile char c = (*counter) & 0xFF;
    volatile short s = (*counter) & 0xFFFF;
    
    /* STRICT_LOW_PART pattern: inline assembly modifying low part */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)
        : "0"(c)
        : "cc"
    );
    
    /* Another STRICT_LOW_PART with short */
    asm volatile (
        "addw $2, %0\n\t"
        : "=r"(s)
        : "0"(s)
        : "cc"
    );
    
    /* SUBREG pattern: type punning with different sizes */
    int i = *counter;
    short *ps = (short*)&i;
    *ps = (*counter) & 0xFFFF;  /* SUBREG store */
    
    /* More SUBREG: access different parts of larger type */
    long long ll = (*counter) * 100LL;
    int *pi = (int*)&ll;
    *pi = (*counter) & 0x7FFFFFFF;  /* Another SUBREG */
    
    /* Mixed-size operations */
    char *pc = (char*)&i;
    pc[1] = (*counter) & 0xFF;  /* Byte access within int */
}

/* Function C: Mixed patterns with ternary and complex expressions */
NOINLINE static void func_c(volatile int *counter, volatile int *result) {
    /* Struct with bit-field for ZERO_EXTRACT */
    struct {
        volatile unsigned int flag : 1;
        volatile unsigned int value : 15;
    } data;
    
    /* Array for MEM patterns */
    static volatile int matrix[8][8];
    
    /* Complex expression mixing patterns */
    int idx = (*counter) & 0x7;
    
    /* Ternary selecting different addressing modes */
    volatile int *target = (data.flag) ? 
                          &matrix[idx][idx] : 
                          &matrix[7 - idx][idx];
    
    /* Assignment that could involve multiple RTL transformations */
    *target = (*counter) * 3;
    
    /* Update bit-field based on condition (ZERO_EXTRACT) */
    data.flag = (*target > 100) ? 1 : 0;
    data.value = (*target) & 0x7FFF;
    
    /* Complex MEM access with multiple indices */
    *result += matrix[(idx + 1) & 0x7][(idx + 2) & 0x7];
}

/* Helper with loop to increase RTL complexity */
NOINLINE static void complex_helper(volatile int iter) {
    volatile int temp = 0;
    
    /* Loop to create repeated RTL patterns */
    for (volatile int i = 0; i < (iter & 0x3); i++) {
        /* Mixed operations */
        struct {
            volatile unsigned int a : 4;
            volatile unsigned int b : 4;
        } s;
        
        s.a = (iter + i) & 0xF;
        s.b = (iter - i) & 0xF;
        
        /* SUBREG pattern */
        int x = iter * i;
        short *ps = (short*)&x;
        *ps = (short)(x & 0xFFFF);
        
        temp += s.a + s.b + x;
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(temp));
}

int main(int argc, char *argv[]) {
    volatile int counter = 0;
    volatile int result = 0;
    
    /* Use argc to bound loops for analysis */
    int iterations = (argc > 1) ? 10 : 5;
    
    /* Main loop generating RTL patterns */
    for (volatile int i = 0; i < iterations; i++) {
        counter = i * 17;  /* Non-linear to prevent optimization */
        
        /* Call pattern-generating functions */
        func_a(&counter);
        func_b(&counter);
        func_c(&counter, &result);
        complex_helper(counter);
        
        /* Additional volatile operations */
        result += counter;
        
        /* Force MEM references with array */
        volatile int local_arr[4] = {counter, counter + 1, counter + 2, counter + 3};
        result += local_arr[i & 0x3];
    }
    
    /* Final dummy use to prevent elimination */
    asm volatile ("" : : "r"(result));
    
    return 0;
}
