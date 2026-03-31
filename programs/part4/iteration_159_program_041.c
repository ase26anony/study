#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
int __attribute__((noinline)) get_input(int x) {
    return x ^ 0x1234;
}

int __attribute__((noinline)) use_result(int x) {
    return x * 2;
}

/* Test function 1: MIPS target with simple jump pattern */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_case_1(int a, int b) {
    /* Initialize temporaries independent of jump condition */
    int temp1 = a + 5;
    int temp2 = b * 3;
    int temp3 = temp1 ^ temp2;
    
    /* Create jump condition that's not trivially predictable */
    int condition = (a > b) && (get_input(a) != 0);
    
    /* Simple goto to label - should generate simplejump_p */
    if (condition) {
        goto target_label_1;
    }
    
    /* Some other code to create basic blocks */
    temp3 = temp3 + 1;
    
target_label_1:
    /* Safe, non-jump instruction after label */
    /* Uses independent temporary variables */
    int result = temp1 + temp2;
    
    /* Use result to prevent dead code elimination */
    return use_result(result);
}

/* Test function 2: SPARC target with different pattern */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y) {
    /* More temporaries to increase scheduling opportunities */
    int t1 = x & 0xFF;
    int t2 = y | 0x55;
    int t3 = t1 * 2;
    int t4 = t2 - 1;
    
    /* Dynamic condition using volatile read */
    volatile int vol = x;
    int cond = (vol > 100) && (y < 200);
    
    /* Simple conditional jump */
    if (cond) {
        goto target_label_2;
    }
    
    /* Alternative path with different operations */
    t3 = t3 ^ t4;
    
target_label_2:
    /* Safe arithmetic operation after label */
    /* No memory access, no division, no function calls */
    int res = (t3 << 2) | (t4 >> 1);
    
    return res + global_accumulator;
}

/* Test function 3: Generic pattern with loop context */
int test_case_3(int n) {
    int sum = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Local temporaries inside loop */
        int tmp_a = i * 3;
        int tmp_b = i + 7;
        int tmp_c = tmp_a ^ tmp_b;
        
        /* Jump condition based on loop iteration */
        if ((i & 1) && (tmp_c > 10)) {
            goto loop_target;
        }
        
        /* Continue normal loop processing */
        sum += tmp_a;
        continue;
        
    loop_target:
        /* Safe instruction after label inside loop */
        int loop_res = tmp_b - tmp_a;
        sum += loop_res;
    }
    
    return sum;
}

/* Test function 4: Nested control flow */
int test_case_4(int a, int b, int c) {
    int x = a + b;
    int y = b + c;
    int z = c + a;
    
    /* First level condition */
    if (x > y) {
        int tmp1 = x * 2;
        int tmp2 = y / 2;  /* Safe: y won't be 0 in our test */
        
        /* Second level - the target jump */
        if (tmp1 > tmp2 && get_input(a) > 0) {
            goto nested_target;
        }
        
        z = tmp1 + tmp2;
    }
    
    /* Some other computation */
    y = y ^ z;
    
nested_target:
    /* Safe bitwise operation after label */
    int final = (x & y) | z;
    
    return final;
}

/* Test function 5: Multiple labels and jumps */
int test_case_5(int val) {
    int a = val + 1;
    int b = val * 2;
    int c = val ^ 0xAA;
    
    /* Multiple conditions leading to same label */
    if (val > 50) {
        if (get_input(val) & 1) {
            goto common_target;
        }
    }
    
    if (val < 20) {
        if ((val * val) > 100) {
            goto common_target;
        }
    }
    
    /* Default path */
    a = b + c;
    
common_target:
    /* Safe logical operation */
    int result = (a & 0xF) | (b & 0xF0) | (c & 0xF00);
    
    return result;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use command line arguments or defaults for variability */
    int a = argc > 1 ? atoi(argv[1]) : 42;
    int b = argc > 2 ? atoi(argv[2]) : 123;
    int c = argc > 3 ? atoi(argv[3]) : 77;
    int n = argc > 4 ? atoi(argv[4]) : 10;
    
    /* Run all test cases */
    total += test_case_1(a, b);
    total += test_case_2(b, c);
    total += test_case_3(n);
    total += test_case_4(a, b, c);
    total += test_case_5(a);
    
    /* Update global to prevent optimization across calls */
    global_accumulator = total;
    
    printf("Total checksum: %d\n", total);
    
    /* Also print individual results for debugging */
    printf("Results: %d %d %d %d %d\n",
           test_case_1(a, b),
           test_case_2(b, c),
           test_case_3(n),
           test_case_4(a, b, c),
           test_case_5(a));
    
    return total != 0 ? 0 : 1;
}
