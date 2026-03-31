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

/* 3. STRUCT TYPE (regular) */
struct my_struct GTY(()) {
    my_scalar field1;
    my_string field2;
    int other_field GTY((skip));
};

/* 4. USER STRUCT TYPE */
struct my_user_struct GTY((user)) {
    int user_field1;
    char user_field2;
};

/* 5. UNION TYPE */
union my_union GTY(()) {
    int int_val;
    char char_val;
    my_scalar scalar_val;
};

/* 6. POINTER TYPE */
typedef struct my_struct *my_ptr GTY((skip));

/* 7. ARRAY TYPE */
typedef int my_array[10] GTY((skip));

/* 8. CALLBACK TYPE (function pointer) */
typedef void (*my_callback)(int) GTY((skip));

/* 9. LANGUAGE-SPECIFIC STRUCTURE */
#ifdef GCC
struct lang_struct GTY(()) {
    int lang_field;
    void *lang_ptr GTY((skip));
};
#endif

/* 10. UNDEFINED TYPE (forward declaration) */
struct undefined_type;

/* 11. Additional pointer variations */
typedef union my_union *union_ptr GTY((skip));
typedef my_array *array_ptr GTY((skip));

/* Root structure that references many types */
struct root_struct GTY((root)) {
    my_scalar scalar_field;
    my_string string_field;
    struct my_struct *struct_ptr;
    union my_union union_field;
    my_array array_field;
    my_callback callback_field;
    struct my_user_struct user_struct_field;
#ifdef GCC
    struct lang_struct *lang_struct_ptr;
#endif
    struct undefined_type *undefined_ptr;
};

#endif /* TEST_STATE_TYPES_H */
