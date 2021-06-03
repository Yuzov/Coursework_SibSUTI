int srchr(char ch, char* str)
{
    int i, idx = -1;
    for (i = 0; (str[i] != '\0') && (str[i] != ch); i++)
        ;
    if (str[i] == ch)
        idx = i;
    return idx;
}

int slen(char* str)
{
    int i, len = 0;
    for (i = 0; str[i] != '\0'; i++)
        len++;
    return len;
}

int plen(char* str)
{
    int i, len = 0;
    for (i = 0; str[i] != ':'; i++)
        len++;
    return len;
}

int is_it_num(char sym)
{
    if (sym > '9' || sym < '0')
        return -1;
    return 0;
}

int is_it_letter(char sym)
{
    if (sym > 'z' || sym < 'a')
        return -1;
    return 0;
}

int stok(char* str, char delim, char** ptr)
{
    char* suf = str;
    *ptr = str;       // первое поле – начало str
    int i = 0, j = 0; // j – счетчик полей
    while (suf[0] != '\0') {
        if ((i = srchr(delim, suf)) < 0) {
            *(ptr + j) = suf;
            return j;
        } else {
            suf[i] = '\0';
            *(ptr + j) = suf;
            j++;
            suf = suf + i + 1;
        }
    }
    return j;
}

int sspn(char* str, char* sym)
{
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        if (srchr(*(str + i), sym) < 0) {
            break;
        }
    }
    return i;
}

int scspn(char* str, char* sym)
{
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        if (srchr(*(str + i), sym) >= 0) {
            break;
        }
    }
    return i;
}

int suntok(char* str, char delim, char** ptr, int cnt)
{
    int i;
    for (i = 1; i < cnt; i++) {
        if (**(ptr + i) == '\0') {
            **(ptr + i) = delim;
        }
        //**(ptr + i - 1) = delim;
    }
    return 0;
}

int scopy(char* str1, char* str2)
{
    int i;
    for (i = 0; *(str1 + i) != '\0'; i++)
        *(str2 + i) = *(str1 + i);
    *(str2 + i) = '\0';
    return 0;
}

int scmp(char* str1, char* str2)
{
    int i, flg = 0;
    for (i = 0; (*(str1 + i) == *(str2 + i)) && *(str1 + i) != '\0'
         && *(str2 + i) != '\0';
         i++)
        ;
    if (*(str1 + i) < *(str2 + i))
        flg = -1; // если строки
    else if (*(str1 + i) > *(str2 + i))
        flg = 1; // разной длины
    return flg;
}

int stoi(char* str)
{
    int minus = 1, number = 0, i = 0;
    if (str[0] == '-') {
        minus = -1;
        i++;
    }
    while (str[i] != '\0') {
        if (is_it_num(str[i]) == 0) {
            number = number * 10 + str[i] - '0';
            i++;
        } else
            return -1;
    }
    return minus * number;
}

int sequal(char* str1, char* str2)
{
    int i, flg = 1;
    for (i = 0; flg && (str1[i] != '\0' || str2[i] != '\0'); i++) {
        if (str1[i] != str2[i])
            flg = 0;
    }
    return flg;
}

int sstr(char* txt, char** p)
{
    char* suf = txt;
    int len = slen(*p);
    int i, pos = -1;
    while (((i = srchr(**p, suf)) >= 0) && (pos < 0)) {
        char tmp;
        suf = suf + i;
        tmp = suf[len];
        suf[len] = '\0';
        if (sequal(suf, *p) == 1) { // посимвольное сравнение строк
            pos = suf - txt; // разность указателей = индекс
        }
        suf[len] = tmp;
        suf++;
    }
    return pos;
}

int find_all_sym(char* str, char sym)
{
    int cnt = 0;
    for (int i = 0; i < slen(str); i++) {
        if (str[i] == sym) {
            cnt++;
        }
    }
    return cnt;
}

void fill_str(char* str, char* substr)
{
    int i, end;
    char* suf = str + 2;
    end = srchr('\\', suf);
    for (i = 2; i <= end + 1; i++) {
        substr[i - 2] = str[i];
    }
    substr[i - 2] = '\0';
}

void scat(char* str1, char* str2)
{
    int len = slen(str1);
    char* suf = str1 + len;
    scopy(str2, suf);
}