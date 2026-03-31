/* Program to trigger GCC delay slot filling logic in reorg.cc lines 2135-2149 */
#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for specific variables to avoid resource conflicts */
#ifdef __mips__
#define REG1 asm("$t0")
#define REG2 asm("$t1")
#define REG3 asm("$t2")
#define REG4 asm("$t3")
#define REG5 asm("$t4")
#define REG6 asm("$t5")
#elif __sparc__
#define REG1 asm("%l0")
#define REG2 asm("%l1")
#define REG3 asm("%l2")
#define REG4 asm("%l3")
#define REG5 asm("%l4")
#define REG6 asm("%l5")
#else
/* Generic register hints - compiler will choose appropriate registers */
#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#define REG6
#endif

int main() {
    /* Declare variables with register hints to control resource allocation */
    register int a REG1 = 0;
    register int b REG2 = 100;
    register int c REG3 = 0;
    register int d REG4 = 50;
    register int e REG5 = 0;
    register int f REG6 = 25;
    
    /* Volatile iteration count to prevent loop unrolling */
    volatile int iterations = 100;
    int i;
    
    /* Result accumulator to keep computations live */
    int result = 0;
    
    for (i = 0; i < iterations; i++) {
        /* Varying branch conditions to create different execution paths */
        int condition = (i % 3);
        
        /* BRANCH 1: Simple conditional with predictable pattern */
        if (__builtin_expect(a > b, 0)) {
            /* Insert nop to create filler opportunity */
            asm volatile("nop" ::: "memory");
target_label1:
            /* Candidate for delay slot: simple arithmetic with different registers */
            e = f + 1;  /* Uses REG5 and REG6, not involved in branch condition */
            /* Continue with other operations */
            c = d + i;
        } else {
            a++;
        }
        
        /* BRANCH 2: Different condition and target */
        if (__builtin_expect(c < d, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Two nops for multiple trial opportunities */
target_label2:
            /* Another candidate: different arithmetic operation */
            f = e * 2;  /* Safe, trap-free operation */
            b = a - 1;
        } else {
            d--;
        }
        
        /* BRANCH 3: More complex pattern with varying nop count */
        switch (condition) {
            case 0:
                if (__builtin_expect((a & 1) == 0, 0)) {
                    asm volatile("nop" ::: "memory");
target_label3:
                    /* Simple move operation - good delay slot candidate */
                    int temp = b;
                    b = a;
                    a = temp;
                }
                break;
            case 1:
                if (__builtin_expect(b > 50, 1)) {
                    asm volatile("nop" ::: "memory");
                    asm volatile("nop" ::: "memory");
                    asm volatile("nop" ::: "memory");
target_label4:
                    /* Arithmetic with immediate - safe and trap-free */
                    c = d + 5;
                    result += c;
                }
                break;
            case 2:
                if (__builtin_expect(d < 75, 0)) {
                    /* No nop here - tests different slot filling scenarios */
target_label5:
                    /* Very simple operation */
                    e = f;
                    result += e;
                }
                break;
        }
        
        /* Modify variables to change branch behavior */
        a = (a + i) % 100;
        b = (b - i) % 100;
        if (b < 0) b += 100;
        c = (c + i * 2) % 100;
        d = (d + i / 2) % 100;
        
        /* Add memory barrier to prevent over-optimization */
        asm volatile("" ::: "memory");
    }
    
    /* Use results to prevent dead code elimination */
    result = a + b + c + d + e + f;
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
