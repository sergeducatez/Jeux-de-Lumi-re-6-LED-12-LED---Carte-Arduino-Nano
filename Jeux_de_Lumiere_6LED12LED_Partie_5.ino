//**************************************************************************************************************************************************************************
//*********************************************** Jeux de Lumière 6 LED 12 LED - Partie 5 - Carte Nano (com49) *************************************************************
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
// Utilisation de la librairie "U8x8lib.h"
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
volatile const int NombreModesMax = 34; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de modes maximum

volatile bool SortieModeCourant = false;                                                                                        // Indicateur de sortie du mode courant

volatile bool ModeAuto = false;                                                                                                 // Indicateur du mode automatique
volatile int compteurAffichageModeCourant = 0;                                                                                  // Compteur d'affichages du mode courant en mode automatique
const int NombreAffichageModeCourant = 3;                                                                                       // Nombre d'affichages du mode courant en mode automatique
volatile int MultipleNombreAffichageModeCourant = 1;                                                                            // Multiple du nombre d'affichages du mode courant en mode automatique
bool AffichageModeManuel = true;                                                                                                // Indicateur d'affichage du mode manuel sur l'écran OLED
bool AffichageModeAuto = false;                                                                                                 // Indicateur d'affichage du mode auto sur l'écran OLED

volatile const unsigned long DureeAntiRebond = 5ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de l'anti-rebonds des boutons poussoirs "BPModePlus" et "BPModeMoins" en ms

int PointeurTableauBrochesLED;                                                                                                  // Pointeur des tableaux des broches de 6 LED
const int TableauBrochesLED_D2D7 [] = {2, 3, 4, 5, 6, 7}; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des broches des 6 LED de (D2 à D7)
const int TableauBrochesLED_D8D13 [] = {8, 9, 10, 11, 12, 13}; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des broches des 6 LED de (D8 à D13)
int PointeurTableauBroches12LED;                                                                                                // Pointeur du tableau des broches 12 LED
int PointeurTableauBroches12LEDTemp;                                                                                            // Pointeur de fin temporaire du tableau des broches 12 LED
const int TableauBroches12LED [] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des broches des 12 LED
int compteurOffsetPointeurTableauBroches12LED;                                                                                  // Compteur de décalage du pointeur du tableau des broches 12 LED
int OffsetPointeurTableauBroches12LED;                                                                                          // Valeur du décalage du pointeur du tableau des broches 12 LED
int CompteurPointeurTableauBroches12LEDMIN;                                                                                     // Valeur min du pointeur du tableau des broches 12 LED

unsigned long DureeHIGH_LOW;                                                                                                    // Temporisation de la LED activée ou désactivée

bool InverseurDeplacement = true;                                                                                               // Indicateur du sens de déplacement

bool InverseurMasquesPORTS = true;                                                                                              // Inverseur des masques des PORTS D et B

int ADRDepartSequenceCouranteLED = 0;                                                                                           // Adresse de départ des tableaux des séquences d'affichage des LED
const byte NombreLED = 12; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de LED
int SequenceCouranteLED = 0;                                                                                                    // Séquence courante des LED affichées

byte buffer_TableauSequencesLED [12];                                                                                           // Tableau buffer contenant les lignes des tableaux des séquences d'affichage des LED

const byte NombreSequencesLED0 = 10; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool TableauSequencesLED0 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 0 des séquences d'affichage des LED
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, // 0 
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
   1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, // 2
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 3
   0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, // 4
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 5
   0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, // 6
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 7
   0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, // 8
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // 9
};

const byte NombreSequencesLED1 = 4; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool TableauSequencesLED1 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 1 des séquences d'affichage des LED
   1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, // 0
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
   0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, // 2
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // 3
};

const byte NombreSequencesLED2 = 15; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED2 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 2 des séquences d'affichage des LED (Slide gauche à droite)
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0
   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
   1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 2
   0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 3
   0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 4
   0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 5
   1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, // 6
   1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, // 7
   1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, // 8
   0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, // 9
   0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, // 10
   0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, // 11
   1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, // 12
   1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, // 13
   1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0  // 14 => 9
};

const byte NombreSequencesLED3 = 36; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED3 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 3 des séquences d'affichage des LED
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

const byte NombreSequencesLED5 = 15; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED5 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 5 des séquences d'affichage des LED (Slide droite à gauche)
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

const byte NombreSequencesLED6 = 18; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED6 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 6 des séquences d'affichage des LED (Slide droite à gauche)
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

const byte NombreSequencesLED7 = 35; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED7 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 7 des séquences d'affichage des LED
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

