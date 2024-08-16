#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define FILE_NAME "data.txt"

// Fonction pour décoder les chaînes URL-encodées
void decode_url(char *str) {
    char *pstr = str;
    char *pdecoded = str;
    while (*pstr) {
        if (*pstr == '%') {
            int value;
            sscanf(pstr + 1, "%2x", &value);
            *pdecoded++ = (char)value;
            pstr += 3;
        } else if (*pstr == '+') {
            *pdecoded++ = ' ';
            pstr++;
        } else {
            *pdecoded++ = *pstr++;
        }
    }
    *pdecoded = '\0';
}

int main() {
    FILE *file;
    char adminUsername[50];
    char adminPassword[50];
    char *content_length_str = getenv("CONTENT_LENGTH");
    int content_length;
    char *data = NULL;
    char username[20];
    char password[20];

    // Imprimer l'en-tête HTTP
    printf("Content-Type: text/html\n\n");
    printf("<html><head><meta charset='UTF-8'></head><body>");

    if (content_length_str != NULL) {
        // Obtenir la longueur du contenu
        content_length = atoi(content_length_str);
        data = (char *)malloc(content_length + 1);
        if (data == NULL) {
            printf("<h1>Error: Unable to allocate memory</h1>");
            return 1;
        }
        fread(data, 1, content_length, stdin);
        data[content_length] = '\0';

        // Extraire les valeurs des paramètres "username" et "password"
        char *username_start = strstr(data, "username=");
        char *password_start = strstr(data, "password=");
        if (username_start && password_start) {
            username_start += strlen("username=");
            password_start += strlen("password=");
            
            char *username_end = strchr(username_start, '&');
            char *password_end = strchr(password_start, '&');
            if (username_end) *username_end = '\0';
            if (password_end) *password_end = '\0';

            // Décoder les caractères URL-encodés
            decode_url(username_start);
            decode_url(password_start);

            // Ouvrir le fichier pour la lecture
            file = fopen(FILE_NAME, "r");
            if (file == NULL) {
                printf("<h1>Error: Unable to open file</h1>");
                free(data);
                return 1;
            }

            // Lire les identifiants administratifs depuis le fichier
            if (fscanf(file, "%49s %49s", adminUsername, adminPassword) != 2) {
                printf("<h1>Error: Invalid file format</h1>");
                fclose(file);
                free(data);
                return 1;
            }
            fclose(file);

            // Comparer les identifiants
            if (strcmp(adminUsername, username_start) == 0 && strcmp(adminPassword, password_start) == 0) {
                printf("<h1>Bienvenue, vous êtes connecté!</h1>");
            } else {
                // Redirection en cas d'échec
                printf("<h1>Nom d'utilisateur ou mot de passe invalide</h1>");
                printf("<meta http-equiv=\"refresh\" content=\"0;url=http://www.site1.com/error.html\">");
            }
        } else {
            printf("<h1>Error: Invalid form data</h1>");
        }

        free(data);
    } else {
        printf("<h1>Error: No content length</h1>");
    }

    printf("</body></html>");
    return 0;
}
