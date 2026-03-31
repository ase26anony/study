/* Test program for delay slot filling in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

volatile int g_volatile = 0;
static int g_accumulator = 0;

/* Optimization barrier */
__attribute__((noinline)) int barrier(int x) {
    return x ^ 0x55AA55AA;
}

/* Test function 1: Simple conditional jump with arithmetic after label */
__attribute__((target("arch=mips32")))
int test_case_1(int a, int b) {
    int temp1 = a + b;
    int temp2 = a - b;
    int temp3 = a * b;
    int result = 0;
    
    /* Create control flow complexity */
    if (a > 0) {
        temp1 = barrier(temp1);
        if (b < 0) {
            temp2 = barrier(temp2);
        }
    }
    
    /* Target pattern: simple conditional jump to label */
    if ((a + b) > (a - b)) {
        /* This should compile to a simple jump to label L1 */
        goto L1;
    }
    
    /* Some other code to avoid fall-through optimization */
    temp3 = temp3 * 2;
    result = temp3;
    goto end;
    
L1:
    /* Instruction after label: safe arithmetic on temporaries */
    /* Uses temp1 and temp2 which are defined before the jump */
    /* and not used in the jump condition (a+b > a-b) */
    temp1 = temp1 + temp2;  /* Simple addition */
    result = temp1;
    
end:
    return barrier(result);
}

/* Test function 2: Different variable pattern */
__attribute__((target("arch=mips32")))
int test_case_2(int x, int y) {
    int local_a = x & 0xFF;
    int local_b = y & 0xFF;
    int local_c = x | y;
    int local_d = x ^ y;
    
    /* More complex control flow */
    for (int i = 0; i < 2; i++) {
        local_a = barrier(local_a + i);
    }
    
    /* Jump condition uses only x and y */
    if (x != y) {
        /* Simple jump to label */
        goto target_label;
    }
    
    /* Alternative path */
    local_c = local_c >> 1;
    return barrier(local_c);
    
target_label:
    /* Safe instruction: logical operation on temporaries */
    /* local_d not used in condition, local_a defined before jump */
    local_d = local_d & local_a;  /* Simple AND operation */
    return barrier(local_d);
}

/* Test function 3: SPARC target variant */
__attribute__((target("arch=sparc")))
int test_case_3(int p, int q) {
    volatile int v = g_volatile;
    int t1 = p + v;
    int t2 = q - v;
    int t3 = p * q;
    int t4 = p ^ q;
    
    /* Nested condition to create basic blocks */
    if (p > 100) {
        t1 = t1 * 2;
        if (q < 50) {
            t2 = t2 / 2;  /* Division but with positive numbers */
        }
    }
    
    /* Target jump pattern */
    if ((p & 1) == 0) {  /* Even check */
        goto even_case;
    }
    
    /* Odd case */
    t3 = t3 + 1;
    return barrier(t3);
    
even_case:
    /* Safe instruction: bit manipulation */
    /* t4 not used in jump condition, t1 defined before jump */
    t4 = (t4 << 1) | (t1 & 1);  /* Shift and OR */
    return barrier(t4);
}

/* Test function 4: Generic pattern with multiple temporaries */
int test_case_4(int m, int n) {
    /* Many temporary variables to avoid resource conflicts */
    int tmp1 = m + 1;
    int tmp2 = n - 1;
    int tmp3 = m * 2;
    int tmp4 = n * 2;
    int tmp5 = m ^ n;
    int tmp6 = m & n;
    
    /* Complex enough to avoid early optimization */
    if (m > n) {
        tmp1 = barrier(tmp1);
        tmp2 = barrier(tmp2);
    } else {
        tmp3 = barrier(tmp3);
        tmp4 = barrier(tmp4);
    }
    
    /* The critical jump pattern */
    if (tmp5 > tmp6) {
        goto compute_result;
    }
    
    /* Alternative computation */
    tmp1 = tmp1 + tmp2 + tmp3;
    return barrier(tmp1);
    
compute_result:
    /* Safe instruction: uses tmp4 and tmp5 which are not live across the jump */
    /* tmp5 was used in condition but we use it differently here */
    tmp4 = tmp4 + (tmp5 & 0xF);  /* Masked addition */
    return barrier(tmp4);
}

/* Test function 5: Minimal pattern focusing on the exact condition */
__attribute__((target("arch=mips32")))
int test_case_5(int val) {
    int a = val;
    int b = a + 10;
    int c = a * 2;
    int d = a & 0xF;
    
    /* Read volatile to prevent constant propagation */
    volatile int v = g_volatile;
    if (v > 0) {
        a = a + v;
    }
    
    /* Simple jump condition */
    if (a > 5) {
        goto process;
    }
    
    c = c - 1;
    return barrier(c);
    
process:
    /* Single safe arithmetic instruction after label */
    /* Uses b and d which are defined before and not in condition */
    b = b + d;  /* Simple addition of two temporaries */
    return barrier(b);
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile */
    g_volatile = 42;
    
    /* Run test cases with different inputs */
    checksum ^= test_case_1(10, 20);
    checksum ^= test_case_2(30, 40);
    checksum ^= test_case_3(150, 25);
    checksum ^= test_case_4(100, 200);
    checksum ^= test_case_5(7);
    
    /* Additional variations */
    checksum ^= test_case_1(-5, 15);
    checksum ^= test_case_2(0, 100);
    checksum ^= test_case_3(80, 80);
    checksum ^= test_case_4(255, 1);
    checksum ^= test_case_5(3);
    
    printf("Checksum: 0x%08X\n", checksum);
    
    /* Use result to prevent optimization */
    g_accumulator = checksum;
    
    return (checksum == 0) ? 0 : 1;
}
