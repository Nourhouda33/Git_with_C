#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // Pour mkstemp et close

int hashFile(char* source, char* dest) {
    // Création d'un fichier temporaire sécurisé
    char template[] = "/tmp/hashXXXXXX";  // Modèle pour mkstemp
    char tmp_path[100];
    strcpy(tmp_path, template);

    int fd = mkstemp(tmp_path);  // Crée le fichier temporaire
    if (fd == -1) return 0;      // Échec de création
    close(fd);  // On ferme le fichier (la suite sera gérée par la commande)

    // Construction de la commande avec le fichier temporaire
    char command[512];
    snprintf(
        command, 
        sizeof(command), 
        "sha256sum \"%s\" | awk '{print $1}' > \"%s\" && mv \"%s\" \"%s\"", 
        source, 
        tmp_path, 
        tmp_path, 
        dest
    );

    // Exécution et vérification
    int status = system(command);
    if (status != 0) {
        remove(tmp_path);  // Nettoyage en cas d'échec
        return 0;
    }

    return 1;
}
char* sha256(char* source) {
    char command[256];
    snprintf(command, sizeof(command), "sha256sum \"%s\" | awk '{print $1}'", source);
    
    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        return NULL;
    }

    char* hash = malloc(65);  // SHA-256 = 64 caractères + 1 pour '\0'
    if (fgets(hash, 65, fp) == NULL) {
        pclose(fp);
        free(hash);
        return NULL;
    }

    pclose(fp);
    return hash;
}


typedef struct cell {
    char* data;
    struct cell* next;
} Cell;

typedef struct {
    Cell* head;
} List;

List* initList() {
    List* newList = (List*)malloc(sizeof(List));
    if (newList == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour la liste\n");
        exit(EXIT_FAILURE);
    }
    newList->head = NULL;
    return newList;
}
Cell* buildCell(char* ch) {
    Cell* newCell = (Cell*)malloc(sizeof(Cell));
    if (newCell == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour la cellule\n");
        exit(EXIT_FAILURE);
    }
    
    newCell->data = strdup(ch);
    if (newCell->data == NULL) {
        fprintf(stderr, "Erreur de duplication de chaîne\n");
        free(newCell);
        exit(EXIT_FAILURE);
    }
    
    newCell->next = NULL;
    return newCell;
}
void insertFirst(List* L, Cell* C) {
    if (L == NULL || C == NULL) return;
    
    C->next = L->head;
    L->head = C;
}
char* ctos(Cell* c) {
    if (c == NULL) return strdup("");
    return strdup(c->data);
}

char* ltos(List* L) {
    if (L == NULL || L->head == NULL) return strdup("");
    
    Cell* current = L->head;
    int totalLength = 0;
    int count = 0;
    
    // Calcul de la longueur totale nécessaire
    while (current != NULL) {
        totalLength += strlen(current->data);
        count++;
        current = current->next;
    }
    
    // Allocation de la chaîne résultante
    char* result = (char*)malloc(totalLength + count); // +count pour les '|' et le '\0'
    if (result == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    result[0] = '\0';
    
    // Construction de la chaîne
    current = L->head;
    while (current != NULL) {
        strcat(result, current->data);
        if (current->next != NULL) {
            strcat(result, "|");
        }
        current = current->next;
    }
    
    return result;
}
Cell* listGet(List* L, int i) {
    if (L == NULL || i < 0) return NULL;
    
    Cell* current = L->head;
    int count = 0;
    
    while (current != NULL && count < i) {
        current = current->next;
        count++;
    }
    
    return current;
}
Cell* searchList(List* L, char* str) {
    if (L == NULL || str == NULL) return NULL;
    
    Cell* current = L->head;
    
    while (current != NULL) {
        if (strcmp(current->data, str) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}
List* stol(char* s) {
    List* L = initList();
    if (s == NULL || strlen(s) == 0) return L;
    
    char* token;
    char* copy = strdup(s);
    if (copy == NULL) {
        fprintf(stderr, "Erreur de duplication de chaîne\n");
        free(L);
        exit(EXIT_FAILURE);
    }
    
    token = strtok(copy, "|");
    while (token != NULL) {
        Cell* newCell = buildCell(token);
        insertFirst(L, newCell);
        token = strtok(NULL, "|");
    }
    
    free(copy);
    
    // Inverser la liste pour garder l'ordre original
    List* reversed = initList();
    Cell* current = L->head;
    while (current != NULL) {
        Cell* next = current->next;
        insertFirst(reversed, current);
        current = next;
    }
    
    free(L);
    return reversed;
}
void ltof(List* L, char* path) {
    if (path == NULL) return;
    
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        fprintf(stderr, "Erreur d'ouverture du fichier %s\n", path);
        return;
    }
    
    char* listStr = ltos(L);
    fprintf(file, "%s", listStr);
    free(listStr);
    
    fclose(file);
}

List* ftol(char* path) {
    if (path == NULL) return initList();
    
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "Erreur d'ouverture du fichier %s\n", path);
        return initList();
    }
    
    // Déterminer la taille du fichier
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Lire le contenu du fichier
    char* content = (char*)malloc(fileSize + 1);
    if (content == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        fclose(file);
        return initList();
    }
    
    fread(content, 1, fileSize, file);
    content[fileSize] = '\0';
    fclose(file);
    
    List* L = stol(content);
    free(content);
    
    return L;
}




void main(){
    int x ;
    x=hashFile("exercice.c","fihier.txt");
    if (x)
        printf("Hash crée avec succès.\n");
    else
        printf("Erreur lors du calcul du hash.\n");
    
    char* hash = sha256("source.c");

    if (hash != NULL) {
        printf("Hash SHA-256 de source.c :\n%s\n", hash);
        free(hash);
    } else {
        printf("Erreur lors du calcul du hash.\n");
    }
}