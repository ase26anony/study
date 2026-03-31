/* Test program for delay slot filling in GCC reorg pass */
/* Targets MIPS/SPARC architectures with branch delay slots */

#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_acc = 0;

/* Optimization barrier functions */
int __attribute__((noinline)) use_value(int x) {
    return x + 1;
}

int __attribute__((noinline)) get_input(void) {
    return rand() & 0xFF;
}

/* Test function 1: Basic pattern with independent temporaries */
int __attribute__((target("arch=mips32"))) 
test_pattern1(int a, int b) {
    /* Independent temporaries for delay slot candidate */
    int temp1 = a + 1;
    int temp2 = b * 2;
    int temp3 = 0;
    
    /* Create non-trivial condition */
    if (a > b && (a - b) < 100) {
        /* Simple jump to label */
        goto target_label1;
    }
    
    /* Fall-through path */
    temp3 = temp1 - temp2;
    return temp3;
    
target_label1:
    /* Safe, non-jump instruction for delay slot filling */
    temp3 = temp1 & 0x7F;  /* Simple bitwise operation */
    return temp3 + temp2;
}

/* Test function 2: Multiple basic blocks with safe operation */
int __attribute__((target("arch=mips32")))
test_pattern2(int x) {
    int local_a = x;
    int local_b = x * 3;
    int local_c = 0;
    int local_d = 42;
    
    /* Some preceding code */
    for (int i = 0; i < 2; i++) {
        local_a += i;
    }
    
    /* Dynamic condition */
    if (local_a % 3 == 0 && x != 0) {
        goto target_label2;
    }
    
    /* Alternative path */
    local_c = local_b >> 2;
    return local_c;
    
target_label2:
    /* Safe arithmetic with independent variable */
    local_d = local_b + 5;  /* Uses variable not in condition */
    return local_d - local_a;
}

/* Test function 3: SPARC target variant */
int __attribute__((target("arch=sparc")))
test_pattern3(int val) {
    volatile int v = val;  /* Prevent constant propagation */
    int t1 = v + 10;
    int t2 = v * 3;
    int t3 = 0;
    int t4 = 100;
    
    /* Complex enough condition to avoid optimization */
    if ((v & 1) && (v < 50) && (t1 > 15)) {
        goto target_label3;
    }
    
    /* Other basic block */
    t3 = t2 | 0xF0;
    return t3;
    
target_label3:
    /* Safe logical operation - no traps, no memory access */
    t4 = t1 ^ t2;  /* XOR is always safe */
    return t4;
}

/* Test function 4: More complex CFG with helper calls */
int __attribute__((target("arch=mips32")))
test_pattern4(int base) {
    int x = use_value(base);
    int y = get_input();
    int z = 0;
    int w = 0;
    
    /* Initialize temporaries */
    int tmp1 = x * 2;
    int tmp2 = y + 7;
    int tmp3 = 0;
    
    /* Multiple conditions */
    if (x > 10) {
        if (y < 200) {
            if ((x + y) % 5 == 0) {
                goto target_label4;
            }
        }
    }
    
    /* Different computation path */
    z = tmp1 - tmp2;
    w = z * 3;
    return w;
    
target_label4:
    /* Safe shift operation */
    tmp3 = tmp2 << 1;  /* Left shift by 1 is always safe */
    return tmp3 + tmp1;
}

/* Test function 5: Minimal pattern for clarity */
int __attribute__((target("arch=mips32")))
test_pattern5(int p, int q) {
    /* Only variables used in condition */
    if (p != q && p > 0) {
        goto target_label5;
    }
    
    /* Independent variable for delay slot */
    int r = 0;
    return r;
    
target_label5:
    /* Single safe operation with constant */
    int r = p & 0xFF;  /* Mask operation */
    return r;
}

/* Main driver */
int main(void) {
    int results[5];
    int checksum = 0;
    
    /* Seed RNG for dynamic conditions */
    srand(42);
    
    /* Execute all test patterns */
    results[0] = test_pattern1(50, 30);
    results[1] = test_pattern2(25);
    results[3] = test_pattern4(15);
    results[4] = test_pattern5(100, 200);
    
    /* SPARC test - compile separately if needed */
    #ifdef __sparc__
    results[2] = test_pattern3(33);
    #else
    results[2] = 77;  /* Default value if not on SPARC */
    #endif
    
    /* Compute checksum and prevent dead code elimination */
    for (int i = 0; i < 5; i++) {
        checksum ^= results[i];
        global_acc += results[i];
    }
    
    printf("Test results: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", results[i]);
    }
    printf("\nChecksum: %d\n", checksum);
    printf("Global accumulator: %d\n", global_acc);
    
    return checksum == 0 ? 0 : 1;
}
