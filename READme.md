Ce `README.md` expliquant comment utiliser le script CGI en C fourni, ainsi que les configurations nécessaires pour Apache :

---

# Utilisation du Script CGI en C

Ce document explique comment configurer et utiliser un script CGI en C pour vérifier les identifiants d'un utilisateur. Le script lit les informations d'identification (nom d'utilisateur et mot de passe) envoyées via une requête HTTP POST, les compare avec des valeurs stockées dans un fichier, et affiche un message en fonction du résultat.

## Étapes Configuration

### 1. Préparer le Script CGI

1. **Créer le fichier CGI :**

   Enregistrez le code suivant dans un fichier nommé `file.cgi` :

   ```c
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
   ```

2. **Rendre le script exécutable :**

   Assurez-vous que le script a les permissions d'exécution :
   ```bash
   chmod +x /var/www/site1/file.cgi
   ```

3. **Créer le fichier `data.txt` :**

   Créez un fichier `data.txt` contenant les identifiants (nom d'utilisateur et mot de passe) au format :
   ```
   admin azertyuiop
   ```

### 2. Configurer Apache

1. **Modifier la configuration d'Apache :**

   Éditez le fichier de configuration d'Apache pour inclure les directives nécessaires :

   ```apache
   <Directory /var/www/site1/>
       AddHandler cgi-script .cgi
       Options Indexes ExecCGI FollowSymLinks
       AllowOverride None
       Require all granted
   </Directory>

   <Directory "/var/www/site1/">
       AllowOverride None
       Options +ExecCGI -MultiViews +SymLinksIfOwnerMatch
       AddHandler cgi-script .cgi
       Require all granted
   </Directory>
   ```

2. **Activer les configurations :**

   Si vous avez modifié ou ajouté un fichier de configuration, assurez-vous que les modifications sont prises en compte en rechargeant Apache :
   ```bash
   sudo a2ensite site1.conf
   sudo systemctl reload apache2
   ```

### 3. Tester le Script

1. **Envoyer une requête POST :**

   Vous pouvez utiliser `curl` pour tester le script CGI en envoyant une requête POST avec les données appropriées :
   ```bash
   curl -d "username=admin&password=azertyuiop" http://www.site1.com/file.cgi
   ```
    Après avoir envoyé la requête, vérifiez la réponse HTTP dans votre navigateur ou via `curl` pour confirmer que le script fonctionne correctement.
    
2. **Se connecter au site :**
    Se connecter dans le navigateur avec `var/www/site1.com/login1.html`. Pour le test, faire entrer le nom et le mot de passe.
    Si le nom ou le mot de passe sont différent de ceux dans `data.txt` alors on est redirigé dans la page `error.html`.

### Résolution des Problèmes

- **Erreur "Unable to open file" :** Assurez-vous que le fichier `data.txt` existe dans le répertoire `/var/www/site1/` et qu'il est accessible par le script CGI.
- **Erreur de permission :** Vérifiez les permissions des fichiers et répertoires pour vous assurer qu'ils sont corrects.
- **Erreur dans les logs :** Consultez les logs d'Apache pour des messages d'erreur plus détaillés.

---
