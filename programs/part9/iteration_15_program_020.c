/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int func1(volatile int x, volatile int y) {
    int result = 0;
    
    /* Loop with conditional inside */
    for (volatile int i = 0; i < x; ++i) {
        if (i % 2 == 0) {
            result += i * 2;
        } else {
            result += i;
        }
        
        /* Nested loop */
        for (volatile int j = 0; j < y; ++j) {
            result += (i * j) / 3;
        }
    }
    
    /* Switch statement */
    switch (x % 4) {
        case 0:
            result *= 2;
            break;
        case 1:
            result += 100;
            break;
        case 2:
            result -= 50;
            break;
        default:
            result /= 2;
    }
    
    return result;
}

/* Function 2: Different control flow pattern */
double func2(volatile int iterations) {
    double total = 0.0;
    volatile int counter = 0;
    
    /* Do-while loop */
    do {
        total += counter * 1.5;
        
        /* Multiple conditionals */
        if (counter < 5) {
            total += 10.0;
        } else if (counter < 10) {
            total += 5.0;
        } else {
            total -= 2.0;
        }
        
        counter++;
    } while (counter < iterations);
    
    /* Early return under condition */
    if (total > 100.0) {
        return total / 2.0;
    }
    
    return total;
}

/* Function 3: Recursive-like pattern */
void func3(volatile int depth, volatile int *accumulator) {
    if (depth <= 0) {
        *accumulator += 1;
        return;
    }
    
    /* Multiple recursive calls simulated with loops */
    for (volatile int i = 0; i < depth; ++i) {
        *accumulator += i;
        
        /* Conditional with complex expression */
        if ((i % 3 == 0) && (*accumulator < 1000)) {
            *accumulator *= 2;
        } else if (i % 5 == 0) {
            *accumulator -= depth;
        }
    }
    
    /* Tail "recursion" */
    func3(depth - 1, accumulator);
}

/* Function 4: String processing simulation */
const char* func4(volatile int mode) {
    static const char* messages[] = {
        "Mode A: Low coverage",
        "Mode B: Medium coverage", 
        "Mode C: High coverage",
        "Mode D: Full coverage"
    };
    
    volatile int index = mode % 4;
    
    /* Complex conditional chain */
    if (mode < 0) {
        return "Invalid mode";
    } else if (mode < 10) {
        return messages[0];
    } else if (mode < 20) {
        return messages[1];
    } else if (mode < 30) {
        return messages[2];
    } else {
        return messages[3];
    }
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int arg_value = 1;
    
    /* Parse command line argument if provided */
    if (argc > 1) {
        arg_value = atoi(argv[1]);
    }
    
    int total = 0;
    
    /* Execute different code paths based on argument */
    if (arg_value == 1) {
        /* Path 1: Execute all functions lightly */
        printf("Path 1 execution\n");
        total += func1(3, 2);
        double res2 = func2(5);
        total += (int)res2;
        
        int acc = 0;
        func3(2, &acc);
        total += acc;
        
        printf("Result: %s\n", func4(5));
    } 
    else if (arg_value == 2) {
        /* Path 2: Different execution pattern */
        printf("Path 2 execution\n");
        total += func1(5, 3);
        double res2 = func2(8);
        total += (int)res2 * 2;
        
        int acc = 10;
        func3(4, &acc);
        total += acc;
        
        printf("Result: %s\n", func4(15));
    }
    else if (arg_value == 3) {
        /* Path 3: Heavy execution */
        printf("Path 3 execution\n");
        total += func1(8, 4);
        double res2 = func2(12);
        total += (int)res2 * 3;
        
        int acc = 20;
        func3(6, &acc);
        total += acc;
        
        printf("Result: %s\n", func4(25));
    }
    else {
        /* Default path */
        printf("Default execution\n");
        total += func1(2, 1);
        printf("Result: %s\n", func4(35));
    }
    
    printf("Final total: %d\n", total);
    return 0;
}
