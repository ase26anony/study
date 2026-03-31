/* Test program for delay slot filling in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

volatile int global_seed = 42;
int global_accumulator = 0;

/* Optimization barrier */
__attribute__((noinline)) int get_value(int x) {
    return x ^ 0x55AA55AA;
}

/* Barrier to prevent cross-jump optimization */
__attribute__((noinline)) void use_value(int x) {
    global_accumulator += x;
}

/* Test function 1: Simple conditional jump with arithmetic after label */
__attribute__((target("arch=mips32")))
int test_case_1(int a, int b) {
    int temp1 = a + b;
    int temp2 = a - b;
    int temp3 = a * b;
    int result = 0;
    
    /* Create independent temporaries for post-label instruction */
    int x = temp1;
    int y = temp2;
    int z = temp3;
    
    /* Dynamic condition to prevent optimization */
    if (a > b && (a % 7) != 0) {
        /* This should compile to a simple jump to label */
        goto target_label_1;
    }
    
    /* Some other code to create basic blocks */
    result = get_value(a);
    goto end_label_1;
    
target_label_1:
    /* Safe, non-jump instruction: simple arithmetic on temporaries */
    /* This instruction should be eligible for delay slot filling */
    x = y + z;  /* Uses variables not in jump condition */
    
    result = x;
    
end_label_1:
    use_value(result);
    return result;
}

/* Test function 2: Different variable pattern */
__attribute__((target("arch=mips32")))
int test_case_2(int p, int q) {
    int local_a = p ^ q;
    int local_b = p | q;
    int local_c = p & q;
    int local_d = p + q;
    
    /* More temporaries for post-label use */
    int t1 = local_a;
    int t2 = local_b;
    int t3 = local_c;
    int t4 = local_d;
    
    /* Volatile read to create unpredictable condition */
    volatile int cond = global_seed;
    if ((p < q) && (cond & 1)) {
        goto target_label_2;
    }
    
    /* Alternative path */
    t1 = get_value(p);
    goto finish_2;
    
target_label_2:
    /* Safe bitwise operation - cannot trap */
    t3 = t2 ^ t4;  /* Independent of jump condition variables */
    
    t1 = t3;
    
finish_2:
    use_value(t1);
    return t1;
}

/* Test function 3: SPARC target variant */
__attribute__((target("arch=sparc")))
int test_case_3(int x, int y) {
    /* Create multiple independent temporary variables */
    int a1 = x + 1;
    int a2 = y - 1;
    int a3 = x * 2;
    int a4 = y * 3;
    int a5 = x ^ y;
    
    /* Variables for post-label instruction */
    int m = a1;
    int n = a2;
    int o = a3;
    
    /* Complex enough condition to not be optimized away */
    if ((x != y) && ((x + y) % 3 == 0)) {
        goto sparc_target;
    }
    
    /* Other control flow */
    if (x > 0) {
        m = get_value(y);
    } else {
        m = get_value(x);
    }
    goto sparc_end;
    
sparc_target:
    /* Safe logical operation */
    o = m & n;  /* Uses variables not referenced in jump condition */
    
    m = o;
    
sparc_end:
    use_value(m);
    return m;
}

/* Test function 4: Nested conditions to create more complex CFG */
__attribute__((target("arch=mips32")))
int test_case_4(int base) {
    int v1 = base * 3;
    int v2 = base / 2;  /* Safe division - base is not zero in our calls */
    int v3 = base + 100;
    int v4 = base - 50;
    
    /* Post-label temporaries */
    int r1 = v1;
    int r2 = v2;
    int r3 = v3;
    
    /* Multiple conditions to create branching */
    if (base > 10) {
        if (base < 100) {
            if ((base & 3) == 0) {
                goto nested_target;
            }
        }
    }
    
    /* Alternative path with loop to create more scheduling context */
    for (int i = 0; i < 3; i++) {
        r1 += i;
    }
    goto nested_end;
    
nested_target:
    /* Safe shift operation */
    r3 = r1 << 2;  /* Independent of condition variables */
    
    r1 = r3;
    
nested_end:
    use_value(r1);
    return r1;
}

/* Test function 5: Minimal pattern with volatile */
__attribute__((target("arch=mips32")))
int test_case_5(int val) {
    volatile int flag = global_seed;
    int tmp1 = val + 5;
    int tmp2 = val * 2;
    int tmp3 = val | 0xFF;
    
    int res = tmp1;
    
    /* Simple volatile-based condition */
    if (flag > 0) {
        goto minimal_target;
    }
    
    res = tmp2;
    goto minimal_end;
    
minimal_target:
    /* Minimal safe instruction */
    tmp3 = tmp1 + 1;  /* Uses variable not in condition */
    
    res = tmp3;
    
minimal_end:
    use_value(res);
    return res;
}

int main() {
    int results[5];
    int checksum = 0;
    
    /* Call test functions with different arguments to avoid
       constant propagation and create varied conditions */
    results[0] = test_case_1(17, 5);    /* a > b, a%7 != 0 */
    results[1] = test_case_2(8, 12);    /* p < q, cond has bit 0 set */
    results[2] = test_case_3(6, 9);     /* x != y, (x+y)%3 == 0 */
    results[3] = test_case_4(24);       /* 10 < base < 100, base%4 == 0 */
    results[4] = test_case_5(100);      /* flag > 0 */
    
    /* Compute checksum to ensure all code executed */
    for (int i = 0; i < 5; i++) {
        checksum ^= results[i];
        printf("Test %d result: %d (0x%08x)\n", i + 1, results[i], results[i]);
    }
    
    printf("Global accumulator: %d\n", global_accumulator);
    printf("Final checksum: 0x%08x\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
