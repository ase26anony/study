/* test_error_mark_node.c - Program to trigger error_mark_node in expr.cc */

/* Global scope errors */
#define BAD_MACRO(x, y) (x & y)
#define CONCAT(a, b) a##b##_invalid

int global = 5 + "string";  /* Type mismatch in global initializer */

/* Function with multiple error patterns */
void test_errors(void) {
    /* Valid code for context */
    int valid = 42;
    float fvalid = 3.14;
    
    /* 1. Type mismatch in binary operation */
    int x = valid + "hello";  /* int + string literal */
    
    /* 2. Invalid pointer arithmetic */
    float* fp = &fvalid;
    float y = fp / 2;  /* pointer division */
    
    /* 3. Bitwise operator on floating point */
    double d = 3.14159 | 2.71828;
    
    /* 4. Address-of on literal/constant */
    int* p = &42;
    
    /* 5. Invalid initializer - excess values */
    int arr[3] = {1, 2, 3, 4, 5};
    
    /* 6. Scalar with multiple initializers */
    int z = {5, 6, 7};
    
    /* 7. Undeclared identifier (forward reference without C23) */
    int a = b;  /* b not declared yet */
    int b = 10;
    
    /* 8. Out of scope reference */
    {
        int inner = 99;
    }
    int outer = inner;  /* inner out of scope */
    
    /* 9. Macro-generated error */
    int macro_err = BAD_MACRO(3.14, "text");
    
    /* 10. Invalid builtin usage */
    int builtin_err = __builtin_ctz("string");
    
    /* 11. Statement expression misuse */
    int stmt_expr = ({ int temp; });
    
    /* 12. Concatenated invalid identifier */
    int CONCAT(very, bad) = 100;
    
    /* 13. Function address arithmetic */
    void (*funcptr)(void) = &test_errors;
    funcptr = funcptr + 1;
    
    /* 14. Return with invalid expression */
    return main + 1;  /* function pointer arithmetic */
}

/* Another function with control flow errors */
void control_flow_errors(void) {
    /* Valid statement */
    int counter = 0;
    
    /* Invalid condition */
    if (5 + "test") {
        counter++;
    }
    
    /* Invalid loop condition */
    while (&counter + "string") {
        counter--;
    }
    
    /* Invalid for loop initializer */
    for (int i = "start"; i < 10; i++) {
        /* empty */
    }
    
    /* Invalid switch case expression */
    switch ("not an integer") {
        case 1: break;
        default: break;
    }
}

/* Main function with mixed valid/invalid code */
int main(int argc, char **argv) {
    /* Valid code */
    int ok1 = 100;
    printf("Starting...\n");
    
    /* Invalid function call argument */
    printf("%d", 5 + "world");
    
    /* Invalid assignment */
    ok1 = &ok1 * 2;
    
    /* More valid code */
    int ok2 = ok1 * 2;
    
    /* Complex invalid expression */
    int complex_err = (ok1 > 0) ? "yes" : 0;
    
    /* Nested invalid expressions */
    int nested = (5 + (float)(&ok1)) * "text";
    
    /* Call error functions */
    test_errors();
    control_flow_errors();
    
    /* Invalid return expression */
    return argc + "argv";
}
