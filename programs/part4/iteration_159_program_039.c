#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
int __attribute__((noinline)) get_input(int x) {
    return x ^ 0x55AA55AA;
}

int __attribute__((noinline)) use_result(int x) {
    return x + 1;
}

/* Test function 1: MIPS target with simple jump pattern */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_case_1(int a, int b) {
    /* Local temporaries - independent of jump condition */
    int temp1 = a + b;
    int temp2 = a - b;
    int temp3 = a * 2;
    int result = 0;
    
    /* Create a non-trivial condition for the jump */
    if (get_input(a) > get_input(b)) {
        /* Simple goto to label - should generate simplejump_p */
        goto target_label_1;
    }
    
    /* Some other code to create basic blocks */
    temp1 = temp1 * 3;
    result = temp1 + temp2;
    
    /* This should not be reached if jump is taken */
    return result;

target_label_1:
    /* Safe, non-jump instruction immediately after label */
    /* Uses independent temporary variables */
    temp3 = temp3 & 0xFF;  /* Simple bitwise operation - won't trap */
    
    /* Use the result to prevent dead code elimination */
    result = use_result(temp3);
    return result;
}

/* Test function 2: SPARC target with different pattern */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y) {
    /* More independent temporaries */
    int t1 = x | y;
    int t2 = x ^ y;
    int t3 = x << 2;
    int t4 = y >> 1;
    
    volatile int v = x;  /* Prevent condition optimization */
    
    /* Another simple jump pattern */
    if (v != 0 && (x % 3) == 0) {
        goto target_label_2;
    }
    
    /* Alternative path */
    t1 = t1 + t2;
    return t1;

target_label_2:
    /* Safe arithmetic operation after label */
    t4 = t3 + t4;  /* Simple addition - safe */
    
    /* Use in computation */
    int r = (t4 * 2) + 1;
    return r;
}

/* Test function 3: Generic pattern with multiple basic blocks */
int test_case_3(int n) {
    int a = n;
    int b = n + 1;
    int c = n * 2;
    int d = n / 3;  /* Division is before jump, so safe */
    
    /* Create more complex control flow */
    for (int i = 0; i < 3; i++) {
        a += i;
        if (i == 1 && get_input(n) > 100) {
            goto target_label_3;
        }
        b -= i;
    }
    
    c = a + b;
    return c;

target_label_3:
    /* Safe logical operation */
    d = d | 0x01;  /* Bitwise OR - cannot trap */
    
    return d + c;
}

/* Test function 4: Pattern with multiple temporaries */
int test_case_4(int p, int q) {
    /* Many independent variables */
    int v1 = p;
    int v2 = q;
    int v3 = p + q;
    int v4 = p - q;
    int v5 = p * q;
    int v6 = p ^ q;
    
    /* Simple condition based on input */
    if ((p & 1) && (q & 2)) {
        goto target_label_4;
    }
    
    v1 = v2 + v3;
    v4 = v5 - v6;
    return v1 + v4;

target_label_4:
    /* Multiple safe operations - compiler might schedule one after label */
    v3 = v3 << 1;    /* Shift operation - safe */
    v6 = v6 & 0x7F;  /* Mask operation - safe */
    
    /* Use results */
    return v3 + v6;
}

/* Test function 5: Minimal pattern */
int test_case_5(int val) {
    int tmp1 = val;
    int tmp2 = val + 5;
    
    /* Very simple jump */
    if (get_input(val) != 0) {
        goto target_label_5;
    }
    
    tmp2 = tmp2 * 2;
    return tmp2;

target_label_5:
    /* Minimal safe instruction */
    tmp1 = tmp1 + 1;  /* Increment - safe */
    
    return tmp1;
}

int main() {
    int checksum = 0;
    
    /* Test with various inputs to explore different paths */
    for (int i = 0; i < 10; i++) {
        checksum ^= test_case_1(i, i + 1);
        checksum ^= test_case_2(i, i + 2);
        checksum ^= test_case_3(i);
        checksum ^= test_case_4(i, i + 3);
        checksum ^= test_case_5(i);
        
        /* Update global to prevent optimization */
        global_accumulator += checksum;
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return 0;
}
