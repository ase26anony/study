/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int complex_function_1(volatile int x) {
    int result = 0;
    
    /* Loop with volatile counter */
    for (volatile int i = 0; i < x; ++i) {
        if (i % 2 == 0) {
            result += i * 2;
        } else {
            result += i;
        }
        
        /* Nested loop */
        for (volatile int j = 0; j < 3; ++j) {
            result += j;
        }
    }
    
    /* Conditional block */
    if (result > 100) {
        result -= 50;
    } else if (result > 50) {
        result -= 25;
    }
    
    return result;
}

/* Function 2: Different control flow pattern */
int complex_function_2(volatile int y) {
    int total = 0;
    volatile int counter = y;
    
    /* While loop */
    while (counter > 0) {
        total += counter;
        
        /* Switch statement */
        switch (counter % 4) {
            case 0:
                total += 10;
                break;
            case 1:
                total += 5;
                break;
            case 2:
                total += 2;
                break;
            case 3:
                total += 1;
                break;
        }
        
        counter--;
    }
    
    /* Multiple if-else chain */
    if (total < 0) {
        total = 0;
    } else if (total < 100) {
        total *= 2;
    } else if (total < 200) {
        total += 100;
    } else {
        total = 200;
    }
    
    return total;
}

/* Function 3: Recursive-like pattern with loops */
int complex_function_3(volatile int z) {
    int sum = 0;
    
    /* Do-while loop */
    volatile int k = z;
    do {
        sum += k;
        
        /* Inner conditional loop */
        if (k > 5) {
            for (volatile int m = 0; m < k % 5; ++m) {
                sum += m * 2;
            }
        }
        
        k--;
    } while (k > 0);
    
    /* Complex conditional */
    if (sum % 2 == 0 && sum > 20) {
        sum /= 2;
    } else if (sum % 3 == 0) {
        sum *= 3;
    }
    
    return sum;
}

/* Function 4: Another variation */
float complex_function_4(volatile int a, volatile int b) {
    float ratio = 0.0;
    
    /* Multiple loops */
    for (volatile int i = 0; i < a; ++i) {
        for (volatile int j = 0; j < b; ++j) {
            ratio += (float)i / (j + 1);
        }
    }
    
    /* Conditional returns */
    if (ratio > 100.0) {
        return ratio / 10.0;
    } else if (ratio > 10.0) {
        return ratio * 2.0;
    }
    
    return ratio;
}

/* Main function with different execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 1;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    int result1, result2, result3;
    float result4;
    
    /* Different execution paths based on mode */
    switch (mode) {
        case 1:
            result1 = complex_function_1(10);
            result2 = complex_function_2(8);
            result3 = complex_function_3(6);
            result4 = complex_function_4(5, 3);
            break;
            
        case 2:
            result1 = complex_function_1(5);
            result3 = complex_function_3(10);
            result4 = complex_function_4(3, 7);
            result2 = 0;  // Different path
            break;
            
        case 3:
            result2 = complex_function_2(12);
            result4 = complex_function_4(8, 2);
            result1 = complex_function_1(3);
            result3 = complex_function_3(4);
            break;
            
        default:
            result1 = complex_function_1(mode % 10);
            result2 = complex_function_2(mode % 8);
            result3 = complex_function_3(mode % 6);
            result4 = complex_function_4(mode % 5, mode % 4);
            break;
    }
    
    /* Force usage of results to prevent optimization */
    printf("Results: %d, %d, %d, %.2f\n", 
           result1, result2, result3, result4);
    
    return 0;
}
