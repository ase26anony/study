#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration that will never be defined */
struct undefined_struct GTY((tag("undefined")));

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY((user));
typedef int another_scalar_t GTY((user));

/* TYPE_STRUCT: Standard C struct with GTY tag */
struct my_struct GTY((tag("my_struct")))
{
    my_scalar_t field1;
    another_scalar_t field2;
    struct my_struct *next GTY((skip));  /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT: Struct marked for special user handling */
struct user_handled_struct GTY((user))
{
    int user_field1;
    float user_field2;
};

/* TYPE_UNION: Union with GTY-tagged members */
union my_union GTY((tag("my_union")))
{
    int int_val;
    float float_val;
    struct my_struct *struct_ptr GTY((skip));
};

/* TYPE_ARRAY: Struct containing various array types */
struct array_container GTY((tag("array_container")))
{
    /* Fixed-size array */
    int fixed_array[10] GTY((skip));
    
    /* Zero-length array at the end */
    char flexible_array[] GTY((skip));
    
    /* Array with length attribute */
    struct my_struct *ptr_array GTY((length("array_len")));
    int array_len;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY((tag("string_container")))
{
    char *name GTY((length("name_len")));
    int name_len;
    
    /* Another string field */
    const char *description GTY((length("desc_len")));
    int desc_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, float) GTY((callback));

struct callback_container GTY((tag("callback_container")))
{
    callback_func_t handler;
    int callback_id;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((tag("lang_struct"), lang_struct(1)))
{
    int lang_field1;
    void *lang_field2 GTY((skip));
};

/* Complex nested types for interdependencies */

/* Forward declaration for circular reference */
struct node_a GTY((tag("node_a")));

struct node_b GTY((tag("node_b")))
{
    struct node_a *link_to_a GTY((skip));  /* TYPE_POINTER to forward-declared type */
    union my_union data;                    /* TYPE_UNION */
    struct string_container str_data;       /* TYPE_STRUCT containing TYPE_STRING */
};

struct node_a GTY((tag("node_a")))
{
    struct node_b *link_to_b GTY((skip));  /* TYPE_POINTER creating circular reference */
    int value;
    
    /* Nested anonymous union */
    union {
        int nested_int;
        float nested_float;
    } GTY((tag("nested_union")));
    
    /* Array of pointers */
    struct node_b *neighbors[5] GTY((skip));  /* TYPE_ARRAY of TYPE_POINTER */
};

/* Another complex type with multiple nested structures */
struct complex_type GTY((tag("complex_type")))
{
    /* Direct struct field */
    struct my_struct direct_struct;
    
    /* Pointer to union */
    union my_union *union_ptr GTY((skip));
    
    /* Array of structs */
    struct string_container strings[3];
    
    /* Pointer to array container */
    struct array_container *array_ptr GTY((skip));
    
    /* Callback function pointer */
    callback_func_t callback;
    
    /* Language-specific struct */
    struct lang_specific_struct lang_struct;
    
    /* User-handled struct */
    struct user_handled_struct user_struct;
};

#endif /* TEST_GENGYPE_H */
