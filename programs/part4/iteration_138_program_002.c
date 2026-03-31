/* test_state_types.h - Header file containing all type categories for gengtype state testing */

#ifndef TEST_STATE_TYPES_H
#define TEST_STATE_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* 1. SCALAR TYPE (TYPE_SCALAR) */
typedef unsigned int my_scalar GTY((skip));

/* 2. STRING TYPE (TYPE_STRING) */
typedef const char *my_string GTY((string));

/* 3. REGULAR STRUCT (TYPE_STRUCT) */
struct my_struct GTY(()) {
    my_scalar field1;
    my_string field2;
    int other_field GTY((skip));
};

/* 4. USER STRUCT (TYPE_USER_STRUCT) */
struct my_user_struct GTY((user)) {
    int user_field1;
    char user_field2;
};

/* 5. UNION (TYPE_UNION) */
union my_union GTY(()) {
    int int_val;
    char char_val;
    void* ptr_val GTY((skip));
};

/* 6. POINTER TYPE (TYPE_POINTER) */
typedef struct my_struct *my_ptr GTY((skip));

/* 7. ARRAY TYPE (TYPE_ARRAY) */
typedef int my_array[10] GTY((skip));

/* 8. CALLBACK/FUNCTION POINTER (TYPE_CALLBACK) */
typedef void (*my_callback)(int) GTY((skip));

/* 9. LANGUAGE-SPECIFIC STRUCTURE (TYPE_LANG_STRUCT) */
#ifdef GCC
struct lang_struct GTY(()) {
    int lang_field;
    void* lang_ptr GTY((skip));
};
#endif

/* 10. UNDEFINED TYPE (TYPE_UNDEFINED) - forward declaration */
struct undefined_type;

/* 11. Additional pointer types for coverage */
typedef union my_union *union_ptr GTY((skip));
typedef my_callback callback_ptr GTY((skip));

#endif /* TEST_STATE_TYPES_H */
