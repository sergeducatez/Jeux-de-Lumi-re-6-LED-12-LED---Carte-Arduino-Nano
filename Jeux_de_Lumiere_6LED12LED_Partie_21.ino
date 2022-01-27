//**************************************************************************************************************************************************************************
//********************************************** Jeux de Lumière 6 LED 12 LED - Partie 21 - Carte Nano (com49) *************************************************************
//**************************************************************************************************************************************************************************
// Auteur : Serge Ducatez - 11/2021
// https://www.youtube.com/channel/UCyGEFYW18IZKpe4uPgp9b8g/videos
// https://www.facebook.com/serge.ducatez.7/photos_albums
//**************************************************************************************************************************************************************************
// Configurez l'affichage de l'IDE à 150%.
//**************************************************************************************************************************************************************************
// Bouton poussoir "BrocheBPModePlus" pour incrémenter le mode courant.
// Bouton poussoir "BrocheBPModeMoins" pour décrémenter le mode courant.
// Bouton poussoir BrocheBPModeAuto" pour activer ou désactiver le mode automatique d'affichages.
// Affichage du mode courant d'affichage sur un écran OLED 1.3" (128x64) SH1106.
// Affichage du mode manuel ou automatique sur l'écran OLED.
// Utilisation de la librairie "U8x8lib.h".
//**************************************************************************************************************************************************************************
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Paramètres modifiables
//**************************************************************************************************************************************************************************
#include "PinChangeInterrupt.h"                                                                                                 // Librairie pour la gestion des interruptions de changement de broche
#include <Wire.h>                                                                                                               // Librairie pour la gestion de la communication I2C avec l'afficheur OLED 1.3" (128x64) SH1106
#include <U8x8lib.h>                                                                                                            // Librairie pour la gestion de l'afficheur OLED 1.3" (128x64) SH1106
#include <avr/pgmspace.h>                                                                                                       // Librairie pour la gestion de la mémoire flash

#define BrocheBuzzer                A0                                                                                          // Broche du buzzer 5v
#define BrocheLED_D2                 2                                                                                          // Broche de la LED D2  (PD2)
#define BrocheLED_D3                 3                                                                                          // Broche de la LED D3  (PD3)
#define BrocheLED_D4                 4                                                                                          // Broche de la LED D4  (PD4)
#define BrocheLED_D5                 5                                                                                          // Broche de la LED D5  (PD5)
#define BrocheLED_D6                 6                                                                                          // Broche de la LED D6  (PD6)
#define BrocheLED_D7                 7                                                                                          // Broche de la LED D7  (PD7)
#define BrocheLED_D8                 8                                                                                          // Broche de la LED D8  (PB0)
#define BrocheLED_D9                 9                                                                                          // Broche de la LED D9  (PB1)
#define BrocheLED_D10               10                                                                                          // Broche de la LED D10 (PB2)
#define BrocheLED_D11               11                                                                                          // Broche de la LED D11 (PB3)
#define BrocheLED_D12               12                                                                                          // Broche de la LED D12 (PB4)
#define BrocheLED_D13               13                                                                                          // Broche de la LED D13 (PB5)
#define BrocheReglagePotentiometre  A6                                                                                          // Broche de l'entrée analogique (A6) pour le réglage de la fréquence ou du rapport cyclique par le potentiomètre
#define BrocheBPModePlus            A1                                                                                          // Broche du bouton poussoir "BPModePlus"
#define BrocheBPModeMoins           A2                                                                                          // Broche du bouton poussoir "BPModeMoins"
#define BrocheBPModeAuto            A3                                                                                          // Broche du bouton poussoir "ModeAuto"

volatile int ModeCourant = 0;                                                                                                   // Mode courant
volatile int DernierModeCourant = -1;                                                                                           // Dernier mode courant
volatile const int NombreModesMax = 10; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de modes maximum

volatile bool SortieModeCourant = false;                                                                                        // Indicateur de sortie du mode courant

volatile bool ModeAuto = false;                                                                                                 // Indicateur du mode automatique
volatile long compteurAffichageModeCourant = 0;                                                                                 // Compteur d'affichages du mode courant en mode automatique
const long NombreAffichageModeCourant = 3;                                                                                      // Nombre d'affichages du mode courant en mode automatique
volatile long MultipleNombreAffichageModeCourant = 1;                                                                           // Multiple du nombre d'affichages du mode courant en mode automatique
bool AffichageModeManuel = true;                                                                                                // Indicateur d'affichage du mode manuel sur l'écran OLED
bool AffichageModeAuto = false;                                                                                                 // Indicateur d'affichage du mode auto sur l'écran OLED

