#include "auth.h"

/* Funcția care citește într-un vector de șiruri folosind un fișier */
linked_list *read_file_char_array(FILE *file, string_l **char_array,
                                  u_int *len)
{
    char file_user_line[MAX_LEN_LINE];
    int c, index = 0, count = 0;

    /* list reprezintă lista ce reține în mod unic ID-urile utilizatorilor */
    linked_list *list = init_list();
    list_node *iter = NULL;

    if (!file)
    {
        printf("Can't open file in read_file_char_array function\n");
        exit(1);
    }
    for (c = getc(file); c != EOF; c = getc(file))
        if (c == '\n')
            count = count + 1;
    count++;
    fseek(file, 0, SEEK_SET);

    *char_array = (string_l *)calloc(count, sizeof(string_l));

    while (1)
    {
        if (!fgets(file_user_line, MAX_LEN_LINE, file))
            break;
        if (file_user_line[strlen(file_user_line) - 1] == '\n')
            file_user_line[strlen(file_user_line) - 1] = '\0';
        (*char_array)[index] =
            (char *)calloc(strlen(file_user_line) + 1, sizeof(char));
        strcpy((*char_array)[index], file_user_line);

        if (!list->head)
            add_list_node(list, strtok(file_user_line, ","));
        else
        {
            iter = list->head;
            char *parser = strtok(file_user_line, ",");
            while (iter)
            {
                /* Dacă ID-ul nu a mai fost adăugat, se formează un nou nod cu el */
                if (strcmp(iter->data, parser) && !iter->next)
                    add_list_node(list, parser);
                else if (!strcmp(iter->data, parser) && iter->next)
                    break;
                iter = iter->next;
            }
        }
        index++;
    }

    *len = index;

    fclose(file);

    return list;
}

/* Funcția care citește într-un listă înlănțuită folosind un fișier */
void read_file_list(FILE *file, linked_list *list, int type)
{

    char file_user_line[MAX_LEN_LINE];
    int num;

    if (!file)
    {
        printf("Can't open file in read_file_list function\n");
        exit(1);
    }

    /* Type-urile corespund tipurilor de fișiere (fișierul de permisiuni, etc.) citite
       de către server */
    if (type == 1 || type == 2)
    {
        /* Se folosește faptul că aceste tipuri de fișiere au, pe prima linie, numărul
           de linii */
        num = (char)fgetc(file) - '0';
        fgetc(file);
        while (num)
        {
            if (!fgets(file_user_line, MAX_LEN_LINE, file))
                break;
            if (file_user_line[strlen(file_user_line) - 1] == '\n')
                file_user_line[strlen(file_user_line) - 1] = '\0';
            add_list_node(list, file_user_line);
            num--;
        }
    }
    else if (type == 3)
    {
        while (1)
        {
            if (!fgets(file_user_line, MAX_LEN_LINE, file))
                break;
            if (file_user_line[strlen(file_user_line) - 1] == '\n')
                file_user_line[strlen(file_user_line) - 1] = '\0';
            add_list_node(list, file_user_line);
        }
    }
    else
        return;

    fclose(file);
}

/* Funcția care generează un jeton aleator pe baza unui ID / alt jeton */
char *generate_access_token(char *clientIdToken)
{
    char *token = (char *)malloc((TOKEN_LEN - 1) * sizeof(char *));
    int i, key, used[TOKEN_LEN - 1];
    int rotationIndex = TOKEN_LEN - 1;

    memset(used, 0, (TOKEN_LEN - 1) * sizeof(int));
    for (i = 0; i < TOKEN_LEN - 1; i++)
    {
        do
        {
            key = rand() % rotationIndex;
        } while (used[key] == 1);
        token[i] = clientIdToken[key];
        used[key] = 1;
    }
    token[TOKEN_LEN - 1] = '\0';
    return token;
}

/* Funcția de inițializare a unei liste */
linked_list *init_list()
{
    linked_list *new_list = (linked_list *)malloc(sizeof(linked_list));
    new_list->head = NULL;
    new_list->tail = new_list->head;
    return new_list;
}

/* Funcția de adăugare a unui nod în listă */
list_node *add_list_node(linked_list *list, char *data)
{
    if (!data || !list)
    {
        if (list)
            return list->tail;
        return NULL;
    }

    if (!list->head)
    {
        list->head = (list_node *)malloc(sizeof(list_node));
        list->head->data = (char *)calloc((strlen(data) + 1), sizeof(char));
        strcpy(list->head->data, data);
        list->head->next = NULL;
    }
    else
    {
        list_node *node;
        node = (list_node *)malloc(sizeof(list_node));
        node->data = (char *)calloc((strlen(data) + 1), sizeof(char));
        strcpy(node->data, data);

        if (!list->head->next)
        {
            list->head->next = node;
            node->prev = list->head;
            node->next = NULL;
        }
        else
        {
            list_node *iter = list->head;
            while (iter->next)
                iter = iter->next;
            iter->next = node;
            node->prev = iter;
            node->next = NULL;
        }
        list->tail = node;
        return node;
    }
    return NULL;
}

/* Funcția de ștergere a unui nod din listă */
int remove_list_node(linked_list *list, char *data)
{
    if (!data || !list)
    {
        return 0;
    }

    list_node *iter = list->head;
    while (iter)
    {
        if (!strcmp(iter->data, data))
        {
            if (iter == list->head)
            {
                if (!iter->next)
                {
                    free(iter->data);
                    free(iter);
                    list->head = NULL;
                }
                else
                {
                    list_node *aux = iter->next;
                    iter->next = NULL;
                    aux->prev = NULL;
                    free(iter->data);
                    free(iter);
                    list->head = aux;
                }
            }
            else if (!iter->next && iter->prev)
            {
                iter->prev->next = NULL;
                list->tail = iter->prev;
                free(iter->data);
                free(iter);
            }
            else
            {
                iter->prev->next = iter->next;
                iter->next->prev = iter->prev;
                free(iter->data);
                free(iter);
            }
            return 1;
        }
        iter = iter->next;
    }
    return 0;
}

/* Funcția de eliminare din memorie a unei liste */
void destroy_list(linked_list *list)
{
    if (!list)
        return;

    list_node *iter = list->head;
    list_node *aux = iter->next;
    free(iter->data);
    free(iter);
    iter = NULL;
    if (!aux)
        return;
    else
    {
        while (aux)
        {
            iter = aux;
            aux = aux->next;
            free(iter->data);
            free(iter);
            iter = NULL;
        }
    }

    free(list);
    list = NULL;
}