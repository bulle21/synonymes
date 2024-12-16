#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LIGNE 1000
#define MAX_MOTS 50
#define MAX_MOT 100

// Structure pour représenter un mot et ses synonymes
typedef struct {
    char *mot;           // Le mot principal
    char **synonymes;    // Tableau de synonymes
    int nb_synonymes;    // Nombre de synonymes
} MotSynonymes;

// Structure pour le dictionnaire complet
typedef struct {
    MotSynonymes *dico;  // Tableau de mots et synonymes
    int nb_mots;         // Nombre total de mots
} DictionnaireSynonymes;




// Fonction pour libérer la mémoire du dictionnaire
void liberer_dictionnaire(DictionnaireSynonymes *dico) 
{
if(dico == NULL) return;

for(int i = 0; i < dico->nb_mots; i++) {
    free(dico->dico[i].mot);
    for(int j = 0; j < dico->dico[i].nb_synonymes; j++) {
        free(dico->dico[i].synonymes[j]);
        }
    free(dico->dico[i].synonymes);
    }
free(dico->dico);
free(dico);
}

// Fonction pour charger le dictionnaire de synonymes
DictionnaireSynonymes *charger_synonymes(const char *nom_fichier) 
{
FILE *fichier;
char ligne[MAX_LIGNE];
    
// Allouer la structure du dictionnaire
DictionnaireSynonymes* dico = malloc(sizeof(DictionnaireSynonymes));
if(dico == NULL) {
    perror("Erreur d'allocation mémoire pour le dictionnaire");
    return(NULL);
    }
    
// Initialiser le nombre de mots
dico->nb_mots = 0;
dico->dico = NULL;

// Ouvrir le fichier
fichier = fopen(nom_fichier, "r");
if(fichier == NULL) {
    perror("Erreur lors de l'ouverture du fichier");
    free(dico);
    return(NULL);
    }

// Premier passage pour compter le nombre de lignes
while (fgets(ligne, sizeof(ligne), fichier)) {
    dico->nb_mots++;
    }
rewind(fichier);

// Allouer de la mémoire pour les mots
dico->dico = malloc(dico->nb_mots * sizeof(MotSynonymes));
if(dico->dico == NULL) {
    perror("Erreur d'allocation mémoire pour les mots");
    fclose(fichier);
    free(dico);
    return NULL;
    }

// Réinitialiser le compteur
dico->nb_mots = 0;

// Lire les lignes
while(fgets(ligne, sizeof(ligne), fichier)) {
    // Supprimer le saut de ligne
    ligne[strcspn(ligne, "\n")] = 0;

    // Séparer les mots
    char *token = strtok(ligne, ",");
    if(token == NULL) continue;

    // Allouer mémoire pour le mot principal
    dico->dico[dico->nb_mots].mot = strdup(token);
        
    // Allouer mémoire pour les synonymes
    dico->dico[dico->nb_mots].synonymes = malloc(MAX_MOTS * sizeof(char*));
    dico->dico[dico->nb_mots].nb_synonymes = 0;

    // Collecter les synonymes
    while((token = strtok(NULL, ",")) != NULL) {
        if(dico->dico[dico->nb_mots].nb_synonymes < MAX_MOTS) {
            dico->dico[dico->nb_mots].synonymes[dico->dico[dico->nb_mots].nb_synonymes] = strdup(token);
            dico->dico[dico->nb_mots].nb_synonymes++;
            }
        }

    dico->nb_mots++;
    }

fclose(fichier);
return(dico);
}

// Fonction pour trouver un synonyme au hasard pour un mot donné
char *fct_synonyme(DictionnaireSynonymes *dico,const char *mot) 
{
if(dico == NULL) return NULL;

// Initialiser le générateur de nombres aléatoires
srand(time(NULL));

// Rechercher le mot dans le dictionnaire
for(int i = 0; i < dico->nb_mots; i++) {
    if(strcasecmp(dico->dico[i].mot, mot) == 0) {
        // Si des synonymes existent
        if(dico->dico[i].nb_synonymes > 0) {
            // Choisir un synonyme au hasard
            int index = rand() % dico->dico[i].nb_synonymes;
            return(strdup(dico->dico[i].synonymes[index]));
            }
        break;
        }
    }
return(NULL);
}

// Fonction principale
int main(int argc, char **argv) 
{
if(argc < 2) {
    printf("Usage: %s <mot>\n", argv[0]);
    return(1);
    }

// Charger le dictionnaire de synonymes
DictionnaireSynonymes* dico = charger_synonymes("synonymes.txt");
if (dico == NULL) {
    printf("Erreur de chargement du dictionnaire.\n");
    return(1);
    }

// Chercher un synonyme pour chaque mot passé en argument
for(int i = 1; i < argc; i++) {
    char *synonyme = fct_synonyme(dico, argv[i]);
    if(synonyme) {
        printf("Synonyme de %s : %s\n", argv[i], synonyme);
        free(synonyme);
        } 
    else {
        printf("Aucun synonyme trouvé pour %s\n", argv[i]);
        }
    }

// Libérer la mémoire
liberer_dictionnaire(dico);

return(0);
}
