/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*simple_func)(void);
typedef simple_func (*func_returning_func_ptr)(int);
typedef int (*(*func_ptr_returning_func_ptr)(double))(char);

/* Global variable using complex function pointer type */
int (*(*global_func_ptr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_dim_array[2][3])(void);

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*func_array[2])(int);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long l;
        double d;
    } u;
};

/* Global struct with complex initializer */
char global_char = 'A';
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { &global_char, { NULL, NULL } },
    { .d = 3.14 }
};

/* 4. Combine constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4} };

struct Param {
    int x;
    int (*callback)(struct Param*);
};

void process(int (*table[])[5], struct Param param);

/* Complex type mixing all delimiters */
typedef struct {
    int (*(*members[3])(int[][2]))(void);
    struct {
        char data[4][5];
    } inner;
} SuperComplex;

/* 5. More global declarations with all delimiter types */
int (*(*global_complex)(struct Nested))[3] = NULL;

/* Array of structures with function pointers */
struct WithFunc {
    int (*operation)(int, int);
    int values[2][2];
} global_ops[2] = {
    { NULL, {{1,2},{3,4}} },
    { NULL, {{5,6},{7,8}} }
};

/* 6. Helper functions for execution */
int simple_callback(int x, int y) {
    return x + y;
}

int func_for_ptr(int x) {
    return x * 2;
}

int (*get_func_ptr(void))(int) {
    return func_for_ptr;
}

/* Main function to use the complex types */
int main(void) {
    /* Local variable using complex typedef */
    func_returning_func_ptr local_complex = NULL;
    
    /* Assign address to complex function pointer */
    int (*local_func_ptr)(int) = get_func_ptr();
    
    /* Access elements from nested array/structure */
    int sum = 0;
    sum += global_nested.a[0][0];          /* Should be 1 */
    sum += global_nested.a[1][2];          /* Should be 6 */
    sum += (int)global_nested.u.d;         /* Should be 3 (truncated) */
    
    /* Use the array of structures */
    global_ops[0].operation = simple_callback;
    sum += global_ops[0].operation(2, 3);  /* Should be 5 */
    
    /* Use the pointer to array */
    sum += ptr_to_array[0][1];             /* Should be 2 */
    sum += ptr_to_array[1][0];             /* Should be 3 */
    
    /* Use the multi-dimensional array of function pointers */
    multi_dim_array[0][0] = func_for_ptr;
    if (multi_dim_array[0][0]) {
        sum += multi_dim_array[0][0](4);   /* Should be 8 */
    }
    
    /* Complex expression with nested accesses */
    sum += ((int[][2]){ {10,20}, {30,40} })[1][1];  /* Should be 40 */
    
    /* Print result - total should be 1+6+3+5+2+3+8+40 = 68 */
    printf("Result: %d\n", sum);
    
    /* Additional complex declarations inside function to exercise parser */
    struct {
        int (*(*nested_in_main)(int[][3]))[2];
    } local_struct = { NULL };
    
    int (*(*another_local)(void))[4] = NULL;
    
    return (sum == 68) ? 0 : 1;
}

/* Function implementation */
void process(int (*table[])[5], struct Param param) {
    /* Empty implementation, just for declaration */
    (void)table;
    (void)param;
}

/* Even more complex global declarations */
union Ultimate {
    struct {
        int (*(*func_field)(int (*)(int[][2])))(void);
        char data[2][(3+2)*2];
    } s;
    long long int big_array[sizeof(struct Nested) > 20 ? 10 : 5][3];
} global_union = {
    .s = { NULL, { {'a','b','c','d','e','f','g','h','i','j'} } }
};

/* Template for C++ compilation (if needed) */
#ifdef __cplusplus
class ComplexClass {
public:
    virtual int (*(*virtual_method(double d))[3])(int) = 0;
    
private:
    int (*(*member_ptr)(int))[2];
};
#endif