const byte NombreSequencesLED8 = 15; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED8 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 8 des séquences d'affichage des LED
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0
   0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
   1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 2
   0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 3
   0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, // 4
   1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, // 5
   0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, // 6
   1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, // 7
   0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, // 8
   0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, // 9
   1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, // 10
   0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, // 11
   1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, // 12
   0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, // 13
   0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0  // 14 => 10
};

const byte NombreSequencesLED9 = 28; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED9 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 9 des séquences d'affichage des LED
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

const byte NombreSequencesLED10 = 24; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED10 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 10 des séquences d'affichage des LED
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

const byte NombreSequencesLED11 = 48; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED11 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 11 des séquences d'affichage des LED
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

const byte NombreSequencesLED12 = 24; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED12 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 12 des séquences d'affichage des LED
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


const byte NombreSequencesLED13 = 24; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED13 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 13 des séquences d'affichage des LED
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

const byte NombreSequencesLED14 = 10; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED14 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 14 des séquences d'affichage des LED
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

const byte NombreSequencesLED15 = 10; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool PROGMEM TableauSequencesLED15 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 15 des séquences d'affichage des LED
   1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, // 0
   0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, // 1
   0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, // 2
   0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 3
   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 4
   0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, // 5
   0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 6
   0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, // 7
   0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, // 8
   1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1  // 9
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
      // ---------------------------------------------------------------------------------------------------------------------- // Si le mode courant est différent des modes courants suivants 
      if (ModeCourant != 7 && ModeCourant != 8)
      {
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
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
//*** Animation 12 LED 1 ***************************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  if (ModeCourant == 0) // **************************************************************************************************** // Si le mode courant "0" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 1;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DureeHIGH_LOW = 500ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= 2; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], HIGH);                                                 // Allume la LED de rang "PointeurTableauBroches12LED + 3" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 6], HIGH);                                                 // Allume la LED de rang "PointeurTableauBroches12LED + 6" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 9], HIGH);                                                 // Allume la LED de rang "PointeurTableauBroches12LED + 9" du tableau des broches des 12 LED
      
      Fonction_Temporisation(int(DureeHIGH_LOW / (PointeurTableauBroches12LED + 1.0)));                                         // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= 2; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED + 3" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 6], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED + 6" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 9], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED + 9" du tableau des broches des 12 LED
      
      Fonction_Temporisation(int(DureeHIGH_LOW / (PointeurTableauBroches12LED + 1.0)));                                         // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= 2; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[11 - 0 - PointeurTableauBroches12LED], HIGH);                                            // Allume la LED de rang "11 - 0 - PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[11 - 3 - PointeurTableauBroches12LED], HIGH);                                            // Allume la LED de rang "11 - 3 - PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[11 - 6 - PointeurTableauBroches12LED], HIGH);                                            // Allume la LED de rang "11 - 6 - PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[11 - 9 - PointeurTableauBroches12LED], HIGH);                                            // Allume la LED de rang "PointeurTableauBroches12LED + 9" du tableau des broches des 12 LED
      
      Fonction_Temporisation(int(DureeHIGH_LOW / (PointeurTableauBroches12LED + 1.0)));                                         // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= 2; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[11 - 0 - PointeurTableauBroches12LED], LOW);                                             // Eteint la LED de rang "11 - 0 - PointeurTableauBroches12LED" du tableau des broches des LED
      digitalWrite(TableauBroches12LED[11 - 3 - PointeurTableauBroches12LED], LOW);                                             // Eteint la LED de rang "11 - 3 - PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[11 - 6 - PointeurTableauBroches12LED], LOW);                                             // Eteint la LED de rang "11 - 6 - PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[11 - 9 - PointeurTableauBroches12LED], LOW);                                             // Eteint la LED de rang "11 - 9 - PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(int(DureeHIGH_LOW / (PointeurTableauBroches12LED + 1.0)));                                         // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED 2 ***************************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 1) // *********************************************************************************************** // Si le mode courant "1" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 1;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DureeHIGH_LOW = 500ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= 2; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], HIGH);                                                 // Allume la LED de rang "PointeurTableauBroches12LED + 3" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 6], HIGH);                                                 // Allume la LED de rang "PointeurTableauBroches12LED + 6" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 9], HIGH);                                                 // Allume la LED de rang "PointeurTableauBroches12LED + 9" du tableau des broches des 12 LED
      
      Fonction_Temporisation(int(DureeHIGH_LOW / (PointeurTableauBroches12LED + 1.0)));                                         // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // ++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED courante de rang "PointeurTableauBroches12LED"
      
      Fonction_Temporisation(int(DureeHIGH_LOW / 5.0));                                                                         // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED courante de rang "PointeurTableauBroches12LED"
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= 9; PointeurTableauBroches12LED--) // ++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED - 3" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 6], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED - 6" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 9], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED - 9" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des LED
      
      Fonction_Temporisation(int(DureeHIGH_LOW / ((3 - PointeurTableauBroches12LED + 8) + 1.0)));                               // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBroches12LED = NombreLED - 1; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // +++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED courante de rang "PointeurTableauBroches12LED"
      
      Fonction_Temporisation(int(DureeHIGH_LOW / 5.0));                                                                         // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED courante de rang "PointeurTableauBroches12LED"
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Slide 12 LED séquencées 3 LED espacées de 3 LED déplacement de gauche à droite ***************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 2) // *********************************************************************************************** // Si le mode courant "2" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 9;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      ADRDepartSequenceCouranteLED = 0;                                                                                         // Initialise l'adresse de départ du tableau 2 des séquences d'affichage des LED
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt les séquences du tableau 2 des séquences d'affichage des LED
    for (SequenceCouranteLED = ADRDepartSequenceCouranteLED; SequenceCouranteLED < NombreSequencesLED2; SequenceCouranteLED++)
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED2 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 2 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    ADRDepartSequenceCouranteLED = 9;                                                                                           // Définit l'adresse de départ du tableau 2 des séquences d'affichage des LED
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 2 ****************************************************************************************************************************************
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
      DureeHIGH_LOW = 200ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    int ADRDepartTableauSequencesLED = 0;                                                                                       // Déclare et initialise l'adresse de départ de chaque séquence du tableau 1 des séquences d'affichage des LED
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED1; SequenceCouranteLED += 2) // +++++++++++++++++++++ // Parcourt les séquences du tableau 1 des séquences d'affichage des LED
    {
      ADRDepartTableauSequencesLED = NombreLED * SequenceCouranteLED;                                                           // Calcule l'adresse de départ de la séquence courante du tableau 1 des séquences d'affichage des LED
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "ADRDepartTableauSequencesLED + PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], TableauSequencesLED1[ADRDepartTableauSequencesLED + PointeurTableauBroches12LED]);
        
        Fonction_Temporisation(int(DureeHIGH_LOW / 4.0));                                                                       // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 3 ****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 4) // *********************************************************************************************** // Si le mode courant "4" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 2;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    int ADRDepartTableauSequencesLED = 0;                                                                                       // Déclare et initialise l'adresse de départ de chaque séquence du tableau 0 des séquences d'affichage des LED
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED0; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 0 des séquences d'affichage des LED
    {
      ADRDepartTableauSequencesLED = NombreLED * SequenceCouranteLED;                                                           // Calcule l'adresse de départ de la séquence courante du tableau 0 des séquences d'affichage des LED
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "ADRDepartTableauSequencesLED + PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], TableauSequencesLED0[ADRDepartTableauSequencesLED + PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 3 / 2.0));                                                                       // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED0; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 0 des séquences d'affichage des LED
    {
      ADRDepartTableauSequencesLED = NombreLED * SequenceCouranteLED;                                                           // Calcule l'adresse de départ de la séquence courante du tableau 0 des séquences d'affichage des LED
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "ADRDepartTableauSequencesLED + PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !TableauSequencesLED0[ADRDepartTableauSequencesLED + PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 3 / 2.0));                                                                       // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED0; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 0 des séquences d'affichage des LED
    {
      ADRDepartTableauSequencesLED = NombreLED * SequenceCouranteLED;                                                           // Calcule l'adresse de départ de la séquence courante du tableau 0 des séquences d'affichage des LED
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "ADRDepartTableauSequencesLED + PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !TableauSequencesLED0[ADRDepartTableauSequencesLED + PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 3 / 2.0));                                                                       // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED0; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 0 des séquences d'affichage des LED
    {
      ADRDepartTableauSequencesLED = NombreLED * SequenceCouranteLED;                                                           // Calcule l'adresse de départ de la séquence courante du tableau 0 des séquences d'affichage des LED
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "ADRDepartTableauSequencesLED + PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], TableauSequencesLED0[ADRDepartTableauSequencesLED + PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 3 / 2.0));                                                                       // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED affichage pair impair ******************************************************************************************************************************
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
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // ++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      if (PointeurTableauBroches12LED % 2 == 0) // ---------------------------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est pair
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // ++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      if (PointeurTableauBroches12LED % 2 == 0) // ---------------------------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est pair
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    for (PointeurTableauBroches12LED = NombreLED - 1; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // +++++ // Parcourt le tableau des broches des 12 LED
    {
      if (PointeurTableauBroches12LED % 2 != 0) // ---------------------------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est impair
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    for (PointeurTableauBroches12LED = NombreLED - 1; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // +++++ // Parcourt le tableau des broches des 12 LED
    {
      if (PointeurTableauBroches12LED % 2 != 0) // ---------------------------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est pair
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED affichage aller effacement retour pair et impair affichage retour effacement aller pair et impair **************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 6) // *********************************************************************************************** // Si le mode courant "6" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 1;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // ++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED"
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = NombreLED  - 1; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // ++++ // Parcourt le tableau des broches des 12 LED
    {
      if (PointeurTableauBroches12LED % 2 == 0) // ---------------------------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est pair
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    for (PointeurTableauBroches12LED = NombreLED - 1; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // +++++ // Parcourt le tableau des broches des 12 LED
    {
      if (PointeurTableauBroches12LED % 2 != 0) // ---------------------------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est pair
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    for (PointeurTableauBroches12LED = NombreLED - 1; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // +++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED"
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= NombreLED - 1; PointeurTableauBroches12LED++) // +++++ // Parcourt le tableau des broches des 12 LED
    {
      if (PointeurTableauBroches12LED % 2 == 0) // ---------------------------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est impair
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= NombreLED - 1; PointeurTableauBroches12LED++) // +++++ // Parcourt le tableau des broches des 12 LED
    {
      if (PointeurTableauBroches12LED % 2 != 0) // ---------------------------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est pair
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED déplacements alternés LED de droite à gauche et remplissage de gauche à droite *********************************************************************
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
      CompteurPointeurTableauBroches12LEDMIN = 0;                                                                               // Initialise la valeur min du pointeur du tableau des broches 12 LED
      InverseurDeplacement = true;                                                                                              // Initialise l'indicateur du sens de déplacement
      DureeHIGH_LOW = 20ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (InverseurDeplacement) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sens est activé
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = NombreLED - 1; PointeurTableauBroches12LED >= CompteurPointeurTableauBroches12LEDMIN; PointeurTableauBroches12LED--)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        // .................................................................................................................... // Si "PointeurTableauBroches12LED + 1" est inférieur à "12" => Eteint la LED de rang "PointeurTableauBroches12LED + 1"
        if (PointeurTableauBroches12LED + 1 < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], LOW);}
        
        Fonction_Temporisation(DureeHIGH_LOW + int(CompteurPointeurTableauBroches12LEDMIN * 2));                                // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      CompteurPointeurTableauBroches12LEDMIN++;                                                                                 // Incrémente la valeur min du pointeur du tableau des broches 12 LED
      if (CompteurPointeurTableauBroches12LEDMIN == 12) // -------------------------------------------------------------------- // Si la valeur min du pointeur du tableau des broches 12 LED est égale à "12"
      {
        InverseurDeplacement = !InverseurDeplacement;                                                                           // Inverse l'indicateur du sens de déplacement
        PointeurTableauBroches12LED = 11;                                                                                       // Réinitialise la valeur min du pointeur du tableau des broches 12 LED
      }
      
      Fonction_Temporisation(int(DureeHIGH_LOW * 5));                                                                           // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    else if (!InverseurDeplacement) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sens est désactivé
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= CompteurPointeurTableauBroches12LEDMIN; PointeurTableauBroches12LED++)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        // -------------------------------------------------------------------------------------------------------------------- // Si "PointeurTableauBroches12LED - 1" est supérieur inférieur à "-1" => Eteint la LED de rang "PointeurTableauBroches12LED - 1"
        if (PointeurTableauBroches12LED - 1 > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], LOW);}
        
        Fonction_Temporisation(DureeHIGH_LOW + int((12 - CompteurPointeurTableauBroches12LEDMIN - 1) * 2));                     // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      CompteurPointeurTableauBroches12LEDMIN--;                                                                                 // Incrémente la valeur min du pointeur du tableau des broches 12 LED
      if (CompteurPointeurTableauBroches12LEDMIN < 0) // ---------------------------------------------------------------------- // Si la valeur min du pointeur du tableau des broches 12 LED est inférieure à "0"
      {
        InverseurDeplacement = !InverseurDeplacement;                                                                           // Inverse l'indicateur du sens de déplacement
        PointeurTableauBroches12LED = 0;                                                                                        // Réinitialise le pointeur du tableau des broches 12 LED
        
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
      
      Fonction_Temporisation(int(DureeHIGH_LOW * 5));                                                                           // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED déplacements alternés LED de droite à gauche et vidage de gauche à droite **************************************************************************
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
      CompteurPointeurTableauBroches12LEDMIN = -1;                                                                              // Initialise la valeur min du pointeur du tableau des broches 12 LED
      InverseurDeplacement = true;                                                                                              // Initialise l'indicateur du sens de déplacement
      DureeHIGH_LOW = 20ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (InverseurDeplacement) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sens est activé
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = NombreLED - 1; PointeurTableauBroches12LED >= CompteurPointeurTableauBroches12LEDMIN; PointeurTableauBroches12LED--)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW + int(abs(CompteurPointeurTableauBroches12LEDMIN) * 2));                           // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        // .................................................................................................................... // Si "PointeurTableauBroches12LED + 1" est inférieur à "12" => Allume la LED de rang "PointeurTableauBroches12LED + 1"
        if (PointeurTableauBroches12LED + 1 < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);}
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      CompteurPointeurTableauBroches12LEDMIN++;                                                                                 // Incrémente la valeur min du pointeur du tableau des broches 12 LED
      if (CompteurPointeurTableauBroches12LEDMIN == 12) // -------------------------------------------------------------------- // Si la valeur min du pointeur du tableau des broches 12 LED est égale à "12"
      {
        InverseurDeplacement = !InverseurDeplacement;                                                                           // Inverse l'indicateur du sens de déplacement
        PointeurTableauBroches12LED = 11;                                                                                       // Réinitialise la valeur min du pointeur du tableau des broches 12 LED
      }
      
      Fonction_Temporisation(int(DureeHIGH_LOW * 5));                                                                           // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    else if (!InverseurDeplacement) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sens est désactivé
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= CompteurPointeurTableauBroches12LEDMIN; PointeurTableauBroches12LED++)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW + int((12 - CompteurPointeurTableauBroches12LEDMIN - 1) * 2));                     // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        // -------------------------------------------------------------------------------------------------------------------- // Si "PointeurTableauBroches12LED - 1" est supérieur inférieur à "-1" => Allume la LED de rang "PointeurTableauBroches12LED - 1"
        if (PointeurTableauBroches12LED - 1 > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);}
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      CompteurPointeurTableauBroches12LEDMIN--;                                                                                 // Incrémente la valeur min du pointeur du tableau des broches 12 LED
      if (CompteurPointeurTableauBroches12LEDMIN < 0) // ---------------------------------------------------------------------- // Si la valeur min du pointeur du tableau des broches 12 LED est inférieure à "0"
      {
        InverseurDeplacement = !InverseurDeplacement;                                                                           // Inverse l'indicateur du sens de déplacement
        PointeurTableauBroches12LED = 0;                                                                                        // Réinitialise le pointeur du tableau des broches 12 LED
        
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
      
      Fonction_Temporisation(int(DureeHIGH_LOW * 5));                                                                           // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 4 ****************************************************************************************************************************************
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
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED3; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 3 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED3 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 3 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 5 ****************************************************************************************************************************************
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
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED3; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 3 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED3 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 3 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Slide 12 LED séquencées 4 LED espacées de 2 LED déplacement de droite à gauche ***************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 11) // ********************************************************************************************** // Si le mode courant "11" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 6;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      ADRDepartSequenceCouranteLED = 0;                                                                                         // Initialise l'adresse de départ du tableau 6 des séquences d'affichage des LED
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt les séquences du tableau 6 des séquences d'affichage des LED
    for (SequenceCouranteLED = ADRDepartSequenceCouranteLED; SequenceCouranteLED < NombreSequencesLED6; SequenceCouranteLED++)
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED6 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 6 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    ADRDepartSequenceCouranteLED = 12;                                                                                          // Définit l'adresse de départ du tableau 6 des séquences d'affichage des LED
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 5 ****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 12) // ********************************************************************************************** // Si le mode courant "12" est sélectionné
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
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED7; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 7 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED7 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 7 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Slide 12 LED séquencées 1 LED espacée de 1 LED espacées de 2 LED déplacement de gauche à droite **********************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 13) // ********************************************************************************************** // Si le mode courant "13" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 9;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      ADRDepartSequenceCouranteLED = 0;                                                                                         // Initialise l'adresse de départ du tableau 8 des séquences d'affichage des LED
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt les séquences du tableau 8 des séquences d'affichage des LED
    for (SequenceCouranteLED = ADRDepartSequenceCouranteLED; SequenceCouranteLED < NombreSequencesLED8; SequenceCouranteLED++)
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED8 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 8 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    ADRDepartSequenceCouranteLED = 10;                                                                                          // Définit l'adresse de départ du tableau 6 des séquences d'affichage des LED
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 6 ****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 14) // ********************************************************************************************** // Si le mode courant "14" est sélectionné
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
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED9; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 9 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED9 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 9 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 7 ****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 15) // ********************************************************************************************** // Si le mode courant "15" est sélectionné
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
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED10; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 10 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED10 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 10 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 8 ****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 16) // ********************************************************************************************** // Si le mode courant "16" est sélectionné
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
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED10; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 10 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED10 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 10 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = NombreLED; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // ------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !(buffer_TableauSequencesLED[PointeurTableauBroches12LED]));
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 9 ****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 17) // ********************************************************************************************** // Si le mode courant "17" est sélectionné
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
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED10; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 10 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED10 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 10 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED10; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 10 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED10 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 10 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = NombreLED; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // ------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !(buffer_TableauSequencesLED[PointeurTableauBroches12LED]));
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 10 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 18) // ********************************************************************************************** // Si le mode courant "18" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 2;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED3; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 3 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED3 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 3 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
        if (PointeurTableauBroches12LED % 2 == 0) continue;                                                                     // Si "PointeurTableauBroches12LED"
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !(buffer_TableauSequencesLED[PointeurTableauBroches12LED]));
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
        if (PointeurTableauBroches12LED % 2 != 0) continue;                                                                     // Si "PointeurTableauBroches12LED"
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !(buffer_TableauSequencesLED[PointeurTableauBroches12LED]));
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD &= 0b00000011;                                                                                                      // Eteint toutes les LED du PORTD
      PORTB &= 0b11000000;                                                                                                      // Eteint toutes les LED du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 11 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 19) // ********************************************************************************************** // Si le mode courant "19" est sélectionné
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
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED7; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 7 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED7 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 7 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !(buffer_TableauSequencesLED[PointeurTableauBroches12LED]));
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 12 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 20) // ********************************************************************************************** // Si le mode courant "20" est sélectionné
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
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = NombreSequencesLED7 - 1; SequenceCouranteLED >= 0; SequenceCouranteLED--) // +++++++++++++++++++ // Parcourt les séquences du tableau 7 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED7 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 7 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = NombreLED - 1; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // --- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD &= 0b00000011;                                                                                                      // Eteint toutes les LED du PORTD
      PORTB &= 0b11000000;                                                                                                      // Eteint toutes les LED du PORTB
      
      Fonction_Temporisation(int(DureeHIGH_LOW / 2.0));                                                                         // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 13 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 21) // ********************************************************************************************** // Si le mode courant "21" est sélectionné
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
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }

    DureeHIGH_LOW = 80ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED11; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 11 des séquences d'affichage des LED
    {
      if (SequenceCouranteLED > 11) {DureeHIGH_LOW = 40ul;} // ---------------------------------------------------------------- // Redéfinit la temporisation de la LED activée ou désactivée
      
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED11 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 11 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !(buffer_TableauSequencesLED[PointeurTableauBroches12LED]));
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }

    DureeHIGH_LOW = 80ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED11; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 11 des séquences d'affichage des LED
    {
      if (SequenceCouranteLED > 11) {DureeHIGH_LOW = 40ul;} // ---------------------------------------------------------------- // Redéfinit la temporisation de la LED activée ou désactivée
      
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED11 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 11 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[12 - PointeurTableauBroches12LED - 1]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 14 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 22) // ********************************************************************************************** // Si le mode courant "22" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 2;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }

    DureeHIGH_LOW = 80ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED11; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 11 des séquences d'affichage des LED
    {
      if (SequenceCouranteLED > 11) {DureeHIGH_LOW = 40ul;} // ---------------------------------------------------------------- // Redéfinit la temporisation de la LED activée ou désactivée
      
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED11 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 11 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }

    DureeHIGH_LOW = 80ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED11; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 11 des séquences d'affichage des LED
    {
      if (SequenceCouranteLED > 11) {DureeHIGH_LOW = 40ul;} // ---------------------------------------------------------------- // Redéfinit la temporisation de la LED activée ou désactivée
      
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED11 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 11 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !(buffer_TableauSequencesLED[12 - PointeurTableauBroches12LED - 1]));
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    DureeHIGH_LOW = 80ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED11; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 11 des séquences d'affichage des LED
    {
      if (SequenceCouranteLED > 11) {DureeHIGH_LOW = 40ul;} // ---------------------------------------------------------------- // Redéfinit la temporisation de la LED activée ou désactivée
      
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED11 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 11 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !(buffer_TableauSequencesLED[PointeurTableauBroches12LED]));
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    DureeHIGH_LOW = 80ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED11; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 11 des séquences d'affichage des LED
    {
      if (SequenceCouranteLED > 11) {DureeHIGH_LOW = 40ul;} // ---------------------------------------------------------------- // Redéfinit la temporisation de la LED activée ou désactivée
      
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED11 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 11 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[12 - PointeurTableauBroches12LED - 1]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 15 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 23) // ********************************************************************************************** // Si le mode courant "23" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 2;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }

    DureeHIGH_LOW = 80ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED11; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 11 des séquences d'affichage des LED
    {
      if (SequenceCouranteLED > 11) {DureeHIGH_LOW = 40ul;} // ---------------------------------------------------------------- // Redéfinit la temporisation de la LED activée ou désactivée
      
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED11 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 11 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }

    DureeHIGH_LOW = 80ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED11; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 11 des séquences d'affichage des LED
    {
      if (SequenceCouranteLED > 11) {DureeHIGH_LOW = 40ul;} // ---------------------------------------------------------------- // Redéfinit la temporisation de la LED activée ou désactivée
      
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED11 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 11 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !(buffer_TableauSequencesLED[12 - PointeurTableauBroches12LED - 1]));
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 16 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 24) // ********************************************************************************************** // Si le mode courant "24" est sélectionné
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
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }

    DureeHIGH_LOW = 100ul;                                                                                                      // Définit la temporisation de la LED activée ou désactivée
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED12; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 12 des séquences d'affichage des LED
    {
      if (SequenceCouranteLED > 5) {DureeHIGH_LOW = 40ul;} // ----------------------------------------------------------------- // Redéfinit la temporisation de la LED activée ou désactivée
      
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED12 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 12 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    DureeHIGH_LOW = 100ul;                                                                                                      // Définit la temporisation de la LED activée ou désactivée
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED13; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 13 des séquences d'affichage des LED
    {
      if (SequenceCouranteLED > 5) {DureeHIGH_LOW = 40ul;} // ----------------------------------------------------------------- // Redéfinit la temporisation de la LED activée ou désactivée
      
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED13 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 13 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 17 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 25) // ********************************************************************************************** // Si le mode courant "25" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 9;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED14; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 14 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED14 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 14 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 18 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 26) // ********************************************************************************************** // Si le mode courant "26" est sélectionné
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
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED14; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 14 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED14 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 14 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (int OSC = 0 ; OSC < 3; OSC++) // ----------------------------------------------------------------------------------- // Boucle 3 fois
      {
        for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // ...... // Parcourt le tableau des broches des 12 LED
        {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
            digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
        }
        
        Fonction_Temporisation(int(DureeHIGH_LOW / 2.0 - 5));                                                                   // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTD &= 0b00000011;                                                                                                    // Eteint toutes les LED du PORTD
        PORTB &= 0b11000000;                                                                                                    // Eteint toutes les LED du PORTB
        
        Fonction_Temporisation(int(DureeHIGH_LOW / 2.0 - 5));                                                                   // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 19 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 27) // ********************************************************************************************** // Si le mode courant "27" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 2;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED14; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 14 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED14 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 14 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (int OSC = 0 ; OSC < 3; OSC++) // ----------------------------------------------------------------------------------- // Boucle 3 fois
      {
        for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // ...... // Parcourt le tableau des broches des 12 LED
        {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
            digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !(buffer_TableauSequencesLED[PointeurTableauBroches12LED]));
        }
        
        Fonction_Temporisation(int(DureeHIGH_LOW / 2.0 - 5));                                                                   // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTD |= 0b11111100;                                                                                                    // Allume toutes les LED du PORTD
        PORTB |= 0b00111111;                                                                                                    // Allume toutes les LED du PORTB
        
        Fonction_Temporisation(int(DureeHIGH_LOW / 2.0 - 5));                                                                   // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 20 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 28) // ********************************************************************************************** // Si le mode courant "28" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 2;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED7; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 7 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED7 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 7 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED7; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 7 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED7 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 7 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = NombreLED; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // ------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !(buffer_TableauSequencesLED[PointeurTableauBroches12LED]));
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 21 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 29) // ********************************************************************************************** // Si le mode courant "29" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 2;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED7; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 7 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED7 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 7 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (int OSC = 0 ; OSC < 4; OSC++) // ----------------------------------------------------------------------------------- // Boucle 4 fois
      {
        for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // ...... // Parcourt le tableau des broches des 12 LED
        {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
            digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
        }
        
        Fonction_Temporisation(int(DureeHIGH_LOW / 2.0));                                                                       // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTD |= 0b10101000;                                                                                                    // Applique le masque "0b10101000" sur le PORTD
        PORTB |= 0b00101010;                                                                                                    // Applique le masque "0b00101010" sur le PORTB
        
        Fonction_Temporisation(int(DureeHIGH_LOW / 2.0));                                                                       // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 22 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 30) // ********************************************************************************************** // Si le mode courant "30" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 2;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      InverseurMasquesPORTS = true;                                                                                             // Définit l'inverseur des masques des PORTS D et B
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED3; SequenceCouranteLED++) // ++++++++++++++++++++++++ // Parcourt les séquences du tableau 3 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED3 + (NombreLED * SequenceCouranteLED), NombreLED);                // Extrait la ligne courante du tableau 3 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (int OSC = 0 ; OSC < 4; OSC++) // ----------------------------------------------------------------------------------- // Boucle 4 fois
      {
        for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // ...... // Parcourt le tableau des broches des 12 LED
        {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
            digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
        }
        
        Fonction_Temporisation(int(DureeHIGH_LOW * 2 / 3.0));                                                                   // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        if (InverseurMasquesPORTS) // ......................................................................................... // Si l'inverseur des masques des PORTS D et B est activé
        {
          PORTD |= 0b10101000;                                                                                                  // Applique le masque "0b10101000" sur le PORTD
          PORTB |= 0b00101010;                                                                                                  // Applique le masque "0b00101010" sur le PORTB
        }
        else if (!InverseurMasquesPORTS) // ................................................................................... // Si l'inverseur des masques des PORTS D et B est désactivé
        {
          PORTD |= 0b01010100;                                                                                                  // Applique le masque "0b01010100" sur le PORTD
          PORTB |= 0b00010101;                                                                                                  // Applique le masque "0b00010101" sur le PORTB
        }
        
        Fonction_Temporisation(int(DureeHIGH_LOW * 2 / 3.0));                                                                   // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      InverseurMasquesPORTS = !InverseurMasquesPORTS;                                                                           // Inverse l'inverseur des masques des PORTS D et B est désactivé
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 23 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 31) // ********************************************************************************************** // Si le mode courant "31" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 2;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      InverseurMasquesPORTS = true;                                                                                             // Définit l'inverseur des masques des PORTS D et B
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED14; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 14 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED14 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 14 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (int OSC = 0 ; OSC < 4; OSC++) // ----------------------------------------------------------------------------------- // Boucle 4 fois
      {
        for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // ...... // Parcourt le tableau des broches des 12 LED
        {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
            digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
        }
        
        Fonction_Temporisation(int(DureeHIGH_LOW * 2 / 3.0));                                                                   // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        if (InverseurMasquesPORTS) // ......................................................................................... // Si l'inverseur des masques des PORTS D et B est activé
        {
          PORTD |= 0b10101000;                                                                                                  // Applique le masque "0b10101000" sur le PORTD
          PORTB |= 0b00101010;                                                                                                  // Applique le masque "0b00101010" sur le PORTB
        }
        else if (!InverseurMasquesPORTS) // ................................................................................... // Si l'inverseur des masques des PORTS D et B est désactivé
        {
          PORTD |= 0b01010100;                                                                                                  // Applique le masque "0b01010100" sur le PORTD
          PORTB |= 0b00010101;                                                                                                  // Applique le masque "0b00010101" sur le PORTB
        }
        
        Fonction_Temporisation(int(DureeHIGH_LOW * 2 / 3.0));                                                                   // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      InverseurMasquesPORTS = !InverseurMasquesPORTS;                                                                           // Inverse l'inverseur des masques des PORTS D et B est désactivé
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 24 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 32) // ********************************************************************************************** // Si le mode courant "32" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 9;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED14; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 14 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED14 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 14 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !(buffer_TableauSequencesLED[PointeurTableauBroches12LED]));
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 25 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 33) // ********************************************************************************************** // Si le mode courant "33" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 9;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED15; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 15 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED15 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 15 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], buffer_TableauSequencesLED[PointeurTableauBroches12LED]);
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées 26 ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 34) // ********************************************************************************************** // Si le mode courant "34" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 9;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante des LED affichées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED15; SequenceCouranteLED++) // +++++++++++++++++++++++ // Parcourt les séquences du tableau 15 des séquences d'affichage des LED
    {
      memcpy_P(buffer_TableauSequencesLED, TableauSequencesLED15 + (NombreLED * SequenceCouranteLED), NombreLED);               // Extrait la ligne courante du tableau 15 des séquences d'affichage des LED et mémorise dans le tableau "buffer_TableauSequencesLED"
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !(buffer_TableauSequencesLED[PointeurTableauBroches12LED]));
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
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
