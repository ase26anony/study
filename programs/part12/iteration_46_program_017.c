// Creating a pointer to int
struct type int_type = {TYPE_SCALAR, NULL};
struct type ptr_type = {TYPE_POINTER, NULL};
ptr_type.u.pointer_to = &int_type;

// Creating an array of floats
struct type float_type = {TYPE_SCALAR, NULL};
struct type array_type = {TYPE_ARRAY, NULL};
array_type.u.element_type = &float_type;
