#include <stdio.h>

#define DEBUGFS_TS "/sys/kernel/debug/laba/thread_struct"
#define DEBUGFS_PD "/sys/kernel/debug/laba/pci_dev"


void my_printf(FILE* fp) {
  char current;
  while((current = fgetc(fp)) != EOF) putchar(current);
}

int main(int argc, char *argv[]) {

    if(strcmp(argv[1], "0") == 0) { // thread_struct case
        FILE *tsf;
        tsf = fopen(DEBUGFS_TS, "w+");
        fprintf(tsf, argv[2]);
        my_printf(tsf);
        fclose(tsf);
    }

    if(strcmp(argv[1], "0") == 1) { // pci_dev case
        FILE *tsf;
        tsf = fopen(DEBUGFS_PD, "w+");
        fprintf(tsf, argv[2]);
        my_printf(tsf);
        fclose(tsf);
    }

    return 0;
}