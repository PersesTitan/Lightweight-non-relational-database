#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include <unistd.h>

#ifdef _WIN32
char fileName[255] = "..\\data_file\\";
#else
char fileName[255] = "../data_file/";
#endif

typedef enum {
    INT, FLOAT, TEXT
} DataType;

void sort(void***, int, int);
char* data_to_string(DataType);
void data_print(int, const DataType*, void**);
int create_data_type(DataType*, char*);
void* create_data(DataType type, char*);
void create_table(DataType*, FILE*, int*);
void str_append(char*, char);
void search_text(regex_t*, const char*);
void menu_print();

// type kind
// int, float, text
int main(int argc, char* argv[]) {
    char temp[INT16_MAX];
    int type_size, data_size = 0;
    regex_t regex;
    // [[data], [data... data count], ...]
    void*** datas = malloc(0);
    DataType* type = malloc(0);
    FILE *file = NULL;

    printf("생성 또는 읽어올 파일명 입력 >>");
    scanf("%s", fileName + strlen(fileName));
    strcat(fileName, ".db");
    // 파일이 존재
    if (access(fileName, F_OK) != -1) {
        file = fopen(fileName, "a+");
        fseek(file, 0, SEEK_SET);
        create_table(type, file, &type_size);
        while (!feof(file)) {
            datas = realloc(datas, sizeof(void**) * (++data_size));
            datas[data_size-1] = calloc(data_size, sizeof(void*));
            for (int i = 0; i < type_size; ++i) {
                fscanf(file, "%s", temp);
                datas[data_size-1][i] = create_data(type[i], temp);
            }
        }
    } else {
        file = fopen(fileName, "w");
        create_table(type, stdin, &type_size);
        strcpy(temp, data_to_string(type[0]));
        for (int i = 1; i < type_size; ++i) {
            str_append(temp, ' ');
            strcat(temp, data_to_string(type[i]));
        }
        fputs(temp, file);
    }

    int index;
    int finish_check = 1;
    int option;
    menu_print();
    for (int i = 0; i < type_size; ++i) printf("%s ", data_to_string(type[i]));
    printf("\n");
    while (finish_check) {
        printf(">>");
        fflush(stdin);
        scanf("%d", &option);
        switch (option) {
            case 0:
                finish_check = 0;
                break;
            case 1:
                fputs("\n", file);
                datas = realloc(datas, sizeof(void**) * (data_size+1));
                datas[data_size] = calloc(data_size+1, sizeof(void*));
                for (int i = 0; ; ++i) {
                    fscanf(stdin, "%s", temp);
                    fputs(temp, file);
                    datas[data_size][i] = create_data(type[i], temp);
                    if (i >= type_size-1) break;
                    fputs(" ", file);
                }
                data_size++;
                break;
            case 2:
                scanf("%d %s", &index, temp);
                sort(datas, index, data_size);
                search_text(&regex, temp);
                for (int i = 0; i < data_size; ++i) {
                    void** data = datas[i];
                    if (!regexec(&regex, (char*)data[index], 0, NULL, 0)) {
                        data_print(type_size, type, datas[i]);
                        printf("\n");
                    }
                }
                break;
            default:
                break;
        }
    }

    // finish
    fclose(file);
    for (int i = 0; i < data_size; ++i) {
        for (int j = 0; j < type_size; ++j) {
            if (type[j] != TEXT) free(datas[i][j]);
        }
        free(datas[i]);
    }

    regfree(&regex);
    free(type);
    free(datas);
    return 0;
}

void sort(void*** datas, int index, int data_size) {
    void** void_temp;
    for (int i = 0; i < data_size; i++) {
        for (int j = 0; j < i; j++) {
            char *text1 = (char *)datas[i][index];
            char *text2 = (char *)datas[j][index];
            if (strcmp(text1, text2) < 0) {
                void_temp = datas[i];
                datas[i] = datas[j];
                datas[j] = void_temp;
            }
        }
    }
}

char* data_to_string(DataType type) {
    switch (type) {
        case INT: return "INT";
        case FLOAT: return "FLOAT";
        case TEXT: return "TEXT";
        default: return NULL;
    }
}

int create_data_type(DataType* dataType, char* text) {
    if (!strcmp(text, "INT")) *dataType = INT;
    else if (!strcmp(text, "FLOAT")) *dataType = FLOAT;
    else if (!strcmp(text, "TEXT")) *dataType = TEXT;
    else return 1;
    return 0;
}

void data_print(int type_size, const DataType *type, void** data) {
    for (int j = 0; j < type_size; ++j) {
        switch (type[j]) {
            case INT:
                printf("%d ", *((int*) data[j]));
                break;
            case FLOAT:
                printf("%.2lf ", *((double*) data[j]));
                break;
            case TEXT:
                printf("%s ", ((char*) data[j]));
                break;
        }
    }
}

void create_table(DataType* dataType, FILE* file, int* type_size) {
    int finish_check = 1; // 정상 종료 체크
    int finish_check_in = 1;
    char text[256];
    int size;
    while (finish_check) {
        size = 0;
        if (file == stdin) {
            printf("데이터 타입 입력 (INT, FLOAT, TEXT) >>");
            fflush(stdin);
        }
        fgets(text, 256, file);
        if (text[strlen(text)-1] == '\n') text[strlen(text)-1] = '\0';
        char *stp = strtok(text, " ");
        while (stp != NULL) {
            dataType = realloc(dataType, sizeof(DataType) * ++size);
            int len = (int) strlen(stp);
            for (int i = 0; i < len; ++i) stp[i] = (char) toupper(stp[i]);
            if (create_data_type(dataType+size-1, stp) && file == stdin) {
                printf("데이터 타입이 유효하지 않습니다. 다시 입력해주세요. \n");
                finish_check_in = 0;
                break;
            }
            stp = strtok(NULL, " ");
        }
        if (finish_check_in) finish_check = 0;
    }

    *type_size = size;
}

void* create_data(DataType type, char* data) {
    char* endPtr = calloc(strlen(data)+1, sizeof(char));
    int *i_p = NULL;
    double *f_p = NULL;
    char *c_p = NULL;
    switch (type) {
        case INT:
            i_p = malloc(sizeof(int));
            *i_p = (int) strtol(data, &endPtr, 10);
            return i_p;
        case FLOAT:
            f_p = malloc(sizeof(double));
            *f_p = (double) strtod(data, &endPtr);
            return f_p;
        case TEXT:
            c_p = calloc(strlen(data) + 1, sizeof(char));
            strcpy(c_p, data);
            return c_p;
        default:
            return NULL;
    }
}

void str_append(char* text, char c) {
    int len = (int) strlen(text);
    text[len] = c;
    text[len+1] = '\0';
}

void search_text(regex_t* regex, const char* text) {
    char find[INT16_MAX];
    const int len = (int) strlen(text) + 1;
    find[0] = '\0';
    for (int i = 0; i < len; ++i) {
        if (text[i] == '*') {
            strcat(find, ".*");
        } else str_append(find, text[i]);
    }
    regcomp(regex, find, REG_EXTENDED);
}

void menu_print() {
    printf("프로그램 종료 : 0\n");
    printf("객체 생성 : 1 [생성될 객체]\n");
    printf("객체 조회 : 2 [인덱스, 검색: 조건]\n");
    printf("=============================\n");
}