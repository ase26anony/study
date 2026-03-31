    case UNORDERED:
      fputs ("unord", file);
      break;
    case ORDERED:
      fputs ("ord", file);
      break;
    case UNEQ:
      fputs ("ueq", file);
      break;
    case UNGE:
      fputs ("nlt", file);
      break;
    case UNGT:
      fputs ("nle", file);
      break;
    case UNLE:
      fputs ("ule", file);
      break;
    case UNLT:
      fputs ("ult", file);
      break;
    case LTGT:
      fputs ("une", file);
      break;
    default:
      output_operand_lossage ("operand is not a condition code, "
                              "invalid condition code");
      break;
