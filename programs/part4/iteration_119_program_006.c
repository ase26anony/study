#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper function to prevent optimization */
static void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Function with complex control flow to generate PHI nodes */
static int process_value(volatile int input, int mode) {
    int result = 0;
    
    /* Chain of assignments creating SSA_NAME chain */
    int a = input;
    int b = a;
    int c = b;
    int d = c;
    
    /* Conditional with constant RHS (0) */
    if (d == 0) {
        result = 1;
    } else {
        result = 2;
    }
    
    /* Another conditional with constant RHS (1) */
    if (result != 1) {
        result += 10;
    }
    
    return result;
}

/* Function with loop creating PHI node scenario */
static int loop_with_phi(volatile int seed, int iterations) {
    int phi_var = 0;  /* Will become PHI node at loop header */
    int temp = seed;
    
    for (int i = 0; i < iterations; i++) {
        /* Chain of assignments */
        int x = temp;
        int y = x;
        int z = y;
        
        /* Conditional with constant RHS (1) */
        if (z == 1) {
            phi_var = 100;
        } else {
            phi_var = 200;
        }
        
        /* Modify temp to create varying conditions */
        temp = (temp * 1103515245 + 12345) & 0x7fffffff;
        
        /* Another conditional with constant RHS (0) */
        if ((temp & 1) != 0) {
            phi_var += 1;
        }
    }
    
    /* Use phi_var after loop - creates merge point requiring PHI */
    int final = phi_var;
    
    /* Conditional using the PHI-defined variable with constant RHS */
    if (final == 100) {
        return 1;
    } else if (final == 200) {
        return 2;
    } else {
        return final;
    }
}

/* Complex function with multiple basic blocks and edges */
static int complex_branching(volatile int a, volatile int b, volatile int c, int n) {
    int sum = 0;
    int phi_val = 0;
    
    for (int i = 0; i < n; i++) {
        /* Assignment chain */
        int t1 = a;
        int t2 = t1;
        int t3 = t2;
        
        /* Multiple conditionals with constant RHS values */
        if (t3 == 0) {
            phi_val = 10;
        } else if (t3 == 1) {
            phi_val = 20;
        } else {
            phi_val = 30;
        }
        
        /* More assignment chains */
        int u1 = b;
        int u2 = u1;
        
        if (u2 != 1) {
            phi_val += 5;
        }
        
        int v1 = c;
        int v2 = v1;
        int v3 = v2;
        
        if (v3 == 0) {
            sum += phi_val;
        } else if (v3 == 1) {
            sum -= phi_val;
        }
        
        /* Modify volatile inputs to prevent optimization */
        a = (a + 1) & 1;
        b = (b * 3 + 1) & 1;
        c = (c + i) & 1;
    }
    
    /* Final conditional using accumulated value */
    if (sum == 0) {
        return 0;
    } else if (sum == 1) {
        return 1;
    }
    
    return sum;
}

/* Function with nested loops for more complex CFG */
static void nested_loop_pattern(volatile int limit) {
    int outer_phi = 0;
    
    for (int i = 0; i < limit; i++) {
        int inner_phi = 0;
        
        for (int j = 0; j < 10; j++) {
            /* Assignment chain inside inner loop */
            int chain1 = i;
            int chain2 = chain1;
            int chain3 = chain2;
            
            /* Conditional with constant RHS */
            if (chain3 == 0) {
                inner_phi += 1;
            } else if (chain3 == 1) {
                inner_phi += 2;
            }
            
            /* Another chain */
            int chain4 = j;
            int chain5 = chain4;
            
            if (chain5 != 1) {
                inner_phi += 3;
            }
        }
        
        /* Merge inner loop result */
        if (inner_phi > 20) {
            outer_phi += 1;
        } else if (inner_phi == 20) {
            outer_phi += 2;
        }
    }
    
    /* Use the result */
    use(outer_phi);
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000000;
        }
    }
    
    /* Volatile variables to prevent constant propagation */
    volatile int v1 = 0;
    volatile int v2 = 1;
    volatile int v3 = 0;
    volatile int v4 = 1;
    
    printf("Starting profile-heavy computation with %d iterations...\n", iterations);
    
    /* Hot loop 1 - creates profile data for basic blocks */
    int result1 = 0;
    for (int i = 0; i < iterations; i++) {
        /* Vary the inputs */
        v1 = (v1 + i) & 1;
        v2 = (v2 * 3 + 1) & 1;
        
        /* Process with assignment chains and conditionals */
        int r = process_value(v1, i & 1);
        result1 += r;
        
        /* Create PHI node scenario */
        int phi_test = 0;
        if (v2 == 0) {
            phi_test = 100;
        } else {
            phi_test = 200;
        }
        
        /* Use phi_test in conditional with constant RHS */
        if (phi_test == 100) {
            result1 += 1;
        } else if (phi_test == 200) {
            result1 += 2;
        }
    }
    
    /* Hot loop 2 - different pattern */
    int result2 = loop_with_phi(v3, iterations / 10);
    
    /* Hot loop 3 - complex branching */
    int result3 = complex_branching(v1, v2, v3, iterations / 5);
    
    /* Nested loop pattern */
    nested_loop_pattern(iterations / 100);
    
    /* Final checksum to prevent dead code elimination */
    int checksum = result1 + result2 + result3;
    checksum = (checksum * 1103515245 + 12345) & 0x7fffffff;
    
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
