// Creating a pointer to integer type
struct type int_type = {TYPE_SCALAR, NULL};
struct type ptr_type = {TYPE_POINTER, NULL};
ptr_type.u.pointer_to = &int_type;

// Creating an array of strings
struct type string_type = {TYPE_STRING, NULL};
struct type array_type = {TYPE_ARRAY, NULL};
array_type.u.element_type = &string_type;
