int main(int argc, char *argv[]) {
    int opt;
    int flag_dump_contents = 0;
    int flag_dump_positions = 0;
    int flag_dump_raw = 0;
    int flag_dump_stable = 0;
    
    while ((opt = getopt(argc, argv, "hvlprs")) != -1) {
        switch (opt) {
        case 'h':
            print_usage();
            break;
        case 'v':
            print_version();
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
            return 1;
        }
    }
    
    // Rest of the program logic here
    // ...
    
    return 0;
}
