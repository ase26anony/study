/* Test program to trigger delay slot filling optimization in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
__attribute__((optimize("O2")))
int test1(int a, int b) {
    int x, y, z;
    volatile int result = 0;
    
    x = a + 10;
    y = b * 2;
    
    if (x > y) {
        goto target_label1;
    }
    
    z = x - y;
    return z;
    
target_label1:
    /* Candidate for delay slot: simple arithmetic */
    z = x + y;  /* next_trial: simple arithmetic operation */
    result = z * 2;
    return result;
}

/* Test 2: Register move/swap pattern */
__attribute__((optimize("O2")))
int test2(int p, int q) {
    int temp1, temp2, temp3;
    volatile int out = 0;
    
    temp1 = p & 0xFF;
    temp2 = q | 0x55;
    
    if (temp1 != temp2) {
        goto compute_label;
    }
    
    return temp1;
    
compute_label:
    /* Candidate: bitwise operation */
    temp3 = temp1 ^ temp2;  /* next_trial: bitwise XOR */
    out = temp3 + 1;
    return out;
}

/* Test 3: Stack-based memory operation */
__attribute__((optimize("O2")))
int test3(int val) {
    int local_array[4] = {0};
    int i, sum = 0;
    volatile int ret = 0;
    
    local_array[0] = val;
    local_array[1] = val + 1;
    
    i = 0;
loop_start:
    if (i >= 2) {
        goto finish;
    }
    
    if (local_array[i] > 100) {
        goto process_label;
    }
    
    i++;
    goto loop_start;
    
process_label:
    /* Candidate: safe stack load */
    sum = local_array[i];  /* next_trial: stack load operation */
    ret = sum * 3;
    return ret;
    
finish:
    return 0;
}

/* Test 4: Comparison operation */
__attribute__((optimize("O2")))
int test4(int a, int b, int c) {
    int cmp1, cmp2, diff;
    volatile int res = 0;
    
    cmp1 = a * b;
    cmp2 = c << 2;
    
    if (cmp1 == cmp2) {
        goto compare_label;
    }
    
    diff = cmp1 - cmp2;
    return diff;
    
compare_label:
    /* Candidate: comparison operation */
    diff = (cmp1 < cmp2) ? -1 : 1;  /* next_trial: comparison */
    res = diff + 10;
    return res;
}

/* Test 5: Multiple basic blocks with simple jumps */
__attribute__((optimize("O2")))
int test5(int x) {
    int a, b, c;
    volatile int output = 0;
    
    a = x + 5;
    b = x * 3;
    
    if (a > 100) {
        goto block_a;
    }
    
    if (b < 50) {
        goto block_b;
    }
    
    c = a + b;
    return c;
    
block_a:
    /* First candidate */
    c = a - b;  /* next_trial: subtraction */
    output = c;
    goto final;
    
block_b:
    /* Second candidate */
    c = b << 1;  /* next_trial: shift operation */
    output = c + 1;
    /* Fall through to final */
    
final:
    return output;
}

/* Test 6: Nested control flow with safe operation */
__attribute__((optimize("O2")))
int test6(int base) {
    int counter = 0;
    int accum = 0;
    int temp;
    
    while (counter < 10) {
        temp = base + counter;
        
        if (temp % 3 == 0) {
            goto update_label;
        }
        
        accum += temp;
        counter++;
        continue;
        
    update_label:
        /* Candidate: simple increment */
        accum += 2;  /* next_trial: addition */
        counter++;
    }
    
    return accum;
}

/* Test 7: Switch-like pattern with goto */
__attribute__((optimize("O2")))
int test7(int selector) {
    int op1 = selector * 2;
    int op2 = selector + 10;
    int result = 0;
    
    switch (selector & 3) {
        case 0:
            goto case0_label;
        case 1:
            goto case1_label;
        case 2:
            goto case2_label;
        default:
            return op1;
    }
    
case0_label:
    result = op1 & 0xF;  /* next_trial: bitwise AND */
    break;
    
case1_label:
    result = op2 | 0x1;  /* next_trial: bitwise OR */
    break;
    
case2_label:
    result = op1 ^ op2;  /* next_trial: bitwise XOR */
    break;
    
    return result + 1;
}

/* Test 8: Function with multiple labels and jumps */
__attribute__((optimize("O3")))  /* Higher optimization */
int test8(int n) {
    int i, j, k;
    volatile int total = 0;
    
    for (i = 0; i < n; i++) {
        j = i * i;
        
        if (j > n) {
            goto large_value;
        }
        
        if (j % 2 == 0) {
            goto even_value;
        }
        
        k = j + 1;
        total += k;
        continue;
        
    large_value:
        k = j >> 1;  /* next_trial: shift right */
        total += k;
        continue;
        
    even_value:
        k = j & ~1;  /* next_trial: bitwise clear */
        total += k;
        continue;
    }
    
    return total;
}

/* Main function to execute all tests */
int main() {
    int sum = 0;
    
    /* Execute all test functions with various inputs */
    sum += test1(20, 5);
    sum += test1(5, 20);
    
    sum += test2(100, 200);
    sum += test2(255, 255);
    
    sum += test3(150);
    sum += test3(50);
    
    sum += test4(10, 5, 12);
    sum += test4(5, 5, 5);
    
    sum += test5(30);
    sum += test5(120);
    
    sum += test6(7);
    sum += test6(1);
    
    sum += test7(0);
    sum += test7(1);
    sum += test7(2);
    sum += test7(3);
    
    sum += test8(5);
    sum += test8(10);
    
    printf("Total sum: %d\n", sum);
    
    /* Use the result to prevent dead code elimination */
    if (sum > 1000) {
        printf("Delay slot patterns executed successfully\n");
    }
    
    return 0;
}
