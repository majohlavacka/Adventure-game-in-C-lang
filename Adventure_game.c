#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME_LENGTH 100
#define MAX_ROOMS 10 // v hre je maximalne 10 miestnosti
#define MAX_ITEMS 5 // hrac moze mat maximalne 5 predmetov v inventari

// zoznam zbrani a ich damage hodnoty
typedef enum {
    MEC = 1,
    LUK
} Weapon;

typedef struct {
    int ma_npc;         // ci miestnost ma NPC
    int npc_hp;         // HP NPC
    char *npc_name;     // meno NPC
    int item_found;     // ci sa v miestnosti nachadza predmet
    char *item_name;    // nazov predmetu
    int visited;        // flag, ci bola miestnost uz navstivena
} Room;

typedef struct {
    char meno[MAX_NAME_LENGTH];
    int hp;             // HP hraca
    int pocet_predmetov; // pocet predmetov v inventari
    char *inventar[MAX_ITEMS];  // inventar hraca
    Weapon weapon;      // aktualna zbran hraca
} Player;

// jednotlive funkcie sa volaju a odozvdavaju im parametre
void ukaz_menu();
void pohni_sa(Player *player, Room *mapa, int *aktualna_miestnost);
int boj(Player *player, Room *miestnost);
void uloz_hru(Player *player, int *aktualna_miestnost, Room *mapa);
void nacitaj_hru(Player *player, int *aktualna_miestnost, Room *mapa);
void zober_predmet(Player *player, Room *miestnost);
void zobraz_inventar(Player *player);
void generuj_miestnosti(Room *mapa, int num_rooms);
void vyber_zbran(Player *player);

// funkcia na generovanie miestnosti
void generuj_miestnosti(Room *mapa, int num_rooms) {
    const char* npc_names[] = {"Paladin", "Obor", "Vlk", "Drak", "Strazca", "Troll", "Goblin", "Mag", "Vampir", "Fenix"}; // mena pre NPC
    const char* item_names[] = {"Zlaty Kluc", "Magicka Kniha", "Elixir Zdravia", "Ohniva gula", "Zlaty Prsten"}; // nazvy predmetov
    
    // nahodne vygeneruje do miestnosti npc, teda jej meno, hp apod.
    for (int i = 0; i < num_rooms; i++) {
        mapa[i].ma_npc = rand() % 2;
        mapa[i].npc_hp = rand() % 50 + 30;
        mapa[i].npc_name = strdup(npc_names[rand() % 10]);
        mapa[i].item_found = (rand() % 100) < 75 ? 1 : 0;
        mapa[i].item_name = mapa[i].item_found ? strdup(item_names[rand() % 5]) : NULL;
        mapa[i].visited = 0;
    }
}

int boj(Player *player, Room *miestnost) {
    char prikaz[10];
    int player_attack, npc_attack;
    int critical_hit_chance = 20;  // 20% sanca na kriticky zasah

    if (player->weapon == MEC) {
        player_attack = rand() % 15 + 15;  // damage mec (15-30)
    } else if (player->weapon == LUK) {
        player_attack = rand() % 10 + 20;  // damage luk (20-30)
    }

    npc_attack = rand() % 15 + 10;  // NPC utok (10-25)

    while (player->hp > 0 && miestnost->npc_hp > 0) {
        printf("\nTvoje HP: %d, HP NPC: %d\n", player->hp, miestnost->npc_hp);
        printf("Zadaj prikaz na utok (utok): ");
        scanf("%s", prikaz);

        if (strcmp(prikaz, "utok") == 0) {
            // kriticky zasah, ktory zdvojnasoby damage vybratej zbrane
            if (rand() % 100 < critical_hit_chance) {
                printf("Kriticky zasah! Tvoj utok je dvojnasobny.\n");
                miestnost->npc_hp -= player_attack * 2;
            } else {
                miestnost->npc_hp -= player_attack;
            }
            printf("Tvoj utok! NPC HP je teraz %d.\n", miestnost->npc_hp);
        }

        if (miestnost->npc_hp > 0) {
            player->hp -= npc_attack;
            printf("NPC utocilo! Tvoje HP je teraz %d.\n", player->hp);
        }
    }

    return player->hp > 0;
}

