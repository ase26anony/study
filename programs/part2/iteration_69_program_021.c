switch (current->type) {
    case TYPE_NONE:
        gcc_unreachable();  // Should never happen
    case TYPE_UNDEFINED:
        write_state_undefined_type(current);
        break;
    // ... other type handlers
}
