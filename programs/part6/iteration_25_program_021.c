/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn1)(void);
typedef fn1 (*fn2)(int);
int (*(*complex_func_ptr)(int (*)(double)))(char);

/* Helper function for function pointers */
int simple_func(void) { return 42; }
int (*func_returning_int(double d))(void) { return simple_func; }
int (*(*get_complex_func(void))(int (*)(double)))(char) {
    return complex_func_ptr;
}

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix[3][4])(float, double);

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*func)(int);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long x;
        double y;
    } u;
};

/* Global struct with initializer */
struct Data { 
    int (*func)(int);
    struct Nested nested;
} global_data = { 
    NULL, 
    { 
        {{1,2,3},{4,5,6}}, 
        { NULL, NULL },
        { .x = 100 }
    } 
};

/* Array with designated initializers */
int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combine constructs in single declarations */
int (*ptr)[2] = (int[][2]){ {1,2}, {3,4} };

/* Complex parameter function prototype */
void process(int (*table[])[5], struct { int x; int y[2]; } param);

/* 5. Global scope complex declarations */
int (*(*global_var)(void))[3];
struct { 
    int (*callback)(int (*(*)(void))[3]); 
    char data[2][2][2];
} global_struct = { NULL, {{{'a','b'},{'c','d'}},{{'e','f'},{'g','h'}}} };

/* Function with complex return type */
int (*(*returns_func_ptr(void))[2])(int) {
    static int (*arr[2])(int);
    return &arr;
}

/* 6. Main function to use all constructs */
int main(void) {
    /* Use typedefs */
    fn1 f1 = simple_func;
    fn2 f2 = NULL;
    
    /* Access nested array from global struct */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            sum += global_data.nested.a[i][j];
        }
    }
    
    /* Access designated initializer array */
    sum += arr[1][2];
    
    /* Use complex pointer with compound literal */
    int val = (*ptr)[1];
    sum += val;
    
    /* Access deeply nested global struct */
    char c = global_struct.data[1][0][1];
    sum += (int)c;
    
    /* Use function returning complex type */
    int (*(*func_ptr_array)[2])(int) = returns_func_ptr();
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", sum + f1());
    
    return 0;
}

/* Additional complex declarations */
union ComplexUnion {
    struct {
        int (*(*func)(int[][2]))(void);
        char *(*strings[4])[3];
    } s;
    void (*vfunc)(int (*)(int), char [][(2+3)*2]);
};

/* Template for complex type (C++ style comment for testing) */
/*
template<typename T>
struct TemplateStruct {
    T (*processor)(T (*)(T), T[][10]);
};
*/

/* K&R style function definition for historical syntax */
int old_style_func(x, y)
    int x;
    char *y[];
{
    return x + (int)y[0];
}
