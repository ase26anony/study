/* Test file for gengtype parser coverage */
/* Parentheses in function pointer declarations */
typedef int (*func_ptr_type)(int, char);
typedef void (*(*complex_funcptr)(void))(int);

/* Brackets in array declarations */
extern int global_array[10];
extern int multi_dim_array[5][(sizeof(int)*2)];

/* Braces in struct definition */
struct GTY(()) TestStruct {
    int a;
    char b;
    int (*callback)(int);
};

/* Array of function pointers (nested [] and ()) */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array */
int (*(*get_array_ptr_func(void))[10]);

/* GTY-marked variable with function pointer */
static GTY(()) int (*global_hook)(int) = 0;

/* Union with nested struct */
union GTY(()) TestUnion {
    int i;
    float f;
    struct {
        int x;
        int y;
    } point;
};

/* Enum with no braces needed but included for completeness */
enum TestEnum { VAL1, VAL2, VAL3 };
