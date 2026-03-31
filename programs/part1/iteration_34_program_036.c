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
  fprintf(stderr, "Error: Unknown option `-%c'\n", opt);
  fprintf(stderr, "Try `%s -h' for more information.\n", program_name);
  exit(EXIT_FAILURE);
  break;