volatile const unsigned long DureeAntiRebond = 5ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de l'anti-rebonds des boutons poussoirs "BPModePlus" et "BPModeMoins" en ms

int PointeurTableauBrochesLED;                                                                                                  // Pointeur des tableaux des broches de 6 LED
const int TableauBrochesLED_D2D7 [] = {2, 3, 4, 5, 6, 7}; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des broches des 6 LED de (D2 à D7)
const int TableauBrochesLED_D8D13 [] = {8, 9, 10, 11, 12, 13}; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des broches des 6 LED de (D8 à D13)
int PointeurTableauBroches12LED;                                                                                                // Pointeur du tableau des broches 12 LED
int PointeurTableauBroches12LEDTemp;                                                                                            // Pointeur de fin temporaire du tableau des broches 12 LED
const int TableauBroches12LED [] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des broches des 12 LED

unsigned long DureeHIGH_LOW;                                                                                                    // Temporisation de la LED activée ou désactivée

const byte NombreLED = 12; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de LED
int SequenceCouranteLED = 0;                                                                                                    // Séquence courante des LED affichées

bool buffer_TableauSequencesLED [12];                                                                                           // Tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
bool bufferInverse_TableauSequencesLED [12];                                                                                    // Tableau contenant les lignes inversées des tableaux des séquences d'affichage des LED
int pt_buffer_TableauSequencesLED;                                                                                              // Pointeur du tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED

const byte NombreSequencesLED1 = 48; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED1 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 1 des séquences d'affichage des LED
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0
   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
   1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 2
   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 3
   1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 4
   1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 5
   1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, // 6
   1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, // 7
   1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 8
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 9
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 10
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 11
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 12
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 13
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 14
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 15
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 16
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 17
   1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 18
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 19
   1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 20
   1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, // 21
   1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 22
   1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, // 23
   1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, // 24
   1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, // 25
   1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, // 26
   1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 27
   1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, // 28
   1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 29
   1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 30
   1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 31
   1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 32
   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 33
   1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 34
   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 35
   1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 36
   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 37
   1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 38
   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 39
   1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 40
   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 41
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 42
   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 43
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 44
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 45
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 46
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // 47
};

const byte NombreSequencesLED2 = 32; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED2 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 2 des séquences d'affichage des LED
   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, // 0
   0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
   1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, // 2
   0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 3
   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, // 4
   0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 5
   1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, // 6
   0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 7
   1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, // 8
   0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, // 9
   1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, // 10
   0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, // 11
   1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, // 12
   0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 13
   1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, // 14
   0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 15
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 16
   0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 17
   1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 18
   0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, // 19
   1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, // 20
   0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, // 21
   1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, // 22
   0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, // 23
   1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, // 24
   0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, // 25
   1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, // 26
   0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, // 27
   1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, // 28
   0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, // 29
   1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, // 30
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // 31
};

const byte NombreSequencesLED3 = 24; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED3 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 3 des séquences d'affichage des LED
   0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, // 0
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 1
   0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 2
   0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 3
   0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 4
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 5
   0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 6
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 7
   0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 8
   0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 9
   0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 10
   0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 11
   0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 12
   0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 13
   0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 14
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 15
   0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 16
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 17
   0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, // 18
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 19
   0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, // 20
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 21
   0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, // 22
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // 23
};

const byte NombreSequencesLED4 = 10; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED4 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 4 des séquences d'affichage des LED
   1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, // 0
   0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, // 1
   0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, // 2
   0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 3
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 4
   0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, // 5
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 6
   0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 7
   0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, // 8
   0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0  // 9
};

const byte NombreSequencesLED5 = 24; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED5 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 5 des séquences d'affichage des LED
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, // 0
   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, // 1
   1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, // 2
   1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, // 3
   1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, // 4
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 5
   1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, // 6
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 7
   1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, // 8
   1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, // 9
   1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, // 10
   1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, // 11
   1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, // 12
   1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, // 13
   1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, // 14
   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, // 15
   1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, // 16
   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, // 17
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, // 18
   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, // 19
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, // 20
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 21
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, // 22
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // 23
};

const byte NombreSequencesLED6 = 35; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED6 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 6 des séquences d'affichage des LED
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, // 0
   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, // 1
   1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, // 2
   1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, // 3
   1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, // 4
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 5
   1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, // 6
   1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, // 7
   1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, // 8
   1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, // 9
   0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, // 10
   0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, // 11
   0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, // 12
   0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, // 13
   0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 14
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, // 15
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16
   0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, // 17
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 18
   0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 19
   0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 20
   0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 21
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 22
   0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 23
   1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, // 24
   1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, // 25
   1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, // 26
   1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, // 27
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, // 28
   0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 29
   0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, // 30
   0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, // 31
   0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, // 32
   0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, // 33
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // 34
};

