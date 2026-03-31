// Creating a pointer to integer type
struct type *int_type = create_type(TYPE_SCALAR);
struct type *ptr_type = create_type(TYPE_POINTER);
ptr_type->u.pointer_to = int_type;

// Creating an array of strings
struct type *string_type = create_type(TYPE_STRING);
struct type *array_type = create_type(TYPE_ARRAY);
array_type->u.element_type = string_type;
