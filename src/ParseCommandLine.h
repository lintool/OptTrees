#ifndef PARSE_COMMAND_LINE_H_GUARD
#define PARSE_COMMAND_LINE_H_GUARD

int isPresentCL(int argc, char** argv, char* flag) {
  int i;
  for(i = 1; i < argc; i++) {
    if(!strcmp(argv[i], flag)) {
      return i;
    }
  }
  return 0;
}

char* getValueCL(int argc, char** argv, char* flag) {
  int i;
  for(i = 1; i < argc; i++) {
    if(!strcmp(argv[i], flag)) {
      return argv[i + 1];
    }
  }
  return NULL;
}

#endif
