/* test_error_mark_node.c - Program to trigger error_mark_node in expr.cc */

/* Global scope errors */
#define BAD_MACRO(x, y) (x & y)
#define CONCAT(a, b) a##b + b##a

/* Invalid global initializer - excess initializers */
int global_arr[3] = {1, 2, 3, 4, 5};  /* line 8: too many initializers */

/* Function taking address of constant */
void* bad_address(void) {
    return &42;  /* line 11: address of constant */
}

int main(void) {
    /* Valid code for context */
    int valid = 10;
    printf("Starting...\n");
    
    /* Pattern 1: Type mismatch in binary operation */
    int x = 5 + "string";  /* line 18: int + string literal */
    
    /* Pattern 2: Bitwise operator on floating point */
    double d = 3.14159;
    double result = d | 2.5;  /* line 21: bitwise OR on doubles */
    
    /* Valid statement to maintain structure */
    int ok = 42;
    
    /* Pattern 3: Invalid initializer - scalar with multiple values */
    int z = {5, 6, 7};  /* line 26: multiple values for scalar */
    
    /* Pattern 4: Using macro to generate type mismatch */
    float f = BAD_MACRO(3.14, 5);  /* line 29: float & int via macro */
    
    /* Pattern 5: Undeclared identifier in expression */
    {
        int inner = 100;
    }
    x = inner * 2;  /* line 35: 'inner' out of scope */
    
    /* Pattern 6: Invalid builtin usage */
    int bits = __builtin_ctz("hello");  /* line 38: string arg to ctz */
    
    /* Pattern 7: Statement expression missing value */
    int se = ({ int a; });  /* line 41: no value in statement expr */
    
    /* Pattern 8: Pointer arithmetic type mismatch */
    int* ptr = &valid;
    float bad_float = ptr / 2;  /* line 45: pointer / int to float */
    
    /* Pattern 9: Token concatenation creating invalid expression */
    int CONCAT(var, 123) = 50;  /* line 48: creates var123 + 123var */
    
    /* Pattern 10: Return with invalid expression */
    return main + 1;  /* line 51: function pointer arithmetic */
}

/* Additional function with control flow errors */
void test_control_flow(void) {
    /* Invalid condition expression */
    if (5 % "text") {  /* line 57: modulo with string */
        printf("Never reached\n");
    }
    
    /* Invalid for loop initializer */
    for (int i = {1, 2}; i < 10; i++) {  /* line 62: multiple values */
        /* Empty */
    }
    
    /* Invalid while condition */
    while (&"constant"[0]) {  /* line 66: address of string element */
        break;
    }
}

/* Global with invalid initializer */
struct Point {
    int x, y;
};

struct Point p1 = {1, 2, 3};  /* line 74: too many initializers */

/* Function with invalid parameter default (GCC extension misuse) */
void bad_default(int n = 5) {  /* line 77: C doesn't have default args */
    /* Try to use address of parameter incorrectly */
    int* addr = &n++;
}
