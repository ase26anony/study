#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
int __attribute__((noinline)) get_input(int x) {
    return x ^ 0x55AA55AA;
}

int __attribute__((noinline)) use_result(int x) {
    return x * 3 + 1;
}

/* Test function 1: MIPS target with simple conditional jump */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_case_1(int a, int b) {
    /* Local temporaries - independent from condition variables */
    int temp1 = a + 100;
    int temp2 = b - 50;
    int temp3 = 0;
    int temp4 = 0;
    
    /* Create a non-trivial condition using input-dependent values */
    if (get_input(a) > get_input(b)) {
        /* Simple goto to label - should generate simplejump_p */
        goto target_label_1;
    }
    
    /* Some other code to create basic blocks */
    temp1 = temp1 * 2;
    temp2 = temp2 / 3;
    
    /* This should not be reached if condition is true */
    return temp1 + temp2;

target_label_1:
    /* Safe, non-jump instruction immediately after label */
    /* Uses independent temporaries not involved in the condition */
    temp3 = temp1 + temp2;  /* Simple arithmetic */
    
    /* Use the result to prevent dead code elimination */
    return use_result(temp3);
}

/* Test function 2: SPARC target with different pattern */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y) {
    /* More independent temporaries */
    int t1 = x * 3;
    int t2 = y * 7;
    int t3 = 0;
    int t4 = 0;
    
    /* Volatile read to prevent constant folding */
    volatile int v = x;
    int condition = v & 0xF;
    
    /* Another simple conditional jump pattern */
    if (condition != (y & 0xF)) {
        goto target_label_2;
    }
    
    /* Alternative path with different operations */
    t1 = t1 | 0xFF;
    t2 = t2 & 0x7F;
    return t1 - t2;

target_label_2:
    /* Different safe operation after label */
    t3 = t1 ^ t2;  /* Bitwise operation - cannot trap */
    
    /* Chain the result to prevent optimization */
    return use_result(t3) + 5;
}

/* Test function 3: Generic pattern with loop context */
int test_case_3(int n) {
    int i, sum = 0;
    int temp_a = n + 10;
    int temp_b = n * 2;
    int temp_c = 0;
    
    /* Create loop with conditional jump inside */
    for (i = 0; i < 3; i++) {
        /* Use loop variable in condition to make it non-trivial */
        if ((i + n) % 2 == 0) {
            goto target_label_3;
        }
        
        sum += i;
        continue;
        
    target_label_3:
        /* Safe instruction after label inside loop */
        temp_c = temp_a - temp_b;  /* Simple subtraction */
        sum += temp_c;
        
        /* Continue the loop */
        temp_a++;  /* Modify to prevent CSE */
    }
    
    return sum;
}

/* Test function 4: Nested condition pattern */
int test_case_4(int a, int b, int c) {
    int t1 = a * b;
    int t2 = c * 2;
    int t3 = 0;
    int result = 0;
    
    /* Outer condition */
    if (a > 0) {
        /* Inner condition for the target jump */
        if (b != c) {
            /* This should be the simple jump we want to target */
            goto target_label_4;
        }
        
        t1 = t1 + 10;
        result = t1;
    } else {
        t2 = t2 - 5;
        result = t2;
    }
    
    return result;

target_label_4:
    /* Safe operation with independent temporaries */
    t3 = t1 | t2;  /* Bitwise OR - safe operation */
    
    /* Use in a way that prevents optimization */
    return (t3 << 2) + 1;
}

/* Test function 5: Multiple basic blocks with phi-like pattern */
int test_case_5(int x) {
    int tmp1 = x + 1;
    int tmp2 = x * 3;
    int tmp3 = 0;
    int tmp4 = 0;
    
    /* Multiple paths leading to same label */
    if (x & 1) {
        tmp1 = tmp1 * 2;
        if (x > 100) {
            goto common_target;
        }
        tmp2 = tmp2 - 1;
    } else {
        tmp1 = tmp1 / 2;
        if (x < 50) {
            goto common_target;
        }
        tmp2 = tmp2 + 1;
    }
    
    return tmp1 + tmp2;

common_target:
    /* Instruction after label reachable from multiple paths */
    tmp3 = tmp1 & 0xFF;  /* Safe bitwise operation */
    tmp4 = tmp2 ^ 0xAA;
    
    return tmp3 + tmp4;
}

int main() {
    int results[5];
    int checksum = 0;
    
    /* Initialize with non-constant values */
    int a = 42;
    int b = 17;
    int c = 99;
    int n = 5;
    
    /* Execute all test cases */
    results[0] = test_case_1(a, b);
    results[1] = test_case_2(b, c);
    results[2] = test_case_3(n);
    results[3] = test_case_4(a, b, c);
    results[4] = test_case_5(n * 2);
    
    /* Compute checksum and print results */
    printf("Test Results:\n");
    for (int i = 0; i < 5; i++) {
        printf("  Case %d: %d\n", i + 1, results[i]);
        checksum ^= results[i];  /* Simple checksum */
        global_accumulator += results[i];  /* Use global to prevent optimization */
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return (checksum == 0) ? 1 : 0;
}
