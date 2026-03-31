/* Target: reorg.cc lines 2135-2149 - Delay slot filling with specific conditions */
#include <stdio.h>
#include <stdlib.h>

/* Force specific register usage to avoid resource conflicts */
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
/* Generic register hints for other RISC architectures */
#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#define REG6
#endif

/* Function to create eligible delay slot candidates */
static int process_values(int count) {
    /* Use register variables to control allocation */
    register int a REG1 = 0;
    register int b REG2 = 1;
    register int c REG3 = 2;
    register int d REG4 = 3;
    register int e REG5 = 4;
    register int f REG6 = 5;
    
    /* Volatile to prevent loop unrolling and preserve branch structure */
    volatile int iterations = count;
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Varying branch conditions to create different filling scenarios */
        int condition = (i % 3);
        
        /* BRANCH 1: Simple conditional with predictable outcome */
        if (__builtin_expect((a > b), 0)) {
            /* Insert nop to create filler opportunity */
            asm volatile("nop" ::: "memory");
            /* Target label for delay slot candidate */
            target_label_1:
            /* Eligible instruction: simple arithmetic, no traps, no resource conflicts */
            c = d + 1;  /* Uses different registers than branch condition */
        } else {
            /* Alternative path */
            a = i + 1;
        }
        
        /* BRANCH 2: Different condition, different register usage */
        if (__builtin_expect((c < d), 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Two nops for multiple slot attempts */
            target_label_2:
            /* Another eligible instruction - register move operation */
            e = f;  /* Simple move, no computation */
        } else {
            b = i * 2;
        }
        
        /* BRANCH 3: More complex condition to vary the pattern */
        if (__builtin_expect((e != f) && (a < iterations), 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Three nops */
            target_label_3:
            /* Arithmetic with immediate - still trap-free */
            f = e + 2;
        }
        
        /* BRANCH 4: Nested condition to create deeper analysis */
        if (__builtin_expect(condition == 0, 1)) {
            /* Multiple trial opportunities before label */
            asm volatile("nop" ::: "memory");
            if (__builtin_expect(b > a, 0)) {
                asm volatile("nop" ::: "memory");
                target_label_4:
                /* Another safe arithmetic operation */
                d = c + a;  /* Mixes registers but still safe */
            }
        }
        
        /* Modify variables to change branch outcomes */
        a = (a + b) % 100;
        b = (b + c) % 100;
        c = (c + 1) % 100;
        d = (d + i) % 100;
        e = (e ^ f) & 0xFF;
        f = (f + 5) % 100;
        
        result += a + b + c + d + e + f;
    }
    
    return result;
}

/* Helper function to create additional context */
static int helper_compute(int x, int y) {
    /* Independent computation using different registers */
    register int r1 REG1 = x;
    register int r2 REG2 = y;
    register int r3 REG3 = 0;
    
    /* Create another branch opportunity */
    if (__builtin_expect(r1 > r2, 0)) {
        asm volatile("nop" ::: "memory");
        helper_label:
        /* Simple arithmetic for delay slot candidate */
        r3 = r1 - r2;
    }
    
    return r3;
}

int main() {
    int total = 0;
    volatile int outer_iterations = 50;
    
    /* Outer loop to create multiple compilation contexts */
    for (int j = 0; j < outer_iterations; j++) {
        /* Call with varying counts to create different optimization scenarios */
        int count = 10 + (j % 5);
        total += process_values(count);
        
        /* Additional branch-heavy section */
        register int x REG1 = j;
        register int y REG2 = total % 100;
        register int z REG3 = 0;
        
        /* Multiple conditional branches in sequence */
        if (__builtin_expect(x > y, 1)) {
            asm volatile("nop" ::: "memory");
            main_label_1:
            z = x * 2;  /* Multiplication is still trap-free for integers */
        }
        
        if (__builtin_expect((x + y) < 100, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            main_label_2:
            z = y + 10;
        }
        
        total += z + helper_compute(x, y);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