// ked hrac vstupi do miestnosti, pred zacatim boja ho hra vyzve vybrat zbran
void vyber_zbran(Player *player) {
    printf("Vyber si zbran:\n");
    printf("1. Mec (Damage: 15-30)\n");
    printf("2. Luk (Damage: 20-30)\n");
    printf("Zadaj cislo zbrane (1 alebo 2): ");
    int volba;
    scanf("%d", &volba);

    if (volba == 1) {
        player->weapon = MEC;
        printf("Vybral si mec!\n");
    } else if (volba == 2) {
        player->weapon = LUK;
        printf("Vybral si luk!\n");
    } else {
        printf("Neplatna volba, vybral si mec.\n");
        player->weapon = MEC;
    }
}

// hrac ma moznost si pred zacatim boja doplnit zivot, avsak maximalne do 100 hp 
void dopln_hp(Player *player) {
    int doplnenie;
    printf("Tvoje aktualne HP je: %d\n", player->hp);  // zobrazenie aktualneho HP
    printf("Chces doplnit HP? Zadaj kolko HP chces doplnit (max. 100): ");
    scanf("%d", &doplnenie);

    if (doplnenie > 0) {
        player->hp += doplnenie;
        if (player->hp > 100) {
            player->hp = 100; // obmedzenie HP na 100
        }
        printf("Tvoje HP je teraz: %d.\n", player->hp);
    } else {
        printf("Neplatny vstup, HP nebolo doplnene.\n");
    }
}

void pohni_sa(Player *player, Room *mapa, int *aktualna_miestnost) {
    printf("Kam sa chces pohybovat? (hore, dole, vlavo, vpravo): ");
    char smer[10];
    scanf("%s", smer);

    if (strcmp(smer, "hore") == 0 && *aktualna_miestnost > 0) {
        (*aktualna_miestnost)--; // pohyb hore
    } else if (strcmp(smer, "dole") == 0 && *aktualna_miestnost < MAX_ROOMS - 1) {
        (*aktualna_miestnost)++; // pohyb dole
    } else if (strcmp(smer, "vlavo") == 0 && *aktualna_miestnost > 0) {
        (*aktualna_miestnost)--; // pohyb vlavo
    } else if (strcmp(smer, "vpravo") == 0 && *aktualna_miestnost < MAX_ROOMS - 1) {
        (*aktualna_miestnost)++; // pohyb vpravo
    } else {
        printf("Neplatny pohyb.\n");
        return;
    }

    printf("Pohol si sa %s. Teraz si v miestnosti %d.\n", smer, *aktualna_miestnost);

    if (mapa[*aktualna_miestnost].ma_npc) {
        printf("Narazil si na miestnost s NPC: %s!\n", mapa[*aktualna_miestnost].npc_name);
        //hrac moze aj nemusi vstupit do boja
        printf("Chces vstupit do miestnosti? (ano/nie): ");
        char odpoved[10];
        scanf("%s", odpoved);

        if (strcmp(odpoved, "ano") == 0) {
            dopln_hp(player);  // po vstupe do miestnosti sa spytame na doplnenie HP
            vyber_zbran(player);  // pred bojom si hrac vyberie zbran
            if (boj(player, &mapa[*aktualna_miestnost])) {
                printf("Vyhral si boj!\n");
                if (mapa[*aktualna_miestnost].item_found) {
                    zober_predmet(player, &mapa[*aktualna_miestnost]);
                } else {
                    printf("Predmet v tejto miestnosti si neziskal.\n");
                }
                mapa[*aktualna_miestnost].ma_npc = 0; // NPC bolo porazene
            } else {
                printf("Prehral si boj.\n");
                printf("Koniec hry.\n");
            }
        }
    } else {
        printf("Miestnost je prazdna.\n");
    }
}

