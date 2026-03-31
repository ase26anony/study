/* test_state_types.h - Header file containing all type categories for gengtype state testing */
#ifndef TEST_STATE_TYPES_H
#define TEST_STATE_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* 1. SCALAR TYPE */
typedef unsigned int my_scalar GTY((skip));

/* 2. STRING TYPE */
typedef const char *my_string GTY((string));

/* 3. STRUCT TYPE */
struct my_struct GTY(()) {
    my_scalar field1;
    my_string field2;
};

/* 4. USER STRUCT TYPE */
struct my_user_struct GTY((user)) {
    int user_field;
};

/* 5. UNION TYPE */
union my_union GTY(()) {
    int int_val;
    my_scalar scalar_val;
    void *ptr_val;
};

/* 6. POINTER TYPE */
typedef struct my_struct *my_ptr GTY((skip));

/* 7. ARRAY TYPE */
typedef int my_array[10] GTY((skip));

/* 8. CALLBACK TYPE (Function Pointer) */
typedef void (*my_callback)(int) GTY((skip));

/* 9. LANGUAGE-SPECIFIC STRUCTURE */
#ifdef GCC
struct lang_struct GTY(()) {
    int lang_field;
};
#endif

/* 10. UNDEFINED TYPE (Forward declaration) */
struct undefined_type;

/* 11. Additional pointer types for coverage */
typedef union my_union *union_ptr GTY((skip));
typedef my_callback *callback_ptr GTY((skip));

/* Nested structure for additional coverage */
struct nested_struct GTY(()) {
    struct my_struct inner GTY((skip));
    my_array arr_field;
};

#endif /* TEST_STATE_TYPES_H */
