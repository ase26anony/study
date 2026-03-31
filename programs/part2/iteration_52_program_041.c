/* test_error_mark_node.c
 * This program contains syntactically valid but semantically invalid expressions
 * designed to trigger the error_mark_node return path in expr.cc line 251.
 */

/* Global scope errors */
int global_bad = 5 + "string";  /* Type mismatch in binary operation */

/* Macro to generate malformed expressions */
#define BAD_BINARY(a, b) (a | b)
#define BAD_ADDRESS(x) (&x)
#define BAD_INITIALIZER {1, 2, 3, 4, 5}

/* Function with various expression errors */
void problematic_function(void) {
    /* Valid code for context */
    int valid_var = 42;
    
    /* 1. Type mismatch in binary operation */
    int type_mismatch = valid_var + "text";
    
    /* 2. Invalid operand combinations */
    double fp = 3.14159;
    double bad_bitwise = fp | 2.71828;  /* Bitwise on floating point */
    
    /* 3. Using address-of on literals */
    int* bad_ptr1 = &42;
    int* bad_ptr2 = &(valid_var + 1);  /* Address of temporary expression */
    
    /* 4. Macro-generated error */
    int macro_error = BAD_BINARY(3.14, "hello");
    
    /* 5. Invalid initializer for scalar */
    int bad_scalar_init = {5, 6, 7};
    
    /* 6. Array with excess initializers */
    int small_array[3] = {1, 2, 3, 4, 5};
    
    /* 7. Statement expression misuse (GCC extension) */
    int stmt_expr = ({ int a; });  /* Missing value */
    
    /* 8. Builtin function with wrong type */
    int bad_builtin = __builtin_ctz("string");
    
    /* 9. Control flow with bad expressions */
    if (valid_var + "text") {
        /* Bad expression in condition */
    }
    
    while (fp & 0x1) {  /* Bitwise on float in condition */
        break;
    }
    
    /* 10. Return with bad expression */
    return valid_var + "text";
}

/* Another function with scope issues */
void scope_problems(void) {
    {
        int inner_var = 100;
    }
    /* Use variable from inner scope */
    int use_out_of_scope = inner_var * 2;
}

/* Main function with mixed valid/invalid code */
int main(void) {
    /* Some valid code for context */
    int ok1 = 10;
    int ok2 = 20;
    int valid_sum = ok1 + ok2;
    
    /* Error 1: Function address arithmetic */
    int func_arithmetic = (int)(main + 1);
    
    /* Error 2: Pointer arithmetic with wrong types */
    int x = 5;
    float* bad_ptr_arith = (&x) / 2;
    
    /* Error 3: Complex expression with multiple issues */
    int complex_bad = (3.14 > "text") ? main : &42;
    
    /* Error 4: Invalid argument to function call */
    printf("%d", 5 + "str");  /* printf declaration needed */
    
    /* Error 5: Misusing offsetof with bad types */
    struct BadStruct {
        int a;
        double b;
    };
    size_t bad_offset = __builtin_offsetof(struct BadStruct, b) | "text";
    
    /* Error 6: Token concatenation creating invalid identifier */
    #define CONCAT(a, b) a##b
    int CONCAT(123, 456) = 789;  /* Invalid identifier starting with digit */
    
    /* Valid statement to separate errors */
    int another_valid = 99;
    
    /* Error 7: Compound literal with address taken */
    int* bad_compound_lit = &(int){42};
    
    /* Error 8: Switch case with bad expression */
    switch(ok1) {
        case 5 + "text":  /* Non-constant case expression */
            break;
        default:
            break;
    }
    
    /* Error 9: GNU ternary extension misuse */
    int gnuc_error = ok1 ?: "default";  /* Wrong type for default */
    
    /* Error 10: Alignment specifier with bad expression */
    _Alignas("string") int bad_align;
    
    return 0;
}

/* Additional global errors */
int* global_bad_ptr = &"constant";  /* Address of string literal */
float global_float_bitwise = 1.23f & 4.56f;

/* Invalid function return type expression */
int* bad_return_type_func(void) {
    return 42;  /* Returning int where pointer expected */
}

/* Array with designator errors */
int bad_designator[10] = {
    [5 + "text"] = 1,  /* Non-integer designator */
    [2] = 3.14,        /* Wrong type in initializer */
};
