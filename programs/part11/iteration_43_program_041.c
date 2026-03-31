case 'h':
  print_usage();
  exit(EXIT_SUCCESS);  // Add exit after showing help
  break;
// ... other cases ...
default:
  fprintf(stderr, "unknown flag `%c'\n", opt);
  print_usage();  // Show usage on error
  exit(EXIT_FAILURE);  // Exit with error code
  break;
