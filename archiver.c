#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
union record {
  char charptr[512];
  struct header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[8];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
  } header;
};
int change_base(int n) {
  int num = n;
  int val = 0;
  int b = 1;
  while (num) {
    int c = num % 10;
    num = num / 10;
    val = val + c * b;

    b = b * 8;
  }

  return val;
}
void mode(union record *x, char copie_linie[512]) {
  int suma = 0, i;
  for (i = 1; i <= 3; i++) {
    if (copie_linie[i] == 'r') {
      suma = suma + 400;
    }
    if (copie_linie[i] == 'w') {
      suma = suma + 200;
    }
    if (copie_linie[i] == 'x') {
      suma = suma + 100;
    }
  }
  for (i = 4; i <= 6; i++) {
    if (copie_linie[i] == 'r') {
      suma = suma + 40;
    }
    if (copie_linie[i] == 'w') {
      suma = suma + 20;
    }
    if (copie_linie[i] == 'x') {
      suma = suma + 10;
    }
  }
  for (i = 7; i <= 9; i++) {
    if (copie_linie[i] == 'r') {
      suma = suma + 4;
    }
    if (copie_linie[i] == 'w') {
      suma = suma + 2;
    }
    if (copie_linie[i] == 'x') {
      suma = suma + 1;
    }
  }
  sprintf(x->header.mode, "%07d", suma);
}
void sum_bytes(union record *x) {
  long int s = 0;
  int i;
  for (i = 0; i < 512; i++) {
    s += x->charptr[i];
  }
  sprintf(x->header.chksum, "%07lo", s);
}
void time_name_magic(union record *x, struct tm timp, char linie[512]) {
  char *cuv;
  int r;
  cuv = strtok(linie, " ");
  cuv = strtok(NULL, " ");
  cuv = strtok(NULL, " ");
  strcpy(x->header.uname, cuv);
  cuv = strtok(NULL, " ");
  strcpy(x->header.gname, cuv);
  cuv = strtok(NULL, " ");
  sprintf(x->header.size, "%011lo", atol(cuv));
  cuv = strtok(NULL, " .:-");
  timp.tm_year = atoi(cuv) - 1900;
  cuv = strtok(NULL, " .:-");
  timp.tm_mon = atoi(cuv) - 1;
  cuv = strtok(NULL, " .:-");
  timp.tm_mday = atoi(cuv);
  cuv = strtok(NULL, " .:-");
  timp.tm_hour = atoi(cuv);
  cuv = strtok(NULL, " .:-");
  timp.tm_min = atoi(cuv);
  cuv = strtok(NULL, " .:-");
  timp.tm_sec = atoi(cuv);
  r = mktime(&timp);
  sprintf(x->header.mtime, "%011o", r);
  cuv = strtok(NULL, " ");
  cuv = strtok(NULL, " ");
  cuv = strtok(NULL, " ");
  cuv[strlen(cuv) - 1] = '\0';
  strcpy(x->header.name, cuv);
  strcpy(x->header.linkname, cuv);
  strcpy(x->header.magic, "GNUtar ");
}
void completare_arhiva_date(union record *x, char nume_arhiva[500]) {
  char ch;
  int dimn, mx;
  FILE *input = fopen(x->header.name, "rb");
  FILE *out = fopen(nume_arhiva, "ab");
  fwrite(x, sizeof(union record), 1, out);
  while (fread(&ch, sizeof(char), 1, input) > 0) {
    fwrite(&ch, sizeof(char), 1, out);
  }
  dimn = change_base(atol(x->header.size));
  mx = 0;
  while (mx < dimn) {
    mx = mx + 512;
  }
  mx = mx - dimn;
  while (mx > 0) {
    fwrite("\0", sizeof(char), 1, out);
    mx--;
  }
  fclose(input);
  fclose(out);
}
void list(union record *x, char nume_arhiva[500], char *cuv) {
  int numar = 0, a;
  char ch;
  while (cuv != NULL && numar < 2) {
    numar++;
    cuv = strtok(NULL, " ");
    if (numar == 1) {
      strcpy(nume_arhiva, cuv);
    }
  }
  numar--;
  if (numar != 1) {
    printf("> Wrong command!\n");
  } else {
    FILE *in = fopen(nume_arhiva, "rb");
    if (in != NULL) {
      while (fread(x, sizeof(union record), 1, in) > 0) {
        if (strcmp(x->header.name, "\0") != 0) {
          printf("> %s\n", x->header.name);
        }
        a = change_base(atol(x->header.size));
        while (a % 512 != 0) {
          a++;
        }
        while (a > 0) {
          fread(&ch, sizeof(char), 1, in);
          a--;
        }
      }
    }
    fclose(in);
  }
}
void extract(union record *x, char *cuv) {
  int a, numar = 0, gasit;
  char caracter, nume_fisier[50], nume_arhiva[500];
  char fisier_extracted[50];
  while (cuv != NULL && numar < 3) {
    numar++;
    cuv = strtok(NULL, " ");
    if (numar == 1) {
      strcpy(nume_fisier, cuv);
    }
    if (numar == 2) {
      strcpy(nume_arhiva, cuv);
    }
  }
  numar--;
  if (numar != 2) {
    printf("> Wrong command!\n");
  } else {
    FILE *in = fopen(nume_arhiva, "rb");
    gasit = 0;
    while (gasit == 0 && fread(x, sizeof(union record), 1, in) > 0) {
      if (strcmp(x->header.name, nume_fisier) == 0) {
        gasit = 1;
        strcpy(fisier_extracted, "extracted_");
        strcat(fisier_extracted, nume_fisier);
        FILE *out = fopen(fisier_extracted, "wb");
        a = change_base(atol(x->header.size));
        while (a > 0) {
          fread(&caracter, sizeof(char), 1, in);
          fwrite(&caracter, sizeof(char), 1, out);
          a--;
        }
        fclose(out);
        printf("> File extracted!\n");
      }
    }
    fclose(in);
    if (gasit == 0) {
      printf("> File not found!\n");
    }
  }
}
void create(union record *x, struct tm timp, char *cuv) {
  int numar = 0, i;
  char nume_arhiva[500], nume_director[500], linie[512], lin[512];
  while (cuv != NULL && numar < 3) {
    numar++;
    cuv = strtok(NULL, " ");
    if (numar == 1) {
      strcpy(nume_arhiva, cuv);
    }
    if (numar == 2) {
      strcpy(nume_director, cuv);
    }
  }
  numar--;
  if (numar != 2) {
    printf("> Wrong command!\n");
  } else {
    FILE *in = fopen("files.txt", "r");
    while (fgets(linie, 512, in) != 0) {
      memset(x, 0, 512);
      memset(&timp, 0, sizeof(struct tm));
      memset(x->header.chksum, ' ', 8);
      mode(x, linie);
      time_name_magic(x, timp, linie);
      FILE *f = fopen("usermap.txt", "r");
      while (fgets(lin, 512, f) != 0) {
        cuv = strtok(lin, ":");
        if (strcmp(cuv, x->header.uname) == 0) {
          cuv = strtok(NULL, ":");
          cuv = strtok(NULL, ":");
          strcpy(x->header.uid, cuv);
          sprintf(x->header.uid, "%07lo", atol(x->header.uid));
          cuv = strtok(NULL, ":");
          strcpy(x->header.gid, cuv);
          sprintf(x->header.gid, "%07lo", atol(x->header.gid));
        }
      }
      fclose(f);
      sum_bytes(x);
      completare_arhiva_date(x, nume_arhiva);
    }
    fclose(in);
    FILE *out = fopen(nume_arhiva, "ab");
    for (i = 1; i <= 512; i++) {
      fwrite("\0", sizeof(char), 1, out);
    }
    fclose(out);
    printf("> Done!\n");
  }
}
int main() {
  int ok = 1;
  struct tm timp;
  union record x;
  char *cuv, nume_arhiva[500], comanda[50];
  while (ok != 0 && fgets(comanda, 50, stdin) != 0) {
    comanda[strlen(comanda) - 1] = '\0';
    if (strcmp(comanda, "exit") != 0) {
      cuv = strtok(comanda, " ");
      if (strcmp(cuv, "create") == 0) {
        create(&x, timp, cuv);
      } else if (strcmp(cuv, "list") == 0) {
        list(&x, nume_arhiva, cuv);
      } else if (strcmp(cuv, "extract") == 0) {
        extract(&x, cuv);
      }
    } else {
      ok = 0;
    }
  }
  return 0;
}
