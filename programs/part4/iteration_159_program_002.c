/* Test program for delay slot filling in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

volatile int g_volatile = 0;
static int g_accumulator = 0;

/* Optimization barrier */
static int __attribute__((noinline)) barrier(int x) {
    return x ^ 0x55AA55AA;
}

/* Test function 1: Simple conditional jump with safe arithmetic after label */
__attribute__((target("arch=mips32")))
static int test_case_1(int a, int b) {
    int temp1 = a * 3;
    int temp2 = b + 7;
    int temp3 = 0;
    
    /* Create other basic blocks first */
    if (a > 100) {
        temp1 = barrier(temp1);
    }
    
    /* Main target pattern */
    if (a > b) {
        /* This should compile to a simple jump to label */
        goto target_label_1;
    }
    
    temp3 = temp1 - temp2;
    return temp3;
    
target_label_1:
    /* Safe, non-jump instruction that uses independent temporaries */
    temp3 = temp1 & 0xFF;  /* Simple bitwise operation */
    return temp3 + 1;
}

/* Test function 2: Different condition, different safe operation */
__attribute__((target("arch=sparc")))
static int test_case_2(int x, int y) {
    volatile int v = g_volatile;
    int local_a = x + v;
    int local_b = y * 2;
    int local_c = 0;
    int local_d = 5;
    
    /* Some control flow before */
    for (int i = 0; i < 2; i++) {
        local_a += i;
    }
    
    /* Dynamic condition using volatile */
    if (local_a > local_b) {
        goto target_label_2;
    }
    
    local_c = local_a | local_b;
    return local_c;
    
target_label_2:
    /* Safe arithmetic with independent variables */
    local_d = local_b + 3;  /* Simple addition */
    return local_d ^ local_a;
}

/* Test function 3: Multiple temporaries, no memory access */
__attribute__((target("arch=mips32")))
static int test_case_3(int p, int q) {
    int t1 = p << 2;
    int t2 = q >> 1;
    int t3 = 0;
    int t4 = 0;
    
    /* Nested if to create more CFG complexity */
    if (p != 0) {
        if (q % 2 == 0) {
            t1 = barrier(t1);
        }
    }
    
    /* Jump condition using function arguments */
    if ((p + q) > 50) {
        goto target_label_3;
    }
    
    t3 = t1 * t2;
    return t3;
    
target_label_3:
    /* Multiple safe operations, but first one is candidate for delay slot */
    t4 = t2 - t1;  /* Simple subtraction */
    t3 = t4 * 2;
    return t3;
}

/* Test function 4: Using only local variables, no function calls */
__attribute__((target("arch=sparc")))
static int test_case_4(int m, int n) {
    int var1 = m + 10;
    int var2 = n - 5;
    int var3 = 0;
    int var4 = 1;
    
    /* Loop to create scheduling context */
    for (int j = 0; j < 3; j++) {
        var4 += j;
    }
    
    /* Condition that's not trivially true/false */
    if (var1 < var2 && m > 0) {
        goto target_label_4;
    }
    
    var3 = var1 % (var2 + 1);
    return var3;
    
target_label_4:
    /* Safe logical operation */
    var3 = var4 | 0x0F;  /* Bitwise OR with constant */
    return var3;
}

/* Test function 5: More complex surrounding code but simple jump */
__attribute__((target("arch=mips32")))
static int test_case_5(int r, int s) {
    int a1 = r ^ s;
    int a2 = r & s;
    int a3 = 0;
    int a4 = 100;
    
    /* Switch to create additional basic blocks */
    switch (r % 4) {
        case 0: a1 += 1; break;
        case 1: a1 -= 1; break;
        default: a1 *= 2; break;
    }
    
    /* The target simple jump */
    if (a1 > a4) {
        goto target_label_5;
    }
    
    a3 = a2 << 1;
    return a3;
    
target_label_5:
    /* Safe shift operation */
    a3 = a2 >> 2;  /* Simple shift */
    return a3 + a1;
}

int main(void) {
    int result = 0;
    int checksum = 0;
    
    /* Initialize volatile to prevent constant folding */
    g_volatile = rand() % 100;
    
    /* Run all test cases with different inputs */
    result = test_case_1(42, 17);
    checksum += result;
    printf("Test 1: %d\n", result);
    
    result = test_case_2(75, 30);
    checksum += result;
    printf("Test 2: %d\n", result);
    
    result = test_case_3(20, 40);
    checksum += result;
    printf("Test 3: %d\n", result);
    
    result = test_case_4(15, 25);
    checksum += result;
    printf("Test 4: %d\n", result);
    
    result = test_case_5(60, 35);
    checksum += result;
    printf("Test 5: %d\n", result);
    
    printf("Checksum: %d\n", checksum);
    
    /* Use result to prevent dead code elimination */
    g_accumulator = checksum;
    
    return (checksum != 0) ? 0 : 1;
}
