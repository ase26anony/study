int main(int argc, char **argv) {
    int opt;
    
    // Initialize flags
    flag_dump_contents = 0;
    flag_dump_positions = 0;
    flag_dump_raw = 0;
    flag_dump_stable = 0;
    
    // Process command-line options
    while ((opt = getopt(argc, argv, "hvlprs")) != -1) {
        switch (opt) {
        case 'h':
            print_usage();
            exit(EXIT_SUCCESS);
            break;
        case 'v':
            print_version();
            exit(EXIT_SUCCESS);
            break;
        case 'l':
            flag_dump_contents = 1;
            break;
        case 'p':
            flag_dump_positions = 1;
            break;
        case 'r':
            flag_dump_raw = 1;
            break;
        case 's':
            flag_dump_stable = 1;
            break;
        default:
            fprintf(stderr, "unknown flag `%c'\n", opt);
            print_usage();
            exit(EXIT_FAILURE);
        }
    }
    
    // Handle non-option arguments (if any)
    if (optind < argc) {
        // Process remaining arguments starting at argv[optind]
        for (int i = optind; i < argc; i++) {
            printf("Non-option argument: %s\n", argv[i]);
        }
    }
    
    // Main program logic here...
    
    return 0;
}
