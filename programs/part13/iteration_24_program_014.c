switch (n) {
    case ENTRY_BLOCK:
        fputs("ENTRY", file);
        break;
    case ENTRY_BLOCK + 1:
        fputs("ENTRY''", file);
        break;
    case 2 * EXIT_BLOCK:
        fputs("EXIT", file);
        break;
    case 2 * EXIT_BLOCK + 1:
        fputs("EXIT''", file);
        break;
    case fixup_graph->new_exit_index:
        fputs("NEW_EXIT", file);
        break;
    case fixup_graph->new_entry_index:
        fputs("NEW_ENTRY", file);
        break;
    default:
        // Print regular node number
        fprintf(file, "%d", n);
}