void uloz_hru(Player *player, int *aktualna_miestnost, Room *mapa) {
    // poziadame hraca o zadanie nazvu suboru
    char filename[MAX_NAME_LENGTH + 10];  // pridame nazov aktualneho checkpointu napr Marian1.bin
    printf("Zadaj nazov suboru, kam chces ulozit hru (napr. Marian1.bin): ");
    scanf("%s", filename);  

    FILE *soubor = fopen(filename, "wb");
    if (soubor == NULL) {
        printf("Nepodarilo sa ulozit hru!\n");
        return;
    }

    // ulozime zakladne udaje o hracovi
    fwrite(player->meno, sizeof(char), MAX_NAME_LENGTH, soubor);
    fwrite(&player->hp, sizeof(int), 1, soubor);
    fwrite(&player->weapon, sizeof(Weapon), 1, soubor);
    fwrite(&player->pocet_predmetov, sizeof(int), 1, soubor);

    // ulzime predmety v inventari
    for (int i = 0; i < player->pocet_predmetov; i++) {
        fwrite(player->inventar[i], sizeof(char), MAX_NAME_LENGTH, soubor);
    }

    // ulozime aktualnu miestnost a informacie o miestnostiach
    fwrite(aktualna_miestnost, sizeof(int), 1, soubor);

    for (int i = 0; i < MAX_ROOMS; i++) {
        fwrite(&mapa[i], sizeof(Room), 1, soubor);
    }

    fclose(soubor);

    printf("Hra bola uspesne ulozena do suboru %s!\n", filename);
}

void nacitaj_hru(Player *player, int *aktualna_miestnost, Room *mapa) {
    // po prikaze nacitaj, musime zadat ulozeny subor napr Marian1.bin
    char filename[MAX_NAME_LENGTH + 10]; 
    printf("Zadajte nazov suboru, ktory chcete nacitat (napr. Marian1.bin): ");
    scanf("%s", filename);

    FILE *soubor = fopen(filename, "rb");
    if (soubor == NULL) {
        printf("Nebolo mozne nacitat hru zo suboru %s!\n", filename);
        return;
    }

    // nacitame udaje o hracovi
    fread(player->meno, sizeof(char), MAX_NAME_LENGTH, soubor);
    fread(&player->hp, sizeof(int), 1, soubor);
    fread(&player->weapon, sizeof(Weapon), 1, soubor);
    fread(&player->pocet_predmetov, sizeof(int), 1, soubor);

    // nacitame predmety do inventara
    for (int i = 0; i < player->pocet_predmetov; i++) {
        player->inventar[i] = malloc(MAX_NAME_LENGTH * sizeof(char));  // alokujeme miesto pre kazdy predmet
        fread(player->inventar[i], sizeof(char), MAX_NAME_LENGTH, soubor);
    }

    // nacitame aktualnu miestnost a informacie o miestnostiach
    fread(aktualna_miestnost, sizeof(int), 1, soubor);

    for (int i = 0; i < MAX_ROOMS; i++) {
        fread(&mapa[i], sizeof(Room), 1, soubor);
    }

    fclose(soubor);

    printf("Hra bola uspesne nacitana z suboru %s!\n", filename);
}


void zober_predmet(Player *player, Room *miestnost) {
    if (miestnost->item_found) {
        printf("Zobral si predmet: %s\n", miestnost->item_name);
        if (player->pocet_predmetov < MAX_ITEMS) {
            player->inventar[player->pocet_predmetov++] = miestnost->item_name;
        } else {
            printf("Tvoj inventar je plny!\n");
        }
    }
}

void odstran_predmet(Player *player) {
    if (player->pocet_predmetov == 0) {
        printf("Tvoj inventar je prazdny, nic nemozes vymazat.\n");
        return;
    }

    // vypise inventar
    printf("Tvoj inventar:\n");
    for (int i = 0; i < player->pocet_predmetov; i++) {
        printf("%d. %s\n", i + 1, player->inventar[i]);
    }

    // opyta sa, ci chce hrac vymazat polozku
    printf("Chces vymazat predmet? Zadaj cislo predmetu alebo back pre navrat: ");
    char volba[10];
    scanf("%s", volba);

    if (strcmp(volba, "back") == 0) {
        return;  // vrati sa spat do menu
    }

    int index = atoi(volba) - 1;  // prevedie zadane cislo na index od 0

    if (index >= 0 && index < player->pocet_predmetov) {
        printf("Predmet %s bol vymazany z tvojho inventara.\n", player->inventar[index]);

        // posunie polozky v inventari, aby vymazal polozku na zadanom mieste
        for (int i = index; i < player->pocet_predmetov - 1; i++) {
            player->inventar[i] = player->inventar[i + 1];
        }

        player->inventar[player->pocet_predmetov - 1] = NULL;  // nastavi posledny objekt na NULL
        player->pocet_predmetov--;  // znizi pocet predmetov
    } else {
        printf("Neplatne cislo predmetu.\n");
    }
}


