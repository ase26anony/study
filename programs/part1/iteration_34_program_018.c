   default:
     fprintf(stderr, "unknown flag `%c'\n", opt);
     print_usage();  // Show help after error
     exit(EXIT_FAILURE);  // Exit with error code
