#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "dirent.h"

#define folder_n 3

int main(void)
{
    char folders_name[folder_n][200] = {"folder", "folder2", "folder/subfolder"};

    for (int i = 0; i < folder_n; i++)
    {
        // Apro la directory
        DIR *directory = opendir(folders_name[i]);
        if (directory == NULL)
        {
            printf("Errore durante l'apertura di %s \n", folders_name[i]);
            return -1;
        }

        struct dirent *directory_item;
        while ((directory_item = readdir(directory)) != NULL)
        {
            if (directory_item->d_type == DT_REG)
            {
                char file_path[300];

                strcpy(file_path, folders_name[i]);
                strcat(file_path, "/");
                strcat(file_path, directory_item->d_name);

                printf("- Sto leggendo il file %s \n", file_path);

                FILE *file = fopen(file_path, "r");

                char line[BUFSIZ];
                while (fgets(line, BUFSIZ, file))
                {
                    if (strstr(line, "ciao") != NULL)
                    {
                        printf("---- Dentro al file %s è contenuta la parola ciao\n", directory_item->d_name);
                    }
                }

                fclose(file);
            }
        }

        closedir(directory);
    }

    return 0;
}