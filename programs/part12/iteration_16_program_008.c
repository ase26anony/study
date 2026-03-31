/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test function 1: Simple phi with 0/1 constants, short copy chain */
__attribute__((noinline, noipa))
int test_phi_simple_copy(int volatile_input) {
    int result;
    
    /* Create phi node with constants 0 and 1 */
    if (volatile_input > 0) {
        result = 1;  /* Constant 1 */
    } else {
        result = 0;  /* Constant 0 */
    }
    
    /* Copy propagation chain */
    int a = result;  /* First copy */
    int b = a;       /* Second copy */
    
    /* Conditional branch comparing to constant 0 */
    if (b == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test function 2: Longer copy chain, compare to constant 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain(int volatile_input) {
    int val;
    
    /* Phi with constants 0 and 1 */
    if (volatile_input % 2 == 0) {
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
    
    /* Compare to constant 1 */
    if (t5 == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test function 3: Phi with multiple predecessors (if-else if-else) */
__attribute__((noinline, noipa))
int test_phi_three_way(int volatile_input) {
    int flag;
    
    /* Three-way branch creating phi with 3 incoming edges */
    if (volatile_input < -10) {
        flag = 0;
    } else if (volatile_input > 10) {
        flag = 1;
    } else {
        flag = 0;  /* Still 0, but different edge */
    }
    
    /* Copy chain */
    int x = flag;
    int y = x;
    
    /* Conditional with != comparison */
    if (y != 0) {
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test function 4: Phi inside a loop */
__attribute__((noinline, noipa))
int test_phi_in_loop(int volatile_input) {
    int sum = 0;
    int iterations = (volatile_input & 3) + 2;  /* 2-5 iterations */
    
    for (int i = 0; i < iterations; i++) {
        int cond;
        
        /* Phi created inside loop */
        if ((i + volatile_input) % 2 == 0) {
            cond = 1;
        } else {
            cond = 0;
        }
        
        /* Copy chain */
        int tmp1 = cond;
        int tmp2 = tmp1;
        
        /* Conditional branch inside loop */
        if (tmp2 == 1) {
            sum += i * 10;
        } else {
            sum += i * 20;
        }
    }
    
    use(sum);
    return sum;
}

/* Test function 5: Nested phi-copy-conditional pattern */
__attribute__((noinline, noipa))
int test_nested_phi(int volatile_input) {
    int outer_val;
    
    /* Outer phi */
    if (volatile_input > 100) {
        outer_val = 1;
    } else {
        outer_val = 0;
    }
    
    /* Copy chain */
    int o1 = outer_val;
    int o2 = o1;
    
    /* Outer conditional */
    if (o2 == 1) {
        /* Inner phi pattern */
        int inner_val;
        if (volatile_input < 50) {
            inner_val = 0;
        } else {
            inner_val = 1;
        }
        
        int i1 = inner_val;
        int i2 = i1;
        
        if (i2 == 0) {
            use(700);
            return 7;
        } else {
            use(800);
            return 8;
        }
    } else {
        use(900);
        return 9;
    }
}

/* Test function 6: Phi with boolean operations in branches */
__attribute__((noinline, noipa))
int test_phi_with_bool_ops(int volatile_input) {
    int bool_result;
    
    /* Create boolean results that become constants 0/1 */
    if ((volatile_input & 1) && (volatile_input > 0)) {
        bool_result = 1;  /* true -> 1 */
    } else {
        bool_result = 0;  /* false -> 0 */
    }
    
    /* Minimal copy chain */
    int b = bool_result;
    
    /* Compare to 0 */
    if (b == 0) {
        use(1000);
        return 10;
    }
    
    /* Another compare to 1 (different block) */
    if (b == 1) {
        use(1100);
        return 11;
    }
    
    return 12;
}

/* Test function 7: Switch-based phi pattern */
__attribute__((noinline, noipa))
int test_phi_from_switch(int volatile_input) {
    int switch_val;
    
    switch (volatile_input % 4) {
        case 0:
            switch_val = 0;
            break;
        case 1:
            switch_val = 1;
            break;
        case 2:
            switch_val = 0;
            break;
        default:
            switch_val = 1;
            break;
    }
    
    /* Copy chain */
    int s1 = switch_val;
    int s2 = s1;
    int s3 = s2;
    
    /* Conditional */
    if (s3 != 1) {
        use(1200);
        return 13;
    } else {
        use(1300);
        return 14;
    }
}

/* Main function that exercises all patterns */
int main() {
    int checksum = 0;
    
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int seed = g_seed;
    if (seed == 0) {
        seed = 12345;  /* Default seed if not set externally */
    }
    
    /* Call each test function multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        int input = seed + i * 17;
        
        checksum += test_phi_simple_copy(input);
        checksum += test_phi_long_chain(input);
        checksum += test_phi_three_way(input);
        checksum += test_phi_in_loop(input);
        checksum += test_nested_phi(input);
        checksum += test_phi_with_bool_ops(input);
        checksum += test_phi_from_switch(input);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

/* Dummy implementation of external functions to avoid linker errors */
void use(int x) {
    /* Empty - just to prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(void* p) {
    /* Empty - just to prevent optimization */
    static volatile void* sink;
    sink = p;
}
