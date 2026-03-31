/* test_structures.h - Contains examples of all type categories tracked by gengtype */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic typedefs */
typedef int my_scalar;
typedef unsigned long my_unsigned_scalar;
typedef double my_float_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char * GTY((string)) my_string_type;

/* TYPE_STRUCT: Regular structures with GTY(()) */
struct GTY(()) regular_struct {
    int field1;
    double field2;
    struct undefined_type *ptr;  /* Uses undefined type */
};

/* TYPE_USER_STRUCT: Structures marked with GTY((user)) */
struct GTY((user)) user_struct {
    void *user_data;
    int user_id;
};

/* Another user struct for variety */
struct GTY((user)) another_user_struct {
    char *name;
    struct user_struct *next;
};

/* TYPE_UNION: Union definitions */
union GTY(()) my_union {
    int as_int;
    double as_double;
    void *as_ptr;
};

/* TYPE_POINTER: Typedefs for pointers */
typedef struct regular_struct *regular_struct_ptr;
typedef union my_union *my_union_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef struct regular_struct struct_array[5];
typedef const char * GTY((string)) string_array[3];

/* TYPE_CALLBACK: Function pointer typedef */
typedef int (*comparison_callback)(const void *, const void *);
typedef void (*event_handler)(int event_id, void *data);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_specific_field;
    void *lang_data;
};
#endif

/* Nested structure for complexity */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int inner_field;
        struct inner_struct *next;
    } *inner;
    
    union GTY(()) inner_union {
        int option_a;
        double option_b;
    } choice;
    
    comparison_callback cmp_func;
    string_array names;
};

/* More pointer types */
typedef struct outer_struct *outer_ptr;
typedef struct outer_struct **outer_double_ptr;

/* Array of pointers */
typedef struct regular_struct *struct_ptr_array[8];

/* Callback in a struct */
struct GTY(()) callback_container {
    event_handler handler;
    void *user_data;
    int_array thresholds;
};

#endif /* TEST_STRUCTURES_H */
