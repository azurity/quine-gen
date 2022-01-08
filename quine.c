#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int file_size(FILE *file)
{
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

int split(char *buffer, char *delim, char ***output)
{
    char *ptr = buffer;
    int count = 0;
    int i = 0;
    while (ptr && *ptr)
    {
        ptr = strpbrk(ptr, delim);
        if (ptr)
            ptr += strspn(ptr, delim);
        count += 1;
    }
    *output = malloc(count * sizeof(char *));
    char *token = strtok(buffer, delim);
    while (token)
    {
        (*output)[i++] = token;
        token = strtok(NULL, delim);
    }
    return count;
}

void trim(char *ptr)
{
    while (*ptr == 0 || *ptr == ' ' || *ptr == '\n')
    {
        *ptr = 0;
        ptr--;
    }
}

char *readfile(char *filename)
{
    FILE *file = fopen(filename, "r");
    int size = file_size(file);
    char *buffer = malloc(size + 1);
    fread(buffer, size, 1, file);
    buffer[size] = 0;
    fclose(file);
    return buffer;
}

void writefile(char *filename, char *data)
{
    FILE *file = fopen(filename, "w");
    fputs(data, file);
    fclose(file);
}

typedef struct _name_list_node
{
    char *name;
    struct _name_list_node *next;
} name_list_node;

void help()
{
    char *data = readfile("help.txt");
    puts(data);
    free(data);
}

char **dict_get(char ***dict, int size, char *key)
{
    for (int i = 0; i < size; i++)
    {
        if (strcmp(dict[i][0], key) == 0)
            return dict[i];
    }
    return 0;
}

char *safecatc(char *buffer, int *size, char cat)
{
    int pos = strlen(buffer);
    if (pos + 1 >= *size)
    {
        buffer = realloc(buffer, *size + 4096);
        buffer[*size] = 0;
        *size += 4096;
    }
    buffer[pos] = cat;
    buffer[pos + 1] = 0;
    return buffer;
}

char *safecat(char *buffer, int *size, char *cat)
{
    if (strlen(buffer) + strlen(cat) >= size)
    {
        buffer = realloc(buffer, *size + 4096);
        buffer[*size] = 0;
        *size += 4096;
    }
    strcat(buffer, cat);
    return buffer;
}

char *render(char *template, char ***details, int size)
{
    int bsize = 4096;
    char *buffer = malloc(bsize);
    buffer[0] = 0;
    int i = 0;
    int len = strlen(template);
    while (i < len)
    {
        char c = template[i];
        if (c == '$')
        {
            int j = 1;
            while (template[i + j] != '$')
                j += 1;
            char *name = malloc(j);
            memcpy(name, &(template[i + 1]), j - 1);
            name[j - 1] = 0;
            char **item = dict_get(details, size, name);
            buffer = safecatc(buffer, &bsize, '"');
            buffer = safecat(buffer, &bsize, item[2]);
            buffer = safecat(buffer, &bsize, dict_get(details, size, item[5])[4]);
            buffer = safecatc(buffer, &bsize, '"');
            i += j;
        }
        else
        {
            buffer = safecatc(buffer, &bsize, c);
        }
        i += 1;
    }
    return buffer;
}

void build(char *argv[])
{
    char *buffer = readfile(argv[2]);
    char **lines;
    int ln = split(buffer, "\n", &lines);
    char ***langs = malloc(sizeof(char **) * ln);
    memset(langs, 0, sizeof(char **) * ln);
    int count = 0;
    for (int i = 0; i < ln; i++)
    {
        if (strlen(lines[i]) == 0)
            continue;
        char **parts;
        int pn = split(lines[i], "#", &parts);
        langs[count++] = parts;
    }
    free(lines);
    char ***details = malloc(sizeof(char **) * count);
    for (int i = 0; i < count; i++)
    {
        details[i] = malloc(sizeof(char *) * 6);
        details[i][0] = langs[i][0];
        char *cmd = malloc(strlen(langs[i][1]) + 10 + strlen(argv[2]) + 8 + 1);
        strcpy(cmd, langs[i][1]);
        strcat(cmd, " template ");
        strcat(cmd, argv[2]);
        strcat(cmd, " > otemp");
        system(cmd);
        free(cmd);
        details[i][1] = readfile("otemp");
        trim(details[i][1] + strlen(details[i][1]));
        system("rm otemp");
    }
    for (int i = 0; i < count; i++)
    {
        writefile("itemp", details[(i + 1) % count][1]);
        char *cmd = malloc(strlen(langs[i][1]) + 5 + 16 + 1);
        strcpy(cmd, langs[i][1]);
        strcat(cmd, " exec");
        strcat(cmd, " > otemp < itemp");
        system(cmd);
        free(cmd);
        char *output = readfile("otemp");
        system("rm itemp");
        system("rm otemp");
        char *ptr = strpbrk(output, "\n");
        *ptr++ = 0;
        trim(ptr + strlen(ptr));
        details[i][2] = output;
        details[i][3] = ptr;
        //
        writefile("itemp", details[i][3]);
        cmd = malloc(strlen(langs[(i - 1 + count) % count][1]) + 12 + 16 + 1);
        strcpy(cmd, langs[(i - 1 + count) % count][1]);
        strcat(cmd, " anti-escape");
        strcat(cmd, " > otemp < itemp");
        system(cmd);
        free(cmd);
        char *antied = readfile("otemp");
        system("rm itemp");
        system("rm otemp");
        trim(antied + strlen(antied));
        details[i][4] = antied;
        details[i][5] = details[(i + 1) % count][0];
    }
    char *code = render(details[0][1], details, count);
    FILE *codefile = fopen(argv[3], "w");
    fwrite(code, strlen(code), 1, codefile);
    free(code);
    fprintf(codefile, "%s\n", details[0][3]);
    fclose(codefile);
    printf("try run: cat %s", argv[3]);
    for (int i = 0; i < count; i++)
    {
        printf("|%s", langs[i][2]);
    }
    printf("|diff %s -\n", argv[3]);
    for (int i = 0; i < count; i++)
    {
        free(langs[i]);
        free(details[i][1]);
        free(details[i][2]);
        free(details[i][4]);
        free(details[i]);
    }
    free(langs);
    free(details);
    free(buffer);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        return -1;
    if (strcmp(argv[1], "template") == 0)
    {
        if (argc < 3)
            return -1;
        char *buffer = readfile(argv[2]);
        puts("#include<stdio.h>");
        char **lines;
        int ln = split(buffer, "\n", &lines);
        for (int i = 0; i < ln; i++)
        {
            if (strlen(lines[i]) == 0)
                continue;
            char **parts;
            int pn = split(lines[i], "#", &parts);
            printf("char*%s=$%s$;", parts[0], parts[0]);
            free(parts);
        }
        putchar('\n');
        free(lines);
        free(buffer);
    }
    else if (strcmp(argv[1], "exec") == 0)
    {
        char c;
        char *total_str = malloc(4096);
        char *cptr = total_str;
        while ((c = getchar()) != EOF)
        {
            *cptr++ = c;
        }
        *cptr = 0;
        trim(cptr);
        name_list_node head;
        name_list_node *tail = &head;
        int len = strlen(total_str);
        for (int i = 0; i < len; i++)
        {
            c = total_str[i];
            switch (c)
            {
            case '"':
                printf("%%c");
                tail->next = malloc(sizeof(name_list_node));
                tail = tail->next;
                tail->name = malloc(3);
                strcpy(tail->name, "34");
                tail->next = NULL;
                break;
            case '\n':
                printf("%%c");
                tail->next = malloc(sizeof(name_list_node));
                tail = tail->next;
                tail->name = malloc(3);
                strcpy(tail->name, "10");
                tail->next = NULL;
                break;
            case '$':
            {
                char *cache = malloc(64);
                strcpy(cache, "34,");
                char *ptr = &(cache[3]);
                int j = 1;
                while (total_str[i + j] != '$')
                {
                    *ptr++ = total_str[i + j];
                    j += 1;
                }
                i += j;
                strcpy(ptr, ",34");
                printf("%%c%%s%%c");
                tail->next = malloc(sizeof(name_list_node));
                tail = tail->next;
                tail->name = cache;
                tail->next = NULL;
            }
            break;
            default:
                putchar(c);
            }
        }
        putchar('\n');
        printf("int main(){printf(c");
        name_list_node *ptr = head.next;
        while (ptr != NULL)
        {
            printf(",%s", ptr->name);
            free(ptr->name);
            name_list_node *temp = ptr;
            ptr = ptr->next;
            free(temp);
        }
        puts(");putchar(10);}");
        free(total_str);
    }
    else if (strcmp(argv[1], "anti-escape") == 0)
    {
        char c;
        char *total_str = malloc(4096);
        char *cptr = total_str;
        while ((c = getchar()) != EOF)
        {
            *cptr++ = c;
        }
        *cptr = 0;
        trim(cptr);
        name_list_node head;
        name_list_node *tail = &head;
        int len = strlen(total_str);
        for (int i = 0; i < len; i++)
        {
            c = total_str[i];
            switch (c)
            {
            case '%':
                printf("%%%%");
                break;
            default:
                putchar(c);
            }
        }
        putchar('\n');
        free(total_str);
    }
    else if (strcmp(argv[1], "build") == 0)
    {
        build(argv);
    }
    else if (strcmp(argv[1], "help") == 0)
    {
        help();
    }
    else
    {
        puts("unkown opntions, use \"help\" to get help.");
        help();
    }
    return 0;
}
