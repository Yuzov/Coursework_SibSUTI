#ifndef STRINGS_
#define STRINGS_

int srchr(char ch, char *str);
int slen(char *str);
int plen(char *str);
int is_it_num(char sym);
int is_it_letter(char sym);
int stok(char *str, char delim, char **ptr);
int sspn(char *str, char *sym);
int scspn(char *str, char *sym);
int suntok(char *str, char delim, char **ptr, int cnt);
int scopy(char *str1, char *str2);
int scmp(char *str1, char *str2);
int stoi(char *str);
int sequal(char *str1, char *str2);
int sstr(char *txt, char **p);
int find_all_sym(char *str, char sym);
void fill_str(char *str, char *substr);
void scat(char *str1, char *str2);

#endif