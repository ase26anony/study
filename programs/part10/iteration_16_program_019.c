#include <stdio.h>
#include <stdlib.h>

/* Helper to create SSA copy chains */
static inline int pass_through(int v) { return v; }
static inline int copy_value(int v) { int t = v; return t; }

/* Pattern 1: Loop with PHI-derived exit condition */
int pattern1_loop_phi_exit(int iterations) {
    int flag = 0;  /* Initial value for PHI */
    int count = 0;
    
    while (!flag) {  /* Conditional on PHI-derived value (flag != 0) */
        count++;
        if (count >= iterations) {
            flag = 1;  /* One assignment to PHI operand */
        } else {
            flag = 0;  /* Other assignment to PHI operand */
        }
    }
    return count;
}

/* Pattern 2: If-else chain setting boolean, then conditional */
int pattern2_if_else_phi(int a, int b) {
    int result;
    
    /* PHI at implicit merge point */
    if (a > b) {
        result = 1;  /* First PHI operand */
    } else {
        result = 0;  /* Second PHI operand */
    }
    
    /* Create SSA copy chain */
    int tmp1 = result;
    int tmp2 = pass_through(tmp1);
    int tmp3 = copy_value(tmp2);
    
    /* Conditional on copy chain end (compares with 0) */
    if (tmp3 == 0) {
        return a + b;
    } else {
        return a * b;
    }
}

/* Pattern 3: Nested control with multiple PHIs */
int pattern3_nested_phi(int x) {
    int flag1, flag2;
    
    /* Outer if creates first PHI */
    if (x > 0) {
        flag1 = 1;
    } else {
        flag1 = 0;
    }
    
    /* Inner loop with PHI-derived condition */
    int sum = 0;
    int i = 0;
    while (i < 10) {
        /* Another PHI for flag2 inside loop */
        if (i % 2 == 0) {
            flag2 = 1;
        } else {
            flag2 = 0;
        }
        
        /* Conditional on flag2 through copy chain */
        int f = flag2;
        int g = pass_through(f);
        if (g != 1) {  /* Compare with 1 */
            sum += i;
        }
        i++;
    }
    
    /* Final conditional on flag1 */
    int check = flag1;
    if (check) {  /* Implicit check against 0 */
        return sum * 2;
    }
    return sum;
}

/* Pattern 4: Switch-case setting 0/1 value */
int pattern4_switch_phi(int val) {
    int code = 0;  /* Default PHI operand */
    
    switch (val % 4) {
        case 0: code = 1; break;
        case 1: code = 0; break;
        case 2: code = 1; break;
        case 3: code = 0; break;
    }
    
    /* Multiple copy steps */
    int a = code;
    int b = a;
    int c = pass_through(b);
    int d = copy_value(c);
    
    /* Conditional comparing with 0 */
    if (d == 0) {
        return val * 2;
    } else {
        return val / 2;
    }
}

/* Pattern 5: Ternary operator creating PHI */
int pattern5_ternary_phi(int x, int y) {
    /* Ternary creates PHI node */
    int is_greater = (x > y) ? 1 : 0;
    
    /* Pass through inline function */
    int checked = pass_through(is_greater);
    
    /* Conditional with explicit 1 comparison */
    if (checked == 1) {
        return x - y;
    }
    return y - x;
}

/* Pattern 6: Complex chain with multiple PHIs */
int pattern6_complex_chains(int n) {
    int a = 0, b = 0;
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        /* First PHI for 'a' */
        if (i % 3 == 0) {
            a = 1;
        } else {
            a = 0;
        }
        
        /* Second PHI for 'b' */
        if (i % 5 == 0) {
            b = 1;
        } else {
            b = 0;
        }
        
        /* Use both PHI values through copies */
        int a_copy = a;
        int b_copy = b;
        
        /* Nested conditionals */
        if (a_copy == 0) {
            total += i;
        }
        
        if (b_copy != 1) {
            total -= i;
        }
    }
    
    /* Final conditional on PHI-derived value from loop */
    int final_check = (total > 0) ? 1 : 0;
    if (final_check) {  /* Compare with 0 */
        return total * 2;
    }
    return total;
}

/* Main function with its own pattern */
int main() {
    int total = 0;
    
    /* Array to vary inputs */
    int test_cases[] = {5, 10, 15, 20, 25};
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Main loop with PHI-derived condition */
    int done = 0;  /* PHI initial value */
    int idx = 0;
    
    while (!done) {  /* Conditional on PHI value (done != 0) */
        int val = test_cases[idx];
        
        /* Call each pattern function */
        total += pattern1_loop_phi_exit(val % 5 + 1);
        total += pattern2_if_else_phi(val, val * 2);
        total += pattern3_nested_phi(val);
        total += pattern4_switch_phi(val);
        total += pattern5_ternary_phi(val, val / 2);
        total += pattern6_complex_chains(val % 10 + 1);
        
        idx++;
        /* Update PHI operand */
        if (idx >= num_cases) {
            done = 1;  /* Set to 1 for exit */
        } else {
            done = 0;  /* Set to 0 to continue */
        }
    }
    
    /* Final conditional through copy chain */
    int result_copy = total;
    int final_result = pass_through(result_copy);
    
    if (final_result > 1000) {
        printf("High result: %d\n", final_result);
    } else {
        printf("Low result: %d\n", final_result);
    }
    
    return 0;
}
