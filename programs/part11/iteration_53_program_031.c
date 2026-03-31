/* Test types for gengtype coverage */

/* TYPE_UNDEFINED - incomplete/opaque type */
struct opaque_struct;

/* TYPE_STRUCT - regular struct */
struct my_struct {
    int a;
    double b;
};

/* TYPE_USER_STRUCT - user-defined struct with GTY marker */
struct GTY((user)) user_struct {
    int user_data;
};

/* TYPE_UNION */
union my_union {
    int i;
    float f;
    char c;
};

/* TYPE_POINTER */
struct with_pointers {
    int *int_ptr;
    struct my_struct *struct_ptr;
    void *void_ptr;
};

/* TYPE_ARRAY */
struct with_arrays {
    int arr[10];
    char str[50];
    struct my_struct structs[5];
};

/* TYPE_SCALAR */
typedef int my_scalar;
typedef unsigned long my_other_scalar;

/* TYPE_STRING */
struct with_strings {
    char *name;
    const char *const_name;
    char buffer[100];
};

/* TYPE_CALLBACK */
typedef int (*callback_func)(int, void*);
typedef void (*simple_callback)(void);

struct with_callbacks {
    callback_func handler;
    simple_callback cleanup;
};

/* Nested structures for complexity */
struct complex_nested {
    struct with_pointers *ptr_member;
    union my_union union_member;
    int scalar_member;
    char *string_member;
    callback_func callback_member;
    int array_member[20];
};
