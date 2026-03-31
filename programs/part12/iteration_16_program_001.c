/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test function 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_copy_chain_compare_zero(int input) {
    int result;
    
    /* Create phi node with constants 0 and 1 */
    if (input > 0) {
        result = 1;  /* Branch 1: constant 1 */
    } else {
        result = 0;  /* Branch 2: constant 0 */
    }
    
    /* Create copy propagation chain */
    int a = result;  /* First copy */
    int b = a;       /* Second copy */
    int c = b;       /* Third copy */
    int d = c;       /* Fourth copy */
    
    /* Conditional branch comparing final copy to 0 */
    if (d == 0) {
        use(100);    /* Side effect to prevent optimization */
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test function 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain_compare_one(int input) {
    int val;
    
    /* Different condition to create phi */
    if (input % 2 == 0) {
        val = 0;
    } else {
        val = 1;
    }
    
    /* Longer copy chain */
    int t1 = val;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    int t6 = t5;
    
    /* Compare to 1 */
    if (t6 == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test function 3: Phi with three incoming edges */
__attribute__((noinline, noipa))
int test_phi_three_way(int input) {
    int x;
    
    /* Three-way branch creating phi with 3 operands */
    if (input < -10) {
        x = 0;
    } else if (input < 10) {
        x = 1;
    } else {
        x = 0;  /* Same constant as first branch */
    }
    
    /* Copy chain */
    int y = x;
    int z = y;
    
    /* Compare to 0 */
    if (z != 0) {  /* Using != instead of == */
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test function 4: Phi inside loop */
__attribute__((noinline, noipa))
int test_phi_in_loop(int iterations) {
    int sum = 0;
    volatile int vol = g_seed;  /* Volatile to prevent optimization */
    
    for (int i = 0; i < iterations; i++) {
        int cond_val;
        
        /* Phi created inside loop */
        if (vol > i) {
            cond_val = 1;
        } else {
            cond_val = 0;
        }
        
        /* Copy chain inside loop */
        int tmp1 = cond_val;
        int tmp2 = tmp1;
        
        /* Conditional branch inside loop */
        if (tmp2 == 1) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        /* Modify volatile to change branch behavior */
        vol = vol / 2;
    }
    
    use(sum);
    return sum;
}

/* Test function 5: Nested phi-copy-conditional */
__attribute__((noinline, noipa))
int test_nested_phi(int input1, int input2) {
    int inner_val;
    
    /* Outer condition */
    if (input1 > 0) {
        /* Inner condition creating nested phi */
        int temp;
        if (input2 > 0) {
            temp = 1;
        } else {
            temp = 0;
        }
        
        /* Copy chain */
        int copy1 = temp;
        int copy2 = copy1;
        
        /* Conditional using copy */
        if (copy2 == 0) {
            inner_val = 10;
        } else {
            inner_val = 20;
        }
    } else {
        inner_val = 30;
    }
    
    /* Final copy and conditional */
    int final_copy = inner_val;
    if (final_copy > 15) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test function 6: Phi with pointer casts (creates different SSA types) */
__attribute__((noinline, noipa))
int test_phi_with_casts(int input) {
    int bool_val;
    
    if (input > 100) {
        bool_val = 1;
    } else {
        bool_val = 0;
    }
    
    /* Cast to different type and back */
    unsigned int u = (unsigned int)bool_val;
    int cast_back = (int)u;
    
    /* Copy chain after cast */
    int final = cast_back;
    
    /* Compare to 1 */
    if (final == 1) {
        use(900);
        return 9;
    } else {
        use(1000);
        return 10;
    }
}

/* Test function 7: Multiple independent phi-copy chains */
__attribute__((noinline, noipa))
int test_multiple_chains(int input) {
    int val1, val2;
    
    /* First phi chain */
    if (input % 3 == 0) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    /* Second phi chain */
    if (input % 5 == 0) {
        val2 = 0;
    } else {
        val2 = 1;
    }
    
    /* Copy chains for both */
    int c1a = val1;
    int c1b = c1a;
    
    int c2a = val2;
    int c2b = c2a;
    int c2c = c2b;
    
    /* Conditionals on both chains */
    int result = 0;
    if (c1b == 0) {
        result += 1;
    }
    if (c2c == 1) {
        result += 2;
    }
    
    use(result);
    return result + 10;
}

/* Dummy implementation of external functions to avoid linker errors */
void use(int x) {
    /* Empty - just to prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(void* p) {
    static volatile void* sink;
    sink = p;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile seed */
    g_seed = 42;
    
    /* Call test functions with different inputs to exercise all paths */
    checksum += test_phi_copy_chain_compare_zero(g_seed);
    checksum += test_phi_copy_chain_compare_zero(-g_seed);
    
    checksum += test_phi_long_chain_compare_one(g_seed);
    checksum += test_phi_long_chain_compare_one(g_seed + 1);
    
    checksum += test_phi_three_way(-20);
    checksum += test_phi_three_way(0);
    checksum += test_phi_three_way(20);
    
    checksum += test_phi_in_loop(5);
    
    checksum += test_nested_phi(1, 1);
    checksum += test_nested_phi(1, -1);
    checksum += test_nested_phi(-1, 1);
    
    checksum += test_phi_with_casts(200);
    checksum += test_phi_with_casts(50);
    
    checksum += test_multiple_chains(15);
    checksum += test_multiple_chains(10);
    checksum += test_multiple_chains(7);
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
