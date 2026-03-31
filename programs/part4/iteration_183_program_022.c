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
    case EQ:
      fputs ("eq", file);
      break;
    case NE:
      fputs ("ne", file);
      break;
    case GE:
      fputs ("ge", file);
      break;
    case GT:
      fputs ("gt", file);
      break;
    case LE:
      fputs ("le", file);
      break;
    case LT:
      fputs ("lt", file);
      break;
    default:
      output_operand_lossage ("operand is not a condition code, "
                              "invalid floating-point comparison");
      break;
  }
