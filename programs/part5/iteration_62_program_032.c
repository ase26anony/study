void count_type(enum type_kind type) {
    switch (type) {
    case TYPE_UNDEFINED:
        nb_undefined++;
        break;
    case TYPE_SCALAR:
        nb_scalar++;
        break;
    case TYPE_STRING:
        nb_string++;
        break;
    case TYPE_STRUCT:
        nb_struct++;
        break;
    case TYPE_USER_STRUCT:
        nb_user_struct++;
        break;
    case TYPE_UNION:
        nb_union++;
        break;
    case TYPE_POINTER:
        nb_pointer++;
        break;
    case TYPE_ARRAY:
        nb_array++;
        break;
    case TYPE_CALLBACK:
        nb_callback++;
        break;
    case TYPE_LANG_STRUCT:
        nb_lang_struct++;
        break;
    case TYPE_NONE:
        gcc_unreachable();  // Should never reach here
        break;
    default:
        // Handle unexpected type values
        fprintf(stderr, "Unknown type: %d\n", type);
        break;
    }
}
