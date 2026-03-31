case 'h':
  print_usage();
  exit(EXIT_SUCCESS);  // Exit after showing help
  break;
// ... other cases ...
default:
  fprintf(stderr, "Error: unknown option `%c'\n", opt);
  fprintf(stderr, "Try '%s -h' for more information.\n", program_name);
  exit(EXIT_FAILURE);  // Exit with error code