const byte NombreSequencesLED7 = 28; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED7 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 7 des séquences d'affichage des LED
   1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, // 0
   0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, // 1
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 2
   0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, // 3
   1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, // 4
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 5
   0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, // 6
   1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, // 7
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 8
   0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, // 9
   1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, // 10
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 11
   1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, // 12
   0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, // 13
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 14
   1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, // 15
   1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, // 16
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 17
   0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, // 18
   1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, // 19
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 20
   0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, // 21
   1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, // 22
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 23
   0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, // 24
   0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, // 25
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 26
   1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1  // 27
};

const byte NombreSequencesLED8 = 4; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED8 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 8 des séquences d'affichage des LED
   1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, // 0
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
   0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, // 2
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // 3
};

const byte NombreSequencesLED15 = 12; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED15 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 15 des séquences d'affichage des LED
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 0
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 2
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 3
   0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, // 4
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 5
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 6
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 7
   0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, // 8
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 9
   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 10
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // 11
};

const byte NombreSequencesLED16 = 54; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED16 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 16 des séquences d'affichage des LED
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 0
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 2
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 3
   0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, // 4
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 5
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 6
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 7
   0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 8
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 9
   0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, // 10
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 11
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 12
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 13
   0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 14
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 15
   0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, // 16
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 17
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 18
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 19
   0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, // 20
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 21
   0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, // 22
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 23
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 24
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 25
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 26
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 27
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 28
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 29
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 30
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 31
   0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, // 32
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 33
   0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, // 34
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 35
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 36
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 37
   0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, // 38
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 39
   0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 40
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 41
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 42
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 43
   0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, // 44
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 45
   0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 46
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 47
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 48
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 49
   0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, // 50
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 51
   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 52
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // 53
};

const byte NombreSequencesLED17 = 22; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED17 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 17 des séquences d'affichage des LED
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0
   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
   0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 2
   0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 3
   1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 4
   1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, // 5
   0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, // 6
   0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, // 7
   1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, // 8
   1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, // 9
   0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, // 10
   0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, // 11
   0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, // 12
   0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, // 13
   0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, // 14
   0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, // 15
   0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, // 16
   0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, // 17
   0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, // 18
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, // 19
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, // 20
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // 21
};

const byte NombreSequencesLED19 = 36; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED19 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 19 des séquences d'affichage des LED
   1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0
   0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 1
   1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 2
   0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 3
   0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 4
   0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 5
   0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 6
   0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, // 7
   0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 8
   0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, // 9
   0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, // 10
   0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, // 11
   0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, // 12
   0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, // 13
   0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, // 14
   0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, // 15
   0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, // 16
   0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, // 17
   0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, // 18
   0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, // 19
   0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, // 20
   0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, // 21
   0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, // 22
   0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, // 23
   0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, // 24
   0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, // 25
   0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, // 26
   0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, // 27
   0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 28
   0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, // 29
   0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 30
   0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 31
   0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 32
   0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 33
   1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 34
   0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0  // 35
};

const byte NombreSequencesLED20 = 15; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED20 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 20 des séquences d'affichage des LED (Slide droite à gauche)
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, // 0
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, // 1
   0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, // 2
   0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, // 3
   0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, // 4
   0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, // 5
   0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, // 6
   0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, // 7
   0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, // 8
   0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, // 9
   0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, // 10
   1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, // 11
   1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, // 12
   1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, // 13
   0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1  // 14 => 9
};

const byte NombreSequencesLED21 = 18; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED21 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 21 des séquences d'affichage des LED (Slide droite à gauche)
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0
   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
   1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 2
   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 3
   0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 4
   0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 5
   1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, // 6
   1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 7
   1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, // 8
   1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, // 9
   0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, // 10
   0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, // 11
   1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, // 12
   1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, // 13
   1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, // 14
   1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, // 15
   0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, // 16
   0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1  // 17 => 12
};

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);                                                               // Crée l'objet "u8x8"

