/* test_state_types.h - Header file with all type categories for gengtype state testing */
#ifndef TEST_STATE_TYPES_H
#define TEST_STATE_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* 1. Scalar Type */
typedef unsigned int my_scalar GTY((skip));

/* 2. String Type */
typedef const char *my_string GTY((string));

/* 3. Regular Structure */
struct my_struct GTY(()) {
    my_scalar field1;
    my_string field2;
};

/* 4. User-defined Structure */
struct my_user_struct GTY((user)) {
    int user_field;
    void *user_data GTY((skip));
};

/* 5. Union */
union my_union GTY(()) {
    int int_val;
    double double_val;
    char *char_ptr GTY((skip));
};

/* 6. Pointer Type */
typedef struct my_struct *my_ptr GTY((skip));

/* 7. Array Type */
typedef int my_array[10] GTY((skip));

/* 8. Callback (Function Pointer) */
typedef void (*my_callback)(int) GTY((skip));

/* 9. Language-Specific Structure */
#ifdef GCC
struct lang_struct GTY(()) {
    int lang_field;
    void *lang_data GTY((skip));
};
#endif

/* 10. Undefined Type (forward declaration) */
struct undefined_type;

/* 11. Another structure to reference undefined type */
struct references_undefined GTY(()) {
    struct undefined_type *undef_ptr GTY((skip));
    int defined_field;
};

#endif /* TEST_STATE_TYPES_H */
