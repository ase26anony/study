/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex conditional logic */
int process_value(int x) {
    volatile int result = 0;
    
    if (x > 100) {
        result = x * 2;
    } else if (x > 50) {
        result = x + 100;
    } else if (x > 0) {
        result = x - 10;
    } else {
        result = 0;
    }
    
    /* Nested condition */
    if (result > 200) {
        result /= 2;
    }
    
    return result;
}

/* Function 2: Loop with break/continue */
int calculate_sum(int limit) {
    volatile int sum = 0;
    volatile int i;
    
    for (i = 0; i < limit * 2; i++) {
        if (i >= limit) {
            break;
        }
        
        if (i % 3 == 0) {
            continue;
        }
        
        sum += i;
        
        /* Inner conditional */
        if (sum > 50) {
            sum -= 10;
        }
    }
    
    return sum;
}

/* Function 3: Switch statement */
int evaluate_grade(int score) {
    volatile int grade = 0;
    
    switch (score / 10) {
        case 10:
        case 9:
            grade = 4;  /* A */
            break;
        case 8:
            grade = 3;  /* B */
            break;
        case 7:
            grade = 2;  /* C */
            break;
        case 6:
            grade = 1;  /* D */
            break;
        default:
            grade = 0;  /* F */
            break;
    }
    
    return grade;
}

/* Function 4: Recursive-like with early returns */
int fibonacci_like(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    
    volatile int a = 0, b = 1, c;
    volatile int i;
    
    for (i = 2; i <= n; i++) {
        c = a + b;
        if (c > 1000) {
            return 999;  /* Early return */
        }
        a = b;
        b = c;
    }
    
    return b;
}

/* Function 5: Multiple exit points */
int complex_logic(int a, int b) {
    volatile int x = a;
    volatile int y = b;
    
    while (x < 100) {
        if (y > 50) {
            return x * y;  /* Exit 1 */
        }
        
        x += y;
        y -= 1;
        
        if (x > 75 && y < 25) {
            break;  /* Exit loop */
        }
    }
    
    if (x > 50) {
        return x + 100;  /* Exit 2 */
    } else {
        return x - 100;  /* Exit 3 */
    }
}

/* Main function with different execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 0;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    /* Execute different code paths based on mode */
    switch (mode % 4) {
        case 0:
            printf("Result1: %d\n", process_value(120));
            printf("Sum1: %d\n", calculate_sum(15));
            break;
        case 1:
            printf("Grade1: %d\n", evaluate_grade(85));
            printf("Fib1: %d\n", fibonacci_like(10));
            break;
        case 2:
            printf("Logic1: %d\n", complex_logic(10, 20));
            printf("Result2: %d\n", process_value(75));
            break;
        case 3:
            printf("Sum2: %d\n", calculate_sum(25));
            printf("Grade2: %d\n", evaluate_grade(72));
            printf("Fib2: %d\n", fibonacci_like(15));
            break;
    }
    
    /* Always execute some common code */
    volatile int common = process_value(30);
    printf("Common: %d\n", common);
    
    return 0;
}