void zobraz_inventar(Player *player) {
    if (player->pocet_predmetov == 0) {
        printf("Inventar hraca %s je prazdny.\n", player->meno);
        return;
    }

    while (1) {
        // zobrazi inventar a meno hraca ktoremu patri
        printf("Inventar hraca %s:\n", player->meno);  
        for (int i = 0; i < player->pocet_predmetov; i++) {
            printf("%d. %s\n", i + 1, player->inventar[i]);
        }

        // opyta sa, ci chce hrac vymazat predmet alebo sa vratit spat do menu
        printf("Chces vymazat predmet alebo sa vratit? Zadaj vymazat alebo back: ");
        char volba[10];
        scanf("%s", volba);

        if (strcmp(volba, "back") == 0) {
            break;  // ak zada back, vrati sa do hlavneho menu
        } else if (strcmp(volba, "vymazat") == 0) {
            // opyta sa, ktory predmet chce vymazat
            printf("Zadaj cislo predmetu, ktory chces vymazat (alebo zadaj back pre navrat): ");
            scanf("%s", volba);

            if (strcmp(volba, "back") == 0) {
                break;  // ak zada back, vrati sa spat do menu
            }

            int index = atoi(volba) - 1;  

            if (index >= 0 && index < player->pocet_predmetov) {
                printf("Predmet %s bol vymazany z tvojho inventara.\n", player->inventar[index]);

                for (int i = index; i < player->pocet_predmetov - 1; i++) {
                    player->inventar[i] = player->inventar[i + 1];
                }

                player->inventar[player->pocet_predmetov - 1] = NULL;  
                player->pocet_predmetov--;  
            } else {
                printf("Neplatne cislo predmetu.\n");
            }
        } else {
            printf("Neplatna volba, skus to znova.\n");
        }
    }
}

void zobraz_profil(Player *player, int aktualna_miestnost) {
    printf("\n--- Profil ---\n");
    printf("Meno: %s\n", player->meno);
    printf("HP: %d\n", player->hp);
    printf("Aktualna miestnost: %d\n", aktualna_miestnost);
    printf("----------------");
}


void ukaz_menu() {
    printf("\n--- MENU ---\n");
    printf("1. Pohyb: pohni\n");
    printf("2. Profil: profil\n");
    printf("3. Inventar: inventar\n");
    printf("4. Ulozit hru: uloz\n");
    printf("5. Nacitat hru: nacitaj\n");
    printf("6. Koniec: koniec");
    printf("\n------------\n");
    printf("Zadaj prikaz: ");
}

int main(int argc, char *argv[]) {
    // hra sa musi zacinat parametrom, v tomto pripade menom profilu hraca napr. hra.exe Marian
    if (argc < 2) {
        printf("Zadajte meno hraca ako parameter.\n");
        return 1;
    }

    Player player;
    strncpy(player.meno, argv[1], MAX_NAME_LENGTH);  // meno hraca z argumentu
    player.hp = 100;
    player.pocet_predmetov = 0;

    srand(time(NULL));

    Room mapa[MAX_ROOMS];
    generuj_miestnosti(mapa, MAX_ROOMS);

    int aktualna_miestnost = 0;

    while (player.hp > 0) {
        ukaz_menu();
        char volba[10];
        scanf("%s", volba);

        if (strcmp(volba, "pohni") == 0) {
            pohni_sa(&player, mapa, &aktualna_miestnost);
        } else if (strcmp(volba, "uloz") == 0) {
            uloz_hru(&player, &aktualna_miestnost, mapa);  // ulozi hru do suboru s menom hraca
        } else if (strcmp(volba, "nacitaj") == 0) {
            nacitaj_hru(&player, &aktualna_miestnost, mapa);  // nacita hru zo suboru podla zadania nazvu
        } else if (strcmp(volba, "inventar") == 0) {
            zobraz_inventar(&player);
        } else if (strcmp(volba, "profil") == 0) {  
            zobraz_profil(&player, aktualna_miestnost);
        } else if (strcmp(volba, "koniec") == 0) {
            printf("Hra skoncila.\n");
            break;
        } else {
            printf("Neznamy prikaz.\n");
        }
    }

    return 0;
}