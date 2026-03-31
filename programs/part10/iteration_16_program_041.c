#include <stdio.h>
#include <stdlib.h>

/* Helper function to create SSA copy chains */
static inline int pass_through(int x) { return x; }
static inline int copy_value(int x) { int y = x; return y; }

/* Pattern 1: Loop with PHI-derived exit condition */
int pattern1_loop_phi_exit(int iterations) {
    int flag = 0;  /* Initial value */
    int count = 0;
    
    /* Loop header creates PHI for 'flag' */
    while (!flag) {  /* if (flag == 0) - compares against 0 */
        count++;
        /* Set flag based on condition - creates PHI at loop header */
        if (count >= iterations) {
            flag = 1;  /* Will be PHI operand */
        } else {
            flag = 0;  /* Will be PHI operand */
        }
    }
    return count;
}

/* Pattern 2: If-else chain with PHI, then conditional on copy chain */
int pattern2_phi_copy_chain(int a, int b) {
    int result;
    
    /* Creates PHI node at merge point */
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    /* Create copy chain */
    int x = result;      /* First copy */
    int y = pass_through(x);  /* Function creates SSA copy */
    int z = copy_value(y);    /* Another copy */
    
    /* Conditional on copy chain result */
    if (z == 1) {  /* Compares against 1 */
        return a + b;
    } else {
        return a - b;
    }
}

/* Pattern 3: Nested control with multiple PHIs */
int pattern3_nested_phi(int n) {
    int total = 0;
    int i = 0;
    
    while (i < n) {
        int inner_flag;
        
        /* Inner if-else creates PHI */
        if (i % 3 == 0) {
            inner_flag = 1;
        } else {
            inner_flag = 0;
        }
        
        /* Copy through temporary */
        int temp = inner_flag;
        
        /* Conditional on copied PHI value */
        if (temp) {  /* Implicit: if (temp != 0) */
            total += i * 2;
        } else {
            total += i;
        }
        
        i++;
    }
    
    /* Final conditional with PHI from loop */
    int final_check = (total > 100) ? 1 : 0;
    if (final_check == 1) {
        return total * 2;
    }
    return total;
}

/* Pattern 4: Switch-case setting 0/1 with later conditional */
int pattern4_switch_phi(int value) {
    int code;
    
    switch (value % 4) {
        case 0: code = 1; break;
        case 1: code = 0; break;
        case 2: code = 1; break;
        default: code = 0; break;
    }
    
    /* Multiple copy levels */
    int a = code;
    int b = a;
    int c = pass_through(b);
    
    /* Conditional with copy chain */
    if (c == 0) {
        return value * 2;
    } else {
        return value / 2;
    }
}

/* Pattern 5: Complex PHI network in loop */
int pattern5_complex_phi(int limit) {
    int state = 0;
    int sum = 0;
    int i = 0;
    
    /* Outer loop with PHI for state */
    while (i < limit) {
        int new_state;
        
        /* Inner condition creates PHI */
        if (sum % 2 == 0) {
            new_state = 1;
        } else {
            new_state = 0;
        }
        
        /* Copy through chain */
        int s1 = new_state;
        int s2 = s1;
        
        /* Conditional on copied value */
        if (s2 == 1) {
            sum += i * 3;
        } else {
            sum += i;
        }
        
        /* Update state PHI */
        state = new_state;
        i++;
        
        /* Another conditional using state */
        int check = state;
        if (check) {  /* if (check != 0) */
            sum += 10;
        }
    }
    
    return sum;
}

/* Pattern 6: Boolean flag from function return */
static int compute_flag(int x) {
    return (x % 5 == 0) ? 1 : 0;
}

int pattern6_function_phi(int x) {
    int flag = compute_flag(x);
    
    /* Multiple assignments creating copy chain */
    int a = flag;
    int b = pass_through(a);
    int c = b;
    
    /* Target conditional */
    if (c == 0) {
        return x + 100;
    } else {
        return x - 50;
    }
}

/* Main function with rich control flow */
int main() {
    int total = 0;
    int i;
    
    /* Initialize array for varied control flow */
    int inputs[] = {5, 10, 15, 20, 25, 30, 35, 40};
    int n = sizeof(inputs) / sizeof(inputs[0]);
    
    /* Pattern 1 */
    total += pattern1_loop_phi_exit(8);
    
    /* Pattern 2 with different paths */
    total += pattern2_phi_copy_chain(10, 5);   /* a > b */
    total += pattern2_phi_copy_chain(5, 10);   /* a <= b */
    
    /* Pattern 3 with nested loops */
    for (i = 0; i < 3; i++) {
        total += pattern3_nested_phi(inputs[i]);
    }
    
    /* Pattern 4 with switch */
    for (i = 0; i < n; i++) {
        total += pattern4_switch_phi(inputs[i]);
    }
    
    /* Pattern 5 - complex */
    total += pattern5_complex_phi(12);
    
    /* Pattern 6 */
    for (i = 0; i < n; i++) {
        total += pattern6_function_phi(inputs[i]);
    }
    
    /* Main loop with flag-based exit (mimics target pattern) */
    int done = 0;
    int counter = 0;
    
    while (!done) {  /* if (done == 0) */
        counter++;
        total += counter;
        
        /* Set flag through copy chain */
        int temp_flag = (counter >= 5) ? 1 : 0;
        int flag_copy = temp_flag;
        done = flag_copy;  /* Creates PHI at loop header */
    }
    
    printf("Final result: %d\n", total);
    
    /* Additional verification */
    int verify = (total > 1000) ? 1 : 0;
    if (verify == 1) {
        printf("Large result detected\n");
    } else {
        printf("Small result\n");
    }
    
    return 0;
}
