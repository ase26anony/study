/* delay_slot_test.c - Target GCC's reorg.cc delay slot filler */
#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for critical variables */
register int r0 asm("$2");  /* MIPS $v0 / general reg */
register int r1 asm("$3");  /* MIPS $v1 */
register int r2 asm("$4");  /* MIPS $a0 */
register int r3 asm("$5");  /* MIPS $a1 */
register int r4 asm("$6");  /* MIPS $a2 */
register int r5 asm("$7");  /* MIPS $a3 */
register int r6 asm("$8");  /* MIPS $t0 */
register int r7 asm("$9");  /* MIPS $t1 */

/* Volatile counter to prevent loop unrolling */
volatile int iterations = 100;

/* Simple trap-free arithmetic operations */
#define SAFE_ADD(a, b) ((a) + (b))
#define SAFE_SUB(a, b) ((a) - (b))
#define SAFE_MUL(a, b) ((a) * (b))

int main(void) {
    int result = 0;
    
    /* Initialize registers with non-zero values */
    r0 = 1; r1 = 2; r2 = 3; r3 = 4;
    r4 = 5; r5 = 6; r6 = 7; r7 = 8;
    
    /* Main loop with varying branch patterns */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Conditional branch with predictable outcome */
        if (__builtin_expect((r0 > r1), 0)) {
            /* This branch is unlikely taken */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target_label_1:
            /* Candidate for delay slot: simple reg-to-reg operation */
            r4 = SAFE_ADD(r5, 1);  /* Uses different regs than branch condition */
            /* Continue with other operations */
            r6 = SAFE_MUL(r7, 2);
        } else {
            r0 = SAFE_ADD(r0, 1);
        }
        
        /* Pattern 2: Another conditional with different register sets */
        if (__builtin_expect((r2 < r3), 1)) {
            /* This branch is likely taken */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target_label_2:
            /* Another delay slot candidate */
            r5 = SAFE_SUB(r6, r7);  /* Independent of branch condition regs */
            r0 = SAFE_ADD(r0, r1);
        }
        
        /* Pattern 3: Nested conditionals to create complex flow */
        if (__builtin_expect((r4 != r5), 0)) {
            asm volatile("nop" ::: "memory");
target_label_3:
            /* Simple move operation - ideal delay slot candidate */
            r2 = r3;
            if (__builtin_expect((r6 > 0), 1)) {
                asm volatile("nop" ::: "memory");
target_label_4:
                /* Another candidate */
                r7 = SAFE_ADD(r0, 3);
            }
        }
        
        /* Pattern 4: Switch-like structure with multiple labels */
        switch (i % 4) {
            case 0:
                if (__builtin_expect((r1 == r2), 0)) {
                    asm volatile("nop" ::: "memory");
                    asm volatile("nop" ::: "memory");
target_label_5:
                    r3 = SAFE_ADD(r4, r5);
                }
                break;
            case 1:
                if (__builtin_expect((r3 != r4), 1)) {
                    asm volatile("nop" ::: "memory");
target_label_6:
                    r6 = SAFE_SUB(r7, 1);
                }
                break;
            default:
                /* Modify registers to change branch outcomes */
                r0 = SAFE_ADD(r0, i);
                r1 = SAFE_SUB(r1, 1);
                break;
        }
        
        /* Force register dependencies to prevent dead code elimination */
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
        
        /* Vary register values to create different branch behaviors */
        r0 = (r0 * 1103515245 + 12345) & 0x7fffffff;
        r1 = (r1 * 1103515245 + 12345) & 0x7fffffff;
        if (i % 10 == 0) {
            /* Occasionally swap registers */
            int tmp = r2;
            r2 = r3;
            r3 = tmp;
        }
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Additional test case: Simple function with clear delay slot opportunity */
    {
        int a = 10, b = 20, c = 0, d = 0;
        
        /* Create a predictable branch */
        if (__builtin_expect(a < b, 1)) {
            /* Multiple nops to give filler options */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
final_target_label:
            /* Perfect delay slot candidate: uses different registers,
               no traps, simple operation */
            d = c + 1;  /* c is 0, so no overflow/trap issues */
        }
        
        printf("Final values: a=%d, b=%d, c=%d, d=%d\n", a, b, c, d);
    }
    
    return result != 0 ? 0 : 1;
}

/* Helper function to create more complex control flow */
static int helper_func(int x, int y) {
    register int t0 asm("$8") = x;
    register int t1 asm("$9") = y;
    
    /* Create branch with delay slot opportunity */
    if (__builtin_expect(t0 > t1, 0)) {
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
helper_label:
        /* Safe, independent operation */
        int z = t0 - t1;
        return z;
    }
    
    return t0 + t1;
}