//**************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************
void setup()
{
  //Serial.begin(9600);                                                                                                           // Démarre la voie série pour la communication avec la console
  
  pinMode(BrocheBuzzer, OUTPUT);                                                                                                // Configure les broches en sorties
  for (int BrocheLED = 2; BrocheLED < 14; BrocheLED++)
  {
    pinMode(BrocheLED, OUTPUT);
  }
  
  pinMode(BrocheBPModePlus, INPUT_PULLUP);                                                                                      // Configure les broches en entrées
  pinMode(BrocheBPModeMoins, INPUT_PULLUP);
  pinMode(BrocheBPModeAuto, INPUT_PULLUP);
  
  u8x8.begin();                                                                                                                 // Initialise la librairie de la gestion de l'afficheur OLED 1.3" (128x64) SH1106
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);                                                                               // Définit la police de caractères
  u8x8.drawString(0, 0, "MODE D'AFFICHAGE");                                                                                    // Positionne et Affiche le texte
  u8x8.drawString(0, 1, "----------------");                                                                                    // Positionne et Affiche le texte
  u8x8.drawString(0, 6, "----------------");                                                                                    // Positionne et Affiche le texte
  u8x8.drawString(0, 7, "- MODE: MANUEL -");                                                                                    // Positionne et Affiche le texte
  
  FonctionAffichageOLED();                                                                                                      // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
  
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(BrocheBPModePlus), InterruptionBPModePlus, FALLING);                  // Attache l'interruption de changement de broche sur la broche "BrocheBPModePlus" et Active la fonction d'événement "InterruptionBPModePlus" sur front descendant
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(BrocheBPModeMoins), InterruptionBPModeMoins, FALLING);                // Attache l'interruption de changement de broche sur la broche "BrocheBPModeMoins" et Active la fonction d'événement "InterruptionBPModeMoins" sur front descendant
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(BrocheBPModeAuto), InterruptionBPModeAuto, FALLING);                  // Attache l'interruption de changement de broche sur la broche "BrocheBPModeAuto" et Active la fonction d'événement "InterruptionBPModeAuto" sur front descendant
  
  delay(1000);                                                                                                                  // Temporise 1 seconde
  
  Buzzer(50, 0, 1);                                                                                                             // Active le buzzer 50ms

//**************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************
}

