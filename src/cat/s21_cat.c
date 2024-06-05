#include "s21_cat.h"

// Function declarations
void process_text(FILE *input, int flags);
int process_line(char *buffer, int flags, int *lastBlank, int *lineNumber);
void number_line(char *line, int *lineNumber);
void handle_flag(int *flags, int option, FlagPair *valueArr);

int main(int argc, char **argv) {
  int flags = 0;
  const char *shortOpts = "benstvTE?";
  const struct option longOpts[] = {{"number-nonblank", no_argument, NULL, 'b'},
                                    {"number", no_argument, NULL, 'n'},
                                    {"squeeze-blank", no_argument, NULL, 's'}};

  flags = read_flags(argc, argv, shortOpts, longOpts, NULL, &handle_flag);
  int currentFile = optind;
  int isStdin = currentFile == argc;

  while (isStdin) {
    FILE *input = stdin;
    process_text(input, flags);
    isStdin = 0;  // To break the loop after processing stdin
  }

  while (currentFile < argc) {
    FILE *input = fopen(argv[currentFile], "rb");
    if (!input) {
      fprintf(stderr, "%s: %s: No such file or directory\n", argv[0],
              argv[currentFile]);
      exit(1);
    }
    process_text(input, flags);
    fclose(input);
    currentFile++;
  }

  return 0;
}

void process_text(FILE *input, int flags) {
  char buffer[BUFFER_SIZE];
  int lastBlankLine = 0;
  int lineNumber = 1;

  while (fgets(buffer, BUFFER_SIZE, (input == NULL ? stdin : input))) {
    if (input == stdin) lineNumber = 1;
    if (process_line(buffer, flags, &lastBlankLine, &lineNumber))
      printf("%s", buffer);
  }
}

int process_line(char *buffer, int flags, int *lastBlank, int *lineNumber) {
  int length = strlen(buffer);
  buffer[length] = '\0';
  if (flags & v_f) replace_32_128chars(buffer);
  if (flags & t_f) {
    replace_char(buffer, "\t", "^I");
  }
  if (flags & s_f) {
    int isCurrentLineBlank = (strlen(buffer) <= 1);
    if (*lastBlank && isCurrentLineBlank) return 0;
    *lastBlank = isCurrentLineBlank;
  }
  if (flags & n_f) {
    if (flags & b_f) {
      flags ^= n_f;
    } else {
      number_line(buffer, lineNumber);
    }
  }
  if (flags & b_f && !(flags & n_f)) {
    if (strlen(buffer) >= 1 && buffer[0] != '\n') {
      number_line(buffer, lineNumber);
    }
  }
  if (flags & e_f) {
    length = strlen(buffer);
    if (buffer[length - 1] == '\n') {
      buffer[length - 1] = '$';
      strcat(buffer, "\n");
    }
  }
  return 1;
}

void number_line(char *line, int *lineNumber) {
  char *temp = calloc(strlen(line) + 1, sizeof(char));
  strcpy(temp, line);
  line[0] = '\0';
  sprintf(line, "%*d\t", 6, (*lineNumber)++);
  strcat(line, temp);
  free(temp);
}

void handle_flag(int *flags, int option, FlagPair *valueArr) {
  if (!valueArr) valueArr = NULL;
  switch (option) {
    case 'b':
      *flags |= b_f;
      break;
    case 'e':
      *flags |= e_f | v_f;
      break;
    case 'E':
      *flags |= e_f;
      break;
    case 'n':
      *flags |= n_f;
      break;
    case 's':
      *flags |= s_f;
      break;
    case 't':
      *flags |= t_f | v_f;
      break;
    case 'T':
      *flags |= t_f;
      break;
    case 'v':
      *flags |= v_f;
      break;
    case '?':
      printf("usage: s21_cat [-benstvTE?] [file ...]\n");
      exit(1);
  }
}
