#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_TEXT_SIZE 30
#define MAX_PATH_SIZE 256
#define FILENAME ".tasks"

struct Task {
        int    id;
        char   text[MAX_TEXT_SIZE];
        char   isdone;
};

void    todu_add(struct Task**, int*, const char*);
void    todu_del(struct Task*,  int*, const char*);
void    todu_init(const char*);
void    todu_load(struct Task**, int*, const char*);
void    todu_show(struct Task*,  int);
void    todu_status(struct Task*, int, const char*, int);
void    todu_update(struct Task*, int, const char*);
void    usage(void);



void
todu_add(struct Task *tasks[], int *size, const char *user_text)
{
        (*size)++;

        struct Task new_task;
        strncpy(new_task.text, user_text, sizeof(new_task.text) - 1);
        new_task.isdone  = 'n';

        *tasks = realloc(*tasks, (*size) * sizeof(struct Task));
        if (*tasks == NULL) {
                fprintf(stderr, "Memory reallocation failed\n");
                exit(EXIT_FAILURE);
        }
        (*tasks)[(*size) - 1] = new_task;
}

void
todu_del(struct Task *tasks, int *size, const char *value)
{
        if (strcmp(value, "all") == 0) {
                *size = 0;
                return;
        }

        int id = atoi(value), element_to_del = -1;

        for (int el = 0; el < *size; el++) {
                if (tasks[el].id == id) {
                        element_to_del = el;
                        break;
                }
        }

        if (element_to_del > -1) {
                for (int el = element_to_del; el < *size - 1; el++)
                        tasks[el] = tasks[el + 1];
                (*size)--;
        }
        else {
                fprintf(stderr, "Can't delete '%d' task\n", id);
                exit(EXIT_FAILURE);
        }
}

void
todu_init(const char *filepath)
{
        FILE *new_file = fopen(filepath, "w");
        if (new_file == NULL) {
                fprintf(stderr, "Failed to create tasks file\n");
                exit(EXIT_FAILURE);
        }
        printf("File created in '%s'\n", filepath);
        fclose(new_file);
}



void
todu_load(struct Task *tasks[], int *size, const char *filepath)
{
        FILE *tasks_file = fopen(filepath, "r");
        if (tasks_file == NULL) {
                fprintf(stderr, "Failed to read tasks file\n");
                exit(EXIT_FAILURE);
        }     

        while (fscanf(tasks_file, "\"%*[^\"]\" %*c\n") != EOF)
                (*size)++;

        rewind(tasks_file);

        *tasks = (struct Task*)malloc((*size) * sizeof(struct Task));
        if (*tasks == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                fclose(tasks_file);
                exit(EXIT_FAILURE);
        }

        for (int el = 0; el < *size; el++) {
                 if (fscanf(tasks_file, "\"%30[^\"]\" %c\n",
                                        (*tasks)[el].text, &(*tasks)[el].isdone) != 2) {

                        fprintf(stderr, "Error reading a file\n");
                        free(*tasks);
                        fclose(tasks_file);
                        exit(EXIT_FAILURE);
                }

                (*tasks)[el].id = el + 1;
        }

        fclose(tasks_file);
}


void
todu_show(struct Task *tasks, int size)
{
        printf(" ID | %-30s | Done\n", "Task");
        printf("-------------------------------------------\n");

        for (int el = 0; el < size; el++) {
                char isdone = tasks[el].isdone == 'y' ? 'x' : ' ';
                printf(" %2d | %-30s | [%c]\n", 
                                tasks[el].id, tasks[el].text, isdone);
        }
        printf("\n");
}

void
todu_status(struct Task *tasks, int size, const char *value, int done)
{
        if (strcmp(value, "all") == 0) {
                for (int el = 0; el < size; el++) {
                        tasks[el].isdone = done == 0 ? 'y' : 'n';
                }
                return;
        }

        int id = atoi(value);
        if (id > 0 && id <= size)
                tasks[id - 1].isdone = done == 0 ? 'y' : 'n';
        else
                fprintf(stderr, "Value is out of bounds\n");
}


void
todu_update(struct Task *tasks, int size, const char *filepath)
{
        FILE *tasks_file = fopen(filepath, "w");
        if (tasks_file == NULL) {
                fprintf(stderr, "Can't write data to file\n");
                return;
        }

        for (int el = 0; el < size; el++)
                fprintf(tasks_file, "\"%s\"\t %c\n",
                                tasks[el].text, tasks[el].isdone);

        fclose(tasks_file);

}

void
usage(void)
{
        fprintf(stderr, "Usage: todu [option] <value>\n"
                        "  add    <text>       Add task\n"
                        "  del    <id/all>     Delete task by id\n"
                        "  done   <id/all>     Mark task as done\n"
                        "  undone <id/all>     Mark task as not finished\n");
}

int
main(int argc, char *argv[])
{
        /* Variable creation */
        char filepath[MAX_PATH_SIZE];
        int tasks_size = 0;
        struct Task *tasks;

        /* Save path to file inside 'filepath' variable */
        snprintf(filepath, MAX_PATH_SIZE, "%s/%s", getenv("HOME"), FILENAME);

        /* Create a file, if not exist */
        if (access(filepath, F_OK) == -1)
                todu_init(filepath);

        /* Get data from file */
        todu_load(&tasks, &tasks_size, filepath);

        if (argc < 2) {
                if (tasks_size < 1) {
                        fprintf(stderr, "Task file is empty\n");
                        free(tasks);
                        return EXIT_FAILURE;
                }
                todu_show(tasks, tasks_size);
                free(tasks);
                return EXIT_SUCCESS;
        }

        char *option = argv[1];
        char *value  = argv[2];

        
        char user_stream[MAX_TEXT_SIZE];

        if (value == NULL) {
                if (!isatty(fileno(stdin))) {
                        fgets(user_stream, MAX_TEXT_SIZE, stdin);
                        if (user_stream[strlen(user_stream) - 1] == '\n')
                                user_stream[strlen(user_stream) - 1] = '\0';
                }
                else {
                        usage();
                        free(tasks);
                        return EXIT_FAILURE;
                }
        }

        if (strcmp(option, "add") == 0)
                todu_add(&tasks, &tasks_size, value ? value : user_stream);

        else if (strcmp(option, "del") == 0)
                todu_del( tasks, &tasks_size, value ? value : user_stream);

        else if (strcmp(option, "done") == 0)
                todu_status(tasks, tasks_size, value ? value : user_stream, 0);

        else if (strcmp(option, "undone") == 0)
                todu_status(tasks, tasks_size, value ? value : user_stream, 1);
        else {
                usage();
                free(tasks);
                return EXIT_FAILURE;
        }


        todu_update(tasks, tasks_size, filepath);
        free(tasks);
        return EXIT_SUCCESS;
}