void loop()
{
//**************************************************************************************************************************************************************************
  if (ModeAuto) // ************************************************************************************************************ // Si le mode automatique est activé
  {
    if (!AffichageModeAuto) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur d'affichage du mode auto sur l'écran OLED est désactivé
    {
      u8x8.drawString(0, 7, "-- MODE: AUTO --");                                                                                // Positionne et Affiche le texte
      AffichageModeAuto = true;                                                                                                 // Active l'indicateur d'affichage du mode auto sur l'écran OLED
      AffichageModeManuel = false;                                                                                              // Désactive l'indicateur d'affichage du mode manuel sur l'écran OLED
      compteurAffichageModeCourant = 0;                                                                                         // Réinitialise le compteur d'affichages du mode courant en mode automatique
    }
    
    if (compteurAffichageModeCourant > NombreAffichageModeCourant * MultipleNombreAffichageModeCourant) // ++++++++++++++++++++ // Si le compteur d'affichages du mode courant en mode automatique est supérieur à "3"
    {
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      
      compteurAffichageModeCourant = 0;                                                                                         // Réinitialise le compteur d'affichages du mode courant en mode automatique
      
      ModeCourant++;                                                                                                            // Incrémente le mode courant
      if (ModeCourant > NombreModesMax) {ModeCourant = 0;} // ----------------------------------------------------------------- // Borne le mode courant
    }
    else // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le compteur d'affichages du mode courant en mode automatique est inférieur ou égal à à "2"
    {
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
    }
  }
  else if (!ModeAuto) // ****************************************************************************************************** // Si le mode automatique est désactivé
  {
    if (!AffichageModeManuel) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur d'affichage du mode manuel sur l'écran OLED est désactivé
    {
      u8x8.drawString(0, 7, "- MODE: MANUEL -");                                                                                // Positionne et Affiche le texte
      AffichageModeAuto = false;                                                                                                // Désactive l'indicateur d'affichage du mode auto sur l'écran OLED
      AffichageModeManuel = true;                                                                                               // Active l'indicateur d'affichage du mode manuel sur l'écran OLED
    }
  }

//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée 0 *****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  if (ModeCourant == 0) // **************************************************************************************************** // Si le mode courant "0" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED16; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 16 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED16 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 16 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED15; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 15 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED15 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 15 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 7);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 7"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 7);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 7"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED - 1);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED - 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED - 1);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED - 1"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    PORTD &= 0b00000011;                                                                                                        // Eteint toutes les LED du PORTD
    PORTB &= 0b11000000;                                                                                                        // Eteint toutes les LED du PORTB
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 8));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED15; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 15 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED15 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 15 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 7);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 7"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 7);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 7"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED - 1);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED - 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED - 1);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED - 1"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED16; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 16 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED16 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 16 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED16; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 16 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED16 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 16 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED15; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 15 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED15 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 15 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED16; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 16 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED16 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 16 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 7);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 7"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 7);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 7"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED);                                                                  // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED);                                                                    // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED15; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 15 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED15 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 15 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, 6 + pt_buffer_TableauSequencesLED - 10);                                                         // Positionne à "0" le PORTD temporaire de rang "6 + pt_buffer_TableauSequencesLED - 10"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, 6 + pt_buffer_TableauSequencesLED - 10);                                                           // Positionne à "1" le PORTD temporaire de rang "6 + pt_buffer_TableauSequencesLED - 10"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED - 1);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED - 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED - 1);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED - 1"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée 1 *****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 1) // *********************************************************************************************** // Si le mode courant "1" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED16; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 16 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED16 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 16 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED16; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 16 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED16 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 16 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = NombreSequencesLED16; SequenceCouranteLED >= 0; SequenceCouranteLED--) // ++++++++++++++++++++++ // Parcourt les séquences du tableau 16 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED16 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 16 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = NombreSequencesLED16; SequenceCouranteLED >= 0; SequenceCouranteLED--) // ++++++++++++++++++++++ // Parcourt les séquences du tableau 16 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED16 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 16 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED16; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 16 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED16 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 16 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED16; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 16 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED16 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 16 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée 2 *****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 2) // *********************************************************************************************** // Si le mode courant "2" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED17; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 17 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED17 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 17 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED17; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 17 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED17 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 17 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED17; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 17 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED17 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 17 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED17; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 17 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED17 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 17 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED17; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 17 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED17 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 17 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED17; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 17 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED17 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 17 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée 3 *****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 3) // *********************************************************************************************** // Si le mode courant "3" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED19; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 19 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED19 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 19 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED19; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 19 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED19 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 19 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED19; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 19 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED19 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 19 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED19; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 19 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED19 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 19 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED19; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 19 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED19 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 19 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée 4 *****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 4) // *********************************************************************************************** // Si le mode courant "4" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED19; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 19 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED19 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 19 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED);                                                                  // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED);                                                                    // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, 6 + pt_buffer_TableauSequencesLED - 10);                                                         // Positionne à "0" le PORTB temporaire de rang "6 + pt_buffer_TableauSequencesLED - 10"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, 6 + pt_buffer_TableauSequencesLED - 10);                                                           // Positionne à "1" le PORTB temporaire de rang "6 + pt_buffer_TableauSequencesLED - 10"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED19; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 19 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED19 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 19 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED);                                                                  // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED);                                                                    // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, 6 + pt_buffer_TableauSequencesLED - 10);                                                         // Positionne à "0" le PORTB temporaire de rang "6 + pt_buffer_TableauSequencesLED - 10"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, 6 + pt_buffer_TableauSequencesLED - 10);                                                           // Positionne à "1" le PORTB temporaire de rang "6 + pt_buffer_TableauSequencesLED - 10"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED19; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 19 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED19 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 19 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED19; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 19 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED19 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 19 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée 5 *****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 5) // *********************************************************************************************** // Si le mode courant "5" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED21; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 21 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED21 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 21 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED20; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 20 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED20 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 20 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(int(DureeHIGH_LOW / 2.0));                                                                         // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED21; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 21 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED21 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 21 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED20; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 20 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED20 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 20 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(int(DureeHIGH_LOW / 2.0));                                                                         // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée 6 *****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 6) // *********************************************************************************************** // Si le mode courant "6" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED2; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquence du tableau 2 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED2 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 2 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED2; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 1 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED2 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 1 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED5; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 5 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED5 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 5 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED2; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 1 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED2 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 1 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED2; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 2 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED2 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 2 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    PORTD &= 0b00000011;                                                                                                        // Eteint toutes les LED du PORTD
    PORTB &= 0b11000000;                                                                                                        // Eteint toutes les LED du PORTB
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED3; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 3 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED3 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 3 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée 7 *****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 7) // *********************************************************************************************** // Si le mode courant "7" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED3; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 3 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED3 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 3 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(int(DureeHIGH_LOW / 2.0));                                                                         // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED5; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 5 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED5 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 5 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED6; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 6 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED6 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 6 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED];
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED6; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 6 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED6 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 6 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED5; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 5 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED5 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 5 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED];
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                              // Positionne à "0" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTD_TEMP, pt_buffer_TableauSequencesLED + 2);                                                                // Positionne à "1" le PORTD temporaire de rang "pt_buffer_TableauSequencesLED + 2"
        }
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                              // Positionne à "0" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, pt_buffer_TableauSequencesLED - 6);                                                                // Positionne à "1" le PORTB temporaire de rang "pt_buffer_TableauSequencesLED - 6"
        }
      }
      
      PORTD = PORTD_TEMP;                                                                                                       // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(int(DureeHIGH_LOW / 2.0));                                                                         // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(DureeHIGH_LOW * 4);                                                                                  // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    PORTD &= 0b00000011;                                                                                                        // Eteint toutes les LED du PORTD
    PORTB &= 0b11000000;                                                                                                        // Eteint toutes les LED du PORTB
    
    Fonction_Temporisation(DureeHIGH_LOW * 4);                                                                                  // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée 8 *****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 8) // *********************************************************************************************** // Si le mode courant "8" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED6; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 6 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED6 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 6 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTB_TEMP << 2;                                                                                                  // Transfère le PORTB temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD = PORTD_TEMP ;                                                                                                      // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTD_TEMP >> 2;                                                                                                  // Transfère le PORTD temporaire vers le PORTB
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED7; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 7 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED7 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 7 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED];
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTB_TEMP << 2;                                                                                                  // Transfère le PORTB temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD = PORTD_TEMP ;                                                                                                      // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTD_TEMP >> 2;                                                                                                  // Transfère le PORTD temporaire vers le PORTB
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED6; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 6 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED6 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 6 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "1"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "0"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTB_TEMP << 2;                                                                                                  // Transfère le PORTB temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD = PORTD_TEMP ;                                                                                                      // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTD_TEMP >> 2;                                                                                                  // Transfère le PORTD temporaire vers le PORTB
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée 9 *****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 9) // *********************************************************************************************** // Si le mode courant "9" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED7; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 7 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED7 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 7 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTB_TEMP << 2;                                                                                                  // Transfère le PORTB temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD = PORTD_TEMP ;                                                                                                      // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTD_TEMP >> 2;                                                                                                  // Transfère le PORTD temporaire vers le PORTB
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED7; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 7 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED7 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 7 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED];
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTB_TEMP << 2;                                                                                                  // Transfère le PORTB temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD = PORTD_TEMP ;                                                                                                      // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTD_TEMP >> 2;                                                                                                  // Transfère le PORTD temporaire vers le PORTB
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED7; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 7 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED7 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 7 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTB_TEMP << 2;                                                                                                  // Transfère le PORTB temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD = PORTD_TEMP ;                                                                                                      // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTD_TEMP >> 2;                                                                                                  // Transfère le PORTD temporaire vers le PORTB
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED7; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 7 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED7 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 7 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTB_TEMP << 2;                                                                                                  // Transfère le PORTB temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD = PORTD_TEMP ;                                                                                                      // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTD_TEMP >> 2;                                                                                                  // Transfère le PORTD temporaire vers le PORTB
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée 10 ****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 10) // ********************************************************************************************** // Si le mode courant "10" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED8; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 8 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED8 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 8 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED];
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTB_TEMP << 2;                                                                                                  // Transfère le PORTB temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD = PORTD_TEMP ;                                                                                                      // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTD_TEMP >> 2;                                                                                                  // Transfère le PORTD temporaire vers le PORTB
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED8; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 8 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED8 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 8 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED];
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTB_TEMP << 2;                                                                                                  // Transfère le PORTB temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD = PORTD_TEMP ;                                                                                                      // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTD_TEMP >> 2;                                                                                                  // Transfère le PORTD temporaire vers le PORTB
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED8; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 8 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED8 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 8 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTB_TEMP << 2;                                                                                                  // Transfère le PORTB temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD = PORTD_TEMP ;                                                                                                      // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTD_TEMP >> 2;                                                                                                  // Transfère le PORTD temporaire vers le PORTB
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED8; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 8 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED8 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 8 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < NombreLED; pt_buffer_TableauSequencesLED++) // -- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
                                                                                                                                // Inverse l'ordre de la ligne courante du tableau buffer et mémorise dans le tableau "bufferInverse_TableauSequencesLED"
        bufferInverse_TableauSequencesLED[12 - pt_buffer_TableauSequencesLED - 1] = !(buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED]);
      }
      
      memcpy(buffer_TableauSequencesLED, bufferInverse_TableauSequencesLED, NombreLED);                                         // Extrait la ligne courante inversée du tableau "bufferInverse_TableauSequencesLED" et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      byte PORTD_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTD temporaire
      byte PORTB_TEMP = 0b00000000;                                                                                             // Déclare et définit un PORTB temporaire
      
      for (int pt_buffer_TableauSequencesLED = 0; pt_buffer_TableauSequencesLED < 6; pt_buffer_TableauSequencesLED++) // ------ // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "0"
        {
          bitClear(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                          // Positionne à "0" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "6 - pt_buffer_TableauSequencesLED + 1" est égale à "1"
        {
          bitSet(PORTD_TEMP, 6 - pt_buffer_TableauSequencesLED + 1);                                                            // Positionne à "1" le PORTD temporaire de rang "6 - pt_buffer_TableauSequencesLED + 1"
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (int pt_buffer_TableauSequencesLED = 6; pt_buffer_TableauSequencesLED < 12; pt_buffer_TableauSequencesLED++) // ----- // Parcourt le tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED
      {
        if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 0) // ................................................ // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "0"
        {
          bitClear(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                          // Positionne à "0" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
        else if (buffer_TableauSequencesLED[pt_buffer_TableauSequencesLED] == 1) // ........................................... // Si l'adresse de rang "pt_buffer_TableauSequencesLED" est égale à "1"
        {
          bitSet(PORTB_TEMP, 6 - pt_buffer_TableauSequencesLED + 5);                                                            // Positionne à "1" le PORTB temporaire de rang "6 - pt_buffer_TableauSequencesLED + 5"
        }
      }
      
      PORTD = PORTB_TEMP << 2;                                                                                                  // Transfère le PORTB temporaire vers le PORTD
      PORTB = PORTB_TEMP;                                                                                                       // Transfère le PORTB temporaire vers le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD = PORTD_TEMP ;                                                                                                      // Transfère le PORTD temporaire vers le PORTD
      PORTB = PORTD_TEMP >> 2;                                                                                                  // Transfère le PORTD temporaire vers le PORTB
    }
  }

//**************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************
}

//**************************************************************************************************************************************************************************
//*** Fonction d'interruption de changement de broche du bouton poussoir "BPModePlus" **************************************************************************************
//**************************************************************************************************************************************************************************
void InterruptionBPModePlus (void)
{
  static unsigned long DateDernierChangement = 0;                                                                               // Déclare et Initialise la date du dernier changement du bouton poussoir "BPModePlus"
  
  unsigned long DateCourante = millis();                                                                                        // Déclare et Mémorise la date courante
  
  if ((DateCourante - DateDernierChangement) > DureeAntiRebond) // ************************************************************ // Si la durée de l'anti-rebonds est écoulée
  {
    cli();                                                                                                                      // Désactive les interruptions
    
    if (ModeAuto) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode automatique est activé
    {
      ModeAuto = false;                                                                                                         // Désactive le mode automatique
      compteurAffichageModeCourant = 0;                                                                                         // Réinitialise le compteur d'affichages du mode courant en mode automatique
      MultipleNombreAffichageModeCourant = 1;                                                                                   // Réinitialise le multiple du nombre d'affichages du mode courant en mode automatique
    }
    
    FonctionReinitialisation();                                                                                                 // Appelle la fonction de réinitialisation
    
    ModeCourant++;                                                                                                              // Incrémente le mode courant
    if (ModeCourant > NombreModesMax) {ModeCourant = 0;} // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Borne le mode courant
    //Serial.println("Int+"); // Débug
    //Serial.println(ModeCourant); // Débug
    
    SortieModeCourant = true;                                                                                                   // Active l'indicateur de sortie du mode courant
    
    DateDernierChangement = DateCourante;                                                                                       // Mémorise la dernière date courante
    
    sei();                                                                                                                      // Active les interruptions
  }
}

//**************************************************************************************************************************************************************************
//*** Fonction d'interruption de changement de broche du bouton poussoir "BPModeMoins" *************************************************************************************
//**************************************************************************************************************************************************************************
void InterruptionBPModeMoins (void)
{
  static unsigned long DateDernierChangement = 0;                                                                               // Déclare et Initialise la date du dernier changement du bouton poussoir "BPModeMoins"
  
  unsigned long DateCourante = millis();                                                                                        // Déclare et Mémorise la date courante
  
  if ((DateCourante - DateDernierChangement) > DureeAntiRebond) // ************************************************************ // Si la durée de l'anti-rebonds est écoulée
  {
    cli();                                                                                                                      // Désactive les interruptions
    
    if (ModeAuto) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode automatique est activé
    {
      ModeAuto = false;                                                                                                         // Désactive le mode automatique
      compteurAffichageModeCourant = 0;                                                                                         // Réinitialise le compteur d'affichages du mode courant en mode automatique
      MultipleNombreAffichageModeCourant = 1;                                                                                   // Réinitialise le multiple du nombre d'affichages du mode courant en mode automatique
    }
    
    FonctionReinitialisation();                                                                                                 // Appelle la fonction de réinitialisation
    
    ModeCourant--;                                                                                                              // Décrémente le mode courant
    if (ModeCourant < 0) {ModeCourant = NombreModesMax;} // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Borne le mode courant
    //Serial.println("Int-"); // Débug
    //Serial.println(ModeCourant); // Débug
    
    SortieModeCourant = true;                                                                                                   // Active l'indicateur de sortie du mode courant
    
    DateDernierChangement = DateCourante;                                                                                       // Mémorise la dernière date courante
    
    sei();                                                                                                                      // Active les interruptions
  }
}

//**************************************************************************************************************************************************************************
//*** Fonction d'interruption de changement de broche du bouton poussoir "BPModeAuto" *************************************************************************************
//**************************************************************************************************************************************************************************
void InterruptionBPModeAuto ()
{
  static unsigned long DateDernierChangement = 0;                                                                               // Déclare et Initialise la date du dernier changement du bouton poussoir "BPModeMoins"
  
  unsigned long DateCourante = millis();                                                                                        // Déclare et Mémorise la date courante
  
  if ((DateCourante - DateDernierChangement) > DureeAntiRebond) // ************************************************************ // Si la durée de l'anti-rebonds est écoulée
  {
    cli();                                                                                                                      // Désactive les interruptions
    
    FonctionReinitialisation();                                                                                                 // Appelle la fonction de réinitialisation
    
    Buzzer(100, 0, 1);                                                                                                          // Active le buzzer 100ms
    
    ModeAuto = !ModeAuto;                                                                                                       // Active ou désactive le mode automatique
    
    ModeCourant = 0;                                                                                                            // Initialise le mode courant
    DernierModeCourant = -1;                                                                                                    // Initialise le dernier mode courant
    MultipleNombreAffichageModeCourant = 1;                                                                                     // Initialise le multiple du nombre d'affichages du mode courant en mode automatique
    
    SortieModeCourant = true;                                                                                                   // Active l'indicateur de sortie du mode courant
    
    DateDernierChangement = DateCourante;                                                                                       // Mémorise la dernière date courante
    
    sei();                                                                                                                      // Active les interruptions
  }
}

//**************************************************************************************************************************************************************************
//*** Fonction de réinitialisation *****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
void FonctionReinitialisation ()
{
  cli();                                                                                                                        // Désactive les interruptions
  
  DDRD |= 0b11111100;                                                                                                           // Configure les broches PD2, PD3, PD4, PD5, PD7 et PD6 en sorties
  DDRB |= 0b00111111;                                                                                                           // Configure les broches PB0, PB1, PB2, PB3, PB4 et PB5 en sorties
  
  PORTD &= 0b00000011;                                                                                                          // Eteint toutes les LED du PORTD
  PORTB &= 0b11000000;                                                                                                          // Eteint toutes les LED du PORTB
  
  compteurAffichageModeCourant = 0;                                                                                             // Réinitialise le compteur d'affichages du mode courant en mode automatique
  
  sei();                                                                                                                        // Active les interruptions
}

//**************************************************************************************************************************************************************************
//*** Fonction de gestion de l'affichage sur l'écran OLED 1.3" *************************************************************************************************************
//**************************************************************************************************************************************************************************
void FonctionAffichageOLED ()
{
  u8x8.draw2x2String(6, 3, u8x8_u16toa(ModeCourant, 2));                                                                        // Positionne et Affiche le mode courant
}

//**************************************************************************************************************************************************************************
//*** Fonction Gestion Buzzer **********************************************************************************************************************************************
//**************************************************************************************************************************************************************************
void Buzzer (int TempsH, int TempsL, int nb)                                                                                    // TempsH => délai buzzer ON, TempsL => délai buzzer OFF, nb => nombre de bips
{
  for (int x = 1; x <= nb; x++) // ******************************************************************************************** // Boucle le nombre de fois voulu passé par l'argument "int nb"
  {
    digitalWrite(BrocheBuzzer, HIGH);                                                                                           // Active le buzzer
    delay(TempsH);                                                                                                              // Temporisation à l'état haut du buzzer pendant la durée passée par l'argument "int TempsH"
    digitalWrite(BrocheBuzzer, LOW);                                                                                            // Désactive le buzzer
    delay(TempsL);                                                                                                              // Temporisation à l'état bas du buzzer pendant la durée passée par l'argument "int TempsL"
  }
}

//**************************************************************************************************************************************************************************
//*** Fonction de temporisation non bloquante ******************************************************************************************************************************
//**************************************************************************************************************************************************************************
void Fonction_Temporisation (unsigned long Delai_Attente)
{
  unsigned long TimeOut = millis();                                                                                             // Démarre la temporisation
  
  while (millis() - TimeOut <= Delai_Attente) // ****************************************************************************** // Tant que la temporisation n'est pas écoulée
  {
    if (SortieModeCourant) // ------------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      return;                                                                                                                   // Retour début loop()
    }
  }
}
