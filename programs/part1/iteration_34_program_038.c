int main(int argc, char *argv[]) {
    int opt;
    
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
        case '?':
            // getopt already printed an error for invalid option
            fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
            exit(EXIT_FAILURE);
            break;
        default:
            fprintf(stderr, "unknown flag `%c'\n", opt);
            exit(EXIT_FAILURE);
        }
    }
    
    // Rest of program logic here
    // ...
}
