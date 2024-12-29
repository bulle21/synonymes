#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_LIGNE 1000
#define SYNONYMES "synonymes.txt"
#define INITIAL_TABLE_SIZE 100

// Structure pour représenter un mot et ses synonymes
struct S_MotSynonymes {
    char *mot;
    char **synonymes;
    int nb_synonymes;
    int capacite_synonymes;
    struct S_MotSynonymes *suivant;
};

// Structure pour le dictionnaire complet (table de hachage)
struct S_DictionnaireSynonymes {
    struct S_MotSynonymes **table;
    int taille_table;
    int nb_mots;
};

// Fonction de hachage
unsigned int hash(const char *str, int table_size) {
    unsigned int hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + tolower(c);

    return hash % table_size;
}

// Fonction pour créer un nouveau mot synonyme
struct S_MotSynonymes *creer_mot_synonyme(const char *mot) {
    struct S_MotSynonymes *nouveau = malloc(sizeof(struct S_MotSynonymes));
    if (nouveau == NULL) {
        perror("Erreur d'allocation mémoire pour un nouveau mot");
        exit(1);
    }

    nouveau->mot = strdup(mot);
    nouveau->synonymes = NULL;
    nouveau->nb_synonymes = 0;
    nouveau->capacite_synonymes = 0;
    nouveau->suivant = NULL;

    return nouveau;
}

// Fonction pour ajouter un synonyme à un mot
void ajouter_synonyme(struct S_MotSynonymes *mot, const char *synonyme) {
    if (mot->nb_synonymes >= mot->capacite_synonymes) {
        int nouvelle_capacite = mot->capacite_synonymes == 0 ? 2 : mot->capacite_synonymes * 2;
        char **nouveaux_synonymes = realloc(mot->synonymes, nouvelle_capacite * sizeof(char*));
        if (nouveaux_synonymes == NULL) {
            perror("Erreur d'allocation mémoire pour les synonymes");
            exit(1);
        }
        mot->synonymes = nouveaux_synonymes;
        mot->capacite_synonymes = nouvelle_capacite;
    }

    mot->synonymes[mot->nb_synonymes] = strdup(synonyme);
    mot->nb_synonymes++;
}

// Fonction pour créer un nouveau dictionnaire
struct S_DictionnaireSynonymes *creer_dictionnaire() {
    struct S_DictionnaireSynonymes *dico = malloc(sizeof(struct S_DictionnaireSynonymes));
    if (dico == NULL) {
        perror("Erreur d'allocation mémoire pour le dictionnaire");
        exit(1);
    }

    dico->taille_table = INITIAL_TABLE_SIZE;
    dico->table = calloc(dico->taille_table, sizeof(struct S_MotSynonymes*));
    if (dico->table == NULL) {
        perror("Erreur d'allocation mémoire pour la table de hachage");
        free(dico);
        exit(1);
    }

    dico->nb_mots = 0;

    return dico;
}

// Fonction pour redimensionner la table de hachage
void redimensionner_table(struct S_DictionnaireSynonymes *dico) {
    int nouvelle_taille = dico->taille_table * 2;
    struct S_MotSynonymes **nouvelle_table = calloc(nouvelle_taille, sizeof(struct S_MotSynonymes*));
    if (nouvelle_table == NULL) {
        perror("Erreur d'allocation mémoire lors du redimensionnement");
        exit(1);
    }

    for (int i = 0; i < dico->taille_table; i++) {
        struct S_MotSynonymes *courant = dico->table[i];
        while (courant != NULL) {
            struct S_MotSynonymes *suivant = courant->suivant;
            unsigned int nouvel_index = hash(courant->mot, nouvelle_taille);
            courant->suivant = nouvelle_table[nouvel_index];
            nouvelle_table[nouvel_index] = courant;
            courant = suivant;
        }
    }

    free(dico->table);
    dico->table = nouvelle_table;
    dico->taille_table = nouvelle_taille;
}

// Fonction pour ajouter un mot au dictionnaire
void ajouter_mot(struct S_DictionnaireSynonymes *dico, const char *mot, const char *synonyme) {
    if ((double)dico->nb_mots / dico->taille_table > 0.7) {
        redimensionner_table(dico);
    }

    unsigned int index = hash(mot, dico->taille_table);
    struct S_MotSynonymes *courant = dico->table[index];

    while (courant != NULL) {
        if (strcmp(courant->mot, mot) == 0) {
            ajouter_synonyme(courant, synonyme);
            return;
        }
        courant = courant->suivant;
    }

    struct S_MotSynonymes *nouveau = creer_mot_synonyme(mot);
    ajouter_synonyme(nouveau, synonyme);
    nouveau->suivant = dico->table[index];
    dico->table[index] = nouveau;
    dico->nb_mots++;
}

// Fonction pour charger le dictionnaire de synonymes
struct S_DictionnaireSynonymes *charger_synonymes(const char *nom_fichier) {
    FILE *fichier = fopen(nom_fichier, "r");
    if (fichier == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return NULL;
    }

    struct S_DictionnaireSynonymes *dico = creer_dictionnaire();
    char ligne[MAX_LIGNE];
    char *saveptr;

    while (fgets(ligne, sizeof(ligne), fichier)) {
        ligne[strcspn(ligne, "\n")] = 0;
        char *mot = strtok_r(ligne, ",", &saveptr);
        if (mot == NULL) continue;

        char *synonyme;
        while ((synonyme = strtok_r(NULL, ",", &saveptr)) != NULL) {
            ajouter_mot(dico, mot, synonyme);
            ajouter_mot(dico, synonyme, mot);
        }
    }

    fclose(fichier);
    return dico;
}

// Fonction pour trouver un synonyme au hasard pour un mot donné
char *fct_synonyme(struct S_DictionnaireSynonymes *dico, const char *mot) {
    if (dico == NULL) return NULL;

    srand(time(NULL));
    unsigned int index = hash(mot, dico->taille_table);
    struct S_MotSynonymes *courant = dico->table[index];

    while (courant != NULL) {
        if (strcasecmp(courant->mot, mot) == 0) {
            if (courant->nb_synonymes > 0) {
                int synonyme_index = rand() % courant->nb_synonymes;
                return strdup(courant->synonymes[synonyme_index]);
            }
            break;
        }
        courant = courant->suivant;
    }
    return NULL;
}

// Fonction pour libérer la mémoire du dictionnaire
void liberer_dictionnaire(struct S_DictionnaireSynonymes *dico) {
    if (dico == NULL) return;

    for (int i = 0; i < dico->taille_table; i++) {
        struct S_MotSynonymes *courant = dico->table[i];
        while (courant != NULL) {
            struct S_MotSynonymes *suivant = courant->suivant;
            free(courant->mot);
            for (int j = 0; j < courant->nb_synonymes; j++) {
                free(courant->synonymes[j]);
            }
            free(courant->synonymes);
            free(courant);
            courant = suivant;
        }
    }
    free(dico->table);
    free(dico);
}

// Fonction principale
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <mot>\n", argv[0]);
        return -1;
    }

    struct S_DictionnaireSynonymes *dico = charger_synonymes(SYNONYMES);
    if (dico == NULL) {
        printf("Erreur de chargement du dictionnaire.\n");
        return -1;
    }

    for (int i = 1; i < argc; i++) {
        char *synonyme = fct_synonyme(dico, argv[i]);
        if (synonyme) {
            printf("Synonyme de %s : %s\n", argv[i], synonyme);
            free(synonyme);
        } else {
            printf("Aucun synonyme trouvé pour %s\n", argv[i]);
        }
    }

    liberer_dictionnaire(dico);
    return 0;
}
