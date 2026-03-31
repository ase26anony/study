/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;

/* TYPE_SCALAR */
typedef int GTY(()) scalar_type;

/* TYPE_STRING */
typedef char *GTY(()) string_type;

/* TYPE_STRUCT */
struct GTY(()) my_struct {
    /* TYPE_SCALAR */
    int scalar_field;
    
    /* TYPE_STRING */
    char *string_field;
    
    /* TYPE_POINTER */
    struct my_struct *next;
    
    /* TYPE_ARRAY */
    int array_field[10];
    
    /* TYPE_UNDEFINED (opaque pointer) */
    struct opaque *opaque_ptr;
};

/* TYPE_USER_STRUCT */
typedef struct GTY(()) my_struct my_struct_t;

/* TYPE_UNION */
union GTY(()) my_union {
    int int_val;
    char *string_val;
    struct my_struct *struct_ptr;
};

/* TYPE_POINTER (standalone) */
typedef struct my_struct *GTY(()) struct_ptr_t;

/* TYPE_ARRAY (standalone) */
typedef int GTY(()) int_array_t[5];

/* TYPE_CALLBACK */
typedef void (*GTY(()) callback_t)(int, char *);

/* Nested structure for additional coverage */
struct GTY(()) outer_struct {
    /* TYPE_STRUCT nested */
    struct GTY(()) inner_struct {
        int x;
        int y;
    } inner;
    
    /* TYPE_UNION nested */
    union GTY(()) inner_union {
        int a;
        float b;
    } u;
    
    /* TYPE_CALLBACK field */
    callback_t handler;
    
    /* TYPE_ARRAY of pointers */
    struct my_struct *GTY(()) ptr_array[8];
    
    /* TYPE_POINTER to array */
    int (*GTY(()) array_ptr)[4];
};

/* Another union with different structure */
union GTY(()) complex_union {
    struct my_struct s;
    struct outer_struct *o;
    callback_t func;
    int numbers[3];
};

/* TYPE_LANG_STRUCT simulation (GCC internal type) */
#ifdef IN_GCC
struct GTY(()) lang_type {
    int lang_specific;
};
#endif

/* Chain structure for pointer testing */
struct GTY(()) chain_node {
    int id;
    struct chain_node *GTY((skip)) prev;  /* Skip for GC */
    struct chain_node *GTY((chain_next ("next"))) next;
};

/* Function pointer with different signature */
typedef int (*GTY(()) compare_func_t)(const void *, const void *);

/* Mixed type structure */
struct GTY(()) mixed_types {
    /* All basic types in one struct */
    char c;
    short s;
    int i;
    long l;
    float f;
    double d;
    char *str;
    void *ptr;
    int fixed[7];
    int *vla;  /* Variable length array pointer */
    callback_t callback1;
    compare_func_t callback2;
    union my_union u;
    struct outer_struct os;
};

#endif /* GTY_TEST_H */
