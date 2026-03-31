/* Test program to cover integer_valued_real_p lines in fold-const.cc */
#include <stdio.h>
#include <math.h>

int main(void) {
    /* Use const variables to encourage constant folding */
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 5.0;
    
    int result = 0;
    
    /* 1. One-argument integer-valued functions with literals */
    double r1 = rint(4.7);          /* Should be 5.0 */
    double t1 = trunc(5.9);         /* Should be 5.0 */
    double f1 = floor(6.2);         /* Should be 6.0 */
    double c1 = ceil(3.1);          /* Should be 4.0 */
    double rd1 = round(7.5);        /* Should be 8.0 */
    
    /* 2. One-argument functions with const variables */
    double r2 = __builtin_rint(a);  /* Should be 11.0 */
    double t2 = __builtin_trunc(a); /* Should be 10.0 */
    double f2 = __builtin_floor(a); /* Should be 10.0 */
    double c2 = __builtin_ceil(a);  /* Should be 11.0 */
    
    /* 3. Two-argument functions with constant pairs */
    /* pow with integer result */
    double p1 = pow(2.0, 3.0);      /* 8.0 - integer */
    double p2 = __builtin_pow(c, d); /* 32.0 - integer */
    
    /* fmod with zero remainder */
    double fm1 = fmod(9.0, 3.0);    /* 0.0 - integer */
    double fm2 = __builtin_fmod(15.0, 5.0); /* 0.0 - integer */
    
    /* remainder with zero remainder */
    double rem1 = remainder(12.0, 4.0); /* 0.0 - integer */
    double rem2 = __builtin_remainder(20.0, 5.0); /* 0.0 - integer */
    
    /* 4. Use results in integer contexts to prompt analysis */
    int i1 = (int)rint(3.14);       /* Implicit conversion analysis */
    int i2 = (int)__builtin_trunc(8.9);
    
    /* Array indexing with floor result */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 1;       /* arr[2] = 1 */
    
    /* Comparisons with integers */
    if (trunc(7.3) == 7) {
        result += 1;
    }
    
    if (__builtin_ceil(4.2) == 5) {
        result += 2;
    }
    
    /* Binary operations with integers */
    double sum1 = pow(3.0, 2.0) + 1;  /* 9.0 + 1 = 10.0 */
    double sum2 = __builtin_floor(6.7) * 2; /* 6.0 * 2 = 12.0 */
    
    /* 5. Mix of function variants */
    /* nearbyint is another integer-valued function */
    double n1 = nearbyint(2.3);     /* Should be 2.0 */
    double n2 = __builtin_nearbyint(5.8); /* Should be 6.0 */
    
    /* 6. Use in switch context (though constant folding may happen earlier) */
    int val = (int)round(3.6);      /* Should be 4 */
    switch (val) {
        case 4:
            result += 4;
            break;
        default:
            result += 8;
    }
    
    /* 7. Complex expression with multiple integer-valued calls */
    double complex_expr = floor(pow(2.0, 4.0) / 2.0); /* floor(16.0/2.0) = 8.0 */
    
    /* 8. Ensure all results are used to prevent dead code elimination */
    result += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)rd1;
    result += (int)r2 + (int)t2 + (int)f2 + (int)c2;
    result += (int)p1 + (int)p2;
    result += (int)fm1 + (int)fm2;
    result += (int)rem1 + (int)rem2;
    result += i1 + i2;
    result += (int)sum1 + (int)sum2;
    result += (int)n1 + (int)n2;
    result += (int)complex_expr;
    
    /* Print result to ensure code isn't optimized away */
    printf("Result: %d\n", result);
    
    return 0;
}
