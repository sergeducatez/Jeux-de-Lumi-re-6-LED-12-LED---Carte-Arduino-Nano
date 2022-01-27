//**************************************************************************************************************************************************************************
//********************************************** Jeux de Lumière 6 LED 12 LED - Partie 1a - Carte Nano (com49) *************************************************************
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
volatile const int NombreModesMax = 34; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de modes maximum

volatile bool SortieModeCourant = false;                                                                                        // Indicateur de sortie du mode courant

volatile bool ModeAuto = false;                                                                                                 // Indicateur du mode automatique
volatile int compteurAffichageModeCourant = 0;                                                                                  // Compteur d'affichages du mode courant en mode automatique
volatile const int NombreAffichageModeCourant = 3; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre d'affichages du mode courant en mode automatique
volatile int MultipleNombreAffichageModeCourant = 1;                                                                            // Multiple du nombre d'affichages du mode courant en mode automatique
volatile bool AffichageModeManuel = true;                                                                                       // Indicateur d'affichage du mode manuel sur l'écran OLED
volatile bool AffichageModeAuto = false;                                                                                        // Indicateur d'affichage du mode auto sur l'écran OLED

volatile const unsigned long DureeAntiRebond = 5ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de l'anti-rebonds des boutons poussoirs "BPModePlus" et "BPModeMoins" en ms

unsigned long t1DebutTempo_EtatHaut;                                                                                            // Début de la temporisation à l'état haut en mode non bloquant
unsigned long t1DebutTempo_EtatBas;                                                                                             // Début de la temporisation à l'état bas en mode non bloquant
unsigned long DureeTempo_EtatBas;                                                                                               // Durée de la temporisation à l'état bas en mode non bloquant
unsigned long DureeTempo_EtatHaut;                                                                                              // Durée de la temporisation à l'état haut en mode non bloquant
int DeltaTempo_EtatHaut;                                                                                                        // Incrément ou décrément de la valeur de la temporisation à l'état haut en mode non bloquant
int DeltaTempo_EtatBas;                                                                                                         // Incrément ou décrément de la valeur de la temporisation à l'état bas en mode non bloquant

byte PORTD_TEMP;                                                                                                                // Registre temporaire du PORTD
byte PORTB_TEMP;                                                                                                                // Registre temporaire du PORTB

unsigned long t1DebutTemporisation;                                                                                             // Date courante de la temporisation en mode non bloquant
unsigned long DerniereFinTemporisation;                                                                                         // Date de la fin de la temporisation en mode non bloquant
unsigned long DureeTemporisation;                                                                                               // Durée de la temporisation en mode non bloquant

bool InverseurClignotementAlterne = false;                                                                                      // Indicateur inverseur de clignotement de LED alternées
bool DernierInverseurClignotementAlterne = false;                                                                               // Dernier état de l'indicateur inverseur de clignotement de LED alternées

bool EtatLED_D2 = HIGH;                                                                                                         // Etat courant de la LED D2
bool EtatLED_D3 = HIGH;                                                                                                         // Etat courant de la LED D3
bool EtatLED_D4 = HIGH;                                                                                                         // Etat courant de la LED D4
bool EtatLED_D5 = HIGH;                                                                                                         // Etat courant de la LED D5
bool EtatLED_D6 = HIGH;                                                                                                         // Etat courant de la LED D6
bool EtatLED_D7 = HIGH;                                                                                                         // Etat courant de la LED D7
bool DernierEtatLED_D2 = HIGH;                                                                                                  // Dernier état courant de la LED D2
bool DernierEtatLED_D3 = HIGH;                                                                                                  // Dernier état courant de la LED D3
bool DernierEtatLED_D4 = HIGH;                                                                                                  // Dernier état courant de la LED D4
bool DernierEtatLED_D5 = HIGH;                                                                                                  // Dernier état courant de la LED D5
bool DernierEtatLED_D6 = HIGH;                                                                                                  // Dernier état courant de la LED D6
bool DernierEtatLED_D7 = HIGH;                                                                                                  // Dernier état courant de la LED D7

volatile int CompteurFlash;                                                                                                     // Compteur de demi-périodes de clignotement
int NombreFlashCourts;                                                                                                          // Nombre de flashs courts

bool MuteBuzzer;                                                                                                                // Indicateur d'activation du buzzer pour certains modes

byte ValeurPWM;                                                                                                                 // Valeur PWM courante
byte DeltaPWM;                                                                                                                  // Incrément ou décrément de la valeur PWM courante
byte ValeurPWM1;                                                                                                                // Valeur PWM1 courante
byte DeltaPWM1;                                                                                                                 // Incrément ou décrément de la valeur PWM1 courante

const byte NombreLED = 6; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de LED
const byte NombreSequencesLED = 44; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
int SequenceCouranteLED = 0;                                                                                                    // Séquence courante des LED affichées
const bool TableauSequencesLED [NombreSequencesLED][NombreLED] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des séquences d'affichage des LED
  {1, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 0, 1, 0, 0, 0},
  {0, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 1, 0}, {0, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 1, 0}, {0, 0, 0, 1, 0, 0}, {0, 0, 1, 0, 0, 0},
  {0, 1, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0},
  
  {0, 0, 0, 0, 0, 0},
  
  {1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0},
  {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 1, 1}, {0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 1}, {0, 0, 0, 1, 1, 0}, {0, 0, 1, 1, 0, 0},
  {0, 1, 1, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0},
  
  {1, 0, 0, 0, 0, 1}, {0, 1, 0, 0, 1, 0}, {0, 0, 1, 1, 0, 0},
  {0, 1, 0, 0, 1, 0}, {1, 0, 0, 0, 0, 1},
  
  {0, 0, 0, 0, 0, 0},
  
  {1, 0, 1, 0, 1, 0}, {0, 1, 0, 1, 0, 1}, {1, 0, 1, 0, 1, 0},
  {0, 0, 0, 0, 0, 0},
  {0, 1, 0, 1, 0, 1}, {1, 0, 1, 0, 1, 0}, {0, 1, 0, 1, 0, 1},
  
  {0, 0, 0, 0, 0, 0}, {1, 1, 1, 1, 1, 1},
  {0, 0, 0, 0, 0, 0}, {1, 1, 1, 1, 1, 1},
  {0, 0, 0, 0, 0, 0}, {1, 1, 1, 1, 1, 1},
  
  {0, 0, 0, 0, 0, 0}
};

int NombreLED_ON;                                                                                                               // Nombre de flashs à générer
int CompteurLED_ON = 0;                                                                                                         // Compteur de flashs
unsigned long t1DebutTemporisation_LED_ON = 0ul;                                                                                // Début de la temporisation d'un flash du multiple flashs non bloquant
const unsigned long DureeTemporisation_LED_ON = 150ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation d'un flash du multiple flashs non bloquant en millisecondes
unsigned long t1DebutTemporisation_LED_OFF = 0ul;                                                                               // Début de la temporisation de la LED éteinte du multiple flashs non bloquant
const unsigned long DureeTemporisation_LED_OFF = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation de la LED éteinte du multiple flashs non bloquant en millisecondes
const bool LongLED_ONFin = true; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Indicateur d'autorisation du long flash de fin du multiple flashs non bloquant
unsigned long t1DebutTemporisation_LongLED_ONFin = 0ul;                                                                         // Début de la temporisation du long flash de fin du multiple flashs non bloquant
const unsigned long DureeTemporisation_LongLED_ONFin = 2000ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation du long flash de fin du multiple flashs non bloquant en millisecondes
unsigned long t1DebutTemporisation_LED_OFF_LongLED_ONFin = 0ul;                                                                 // Début de la temporisation de la LED éteinte avant le long flash de fin du multiple flashs non bloquant
const unsigned long DureeTemporisation_LED_OFF_LongLED_ONFin = 1000ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation de la LED éteinte avant le long flash de fin du multiple flashs non bloquant

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

const int Nombre_Sequences = 16; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences
const byte Tableau_Sequenceur [Nombre_Sequences] = {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0}; // >>>>>>>>>>>>>>>>>>>>>>> // Tableau de séquences
byte pointeur_TableauSequenceur;                                                                                                // Pointeur du tableau de séquences
unsigned long Duree_EntreSequences;                                                                                             // Temporisation entre les séquences

int Nombre_Sequences_BitsRegSeqs;                                                                                               // Nombre de séquences du registre 16 bits de séquences
long int BitsRegSeqs;                                                                                                           // Registre 16 bits de séquences
byte BitsRegInverseurs;                                                                                                         // Registre inverseur des sorties des 6 LED
int pointeur_BitsRegSeqs;                                                                                                       // Pointeur de bits du registre 16 bits de séquences
int decalage_BitsRegInverseurs = 0;                                                                                             // Valeur du décalage du registre inverseur des sorties des 6 LED

volatile int decalage_PORTD;                                                                                                    // Valeur du décalage du PORTD
volatile bool Sens_decalage_PORTD;                                                                                              // Sens de décalage du PORTD
volatile byte Masque_PORTD;                                                                                                     // Masque du PORTD
volatile int compteur_Decalage_PORTD = 0;                                                                                       // Compteur de décalage du PORTD
volatile int decalage_PORTB;                                                                                                    // Valeur du décalage du PORTB
volatile bool Sens_decalage_PORTB;                                                                                              // Sens de décalage du PORTB
volatile byte Masque_PORTB;                                                                                                     // Masque du PORTB
volatile int compteur_Decalage_PORTB = 0;                                                                                       // Compteur de décalage du PORTB

unsigned long PeriodeClignotement;                                                                                              // Période du clignotement en ms
float Periode_Signal_Secondes;                                                                                                  // Période du clignotement en secondes
unsigned long Demi_PeriodeEtatHaut;                                                                                             // Demi-période à l'état haut en ms 
unsigned long Demi_PeriodeEtatBas;                                                                                              // Demi-période à l'état bas en ms
int RapportCyclique;                                                                                                            // Valeur en pourcentage du rapport cyclique

byte MaskLedsInitial;                                                                                                           // Masque initial des LED pour déterminer l'état des LED
volatile byte MaskLeds = MaskLedsInitial;                                                                                       // Masque des LED pour déterminer l'état des LED
byte EtatInitial;                                                                                                               // Etat initial des LED
byte etatLeds = EtatInitial;                                                                                                    // Etat courant des LED
byte Dernier_etatLeds;                                                                                                          // Dernier état courant des LED

unsigned long t1DebutTempoFlash1_ON;                                                                                            // Début de la temporisation du flash 1 ON
unsigned long t1DebutTempoFlash2_ON;                                                                                            // Début de la temporisation du flash 2 ON
unsigned long t1DebutTempoFlash_OFF;                                                                                            // Début de la temporisation entre les flashs
unsigned long DureeTempoFlash1_ON = 50ul;                                                                                       // Duree de la temporisation du flash 1 ON
unsigned long DureeTempoFlash2_ON = 50ul;                                                                                       // Duree de la temporisation du flash 2 ON
unsigned long DureeTempoFlash_OFF;                                                                                              // Durée de la temporisation entre les flashs

unsigned long t1DebutTempoMultipleFlash_ON;                                                                                     // Début de la temporisation des multiples flashs ON
unsigned long t1DebutTempoMultipleFlash_OFF;                                                                                    // Début de la temporisation entre les multiples flashs
unsigned long DureeTempoMultipleFlash_ON;                                                                                       // Durée de la temporisation des multiples flashs ON
unsigned long DureeTempoMultipleFlash_OFF;                                                                                      // Durée de la temporisation entre les multiples flashs
int NombreFlashs;                                                                                                               // Valeur du nombre de flashs
int CompteurFlashs;                                                                                                             // Compteur de flashs

int PourcentageDureeHIGHMaxMin;                                                                                                 // Valeur en pourcentage pour calculer les temporisations Max et Min de la LED à l'état haut
unsigned long DureeHIGHMax;                                                                                                     // Temporisation Max de la LED à l'état haut
unsigned long DureeHIGHMin;                                                                                                     // Temporisation Min de la LED à l'état haut
int variateurDureeHIGH;                                                                                                         // Variateur de la temporisation de la LED à l'état haut (= 1 ou -1)

byte MaskEtatLedsInitial;                                                                                                       // Masque initial de l'état courant des LED
byte MaskEtatLeds;                                                                                                              // Masque de l'état courant des LED

int CompteurDeplacementsComplets = 0;                                                                                           // Compteur de déplacements complets

bool Mute;                                                                                                                      // Indicateur d'activation du buzzer

int CompteurDecalageEtatLeds;                                                                                                   // Compteur de décalage de l'état courant des LED

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
//*** Clignotement de 8 LED simultanées et alternées avec 4 autres LED non bloquant ****************************************************************************************
//**************************************************************************************************************************************************************************
  if (ModeCourant == 0) // **************************************************************************************************** // Si le mode courant "0" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 2;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      t1DebutTempo_EtatHaut = 0ul;                                                                                              // Réinitialise le début de la temporisation à l'état haut en mode non bloquant
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Réinitialise le début de la temporisation à l'état bas en mode non bloquant
      DureeTempo_EtatHaut = 300ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation à l'état haut en mode non bloquant
      DureeTempo_EtatBas = 300ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation à l'état bas en mode non bloquant
      DeltaTempo_EtatHaut = 20; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit l'incrément ou le décrément de la valeur de la temporisation à l'état haut en mode non bloquant
      DeltaTempo_EtatBas = 20; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit l'incrément ou le décrément de la valeur de la temporisation à l'état bas en mode non bloquant
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    PORTD_TEMP = PORTD & 0b00000011;                                                                                            // Masque RX et TX
    PORTD = PORTD_TEMP | 0b00110000;                                                                                            // Allume les LED D4 et D5 et éteint les LED D2, D3, D6 et D7
    
    PORTB_TEMP = PORTB & 0b11000000;                                                                                            // Masque les bits "6" et "7"
    PORTB = PORTB_TEMP | 0b00001100;                                                                                            // Allume les LED D10 et D11 et éteint les LED D8, D9, D12 et D13
    
    t1DebutTempo_EtatHaut = millis();                                                                                           // Démarre la temporisation à l'état haut non bloquant
    while (millis() - t1DebutTempo_EtatHaut <= DureeTempo_EtatHaut) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Tant que la temporisation à l'état haut n'est pas écoulée
    {
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs "BPModePlus" ou "BPModeMoins"
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    PORTD_TEMP = PORTD & 0b00000011;                                                                                            // Masque RX et TX
    PORTD = PORTD_TEMP | 0b11001100;                                                                                            // Eteint les LED D4 et D5 et allume les LED D2, D3, D6 et D7
    
    PORTB_TEMP = PORTB & 0b11000000;                                                                                            // Masque les bits "6" et "7"
    PORTB = PORTB_TEMP | 0b00110011;                                                                                            // Eteint les LED D10 et D11 et allume les LED D8, D9, D12 et D13
    
    t1DebutTempo_EtatBas = millis();                                                                                            // Démarre la temporisation à l'état haut non bloquant
    while (millis() - t1DebutTempo_EtatBas <= DureeTempo_EtatBas) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Tant que la temporisation à l'état bas n'est pas écoulée
    {
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs "BPModePlus" ou "BPModeMoins"
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    if (DureeTempo_EtatHaut > 300ul) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation à l'état haut est supérieure à "300"
    {
      DeltaTempo_EtatHaut = -DeltaTempo_EtatHaut;                                                                               // Inverse l'incrément ou le décrément de la valeur de la temporisation à l'état haut
    }
    else if (DureeTempo_EtatHaut < 60ul) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation à l'état haut est inférieure à "60"
    {
      DeltaTempo_EtatHaut = -DeltaTempo_EtatHaut;                                                                               // Inverse l'incrément ou le décrément de la valeur de la temporisation à l'état haut
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
    }
    DureeTempo_EtatHaut += DeltaTempo_EtatHaut;                                                                                 // Incrémente ou décremente la temporisation à l'état haut
    
    if (DureeTempo_EtatBas > 300ul) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation à l'état bas est supérieure à "300"
    {
      DeltaTempo_EtatBas = -DeltaTempo_EtatBas;                                                                                 // Inverse l'incrément ou le décrément de la valeur de la temporisation à l'état bas
    }
    else if (DureeTempo_EtatBas < 60ul) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation à l'état bas est inférieure à "60"
    {
      DeltaTempo_EtatBas = -DeltaTempo_EtatBas;                                                                                 // Inverse l'incrément ou le décrément de la valeur de la temporisation à l'état bas
    }
    DureeTempo_EtatBas += DeltaTempo_EtatBas;                                                                                   // Incrémente ou décremente la temporisation à l'état bas
  }
//**************************************************************************************************************************************************************************
//*** Clignotement de 4 LED simultanées et alternées avec 8 autres LED non bloquant ****************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 1) // *********************************************************************************************** // Si le mode courant "1" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 2;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      t1DebutTempo_EtatHaut = 0ul;                                                                                              // Réinitialise le début de la temporisation à l'état haut en mode non bloquant
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Réinitialise le début de la temporisation à l'état bas en mode non bloquant
      DureeTempo_EtatHaut = 300ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation à l'état haut en mode non bloquant
      DureeTempo_EtatBas = 300ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation à l'état bas en mode non bloquant
      DeltaTempo_EtatHaut = 20; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit l'incrément ou le décrément de la valeur de la temporisation à l'état haut en mode non bloquant
      DeltaTempo_EtatBas = 20; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit l'incrément ou le décrément de la valeur de la temporisation à l'état bas en mode non bloquant
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    PORTD_TEMP = PORTD & 0b00000011;                                                                                            // Masque RX et TX
    PORTD = PORTD_TEMP | 0b01001000;                                                                                            // Allume les LED D3 et D6 et éteint les LED D2, D4, D5 et D7
    
    PORTB_TEMP = PORTB & 0b11000000;                                                                                            // Masque les bits "6" et "7"
    PORTB = PORTB_TEMP | 0b00010010;                                                                                            // Allume les LED D9 et D12 et éteint les LED D8, D10, D11 et D13
    
    t1DebutTempo_EtatHaut = millis();                                                                                           // Démarre la temporisation à l'état haut non bloquant
    while (millis() - t1DebutTempo_EtatHaut <= DureeTempo_EtatHaut) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Tant que la temporisation à l'état haut n'est pas écoulée
    {
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs "BPModePlus" ou "BPModeMoins"
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    PORTD_TEMP = PORTD & 0b00000011;                                                                                            // Masque RX et TX
    PORTD = PORTD_TEMP | 0b10110100;                                                                                            // Eteint les LED D3 et D6 et allume les LED D2, D4, D5 et D7
    
    PORTB_TEMP = PORTB & 0b11000000;                                                                                            // Masque les bits "6" et "7"
    PORTB = PORTB_TEMP | 0b00101101;                                                                                            // Eteint les LED D9 et D12 et allume les LED D8, D10, D11 et D13
    
    t1DebutTempo_EtatBas = millis();                                                                                            // Démarre la temporisation à l'état haut non bloquant
    while (millis() - t1DebutTempo_EtatBas <= DureeTempo_EtatBas) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Tant que la temporisation à l'état bas n'est pas écoulée
    {
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs "BPModePlus" ou "BPModeMoins"
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    if (DureeTempo_EtatHaut > 300ul) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation à l'état haut est supérieure à "300"
    {
      DeltaTempo_EtatHaut = -DeltaTempo_EtatHaut;                                                                               // Inverse l'incrément ou le décrément de la valeur de la temporisation à l'état haut
    }
    else if (DureeTempo_EtatHaut < 60ul) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation à l'état haut est inférieure à "60"
    {
      DeltaTempo_EtatHaut = -DeltaTempo_EtatHaut;                                                                               // Inverse l'incrément ou le décrément de la valeur de la temporisation à l'état haut
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
    }
    DureeTempo_EtatHaut += DeltaTempo_EtatHaut;                                                                                 // Incrémente ou décremente la temporisation à l'état haut
    
    if (DureeTempo_EtatBas > 300ul) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation à l'état bas est supérieure à "300"
    {
      DeltaTempo_EtatBas = -DeltaTempo_EtatBas;                                                                                 // Inverse l'incrément ou le décrément de la valeur de la temporisation à l'état bas
    }
    else if (DureeTempo_EtatBas < 60ul) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation à l'état bas est inférieure à "60"
    {
      DeltaTempo_EtatBas = -DeltaTempo_EtatBas;                                                                                 // Inverse l'incrément ou le décrément de la valeur de la temporisation à l'état bas
    }
    DureeTempo_EtatBas += DeltaTempo_EtatBas;                                                                                   // Incrémente ou décremente la temporisation à l'état bas
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED avec l'opérateur XOR non bloquant ****************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 2) // *********************************************************************************************** // Si le mode courant "2" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 40;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      t1DebutTemporisation = 0ul;                                                                                               // Réinitialise la date courante de la temporisation en mode non bloquant
      DerniereFinTemporisation = 0ul;                                                                                           // Réinitialise la fin de la temporisation en mode non bloquant
      DureeTemporisation = 200ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la temporisation à l'état haut et à l'état bas en mode non bloquant
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    t1DebutTemporisation = millis();                                                                                            // Mémorise la date courante de la temporisation
    
    if (t1DebutTemporisation - DerniereFinTemporisation >= DureeTemporisation) // +++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation est écoulée
    {
      PORTD = PORTD ^ 0b11111100;                                                                                               // Inverse les états de toutes le LED du PORTD avec l'opérateur XOR
      PORTB = PORTB ^ 0b00111111;                                                                                               // Inverse les états de toutes le LED du PORTB avec l'opérateur XOR
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
      
      DerniereFinTemporisation = t1DebutTemporisation;                                                                          // Mémorise la date de la fin de la temporisation
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement de 6 LED simultanées et semi-alternées avec 6 autres LED non bloquant ***********************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 3) // *********************************************************************************************** // Si le mode courant "3" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 50;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      t1DebutTemporisation = 0ul;                                                                                               // Réinitialise la date courante de la temporisation en mode non bloquant
      DerniereFinTemporisation = 0ul;                                                                                           // Réinitialise la fin de la temporisation en mode non bloquant
      DureeTemporisation = 200ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la temporisation à l'état haut et à l'état bas en mode non bloquant
      InverseurClignotementAlterne = true;                                                                                      // Définit l'indicateur inverseur de clignotement de LED alternées
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    t1DebutTemporisation = millis();                                                                                            // Mémorise la date courante de la temporisation
    
    if (t1DebutTemporisation - DerniereFinTemporisation >= DureeTemporisation) // +++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation est écoulée
    {
      if (InverseurClignotementAlterne) // ------------------------------------------------------------------------------------ // Si l'indicateur inverseur de clignotement de LED alternées est activé
      {
        PORTD = PORTD ^ 0b01010100;                                                                                             // Inverse les états des LED D2, D4 et D6 avec l'opérateur XOR
        PORTB = PORTB ^ 0b00010101;                                                                                             // Inverse les états des LED D8, D10 et D12 avec l'opérateur XOR
      }
      else if (!InverseurClignotementAlterne) // ------------------------------------------------------------------------------ // Si l'indicateur inverseur de clignotement de LED alternées est désactivé
      {
        PORTD = PORTD ^ 0b10101000;                                                                                             // Inverse les états des LED D3, D5 et D7
        PORTB = PORTB ^ 0b00101010;                                                                                             // Inverse les états des LED D9, D11 et D13
      }
      
      InverseurClignotementAlterne = !InverseurClignotementAlterne;                                                             // Inverse l'indicateur inverseur de clignotement de LED alternées
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
      
      DerniereFinTemporisation = t1DebutTemporisation;                                                                          // Mémorise la date de la fin de la temporisation
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement de 6 LED simultanées et alternées avec 6 autres LED non bloquant ****************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 4) // *********************************************************************************************** // Si le mode courant "4" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 40;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      t1DebutTemporisation = 0ul;                                                                                               // Réinitialise la date courante de la temporisation en mode non bloquant
      DerniereFinTemporisation = 0ul;                                                                                           // Réinitialise la fin de la temporisation en mode non bloquant
      DureeTemporisation = 150ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la temporisation à l'état haut et à l'état bas en mode non bloquant
      InverseurClignotementAlterne = true;                                                                                      // Définit l'indicateur inverseur de clignotement de LED alternées
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    t1DebutTemporisation = millis();                                                                                            // Mémorise la date courante de la temporisation
    
    if (t1DebutTemporisation - DerniereFinTemporisation >= DureeTemporisation) // +++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation est écoulée
    {
      if (InverseurClignotementAlterne) // ------------------------------------------------------------------------------------ // Si l'indicateur inverseur de clignotement de LED alternées est activé
      {
        PORTD &= 0b11110011;                                                                                                    // Eteint les LED D2 et D3
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTD = PORTD ^ 0b00110000;                                                                                             // Inverse les états des LED D4 et D5 avec l'opérateur XOR
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTD = PORTD ^ 0b11000000;                                                                                             // Inverse les états de la LED D6 et D7 avec l'opérateur XOR
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTB &= 0b11111100;                                                                                                    // Eteint les LED D8 et D9
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTB = PORTB ^ 0b00001100;                                                                                             // Inverse les états des LED D10 et D11 avec l'opérateur XOR
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTB = PORTB ^ 0b00110000;                                                                                             // Inverse les états de la LED D12 et D13 avec l'opérateur XOR
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      else if (!InverseurClignotementAlterne) // ------------------------------------------------------------------------------ // Si l'indicateur inverseur de clignotement de LED alternées est désactivé
      {
        PORTD &= 0b00111111;                                                                                                    // Eteint les LED D6 et D7
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTD = PORTD ^ 0b00001100;                                                                                             // Inverse les états des LED D2 et D3 avec l'opérateur XOR
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTB &= 0b11001111;                                                                                                    // Eteint les LED D12 et D13
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTB = PORTB ^ 0b00000011;                                                                                             // Inverse les états des LED D8 et D9 avec l'opérateur XOR
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      InverseurClignotementAlterne = !InverseurClignotementAlterne;                                                             // Inverse l'indicateur inverseur de clignotement de LED alternées
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
      
      DerniereFinTemporisation = t1DebutTemporisation;                                                                          // Mémorise la date de la fin de la temporisation
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED avec le modulo (%) non bloquant - Exemple 1 ******************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 5) // *********************************************************************************************** // Si le mode courant "5" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 80;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    EtatLED_D2 = millis() / 100 % 2;                                                                                            // Définit l'état de la LED D2
    if (DernierEtatLED_D2 != EtatLED_D2) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'état de la LED D2 à changé
    {
      digitalWrite(BrocheLED_D2, EtatLED_D2);                                                                                   // Active le clignotement de la LED D2
      digitalWrite(BrocheLED_D8, EtatLED_D2);                                                                                   // Active le clignotement de la LED D8
      
      DernierEtatLED_D2 = EtatLED_D2;                                                                                           // Mémorise le dernier état de la LED D2
    }
    
    EtatLED_D3 = millis() / 200 % 2;                                                                                            // Définit l'état de la LED D3
    if (DernierEtatLED_D3 != EtatLED_D3) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'état de la LED D3 à changé
    {
      digitalWrite(BrocheLED_D3, EtatLED_D3);                                                                                   // Active le clignotement de la LED D3
      digitalWrite(BrocheLED_D9, EtatLED_D3);                                                                                   // Active le clignotement de la LED D9
      
      DernierEtatLED_D3 = EtatLED_D3;                                                                                           // Mémorise le dernier état de la LED D3
    }
    
    EtatLED_D4 = millis() / 300 % 2;                                                                                            // Définit l'état de la LED D4
    if (DernierEtatLED_D4 != EtatLED_D4) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'état de la LED D4 à changé
    {
      digitalWrite(BrocheLED_D4, EtatLED_D4);                                                                                   // Active le clignotement de la LED D4
      digitalWrite(BrocheLED_D10, EtatLED_D4);                                                                                  // Active le clignotement de la LED D10
      
      DernierEtatLED_D4 = EtatLED_D4;                                                                                           // Mémorise le dernier état de la LED D4
    }
    
    EtatLED_D5 = millis() / 300 % 2;                                                                                            // Définit l'état de la LED D5
    if (DernierEtatLED_D5 != EtatLED_D5) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'état de la LED D5 à changé
    {
      digitalWrite(BrocheLED_D5, EtatLED_D5);                                                                                   // Active le clignotement de la LED D5
      digitalWrite(BrocheLED_D11, EtatLED_D5);                                                                                  // Active le clignotement de la LED D11
      
      DernierEtatLED_D5 = EtatLED_D5;                                                                                           // Mémorise le dernier état de la LED D5
    }
    
    EtatLED_D6 = millis() / 200 % 2;                                                                                            // Définit l'état de la LED D6
    if (DernierEtatLED_D6 != EtatLED_D6) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'état de la LED D6 à changé
    {
      digitalWrite(BrocheLED_D6, EtatLED_D6);                                                                                   // Active le clignotement de la LED D6
      digitalWrite(BrocheLED_D12, EtatLED_D6);                                                                                  // Active le clignotement de la LED D12
      
      DernierEtatLED_D6 = EtatLED_D6;                                                                                           // Mémorise le dernier état de la LED D6
    }
    
    EtatLED_D7 = millis() / 100 % 2;                                                                                            // Définit l'état de la LED D7
    if (DernierEtatLED_D7 != EtatLED_D7) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'état de la LED D7 à changé
    {
      digitalWrite(BrocheLED_D7, EtatLED_D7);                                                                                   // Active le clignotement de la LED D7
      digitalWrite(BrocheLED_D13, EtatLED_D7);                                                                                  // Active le clignotement de la LED D13
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
      
      DernierEtatLED_D7 = EtatLED_D7;                                                                                           // Mémorise le dernier état de la LED D7
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED avec le modulo (%) non bloquant - Exemple 2 ******************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 6) // *********************************************************************************************** // Si le mode courant "6" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 80;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    EtatLED_D2 = millis() / 1600 % 2;                                                                                           // Définit l'état de la LED D2
    if (DernierEtatLED_D2 != EtatLED_D2) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'état de la LED D2 à changé
    {
      digitalWrite(BrocheLED_D2, EtatLED_D2);                                                                                   // Active le clignotement de la LED D2
      digitalWrite(BrocheLED_D8, EtatLED_D2);                                                                                   // Active le clignotement de la LED D8
      
      DernierEtatLED_D2 = EtatLED_D2;                                                                                           // Mémorise le dernier état de la LED D2
    }
    
    EtatLED_D3 = millis() / 800 % 2;                                                                                            // Définit l'état de la LED D3
    if (DernierEtatLED_D3 != EtatLED_D3) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'état de la LED D3 à changé
    {
      digitalWrite(BrocheLED_D3, EtatLED_D3);                                                                                   // Active le clignotement de la LED D3
      digitalWrite(BrocheLED_D9, EtatLED_D3);                                                                                   // Active le clignotement de la LED D9
      
      DernierEtatLED_D3 = EtatLED_D3;                                                                                           // Mémorise le dernier état de la LED D3
    }
    
    EtatLED_D4 = millis() / 400 % 2;                                                                                            // Définit l'état de la LED D4
    if (DernierEtatLED_D4 != EtatLED_D4) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'état de la LED D4 à changé
    {
      digitalWrite(BrocheLED_D4, EtatLED_D4);                                                                                   // Active le clignotement de la LED D4
      digitalWrite(BrocheLED_D10, EtatLED_D4);                                                                                  // Active le clignotement de la LED D10
      
      DernierEtatLED_D4 = EtatLED_D4;                                                                                           // Mémorise le dernier état de la LED D4
    }
    
    EtatLED_D5 = millis() / 200 % 2;                                                                                            // Définit l'état de la LED D5
    if (DernierEtatLED_D5 != EtatLED_D5) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'état de la LED D5 à changé
    {
      digitalWrite(BrocheLED_D5, EtatLED_D5);                                                                                   // Active le clignotement de la LED D5
      digitalWrite(BrocheLED_D11, EtatLED_D5);                                                                                  // Active le clignotement de la LED D11
      
      DernierEtatLED_D5 = EtatLED_D5;                                                                                           // Mémorise le dernier état de la LED D5
    }
    
    EtatLED_D6 = millis() / 100 % 2;                                                                                            // Définit l'état de la LED D6
    if (DernierEtatLED_D6 != EtatLED_D6) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'état de la LED D6 à changé
    {
      digitalWrite(BrocheLED_D6, EtatLED_D6);                                                                                   // Active le clignotement de la LED D6
      digitalWrite(BrocheLED_D12, EtatLED_D6);                                                                                  // Active le clignotement de la LED D12
      
      DernierEtatLED_D6 = EtatLED_D6;                                                                                           // Mémorise le dernier état de la LED D6
    }
    
    EtatLED_D7 = millis() / 50 % 2;                                                                                             // Définit l'état de la LED D7
    if (DernierEtatLED_D7 != EtatLED_D7) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'état de la LED D7 à changé
    {
      digitalWrite(BrocheLED_D7, EtatLED_D7);                                                                                   // Active le clignotement de la LED D7
      digitalWrite(BrocheLED_D13, EtatLED_D7);                                                                                  // Active le clignotement de la LED D13
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
      
      DernierEtatLED_D7 = EtatLED_D7;                                                                                           // Mémorise le dernier état de la LED D7
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED 2 flashs courts et 1 flash long non bloquant *****************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 7) // *********************************************************************************************** // Si le mode courant "7" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 15;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DerniereFinTemporisation = 0ul;                                                                                           // Réinitialise la fin de la temporisation en mode non bloquant
      DureeTemporisation = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la temporisation à l'état haut et à l'état bas en mode non bloquant
      CompteurFlash = 0;                                                                                                        // Définit le compteur de demi-périodes de clignotement
      NombreFlashCourts = 2; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit le nombre de flashs courts
      MuteBuzzer = false; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit l'indicateur d'activation du buzzer pour certains modes
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    t1DebutTemporisation = millis();                                                                                            // Mémorise la date courante de la temporisation
    
    if (t1DebutTemporisation - DerniereFinTemporisation >= DureeTemporisation) // +++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation est écoulée
    {
      CompteurFlash++;                                                                                                          // Incrémente le compteur de demi-périodes de clignotement
      
      if (MuteBuzzer) // ------------------------------------------------------------------------------------------------------ // Si l'indicateur d'activation du buzzer pour certains modes est activé
      {
        PORTC = PORTC ^ 0b00000001;                                                                                             // Inverse l'état du buzzer
      }
      
      PORTD = PORTD ^ 0b11111100;                                                                                               // Inverse les états de toutes les LED du PORTD avec l'opérateur XOR
      PORTB = PORTB ^ 0b00111111;                                                                                               // Inverse les états de toutes les LED du PORTB avec l'opérateur XOR
      
      DerniereFinTemporisation = t1DebutTemporisation;                                                                          // Mémorise la date de la fin de la temporisation
    }
    
    if (CompteurFlash == NombreFlashCourts + 1) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le compteur de demi-périodes de clignotement est égal au nombre de flashs courts + 1
    {
      DureeTemporisation = 500ul;                                                                                               // Définit la durée de la temporisation à l'état haut et à l'état bas en mode non bloquant
    }
    else if (CompteurFlash > NombreFlashCourts + 1) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le compteur de demi-périodes de clignotement est supérieur au nombre de flashs courts + 1
    {
      CompteurFlash = 0;                                                                                                        // Réinitialise le compteur de demi-périodes de clignotement
      DureeTemporisation = 100ul;                                                                                               // Définit la durée de la temporisation à l'état haut et à l'état bas en mode non bloquant
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement progressif de 2 LED alternée avec 2 autre LED avec la PWM non bloquant **********************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 8) // *********************************************************************************************** // Si le mode courant "8" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 150;                                                                                 // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DerniereFinTemporisation = 0ul;                                                                                           // Réinitialise la fin de la temporisation en mode non bloquant
      DureeTemporisation = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la temporisation à l'état haut et à l'état bas en mode non bloquant
      ValeurPWM = 128;                                                                                                          // Définit la valeur PWM courante
      DeltaPWM = 8; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit l'incrément ou le décrément de la valeur PWM courante
      ValeurPWM1 = 128;                                                                                                         // Définit la valeur PWM1 courante
      DeltaPWM1 = -8; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit l'incrément ou le décrément de la valeur PWM1 courante
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    t1DebutTemporisation = millis();                                                                                            // Mémorise la date courante de la temporisation
    
    if ((t1DebutTemporisation - DerniereFinTemporisation) < DureeTemporisation) {return;} // ++++++++++++++++++++++++++++++++++ // Si la temporisation n'est pas écoulée => Retour début loop()
    
    DerniereFinTemporisation = t1DebutTemporisation;                                                                            // Mémorise la date de la fin de la temporisation
    
         if (ValeurPWM > 216) DeltaPWM = -DeltaPWM; // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur PWM courante est supérieure à "216"
    else if (ValeurPWM < 8)   DeltaPWM = -DeltaPWM; // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur PWM courante est inférieure à "8"
    ValeurPWM += DeltaPWM;                                                                                                      // Incrémente ou décremente la valeur PWM courante
    
         if (ValeurPWM1 > 216) DeltaPWM1 = -DeltaPWM1; // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur PWM1 courante est supérieure à "216"
    else if (ValeurPWM1 < 8)   DeltaPWM1 = -DeltaPWM1; // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur PWM1 courante est inférieure à "8"
    ValeurPWM1 += DeltaPWM1;                                                                                                    // Incrémente ou décremente la valeur PWM1 courante
    
    analogWrite(BrocheLED_D3, ValeurPWM);                                                                                       // Active la LED D3 avec la valeur PWM courante
    analogWrite(BrocheLED_D5, ValeurPWM1);                                                                                      // Active la LED D5 avec la valeur PWM1 courante
    analogWrite(BrocheLED_D11, ValeurPWM);                                                                                      // Active la LED D11 avec la valeur PWM courante
    analogWrite(BrocheLED_D9, ValeurPWM1);                                                                                      // Active la LED D9 avec la valeur PWM1 courante
    
    compteurAffichageModeCourant++;                                                                                             // Incrémente le compteur d'affichages du mode courant en mode automatique
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 6 LED avec un tableau de séquences de LED non bloquant **************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 9) // *********************************************************************************************** // Si le mode courant "9" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 2;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      t1DebutTemporisation = 0ul;                                                                                               // Initialise de la date courante de la temporisation en mode non bloquant
      DureeTemporisation = 200ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la temporisation à l'état haut et à l'état bas en mode non bloquant
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante de LED affichée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() - t1DebutTemporisation >= DureeTemporisation) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation est écoulée
    {
      digitalWrite(BrocheLED_D2, TableauSequencesLED[SequenceCouranteLED][0]);                                                  // Allume ou éteint la LED D2 en fonction de l'état de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D3, TableauSequencesLED[SequenceCouranteLED][1]);                                                  // Allume ou éteint la LED D3 en fonction de l'état de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D4, TableauSequencesLED[SequenceCouranteLED][2]);                                                  // Allume ou éteint la LED D4 en fonction de l'état de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D5, TableauSequencesLED[SequenceCouranteLED][3]);                                                  // Allume ou éteint la LED D5 en fonction de l'état de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D6, TableauSequencesLED[SequenceCouranteLED][4]);                                                  // Allume ou éteint la LED D6 en fonction de l'état de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D7, TableauSequencesLED[SequenceCouranteLED][5]);                                                  // Allume ou éteint la LED D7 en fonction de l'état de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      
      digitalWrite(BrocheLED_D8,  TableauSequencesLED[SequenceCouranteLED][0]);                                                 // Allume ou éteint la LED D8 en fonction de l'état de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D9,  TableauSequencesLED[SequenceCouranteLED][1]);                                                 // Allume ou éteint la LED D9 en fonction de l'état de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D10, TableauSequencesLED[SequenceCouranteLED][2]);                                                 // Allume ou éteint la LED D10 en fonction de l'état de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D11, TableauSequencesLED[SequenceCouranteLED][3]);                                                 // Allume ou éteint la LED D11 en fonction de l'état de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D12, TableauSequencesLED[SequenceCouranteLED][4]);                                                 // Allume ou éteint la LED D12 en fonction de l'état de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D13, TableauSequencesLED[SequenceCouranteLED][5]);                                                 // Allume ou éteint la LED D13 en fonction de l'état de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      
      SequenceCouranteLED++;                                                                                                    // Incrémente la valeur de la séquence courante des LED affichées
      if (SequenceCouranteLED > NombreSequencesLED - 1) // -------------------------------------------------------------------- // Si la séquence courante de LED affichée est supérieure au nombre de séquences d'affichage des LED
      {  
        SequenceCouranteLED = 0;                                                                                                // Borne la valeur de la séquence courante des LED affichées
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
      
      t1DebutTemporisation = millis();                                                                                          // (Re)Démarre la temporisation
    }
 }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED multiple flashs et flash long de fin configurables non bloquant **********************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 10) // ********************************************************************************************** // Si le mode courant "10" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      if (DernierModeCourant != -1) Buzzer(3, 0, 1); // ----------------------------------------------------------------------- // Si le dernier mode courant est différent de "-1" => Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      t1DebutTemporisation_LED_ON = 0ul;                                                                                        // Initialise la temporisation d'un flash du multiple flashs non bloquant
      t1DebutTemporisation_LED_OFF = 0ul;                                                                                       // Initialise la temporisation de la LED éteinte du multiple flashs non bloquant
      t1DebutTemporisation_LongLED_ONFin = 0ul;                                                                                 // Réinitialise la temporisation du long flash de fin du multiple flashs non bloquant
      CompteurLED_ON = 0;                                                                                                       // Initialise le compteurs de flashs
      NombreLED_ON = 6;                                                                                                         // Initialise le nombre de flashs à générer
      t1DebutTemporisation_LED_ON = millis();                                                                                   // Démarre la temporisation d'un flash du multiple flashs non bloquant
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() - t1DebutTemporisation_LED_ON >= DureeTemporisation_LED_ON && t1DebutTemporisation_LED_ON != 0ul) // +++++++++ // Si la temporisation d'un flash du multiple flashs non bloquant est écoulée
    {
      CompteurLED_ON++;                                                                                                         // Incrémente le compteur de flashs
      t1DebutTemporisation_LED_ON = 0ul;                                                                                        // Réinitialise la temporisation d'un flash du multiple flashs non bloquant
      if (CompteurLED_ON == NombreLED_ON && !LongLED_ONFin) // ---------------------------------------------------------------- // Si le compteur de flashs est égal au nombre de flashs à générer et Si l'indicateur d'autorisation du long flash de fin du multiple flashs non bloquant est désactivé
      {
        PORTD &= 0b00000011;                                                                                                    // Désactive toutes les LED du PORTD
        PORTB &= 0b11000000;                                                                                                    // Désactive toutes les LED du PORTB
        
        CompteurLED_ON = 0;                                                                                                     // Réinitialise le compteur de flashs
        t1DebutTemporisation_LED_OFF = 0ul;                                                                                     // Réinitialise la temporisation de la LED éteinte du multiple flashs non bloquant
        DernierModeCourant = -1;                                                                                                // Réinitialise le dernier mode courant
        return;                                                                                                                 // Retour début loop()
      }
      else if (CompteurLED_ON == NombreLED_ON && LongLED_ONFin) // ------------------------------------------------------------ // Si le compteur de flashs est égal au nombre de flashs à générer et Si l'indicateur d'autorisation du long flash de fin du multiple flashs non bloquant est activé
      {
        PORTD &= 0b00000011;                                                                                                    // Désactive toutes les LED du PORTD
        PORTB &= 0b11000000;                                                                                                    // Désactive toutes les LED du PORTB
        
        t1DebutTemporisation_LED_OFF = millis();                                                                                // Démarre la temporisation de la LED éteinte du multiple flashs non bloquant
      }
      else if (CompteurLED_ON != NombreLED_ON) // ----------------------------------------------------------------------------- // Si le compteur de flashs est différent du nombre de flashs à générer
      {
        PORTD &= 0b00000011;                                                                                                    // Désactive toutes les LED du PORTD
        PORTB &= 0b11000000;                                                                                                    // Désactive toutes les LED du PORTB
        
        t1DebutTemporisation_LED_OFF = millis();                                                                                // Démarre la temporisation de la LED éteinte du multiple flashs non bloquant
      }
    }
    else if (millis() - t1DebutTemporisation_LED_OFF >= DureeTemporisation_LED_OFF && t1DebutTemporisation_LED_OFF != 0ul) // * // Si la temporisation de la LED éteinte du multiple flashs non bloquant est écoulée
    {
      if (CompteurLED_ON == NombreLED_ON && LongLED_ONFin) // ----------------------------------------------------------------- // Si le compteur de flashs est égal au nombre de flashs à générer et Si l'indicateur d'autorisation du long flash de fin du multiple flashs non bloquant est activé
      {
        t1DebutTemporisation_LED_ON = 0ul;                                                                                      // Réinitialise la temporisation d'un flash du multiple flashs non bloquant
        t1DebutTemporisation_LED_OFF = 0ul;                                                                                     // Réinitialise la temporisation de la LED éteinte du multiple flashs non bloquant
        t1DebutTemporisation_LongLED_ONFin = 0ul;                                                                               // Réinitialise la temporisation du long flash de fin du multiple flashs non bloquant
        t1DebutTemporisation_LED_OFF_LongLED_ONFin = millis();                                                                  // Démarre la temporisation de la LED éteinte avant le long flash de fin du multiple flashs non bloquant
      }
      else if (CompteurLED_ON != NombreLED_ON) // ----------------------------------------------------------------------------- // Si le compteur de flashs est différent du nombre de flashs à générer
      {
        PORTD |= 0b11111100;                                                                                                    // Active toutes les LED du PORTD
        PORTB |= 0b00111111;                                                                                                    // Active toutes les LED du PORTB
        
        t1DebutTemporisation_LED_OFF = 0ul;                                                                                     // Réinitialise la temporisation de la LED éteinte du multiple flashs non bloquant
        t1DebutTemporisation_LED_ON = millis();                                                                                 // Démarre la temporisation d'un flash du multiple flashs non bloquant
      }
    }
    else if (CompteurLED_ON == NombreLED_ON && LongLED_ONFin) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le compteur de flashs est égal au nombre de flashs à générer et Si l'indicateur d'autorisation du long flash de fin du multiple flashs non bloquant est activé
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Si la temporisation de la LED éteinte avant le long flash de fin du multiple flashs non bloquant est écoulée
      if (millis() - t1DebutTemporisation_LED_OFF_LongLED_ONFin >= DureeTemporisation_LED_OFF_LongLED_ONFin && t1DebutTemporisation_LED_OFF_LongLED_ONFin != 0ul)
      {
        PORTD |= 0b11111100;                                                                                                    // Active toutes les LED du PORTD
        PORTB |= 0b00111111;                                                                                                    // Active toutes les LED du PORTB
        
        t1DebutTemporisation_LED_OFF_LongLED_ONFin = 0ul;                                                                       // Réinitialise la temporisation de la LED éteinte avant le long flash de fin du multiple flashs non bloquant
        t1DebutTemporisation_LongLED_ONFin = millis();                                                                          // Démarre la temporisation du long flash de fin du multiple flashs non bloquant
      }
      // ---------------------------------------------------------------------------------------------------------------------- // Si la temporisation du long flash de fin du multiple flashs non bloquant est écoulée
      else if (millis() - t1DebutTemporisation_LongLED_ONFin >= DureeTemporisation_LongLED_ONFin && t1DebutTemporisation_LongLED_ONFin != 0ul)
      {
        PORTD &= 0b00000011;                                                                                                    // Désactive toutes les LED du PORTD
        PORTB &= 0b11000000;                                                                                                    // Désactive toutes les LED du PORTB
        
        DernierModeCourant = -1;                                                                                                // Réinitialise le dernier mode courant
        CompteurLED_ON = 0;                                                                                                     // Réinitialise le compteur de flashs
        t1DebutTemporisation_LED_ON = 0ul;                                                                                      // Réinitialise la temporisation d'un flash du multiple flashs non bloquant
        t1DebutTemporisation_LED_OFF = 0ul;                                                                                     // Réinitialise la temporisation de la LED éteinte du multiple flashs non bloquant
        t1DebutTemporisation_LongLED_ONFin = 0ul;                                                                               // Réinitialise la temporisation du long flash de fin du multiple flashs non bloquant
        
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED avec un tableau de séquences de LED non bloquant *************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 11) // ********************************************************************************************** // Si le mode courant "11" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 2;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      t1DebutTemporisation = 0ul;                                                                                               // Initialise de la date courante de la temporisation en mode non bloquant
      DureeTemporisation = 200ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la temporisation à l'état haut et à l'état bas en mode non bloquant
      SequenceCouranteLED = 0;                                                                                                  // Initialise la séquence courante de LED affichée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() - t1DebutTemporisation >= DureeTemporisation) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation est écoulée
    {
      digitalWrite(BrocheLED_D2, TableauSequencesLED[SequenceCouranteLED][0]);                                                  // Allume ou éteint la LED D2 en fonction des informations de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D3, TableauSequencesLED[SequenceCouranteLED][1]);                                                  // Allume ou éteint la LED D3 en fonction des informations de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D4, TableauSequencesLED[SequenceCouranteLED][2]);                                                  // Allume ou éteint la LED D4 en fonction des informations de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D5, TableauSequencesLED[SequenceCouranteLED][3]);                                                  // Allume ou éteint la LED D5 en fonction des informations de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D6, TableauSequencesLED[SequenceCouranteLED][4]);                                                  // Allume ou éteint la LED D6 en fonction des informations de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D7, TableauSequencesLED[SequenceCouranteLED][5]);                                                  // Allume ou éteint la LED D7 en fonction des informations de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      
      digitalWrite(BrocheLED_D13, TableauSequencesLED[SequenceCouranteLED][0]);                                                 // Allume ou éteint la LED D13 en fonction des informations de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D12, TableauSequencesLED[SequenceCouranteLED][1]);                                                 // Allume ou éteint la LED D12 en fonction des informations de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D11, TableauSequencesLED[SequenceCouranteLED][2]);                                                 // Allume ou éteint la LED D11 en fonction des informations de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D10, TableauSequencesLED[SequenceCouranteLED][3]);                                                 // Allume ou éteint la LED D10 en fonction des informations de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D9,  TableauSequencesLED[SequenceCouranteLED][4]);                                                 // Allume ou éteint la LED D9 en fonction des informations de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      digitalWrite(BrocheLED_D8,  TableauSequencesLED[SequenceCouranteLED][5]);                                                 // Allume ou éteint la LED D8 en fonction des informations de rang "SequenceCouranteLED" du tableau "TableauSequencesLED"
      
      SequenceCouranteLED++;                                                                                                    // Incrémente la valeur de la séquence courante des LED affichées
      if (SequenceCouranteLED > NombreSequencesLED - 1) // -------------------------------------------------------------------- // Si la séquence courante de LED affichée est supérieure au nombre de séquences d'affichage des LED
      {  
        SequenceCouranteLED = 0;                                                                                                // Borne la valeur de la séquence courante des LED affichées
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
      
      t1DebutTemporisation = millis();                                                                                          // (Re)Démarre la temporisation
    }
 }
//**************************************************************************************************************************************************************************
//*** Clignotement aléatoire de 12 LED non bloquant ************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 12) // ********************************************************************************************** // Si le mode courant "12" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 100;                                                                                 // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DureeTemporisation = random(20, 100); // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit une durée aléatoire de la temporisation en mode non bloquant
      DerniereFinTemporisation = 0ul;                                                                                           // Réinitialise la fin de la temporisation en mode non bloquant
      InverseurClignotementAlterne = HIGH;                                                                                      // Définit l'indicateur inverseur de clignotement de LED alternées
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    t1DebutTemporisation = millis();                                                                                            // Mémorise la date courante de la temporisation
    
    if (t1DebutTemporisation - DerniereFinTemporisation >= DureeTemporisation) // +++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation est écoulée
    {
      if (InverseurClignotementAlterne) // ------------------------------------------------------------------------------------ // Si l'indicateur inverseur de clignotement de LED alternées est activé
      {
        digitalWrite(TableauBrochesLED_D2D7[random(0, sizeof(TableauBrochesLED_D2D7) / sizeof(int))], HIGH);                    // Allume une LED aléatoirement du PORTD
        digitalWrite(TableauBrochesLED_D8D13[random(0, sizeof(TableauBrochesLED_D8D13) / sizeof(int))], HIGH);                  // Allume une LED aléatoirement du PORTB
      }
      else if (!InverseurClignotementAlterne) // ------------------------------------------------------------------------------ // Si l'indicateur inverseur de clignotement de LED alternées est désactivé
      {
        digitalWrite(TableauBrochesLED_D2D7[random(0, sizeof(TableauBrochesLED_D2D7) / sizeof(int))], LOW);                     // Eteint une LED aléatoirement du PORTD
        digitalWrite(TableauBrochesLED_D8D13[random(0, sizeof(TableauBrochesLED_D8D13) / sizeof(int))], LOW);                   // Eteint une LED aléatoirement du PORTB
      }
      
      DureeTemporisation = random(20, 100);                                                                                     // Définit une durée aléatoire de la temporisation en mode non bloquant
      
      InverseurClignotementAlterne = !InverseurClignotementAlterne;                                                             // Inverse l'indicateur inverseur de clignotement de LED alternées
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
      
      DerniereFinTemporisation = t1DebutTemporisation;                                                                          // Mémorise la date de la fin de la temporisation
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement simultané de 4 LED inversées avec 8 autres LED avec un tableau de séquences de LED non bloquant *********************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 13) // ********************************************************************************************** // Si le mode courant "13" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 100;                                                                                 // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DureeTemporisation = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la demi-période en mode non bloquant
      pointeur_TableauSequenceur = 0;                                                                                           // Initialise le pointeur du tableau de séquences
      Duree_EntreSequences = 0;                                                                                                 // Initialise la temporisation entre les séquences
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() > Duree_EntreSequences) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation entre les séquences est écoulée
    {
      Duree_EntreSequences = millis() + DureeTemporisation;                                                                     // Redémarre la temporisation entre les séquences
      
      digitalWrite(BrocheLED_D2, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                               // Allume ou éteint la LED D2 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D3, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                               // Allume ou éteint la LED D3 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D4, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D4 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D5, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D5 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D6, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                               // Allume ou éteint la LED D6 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D7, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                               // Allume ou éteint la LED D7 en fonction de l'état du contenu du pointeur du tableau de séquences
      
      digitalWrite(BrocheLED_D8,  Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D8 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D9,  Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D9 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D10, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                             // Allume ou éteint la LED D10 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D11, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                             // Allume ou éteint la LED D11 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D12, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D12 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D13, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D13 en fonction de l'état du contenu du pointeur du tableau de séquences
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
    }
    
    pointeur_TableauSequenceur = (pointeur_TableauSequenceur + 1) % Nombre_Sequences;                                           // Borne le tableau de séquences
  }
//**************************************************************************************************************************************************************************
//*** Clignotement simultané de 4 LED inversées avec 8 autres LED avec un registre 16 bits de séquences de LED non bloquant - Exemple 1 ************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 14) // ********************************************************************************************** // Si le mode courant "14" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 30;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DureeTemporisation = 40ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la demi-période en mode non bloquant
      pointeur_TableauSequenceur = 0;                                                                                           // Initialise le pointeur du tableau de séquences
      Duree_EntreSequences = 0;                                                                                                 // Initialise la temporisation entre les séquences
      Nombre_Sequences_BitsRegSeqs = 16; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences du registre 16 bits de séquences
      BitsRegSeqs = 0b1000100010101010; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Registre 16 bits de séquences
      BitsRegInverseurs = 0b11001100; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Registre inverseur des sorties des 6 LED
      pointeur_BitsRegSeqs = 15;                                                                                                // Pointeur de bits du registre 16 bits de séquences
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() > Duree_EntreSequences) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation entre les séquences est écoulée
    {
      Duree_EntreSequences = millis() + DureeTemporisation;                                                                     // Redémarre la temporisation entre les séquences
      
      if ((BitsRegSeqs & (1 << pointeur_BitsRegSeqs)) >> pointeur_BitsRegSeqs == 1) // ---------------------------------------- // Si le bit de rang "pointeur_BitsRegSeqs" est égal à "1"
      {
        PORTD &= 0b00000011;                                                                                                    // Réinitialise le PORTD sans modifier les bits "0" et "1"
        PORTD = 0b11111111 & (PORTD | BitsRegInverseurs);                                                                       // Calcule la valeur du PORTD sans modifier les bits "0" et "1"
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB = 0b11111111 & (PORTB | BitsRegInverseurs);                                                                       // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      }
      else // ----------------------------------------------------------------------------------------------------------------- // Si le bit de rang "pointeur_BitsRegSeqs" est égal à "0"
      {
        PORTD |= 0b11111100;                                                                                                    // Réinitialise le PORTD sans modifier les bits "0" et "1"
        PORTD = 0b00000000 | (PORTD & ~BitsRegInverseurs);                                                                      // Calcule la valeur du PORTD sans modifier les bits "0" et "1"
        
        PORTB |= 0b00111111;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB = 0b00000000 | (PORTB & ~BitsRegInverseurs);                                                                      // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      }
      
      pointeur_BitsRegSeqs = (pointeur_BitsRegSeqs - 1);                                                                        // Incrémente le pointeur du registre 16 bits de séquences
      if (pointeur_BitsRegSeqs < 0) // ........................................................................................ // Si le pointeur de bits du registre 16 bits de séquences est inférieur à "0"
      {
        pointeur_BitsRegSeqs = 15;                                                                                              // Borne le pointeur de bits du registre 16 bits de séquences
        
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED avec un registre 16 bits de séquences de LED non bloquant - Exemple 2 ****************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 15) // ********************************************************************************************** // Si le mode courant "15" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 30;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DureeTemporisation = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la demi-période en mode non bloquant
      pointeur_TableauSequenceur = 0;                                                                                           // Initialise le pointeur du tableau de séquences
      Duree_EntreSequences = 0;                                                                                                 // Initialise la temporisation entre les séquences
      Nombre_Sequences_BitsRegSeqs = 16; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences du registre 16 bits de séquences
      BitsRegSeqs = 0b1100110010101010; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Registre 16 bits de séquences
      BitsRegInverseurs = 0b00000100; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Registre inverseur des sorties des 6 LED
      pointeur_BitsRegSeqs = 15;                                                                                                // Pointeur de bits du registre 16 bits de séquences
      decalage_BitsRegInverseurs = 0;                                                                                           // Réinitialise la valeur du décalage du registre inverseur des sorties des 6 LED
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() > Duree_EntreSequences) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation entre les séquences est écoulée
    {
      Duree_EntreSequences = millis() + DureeTemporisation;                                                                     // Redémarre la temporisation entre les séquences
      
      if ((BitsRegSeqs & (1 << pointeur_BitsRegSeqs)) >> pointeur_BitsRegSeqs == 1) // ---------------------------------------- // Si le bit de rang "pointeur_BitsRegSeqs" est égal à "1"
      {
        PORTD &= 0b00000011;                                                                                                    // Réinitialise le PORTD sans modifier les bits "0" et "1"
        PORTD = 0b11111111 & (PORTD | BitsRegInverseurs);                                                                       // Calcule la valeur du PORTD sans modifier les bits "0" et "1"
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      }
      else // ----------------------------------------------------------------------------------------------------------------- // Si le bit de rang "pointeur_BitsRegSeqs" est égal à "0"
      {
        PORTD |= 0b11111100;                                                                                                    // Réinitialise le PORTD sans modifier les bits "0" et "1"
        PORTD = 0b00000000 | (PORTD & ~BitsRegInverseurs);                                                                      // Calcule la valeur du PORTD sans modifier les bits "0" et "1"
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      }
      
      pointeur_BitsRegSeqs = (pointeur_BitsRegSeqs - 1);                                                                        // Incrémente le pointeur du registre 16 bits de séquences
      if (pointeur_BitsRegSeqs < 0) {pointeur_BitsRegSeqs = 15;}                                                                // Borne le pointeur de bits du registre 16 bits de séquences
      
      BitsRegInverseurs <<= decalage_BitsRegInverseurs;                                                                         // Décale le registre inverseur des sorties des 6 LED
      decalage_BitsRegInverseurs++;                                                                                             // Incrémente la valeur du décalage du registre inverseur des sorties des 6 LED
      if (decalage_BitsRegInverseurs > 5) // ---------------------------------------------------------------------------------- // Si la valeur du décalage du registre inverseur des sorties des 6 LED est supérieure à 6
      {
        decalage_BitsRegInverseurs = 0;                                                                                         // Réinitialise la valeur du décalage du registre inverseur des sorties des 6 LED est supérieure à 6
        BitsRegInverseurs = 0b00000100;                                                                                         // Réinitialise le registre inverseur des sorties des 6 LED
        
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED avec un registre 16 bits de séquences de LED non bloquant - Exemple 3 ****************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 16) // ********************************************************************************************** // Si le mode courant "16" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 10;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DureeTemporisation = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la demi-période en mode non bloquant
      pointeur_TableauSequenceur = 0;                                                                                           // Initialise le pointeur du tableau de séquences
      Duree_EntreSequences = 0;                                                                                                 // Initialise la temporisation entre les séquences
      Nombre_Sequences_BitsRegSeqs = 16; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences du registre 16 bits de séquences
      BitsRegSeqs = 0b1100110010101010; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Registre 16 bits de séquences
      BitsRegInverseurs = 0b00000000; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Registre inverseur des sorties des 6 LED
      pointeur_BitsRegSeqs = 15;                                                                                                // Pointeur de bits du registre 16 bits de séquences
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() > Duree_EntreSequences) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation entre les séquences est écoulée
    {
      Duree_EntreSequences = millis() + DureeTemporisation;                                                                     // Redémarre la temporisation entre les séquences
      
      if ((BitsRegSeqs & (1 << pointeur_BitsRegSeqs)) >> pointeur_BitsRegSeqs == 1) // ---------------------------------------- // Si le bit de rang "pointeur_BitsRegSeqs" est égal à "1"
      {
        PORTD &= 0b00000011;                                                                                                    // Réinitialise le PORTD sans modifier les bits "0" et "1"
        PORTD = 0b11111111 & (PORTD | BitsRegInverseurs);                                                                       // Calcule la valeur du PORTD sans modifier les bits "0" et "1"
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB = 0b11111111 & (PORTB | BitsRegInverseurs);                                                                       // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      }
      else // ----------------------------------------------------------------------------------------------------------------- // Si le bit de rang "pointeur_BitsRegSeqs" est égal à "0"
      {
        PORTD |= 0b11111100;                                                                                                    // Réinitialise le PORTD sans modifier les bits "0" et "1"
        PORTD = 0b00000000 | (PORTD & ~BitsRegInverseurs);                                                                      // Calcule la valeur du PORTD sans modifier les bits "0" et "1"
        
        PORTB |= 0b00111111;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB = 0b00000000 | (PORTB & ~BitsRegInverseurs);                                                                      // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      }
      
      pointeur_BitsRegSeqs = (pointeur_BitsRegSeqs - 1);                                                                        // Incrémente le pointeur du registre 16 bits de séquences
      if (pointeur_BitsRegSeqs < 0) // ........................................................................................ // Si le pointeur de bits du registre 16 bits de séquences est inférieur à "0"
      {
        pointeur_BitsRegSeqs = 15;                                                                                              // Borne le pointeur de bits du registre 16 bits de séquences
        
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED avec un registre 16 bits de séquences de LED non bloquant - Exemple 4 ****************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 17) // ********************************************************************************************** // Si le mode courant "17" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 10;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DureeTemporisation = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la demi-période en mode non bloquant
      pointeur_TableauSequenceur = 0;                                                                                           // Initialise le pointeur du tableau de séquences
      Duree_EntreSequences = 0;                                                                                                 // Initialise la temporisation entre les séquences
      Nombre_Sequences_BitsRegSeqs = 16; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences du registre 16 bits de séquences
      BitsRegSeqs = 0b1100110010101010; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Registre 16 bits de séquences
      BitsRegInverseurs = 0b01001000; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Registre inverseur des sorties des 6 LED
      pointeur_BitsRegSeqs = 15;                                                                                                // Pointeur de bits du registre 16 bits de séquences
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() > Duree_EntreSequences) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation entre les séquences est écoulée
    {
      Duree_EntreSequences = millis() + DureeTemporisation;                                                                     // Redémarre la temporisation entre les séquences
      
      if ((BitsRegSeqs & (1 << pointeur_BitsRegSeqs)) >> pointeur_BitsRegSeqs == 1) // ---------------------------------------- // Si le bit de rang "pointeur_BitsRegSeqs" est égal à "1"
      {
        PORTD &= 0b00000011;                                                                                                    // Réinitialise le PORTD sans modifier les bits "0" et "1"
        PORTD = 0b11111111 & (PORTD | BitsRegInverseurs);                                                                       // Calcule la valeur du PORTD sans modifier les bits "0" et "1"
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB = 0b11111111 & (PORTB | BitsRegInverseurs);                                                                       // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      }
      else // ----------------------------------------------------------------------------------------------------------------- // Si le bit de rang "pointeur_BitsRegSeqs" est égal à "0"
      {
        PORTD |= 0b11111100;                                                                                                    // Réinitialise le PORTD sans modifier les bits "0" et "1"
        PORTD = 0b00000000 | (PORTD & ~BitsRegInverseurs);                                                                      // Calcule la valeur du PORTD sans modifier les bits "0" et "1"
        
        PORTB |= 0b00111111;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB = 0b00000000 | (PORTB & ~BitsRegInverseurs);                                                                      // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      }
      
      pointeur_BitsRegSeqs = (pointeur_BitsRegSeqs - 1);                                                                        // Incrémente le pointeur du registre 16 bits de séquences
      if (pointeur_BitsRegSeqs < 0) // ........................................................................................ // Si le pointeur de bits du registre 16 bits de séquences est inférieur à "0"
      {
        pointeur_BitsRegSeqs = 15;                                                                                              // Borne le pointeur de bits du registre 16 bits de séquences
        
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED avec un registre 16 bits de séquences de LED non bloquant - Exemple 5 ****************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 18) // ********************************************************************************************** // Si le mode courant "18" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 10;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DureeTemporisation = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la demi-période en mode non bloquant
      pointeur_TableauSequenceur = 0;                                                                                           // Initialise le pointeur du tableau de séquences
      Duree_EntreSequences = 0;                                                                                                 // Initialise la temporisation entre les séquences
      Nombre_Sequences_BitsRegSeqs = 16; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences du registre 16 bits de séquences
      BitsRegSeqs = 0b1100110010101010; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Registre 16 bits de séquences
      BitsRegInverseurs = 0b10101000; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Registre inverseur des sorties des 6 LED
      pointeur_BitsRegSeqs = 15;                                                                                                // Pointeur de bits du registre 16 bits de séquences
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() > Duree_EntreSequences) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation entre les séquences est écoulée
    {
      Duree_EntreSequences = millis() + DureeTemporisation;                                                                     // Redémarre la temporisation entre les séquences
      
      if ((BitsRegSeqs & (1 << pointeur_BitsRegSeqs)) >> pointeur_BitsRegSeqs == 1) // ---------------------------------------- // Si le bit de rang "pointeur_BitsRegSeqs" est égal à "1"
      {
        PORTD &= 0b00000011;                                                                                                    // Réinitialise le PORTD sans modifier les bits "0" et "1"
        PORTD = 0b11111111 & (PORTD | BitsRegInverseurs);                                                                       // Calcule la valeur du PORTD sans modifier les bits "0" et "1"
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB = 0b11111111 & (PORTB | BitsRegInverseurs);                                                                       // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      }
      else // ----------------------------------------------------------------------------------------------------------------- // Si le bit de rang "pointeur_BitsRegSeqs" est égal à "0"
      {
        PORTD |= 0b11111100;                                                                                                    // Réinitialise le PORTD sans modifier les bits "0" et "1"
        PORTD = 0b00000000 | (PORTD & ~BitsRegInverseurs);                                                                      // Calcule la valeur du PORTD sans modifier les bits "0" et "1"
        
        PORTB |= 0b00111111;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB = 0b00000000 | (PORTB & ~BitsRegInverseurs);                                                                      // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      }
      
      pointeur_BitsRegSeqs = (pointeur_BitsRegSeqs - 1);                                                                        // Incrémente le pointeur du registre 16 bits de séquences
      if (pointeur_BitsRegSeqs < 0) // ........................................................................................ // Si le pointeur de bits du registre 16 bits de séquences est inférieur à "0"
      {
        pointeur_BitsRegSeqs = 15;                                                                                              // Borne le pointeur de bits du registre 16 bits de séquences
        
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
    }
  }

//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED avec un registre 16 bits de séquences de LED non bloquant - Exemple 6 ****************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 19) // ********************************************************************************************** // Si le mode courant "19" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 10;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DureeTemporisation = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la demi-période en mode non bloquant
      pointeur_TableauSequenceur = 0;                                                                                           // Initialise le pointeur du tableau de séquences
      Duree_EntreSequences = 0;                                                                                                 // Initialise la temporisation entre les séquences
      Nombre_Sequences_BitsRegSeqs = 16; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences du registre 16 bits de séquences
      BitsRegSeqs = 0b1100010100111010; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Registre 16 bits de séquences
      BitsRegInverseurs = 0b01010100; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Registre inverseur des sorties des 6 LED
      pointeur_BitsRegSeqs = 15;                                                                                                // Pointeur de bits du registre 16 bits de séquences
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() > Duree_EntreSequences) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation entre les séquences est écoulée
    {
      Duree_EntreSequences = millis() + DureeTemporisation;                                                                     // Redémarre la temporisation entre les séquences
      
      if ((BitsRegSeqs & (1 << pointeur_BitsRegSeqs)) >> pointeur_BitsRegSeqs == 1) // ---------------------------------------- // Si le bit de rang "pointeur_BitsRegSeqs" est égal à "1"
      {
        PORTD &= 0b00000011;                                                                                                    // Réinitialise le PORTD sans modifier les bits "0" et "1"
        PORTD = 0b11111111 & (PORTD | BitsRegInverseurs);                                                                       // Calcule la valeur du PORTD sans modifier les bits "0" et "1"
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB = 0b11111111 & (PORTB | BitsRegInverseurs);                                                                       // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      }
      else // ----------------------------------------------------------------------------------------------------------------- // Si le bit de rang "pointeur_BitsRegSeqs" est égal à "0"
      {
        PORTD |= 0b11111100;                                                                                                    // Réinitialise le PORTD sans modifier les bits "0" et "1"
        PORTD = 0b00000000 | (PORTD & ~BitsRegInverseurs);                                                                      // Calcule la valeur du PORTD sans modifier les bits "0" et "1"
        
        PORTB |= 0b00111111;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB = 0b00000000 | (PORTB & ~BitsRegInverseurs);                                                                      // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      }
      
      pointeur_BitsRegSeqs = (pointeur_BitsRegSeqs - 1);                                                                        // Incrémente le pointeur du registre 16 bits de séquences
      if (pointeur_BitsRegSeqs < 0) // ........................................................................................ // Si le pointeur de bits du registre 16 bits de séquences est inférieur à "0"
      {
        pointeur_BitsRegSeqs = 15;                                                                                              // Borne le pointeur de bits du registre 16 bits de séquences
        
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED avec un registre 16 bits de séquences de LED non bloquant - Exemple 1 ****************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 20) // ********************************************************************************************** // Si le mode courant "20" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 10;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DureeTemporisation = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la demi-période en mode non bloquant
      pointeur_TableauSequenceur = 0;                                                                                           // Initialise le pointeur du tableau de séquences
      Duree_EntreSequences = 0;                                                                                                 // Initialise la temporisation entre les séquences
      Nombre_Sequences_BitsRegSeqs = 16; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences du registre 16 bits de séquences
      BitsRegSeqs = 0b1100010100111010; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Registre 16 bits de séquences
      BitsRegInverseurs = 0b00110000; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Registre inverseur des sorties des 6 LED
      pointeur_BitsRegSeqs = 15;                                                                                                // Pointeur de bits du registre 16 bits de séquences
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() > Duree_EntreSequences) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation entre les séquences est écoulée
    {
      Duree_EntreSequences = millis() + DureeTemporisation;                                                                     // Redémarre la temporisation entre les séquences
      
      if ((BitsRegSeqs & (1 << pointeur_BitsRegSeqs)) >> pointeur_BitsRegSeqs == 1) // ---------------------------------------- // Si le bit de rang "pointeur_BitsRegSeqs" est égal à "1"
      {
        PORTD &= 0b00000011;                                                                                                    // Réinitialise le PORTD sans modifier les bits "0" et "1"
        PORTD = 0b11111111 & (PORTD | BitsRegInverseurs);                                                                       // Calcule la valeur du PORTD sans modifier les bits "0" et "1"
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB = 0b11111111 & (PORTB | BitsRegInverseurs);                                                                       // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      }
      else // ----------------------------------------------------------------------------------------------------------------- // Si le bit de rang "pointeur_BitsRegSeqs" est égal à "0"
      {
        PORTD |= 0b11111100;                                                                                                    // Réinitialise le PORTD sans modifier les bits "0" et "1"
        PORTD = 0b00000000 | (PORTD & ~BitsRegInverseurs);                                                                      // Calcule la valeur du PORTD sans modifier les bits "0" et "1"
        
        PORTB |= 0b00111111;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB = 0b00000000 | (PORTB & ~BitsRegInverseurs);                                                                      // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      }
      
      pointeur_BitsRegSeqs = (pointeur_BitsRegSeqs - 1);                                                                        // Incrémente le pointeur du registre 16 bits de séquences
      if (pointeur_BitsRegSeqs < 0) // ........................................................................................ // Si le pointeur de bits du registre 16 bits de séquences est inférieur à "0"
      {
        pointeur_BitsRegSeqs = 15;                                                                                              // Borne le pointeur de bits du registre 16 bits de séquences
        
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED avec le modulo (%) non bloquant - Exemple 1 ******************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 21) // ********************************************************************************************** // Si le mode courant "21" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 10000;                                                                               // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() % 1000 < 500) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 1000 est inférieure à 500ms (A chaque période de 1 seconde)
    {
      digitalWrite(BrocheLED_D2, HIGH);                                                                                         // Allume la LED D2
      digitalWrite(BrocheLED_D7, LOW);                                                                                          // Eteint la LED D7
      
      digitalWrite(BrocheLED_D8, HIGH);                                                                                         // Allume la LED D8
      digitalWrite(BrocheLED_D13, LOW);                                                                                         // Eteint la LED D13
    }
    else // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 1000 est supérieure ou égale à 500ms (A chaque période de 1 seconde)
    {
      digitalWrite(BrocheLED_D2, LOW);                                                                                          // Eteint la LED D2
      digitalWrite(BrocheLED_D7, HIGH);                                                                                         // Allume la LED D7
      
      digitalWrite(BrocheLED_D8, LOW);                                                                                          // Eteint la LED D8
      digitalWrite(BrocheLED_D13, HIGH);                                                                                        // Allume la LED D13
    }
    
    if (millis() % 500 < 250) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 500 est inférieure à 250ms (A chaque période de 500ms)
    {
      digitalWrite(BrocheLED_D6, HIGH);                                                                                         // Allume la LED D6
      digitalWrite(BrocheLED_D3, LOW);                                                                                          // Eteint la LED D3
      
      digitalWrite(BrocheLED_D12, HIGH);                                                                                        // Allume la LED D12
      digitalWrite(BrocheLED_D9, LOW);                                                                                          // Eteint la LED D9
    }
    else // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 500 est supérieure ou égale à 250ms (A chaque période de 500ms)
    {
      digitalWrite(BrocheLED_D6, LOW);                                                                                          // Eteint la LED D6
      digitalWrite(BrocheLED_D3, HIGH);                                                                                         // Allume la LED D3
      
      digitalWrite(BrocheLED_D12, LOW);                                                                                         // Eteint la LED D12
      digitalWrite(BrocheLED_D9, HIGH);                                                                                         // Allume la LED D9
    }
    
    if (millis() % 250 < 125) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 250 est inférieure à 125ms (A chaque période de 250ms)
    {
      digitalWrite(BrocheLED_D4, HIGH);                                                                                         // Allume la LED D4
      digitalWrite(BrocheLED_D5, LOW);                                                                                          // Eteint la LED D5
      
      digitalWrite(BrocheLED_D10, HIGH);                                                                                        // Allume la LED D10
      digitalWrite(BrocheLED_D11, LOW);                                                                                         // Eteint la LED D11
    }
    else // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 250 est supérieur ou égal à 125ms (A chaque période de 250ms)
    {
      digitalWrite(BrocheLED_D4, LOW);                                                                                          // Eteint la LED D4
      digitalWrite(BrocheLED_D5, HIGH);                                                                                         // Allume la LED D5

      digitalWrite(BrocheLED_D10, LOW);                                                                                         // Eteint la LED D10
      digitalWrite(BrocheLED_D11, HIGH);                                                                                        // Allume la LED D11
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED avec le modulo (%) non bloquant - Exemple 2 ******************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 22) // ********************************************************************************************** // Si le mode courant "22" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 10000;                                                                               // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() % 1000 < 500) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 1000 est inférieure à 500ms (A chaque période de 1 seconde)
    {
      digitalWrite(BrocheLED_D5, HIGH);                                                                                         // Allume la LED D5
      digitalWrite(BrocheLED_D4, LOW);                                                                                          // Eteint la LED D4
      
      digitalWrite(BrocheLED_D11, HIGH);                                                                                        // Allume la LED D11
      digitalWrite(BrocheLED_D10, LOW);                                                                                         // Eteint la LED D10
    }
    else // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 1000 est supérieure ou égale à 500ms (A chaque période de 1 seconde)
    {
      digitalWrite(BrocheLED_D5, LOW);                                                                                          // Eteint la LED D5
      digitalWrite(BrocheLED_D4, HIGH);                                                                                         // Allume la LED D4
      
      digitalWrite(BrocheLED_D11, LOW);                                                                                         // Eteint la LED D11
      digitalWrite(BrocheLED_D10, HIGH);                                                                                        // Allume la LED D10
    }
    
    if (millis() % 500 < 250) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 500 est inférieure à 250ms (A chaque période de 500ms)
    {
      digitalWrite(BrocheLED_D6, HIGH);                                                                                         // Allume la LED D6
      digitalWrite(BrocheLED_D3, LOW);                                                                                          // Eteint la LED D3
      
      digitalWrite(BrocheLED_D12, HIGH);                                                                                        // Allume la LED D12
      digitalWrite(BrocheLED_D9, LOW);                                                                                          // Eteint la LED D9
    }
    else // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 500 est supérieure ou égale à 250ms (A chaque période de 500ms)
    {
      digitalWrite(BrocheLED_D6, LOW);                                                                                          // Eteint la LED D6
      digitalWrite(BrocheLED_D3, HIGH);                                                                                         // Allume la LED D3
      
      digitalWrite(BrocheLED_D12, LOW);                                                                                         // Eteint la LED D12
      digitalWrite(BrocheLED_D9, HIGH);                                                                                         // Allume la LED D9
    }
    
    if (millis() % 250 < 125) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 250 est inférieure à 125ms (A chaque période de 250ms)
    {
      digitalWrite(BrocheLED_D7, HIGH);                                                                                         // Allume la LED D7
      digitalWrite(BrocheLED_D2, LOW);                                                                                          // Eteint la LED D2
      
      digitalWrite(BrocheLED_D13, HIGH);                                                                                        // Allume la LED D13
      digitalWrite(BrocheLED_D8, LOW);                                                                                          // Eteint la LED D8
    }
    else // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 250 est supérieur ou égal à 125ms (A chaque période de 250ms)
    {
      digitalWrite(BrocheLED_D7, LOW);                                                                                          // Eteint la LED D7
      digitalWrite(BrocheLED_D2, HIGH);                                                                                         // Allume la LED D2
      
      digitalWrite(BrocheLED_D13, LOW);                                                                                         // Eteint la LED D13
      digitalWrite(BrocheLED_D8, HIGH);                                                                                         // Allume la LED D8
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED avec le modulo (%) non bloquant - Exemple 3 ******************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 23) // ********************************************************************************************** // Si le mode courant "23" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 10000;                                                                               // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() % 1000 < 500) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 1000 est inférieure à 500ms (A chaque période de 1 seconde)
    {
      digitalWrite(BrocheLED_D2, HIGH);                                                                                         // Allume la LED D2
      digitalWrite(BrocheLED_D3, LOW);                                                                                          // Eteint la LED D3
      
      digitalWrite(BrocheLED_D8, HIGH);                                                                                         // Allume la LED D8
      digitalWrite(BrocheLED_D9, LOW);                                                                                          // Eteint la LED D9
    }
    else // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 1000 est supérieure ou égale à 500ms (A chaque période de 1 seconde)
    {
      digitalWrite(BrocheLED_D2, LOW);                                                                                          // Eteint la LED D2
      digitalWrite(BrocheLED_D3, HIGH);                                                                                         // Allume la LED D3
      
      digitalWrite(BrocheLED_D8, LOW);                                                                                          // Eteint la LED D8
      digitalWrite(BrocheLED_D9, HIGH);                                                                                         // Allume la LED D9
    }
    
    if (millis() % 500 < 250) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 500 est inférieure à 250ms (A chaque période de 500ms)
    {
      digitalWrite(BrocheLED_D4, HIGH);                                                                                         // Allume la LED D4
      digitalWrite(BrocheLED_D5, LOW);                                                                                          // Eteint la LED D5
      
      digitalWrite(BrocheLED_D10, HIGH);                                                                                        // Allume la LED D10
      digitalWrite(BrocheLED_D11, LOW);                                                                                         // Eteint la LED D11
    }
    else // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 500 est supérieure ou égale à 250ms (A chaque période de 500ms)
    {
      digitalWrite(BrocheLED_D4, LOW);                                                                                          // Eteint la LED D4
      digitalWrite(BrocheLED_D5, HIGH);                                                                                         // Allume la LED D5
      
      digitalWrite(BrocheLED_D10, LOW);                                                                                         // Eteint la LED D10
      digitalWrite(BrocheLED_D11, HIGH);                                                                                        // Allume la LED D11
    }
    
    if (millis() % 250 < 125) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 250 est inférieure à 125ms (A chaque période de 250ms)
    {
      digitalWrite(BrocheLED_D6, HIGH);                                                                                         // Allume la LED D6
      digitalWrite(BrocheLED_D7, LOW);                                                                                          // Eteint la LED D7
      
      digitalWrite(BrocheLED_D12, HIGH);                                                                                        // Allume la LED D12
      digitalWrite(BrocheLED_D13, LOW);                                                                                         // Eteint la LED D13
    }
    else // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 250 est supérieur ou égal à 125ms (A chaque période de 250ms)
    {
      digitalWrite(BrocheLED_D6, LOW);                                                                                          // Eteint la LED D6
      digitalWrite(BrocheLED_D7, HIGH);                                                                                         // Allume la LED D7
      
      digitalWrite(BrocheLED_D12, LOW);                                                                                         // Eteint la LED D12
      digitalWrite(BrocheLED_D13, HIGH);                                                                                        // Allume la LED D13
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED avec le modulo (%) non bloquant - Exemple 4 ******************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 24) // ********************************************************************************************** // Si le mode courant "24" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 10000;                                                                               // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() % 600 < 300) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 600 est inférieure à 300ms (A chaque période de 1 seconde)
    {
      digitalWrite(BrocheLED_D5, HIGH);                                                                                         // Allume la LED D5
      digitalWrite(BrocheLED_D4, LOW);                                                                                          // Eteint la LED D4
      
      digitalWrite(BrocheLED_D11, HIGH);                                                                                        // Allume la LED D11
      digitalWrite(BrocheLED_D10, LOW);                                                                                         // Eteint la LED D10
    }
    else // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 600 est supérieure ou égale à 300ms (A chaque période de 1 seconde)
    {
      digitalWrite(BrocheLED_D5, LOW);                                                                                          // Eteint la LED D5
      digitalWrite(BrocheLED_D4, HIGH);                                                                                         // Allume la LED D4
      
      digitalWrite(BrocheLED_D11, LOW);                                                                                         // Eteint la LED D11
      digitalWrite(BrocheLED_D10, HIGH);                                                                                        // Allume la LED D10
    }
    
    if (millis() % 300 < 150) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 300 est inférieure à 150ms (A chaque période de 500ms)
    {
      digitalWrite(BrocheLED_D3, HIGH);                                                                                         // Allume la LED D3
      digitalWrite(BrocheLED_D6, LOW);                                                                                          // Eteint la LED D6
      
      digitalWrite(BrocheLED_D9, HIGH);                                                                                         // Allume la LED D9
      digitalWrite(BrocheLED_D12, LOW);                                                                                         // Eteint la LED D12
    }
    else // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 300 est supérieure ou égale à 150ms (A chaque période de 500ms)
    {
      digitalWrite(BrocheLED_D3, LOW);                                                                                          // Eteint la LED D3
      digitalWrite(BrocheLED_D6, HIGH);                                                                                         // Allume la LED D6
      
      digitalWrite(BrocheLED_D9, LOW);                                                                                          // Eteint la LED D9
      digitalWrite(BrocheLED_D12, HIGH);                                                                                        // Allume la LED D12
    }
    
    if (millis() % 150 < 75) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 150 est inférieure à 75ms (A chaque période de 250ms)
    {
      digitalWrite(BrocheLED_D7, HIGH);                                                                                         // Allume la LED D7
      digitalWrite(BrocheLED_D2, LOW);                                                                                          // Eteint la LED D2
      
      digitalWrite(BrocheLED_D13, HIGH);                                                                                        // Allume la LED D13
      digitalWrite(BrocheLED_D8, LOW);                                                                                          // Eteint la LED D8
    }
    else // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la valeur courante de millis() modulo 150 est supérieur ou égal à 75ms (A chaque période de 250ms)
    {
      digitalWrite(BrocheLED_D7, LOW);                                                                                          // Eteint la LED D7
      digitalWrite(BrocheLED_D2, HIGH);                                                                                         // Allume la LED D2
      
      digitalWrite(BrocheLED_D13, LOW);                                                                                         // Eteint la LED D13
      digitalWrite(BrocheLED_D8, HIGH);                                                                                         // Allume la LED D8
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED rotatif non bloquant *******************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 25) // ********************************************************************************************** // Si le mode courant "25" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 80;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      t1DebutTemporisation = 0ul;                                                                                               // Réinitialise la date courante de la temporisation en mode non bloquant
      DerniereFinTemporisation = 0ul;                                                                                           // Initialise la fin de la temporisation en mode non bloquant
      DureeTemporisation = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la temporisation à l'état haut et à l'état bas en mode non bloquant
      InverseurClignotementAlterne = true;                                                                                      // Définit l'indicateur inverseur de clignotement de LED alternées
      decalage_PORTD = 0;                                                                                                       // Définit la valeur du décalage du PORTD
      Masque_PORTD = 0b10000000;                                                                                                // Définit le masque du PORTD
      decalage_PORTB = 0;                                                                                                       // Définit la valeur du décalage du PORTB
      Masque_PORTB = 0b00100000;                                                                                                // Définit le masque du PORTB
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    t1DebutTemporisation = millis();                                                                                            // Mémorise la date courante de la temporisation
    
    if (t1DebutTemporisation - DerniereFinTemporisation >= DureeTemporisation) // +++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation est écoulée
    {
      if (InverseurClignotementAlterne) // ------------------------------------------------------------------------------------ // Si l'indicateur inverseur de clignotement de LED alternées est activé
      {
        byte Masque_PORTD_TEMP = Masque_PORTD >> decalage_PORTD;                                                                // Déclare un masque du PORTD temporaire pour stocker le calcul du décalage du masque du PORTD
        PORTD |= Masque_PORTD_TEMP;                                                                                             // Allume la LED courante du PORTD
        
        byte Masque_PORTB_TEMP = Masque_PORTB >> decalage_PORTB;                                                                // Déclare un masque du PORTB temporaire pour stocker le calcul du décalage du masque du PORTB
        PORTB |= Masque_PORTB_TEMP;                                                                                             // Allume la LED courante du PORTB
      }
      else if (!InverseurClignotementAlterne) // ------------------------------------------------------------------------------ // Si l'indicateur inverseur de clignotement de LED alternées est désactivé
      {
        byte Masque_PORTD_TEMP = ~Masque_PORTD >> decalage_PORTD;                                                               // Déclare un masque du PORTD temporaire pour stocker le calcul du décalage du masque du PORTD
        PORTD &= ~Masque_PORTD_TEMP;                                                                                            // Eteint la LED courante du PORTD
        
        byte Masque_PORTB_TEMP = ~Masque_PORTB >> decalage_PORTB;                                                               // Déclare un masque du PORTB temporaire pour stocker le calcul du décalage du masque du PORTB
        PORTB &= ~Masque_PORTB_TEMP;                                                                                            // Eteint la LED courante du PORTB
        
        decalage_PORTB++;                                                                                                       // Incrémente la valeur du décalage du PORTB
        decalage_PORTD++;                                                                                                       // Incrémente la valeur du décalage du PORTD
        if (decalage_PORTD > 5 && decalage_PORTB > 5) {decalage_PORTD = 0; decalage_PORTB = 0;} // ............................ // Borne la valeur du décalage du PORTD et du PORTB
      }
      
      InverseurClignotementAlterne = !InverseurClignotementAlterne;                                                             // Inverse l'indicateur inverseur de clignotement de LED alternées
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
      
      DerniereFinTemporisation = t1DebutTemporisation;                                                                          // Mémorise la date de la fin de la temporisation
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED classique aller-retour non bloquant ****************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 26) // ********************************************************************************************** // Si le mode courant "26" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 6;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PeriodeClignotement = 200ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la période du clignotement en ms
      RapportCyclique = 50; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la valeur en pourcentage du rapport cyclique
      Demi_PeriodeEtatHaut = (unsigned long) (RapportCyclique * PeriodeClignotement / 100.0);                                   // Calcule la demi-période à l'état haut en ms
      Demi_PeriodeEtatBas = (unsigned long) (PeriodeClignotement - Demi_PeriodeEtatHaut);                                       // Calcule la demi-période à l'état bas en ms
      //Serial.print("Demi_PeriodeEtatHaut = "); Serial.println(Demi_PeriodeEtatHaut); // Débug
      //Serial.print("Demi_PeriodeEtatBas = "); Serial.println(Demi_PeriodeEtatBas); // Débug
      decalage_PORTD = 0;                                                                                                       // Définit la valeur du décalage du PORTD
      compteur_Decalage_PORTD = 0;                                                                                              // Initialise le compteur de décalage du PORTD
      Masque_PORTD = 0b10000000;                                                                                                // Définit le masque du PORTD
      Sens_decalage_PORTD = true;                                                                                               // Définit l'indicateur du sens de décalage du PORTD
      decalage_PORTB = 0;                                                                                                       // Définit la valeur du décalage du PORTB
      compteur_Decalage_PORTB = 0;                                                                                              // Initialise le compteur de décalage du PORTB
      Masque_PORTB = 0b00100000;                                                                                                // Définit le masque du PORTB
      Sens_decalage_PORTB = true;                                                                                               // Définit l'indicateur du sens de décalage du PORTB
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Initialise la temporisation à l'état bas en mode non bloquant
      t1DebutTempo_EtatHaut = millis();                                                                                         // Démarre la temporisation à l'état haut en mode non bloquant
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() - t1DebutTempo_EtatHaut >= Demi_PeriodeEtatHaut && t1DebutTempo_EtatHaut != 0)// +++++++++++++++++++++++++++++ // Si la temporisation de la demi-période à l'état haut en ms est écoulée et Si elle a déjà démarré
    {
      PORTD |= Masque_PORTD;                                                                                                    // Allume la LED courante du PORTD
      PORTB |= Masque_PORTB;                                                                                                    // Allume la LED courante du PORTB
      
      t1DebutTempo_EtatHaut = 0ul;                                                                                              // Réinitialise la temporisation à l'état haut en mode non bloquant
      t1DebutTempo_EtatBas = millis();                                                                                          // Démarre la temporisation à l'état bas en mode non bloquant
    }
    else if (millis() - t1DebutTempo_EtatBas >= Demi_PeriodeEtatBas && t1DebutTempo_EtatBas != 0)// +++++++++++++++++++++++++++ // Si la temporisation de la demi-période de l'état bas en ms est écoulée et Si elle a déjà démarré
    {
      PORTD &= ~Masque_PORTD;                                                                                                   // Eteint la LED courante du PORTD
      PORTB &= ~Masque_PORTB;                                                                                                   // Eteint la LED courante du PORTB
      
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Réinitialise la temporisation à l'état bas en mode non bloquant
      t1DebutTempo_EtatHaut = millis();                                                                                         // Démarre la temporisation à l'état haut en mode non bloquant
      
      if (Sens_decalage_PORTD) {Masque_PORTD = Masque_PORTD >> 1;} // --------------------------------------------------------- // Si l'indicateur du sens de décalage du PORTD est activé => Décalage vers la droite
      else {Masque_PORTD = Masque_PORTD << 1;} // ----------------------------------------------------------------------------- // Si l'indicateur du sens de décalage du PORTD est désactivé => Décalage vers la gauche
      
      if (Sens_decalage_PORTB) {Masque_PORTB = Masque_PORTB >> 1;} // --------------------------------------------------------- // Si l'indicateur du sens de décalage du PORTB est activé => Décalage vers la droite
      else {Masque_PORTB = Masque_PORTB << 1;} // ----------------------------------------------------------------------------- // Si l'indicateur du sens de décalage du PORTB est désactivé => Décalage vers la gauche
      
      compteur_Decalage_PORTB++;                                                                                                // Incrémente le compteur de décalage du PORTB
      compteur_Decalage_PORTD++;                                                                                                // Incrémente le compteur de décalage du PORTD
      if (compteur_Decalage_PORTD == 5 && compteur_Decalage_PORTB == 5) // ---------------------------------------------------- // Borne les compteurs de décalages du PORTD et du PORTB
      {
        Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                             // Inverse l'indicateur du sens de décalage du PORTD
        compteur_Decalage_PORTD = 0;                                                                                            // Réinitialise le compteur de décalage du PORTD
        
        Sens_decalage_PORTB = !Sens_decalage_PORTB;                                                                             // Inverse l'indicateur du sens de décalage du PORTBD
        compteur_Decalage_PORTB = 0;                                                                                            // Réinitialise le compteur de décalage du PORTB
        
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED classique aller-retour inversé non bloquant ********************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 27) // ********************************************************************************************** // Si le mode courant "27" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 5;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PeriodeClignotement = 250ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la période du clignotement en ms
      RapportCyclique = 50; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la valeur en pourcentage du rapport cyclique
      Demi_PeriodeEtatHaut = (unsigned long) (RapportCyclique * PeriodeClignotement / 100.0);                                   // Calcule la demi-période à l'état haut en ms
      Demi_PeriodeEtatBas = (unsigned long) (PeriodeClignotement - Demi_PeriodeEtatHaut);                                       // Calcule la demi-période à l'état bas en ms
      //Serial.print("Demi_PeriodeEtatHaut = "); Serial.println(Demi_PeriodeEtatHaut); // Débug
      //Serial.print("Demi_PeriodeEtatBas = "); Serial.println(Demi_PeriodeEtatBas); // Débug
      decalage_PORTD = 0;                                                                                                       // Définit la valeur du décalage du PORTD
      compteur_Decalage_PORTD = 0;                                                                                              // Initialise le compteur de décalage du PORTD
      Masque_PORTD = 0b10000000;                                                                                                // Définit le masque du PORTD
      Sens_decalage_PORTD = true;                                                                                               // Définit l'indicateur du sens de décalage du PORTD
      decalage_PORTB = 0;                                                                                                       // Définit la valeur du décalage du PORTB
      compteur_Decalage_PORTB = 0;                                                                                              // Initialise le compteur de décalage du PORTB
      Masque_PORTB = 0b00000001;                                                                                                // Définit le masque du PORTB
      Sens_decalage_PORTB = false;                                                                                              // Définit l'indicateur du sens de décalage du PORTB
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Initialise la temporisation à l'état bas en mode non bloquant
      t1DebutTempo_EtatHaut = millis();                                                                                         // Démarre la temporisation à l'état haut en mode non bloquant
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() - t1DebutTempo_EtatHaut >= Demi_PeriodeEtatHaut && t1DebutTempo_EtatHaut != 0)// +++++++++++++++++++++++++++++ // Si la temporisation de la demi-période à l'état haut en ms est écoulée et Si elle a déjà démarré
    {
      PORTD |= Masque_PORTD;                                                                                                    // Allume la LED courante du PORTD
      PORTB |= Masque_PORTB;                                                                                                    // Allume la LED courante du PORTB
      
      t1DebutTempo_EtatHaut = 0ul;                                                                                              // Réinitialise la temporisation à l'état haut en mode non bloquant
      t1DebutTempo_EtatBas = millis();                                                                                          // Démarre la temporisation à l'état bas en mode non bloquant
    }
    else if (millis() - t1DebutTempo_EtatBas >= Demi_PeriodeEtatBas && t1DebutTempo_EtatBas != 0)// +++++++++++++++++++++++++++ // Si la temporisation de la demi-période de l'état bas en ms est écoulée et Si elle a déjà démarré
    {
      PORTD &= ~Masque_PORTD;                                                                                                   // Eteint la LED courante du PORTD
      PORTB &= ~Masque_PORTB;                                                                                                   // Eteint la LED courante du PORTB
      
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Réinitialise la temporisation à l'état bas en mode non bloquant
      t1DebutTempo_EtatHaut = millis();                                                                                         // Démarre la temporisation à l'état haut en mode non bloquant
      
      if (Sens_decalage_PORTD) {Masque_PORTD = Masque_PORTD >> 1;} // --------------------------------------------------------- // Si l'indicateur du sens de décalage du PORTD est activé => Décalage vers la droite
      else {Masque_PORTD = Masque_PORTD << 1;} // ----------------------------------------------------------------------------- // Si l'indicateur du sens de décalage du PORTD est désactivé => Décalage vers la gauche
      
      if (Sens_decalage_PORTB) {Masque_PORTB = Masque_PORTB >> 1;} // --------------------------------------------------------- // Si l'indicateur du sens de décalage du PORTB est activé => Décalage vers la droite
      else {Masque_PORTB = Masque_PORTB << 1;} // ----------------------------------------------------------------------------- // Si l'indicateur du sens de décalage du PORTB est désactivé => Décalage vers la gauche
      
      compteur_Decalage_PORTB++;                                                                                                // Incrémente le compteur de décalage du PORTB
      compteur_Decalage_PORTD++;                                                                                                // Incrémente le compteur de décalage du PORTD
      if (compteur_Decalage_PORTD == 5 && compteur_Decalage_PORTB == 5) // ---------------------------------------------------- // Borne les compteurs de décalages du PORTD et du PORTB
      {
        Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                             // Inverse l'indicateur du sens de décalage du PORTD
        compteur_Decalage_PORTD = 0;                                                                                            // Réinitialise le compteur de décalage du PORTD
        
        Sens_decalage_PORTB = !Sens_decalage_PORTB;                                                                             // Inverse l'indicateur du sens de décalage du PORTBD
        compteur_Decalage_PORTB = 0;                                                                                            // Réinitialise le compteur de décalage du PORTB
        
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 6 LED multiple flashs par LED de gauche à droite et de droite à gauche non bloquant ***********************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 28) // ********************************************************************************************** // Si le mode courant "28" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      MaskLeds = 0b11111100;                                                                                                    // Définit le masque des LED pour déterminer la LED allumée
      EtatInitial = 0b00000100;                                                                                                 // Définit l'état initial des LED
      etatLeds = EtatInitial;                                                                                                   // Définit l'état courant des LED égal à l'état initial des LED
      NombreFlashs = 5; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Valeur du nombre de flashs
      CompteurFlashs = 0;                                                                                                       // Compteur de flashs
      t1DebutTempoMultipleFlash_ON = 0ul;                                                                                       // Initialise la temporisation des multiples flashs ON
      DureeTempoFlash1_ON = 30ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Duree de la temporisation du flash 1 ON
      t1DebutTempoMultipleFlash_OFF = 0ul;                                                                                      // Initialise la temporisation entre les multiples flashs
      DureeTempoMultipleFlash_OFF = 10ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation entre les multiples flashs
      Sens_decalage_PORTD = false;                                                                                              // Sens de départ de gauche à droite du décalage du PORTD
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (!Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de gauche à droite
    {
      if (t1DebutTempoMultipleFlash_ON == 0ul) // ----------------------------------------------------------------------------- // Si la temporisation des multiples flashs ON n'a pas démarré
      {
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        CompteurFlashs++;                                                                                                       // Incrémente le compteur de flashs
        
        t1DebutTempoMultipleFlash_ON = millis();                                                                                // Démarre la temporisation des multiples flashs ON
      }
      // ---------------------------------------------------------------------------------------------------------------------- // Si la temporisation des multiples flashs ON est écoulée et Si la temporisation entre les multiples flashs n'a pas démarré
      else if (millis() - t1DebutTempoMultipleFlash_ON >= DureeTempoFlash1_ON && t1DebutTempoMultipleFlash_ON != 0ul && t1DebutTempoMultipleFlash_OFF == 0ul)
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempoMultipleFlash_OFF = millis();                                                                               // Démarre la temporisation entre les flashs
      }
      // ---------------------------------------------------------------------------------------------------------------------- // Si la temporisation entre les multiples flashs est écoulée
      else if (millis() - t1DebutTempoMultipleFlash_OFF >= DureeTempoMultipleFlash_OFF && t1DebutTempoMultipleFlash_OFF != 0ul)
      {
        if (CompteurFlashs == NombreFlashs) // ................................................................................ // Si le compteur de flashs est égal à la valeur du nombre de flashs
        {
          t1DebutTempoMultipleFlash_ON = 0ul;                                                                                   // Réinitialise la temporisation des multiples flashs ON
          t1DebutTempoMultipleFlash_OFF = 0ul;                                                                                  // Réinitialise la temporisation entre les multiples flashs
          
          CompteurFlashs = 0;                                                                                                   // Réinitialise le compteur de flashs
          
          etatLeds <<= 1;                                                                                                       // Décale l'état courant des LED à gauche de 1 position
          
          if ((etatLeds & MaskLeds) == 0)                                                                                       // Si l'état courant des LED a été décalé 6 fois à gauche de 1 position
          {
            etatLeds = 0b01000000;                                                                                              // Définit l'état initial pour le sens contraire égal à "B01000000"
            Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                         // Inverse le sens du décalage du PORTD
          }
        }
        else if (CompteurFlashs < NombreFlashs) // ............................................................................ // Si le compteur de flashs est inférieur à la valeur du nombre de flashs
        {
          t1DebutTempoMultipleFlash_ON = 0ul;                                                                                   // Réinitialise la temporisation des multiples flashs ON
          t1DebutTempoMultipleFlash_OFF = 0ul;                                                                                  // Réinitialise la temporisation entre les multiples flashs
        }
      }
    }
    else if (Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de droite à gauche
    {
      if (t1DebutTempoMultipleFlash_ON == 0ul) // ----------------------------------------------------------------------------- // Si la temporisation des multiples flashs ON n'a pas démarré
      {
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        CompteurFlashs++;                                                                                                       // Incrémente le compteur de flashs
        
        t1DebutTempoMultipleFlash_ON = millis();                                                                                // Démarre la temporisation des multiples flashs ON
      }
      // ---------------------------------------------------------------------------------------------------------------------- // Si la temporisation des multiples flashs ON est écoulée et Si la temporisation entre les multiples flashs n'a pas démarré
      else if (millis() - t1DebutTempoMultipleFlash_ON >= DureeTempoFlash1_ON && t1DebutTempoMultipleFlash_ON != 0ul && t1DebutTempoMultipleFlash_OFF == 0ul)
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempoMultipleFlash_OFF = millis();                                                                               // Démarre la temporisation entre les flashs
      }
      // ---------------------------------------------------------------------------------------------------------------------- // Si la temporisation entre les multiples flashs est écoulée
      else if (millis() - t1DebutTempoMultipleFlash_OFF >= DureeTempoMultipleFlash_OFF && t1DebutTempoMultipleFlash_OFF != 0ul)
      {
        //Serial.println("Fin t1DebutTempoMultipleFlash_OFF"); // Débug
        if (CompteurFlashs == NombreFlashs) // ................................................................................ // Si le compteur de flashs est égal à la valeur du nombre de flashs
        {
          t1DebutTempoMultipleFlash_ON = 0ul;                                                                                   // Réinitialise la temporisation des multiples flashs ON
          t1DebutTempoMultipleFlash_OFF = 0ul;                                                                                  // Réinitialise la temporisation entre les multiples flashs
          
          CompteurFlashs = 0;                                                                                                   // Réinitialise le compteur de flashs
          
          etatLeds >>= 1;                                                                                                       // Décale l'état courant des LED à droite de 1 position
          if ((etatLeds & MaskLeds) == 0)                                                                                       // Si l'état courant des LED a été décalé 6 fois à droite de 1 position
          {
            etatLeds = 0b00001000;                                                                                              // Définit l'état initial pour le sens contraire égal à "B00010000"
            Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                         // Inverse le sens du décalage du PORTD
            
            compteurAffichageModeCourant++;                                                                                     // Incrémente le compteur d'affichages du mode courant en mode automatique
          }
        }
        else if (CompteurFlashs < NombreFlashs) // ............................................................................ // Si le compteur de flashs est inférieur à la valeur du nombre de flashs
        {
          t1DebutTempoMultipleFlash_ON = 0ul;                                                                                   // Réinitialise la temporisation des multiples flashs ON
          t1DebutTempoMultipleFlash_OFF = 0ul;                                                                                  // Réinitialise la temporisation entre les multiples flashs
        }
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 6 LED de gauche à droite avec variation de la vitesse de défilement non bloquant **************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 29) // ********************************************************************************************** // Si le mode courant "29" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 5;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      MaskLeds = 0b00000100;                                                                                                    // Valeur initiale du masque pour déterminer la LED courante
      DureeTempo_EtatBas = 10ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état bas
      DureeTempo_EtatHaut = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état haut
      PourcentageDureeHIGHMaxMin = 50; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Valeur en pourcentage pour calculer les temporisations Max et Min de la LED à l'état haut
      DureeHIGHMax = DureeTempo_EtatHaut + DureeTempo_EtatHaut * PourcentageDureeHIGHMaxMin / 100;                              // Temporisation Max de la LED à l'état haut
      DureeHIGHMin = DureeTempo_EtatHaut - DureeTempo_EtatHaut * PourcentageDureeHIGHMaxMin / 100;                              // Temporisation Min de la LED à l'état haut
      variateurDureeHIGH = 1;                                                                                                   // Variateur de la temporisation de la LED à l'état haut (= 1 ou -1)
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (decalage_PORTD = 0; decalage_PORTD < 6; decalage_PORTD++) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt les bits 2 à 7
    {
      PORTD = PORTD | (MaskLeds << decalage_PORTD);                                                                             // Active la LED de rang "decalage_PORTD"
      
      PORTB &= 0b11000000;                                                                                                      // Réinitialise le PORTB sans modifier les bits "6" et "7"
      PORTB |= PORTD >> 2;                                                                                                      // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      
      t1DebutTempo_EtatHaut = millis();                                                                                         // Démarre la temporisation à l'état haut non bloquant
      while (millis() - t1DebutTempo_EtatHaut <= DureeTempo_EtatHaut) // ------------------------------------------------------ // Tant que la temporisation à l'état haut n'est pas écoulée
      {
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs "BPModePlus" ou "BPModeMoins"
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      DureeTempo_EtatHaut = DureeTempo_EtatHaut + variateurDureeHIGH;                                                           // Incrémente ou décrémente la temporisation de la LED à l'état haut en fonction de la valeur du variateur
      if (DureeTempo_EtatHaut > DureeHIGHMax) {variateurDureeHIGH = -variateurDureeHIGH;} // ---------------------------------- // Si la temporisation de la LED à l'état haut est supérieure à "DureeHIGHMax" => Inverse la valeur du variateur
      if (DureeTempo_EtatHaut < DureeHIGHMin) {variateurDureeHIGH = -variateurDureeHIGH;} // ---------------------------------- // Si la temporisation de la LED à l'état haut est inférieure à "DureeHIGHMin" => Inverse la valeur du variateur
      
      PORTD = PORTD & ~(MaskLeds << decalage_PORTD);                                                                            // Désactive la LED de rang "decalage_PORTD"
      
      PORTB &= 0b11000000;                                                                                                      // Réinitialise le PORTB sans modifier les bits "6" et "7"
      PORTB |= PORTD >> 2;                                                                                                      // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      
      t1DebutTempo_EtatBas = millis();                                                                                          // Démarre la temporisation à l'état bas non bloquant
      while (millis() - t1DebutTempo_EtatBas <= DureeTempo_EtatBas) // -------------------------------------------------------- // Tant que la temporisation à l'état bas n'est pas écoulée
      {
        if (SortieModeCourant) //.............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs "BPModePlus" ou "BPModeMoins"
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    compteurAffichageModeCourant++;                                                                                             // Incrémente le compteur d'affichages du mode courant en mode automatique
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 6 LED de gauche à droite et de droite à gauche simultanément **********************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 30) // ********************************************************************************************** // Si le mode courant "30" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 20;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      MaskLeds = 0b11111100;                                                                                                    // Définit le masque des LED pour déterminer la LED allumée
      EtatInitial = 0b10000100;                                                                                                 // Définit l'état initial des LED
      etatLeds = EtatInitial;                                                                                                   // Définit l'état courant des LED égal à l'état initial des LED
      Dernier_etatLeds = EtatInitial;                                                                                           // Définit le dernier état courant des LED égal à l'état initial des LED
      DureeTempo_EtatBas = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état bas
      DureeTempo_EtatHaut = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état haut
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Initialise la temporisation à l'état bas en mode non bloquant
      t1DebutTempo_EtatHaut = millis();                                                                                         // Démarre la temporisation à l'état haut en mode non bloquant
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // +++++++++++++++++++++++++++ // Si la temporisation à l'état haut en mode non bloquant est écoulée
    {
      PORTD |= (etatLeds & MaskLeds);                                                                                           // Allume la LED courante
      
      PORTB &= 0b11000000;                                                                                                      // Réinitialise le PORTB sans modifier les bits "6" et "7"
      PORTB |= PORTD >> 2;                                                                                                      // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
      
      t1DebutTempo_EtatHaut = 0ul;                                                                                              // Initialise la temporisation à l'état haut en mode non bloquant
      t1DebutTempo_EtatBas = millis();                                                                                          // Démarre la temporisation à l'état bas en mode non bloquant
    }
    else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // +++++++++++++++++++++++++ // Si la temporisation à l'état bas en mode non bloquant est écoulée
    {
      PORTD &= ~(etatLeds & MaskLeds);                                                                                          // Eteint la LED courante
      
      t1DebutTempo_EtatHaut = 0ul;                                                                                              // Initialise la temporisation à l'état haut en mode non bloquant
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Initialise la temporisation à l'état bas en mode non bloquant
    }
    
    if (t1DebutTempo_EtatHaut == 0ul && t1DebutTempo_EtatBas == 0ul) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si les temporisations à l'état haut et à l'état bas sont arrêtées
    {
           if (etatLeds == 0b10000100 && Dernier_etatLeds == 0b10000100) {etatLeds = 0b01001000; Dernier_etatLeds = 0b10000100;}// Si l'état courant des LED est égal à l'état initial des LED et Si le dernier état courant des LED est égal à l'état initial des LED
      else if (etatLeds == 0b01001000 && Dernier_etatLeds == 0b10000100) {etatLeds = 0b00110000; Dernier_etatLeds = 0b01001000;}// Si l'état courant des LED est égal à "0b01001000" et Si le dernier état courant des LED est égal à l'état initial des LED
      else if (etatLeds == 0b00110000 && Dernier_etatLeds == 0b01001000) {etatLeds = 0b00110000; Dernier_etatLeds = 0b00110000;}// Si l'état courant des LED est égal à "0b00110000" et Si le dernier état courant des LED est égal à "0b01001000"
      else if (etatLeds == 0b00110000 && Dernier_etatLeds == 0b00110000) {etatLeds = 0b01001000; Dernier_etatLeds = 0b00110000;}// Si l'état courant des LED est égal à "0b00110000" et Si le dernier état courant des LED est égal à "0b00110000"
      else if (etatLeds == 0b01001000 && Dernier_etatLeds == 0b00110000) {etatLeds = 0b10000100; Dernier_etatLeds = 0b10000100;}// Si l'état courant des LED est égal à "0b01001000" et Si le dernier état courant des LED est égal à "0b00110000"
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
      
      t1DebutTempo_EtatHaut = millis();                                                                                         // Démarre la temporisation à l'état haut en mode non bloquant
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 6 LED de gauche à droite et de droite à gauche négatif ****************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 31) // ********************************************************************************************** // Si le mode courant "31" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 10;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      MaskLeds = 0b11111100;                                                                                                    // Définit le masque des LED pour déterminer la LED allumée
      MaskEtatLedsInitial = 0b00000100;                                                                                         // Définit le masque initial de l'état courant des LED
      MaskEtatLeds = MaskEtatLedsInitial;                                                                                       // Définit le masque de l'état courant des LED égal au masque initial de l'état courant des LED
      EtatInitial = 0b11111000;                                                                                                 // Définit l'état initial
      etatLeds = EtatInitial;                                                                                                   // Définit l'état courant des LED égal à l'état initial des LED
      Sens_decalage_PORTD = false;                                                                                              // Définit l'indicateur du sens de décalage du PORTD
      PORTD = 0b11111100;                                                                                                       // Allume les 6 LED
      DureeTempo_EtatBas = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état bas
      DureeTempo_EtatHaut = 10ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état haut
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Initialise la temporisation à l'état bas en mode non bloquant
      t1DebutTempo_EtatHaut = millis();                                                                                         // Démarre la temporisation à l'état haut en mode non bloquant
            
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (!Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de gauche à droite
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        PORTD &= (etatLeds & MaskLeds);                                                                                         // Eteint la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Initialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = millis();                                                                                        // Démarre la temporisation à l'état bas en mode non bloquant
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD |= (etatLeds | MaskLeds);                                                                                         // Allume la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Initialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = 0ul;                                                                                             // Initialise la temporisation à l'état bas en mode non bloquant
      }
      
      if (t1DebutTempo_EtatHaut == 0ul && t1DebutTempo_EtatBas == 0ul) // ----------------------------------------------------- // Si les temporisations à l'état haut et à l'état bas sont arrêtées
      {
        etatLeds = (etatLeds | MaskEtatLeds) & ~(MaskEtatLeds << 1);                                                            // Calcule la nouvelle valeur de l'état courant des LED
        MaskEtatLeds <<= 1;                                                                                                     // Décale l'état courant des LED à gauche de 1 position
        if (etatLeds == MaskLeds) // .......................................................................................... // Si le masque de l'état courant des LED a été décalé 6 fois à gauche de 1 position
        {
          MaskEtatLedsInitial = 0b10000000;                                                                                     // Définit le masque initial de l'état courant des LED
          MaskEtatLeds = MaskEtatLedsInitial;                                                                                   // Transfère le masque initial de l'état courant des LED dans le masque de l'état courant des LED
          EtatInitial = 0b01111100;                                                                                             // Définit l'état initial
          etatLeds = EtatInitial;                                                                                               // Transfère l'état initial des LED dans l'état courant des LED
          Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                           // Inverse l'indicateur du sens de décalage du PORTD
        }
        
        t1DebutTempo_EtatHaut = millis();                                                                                       // Démarre la temporisation à l'état haut en mode non bloquant
      }
    }
    else if (Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de droite à gauche
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        PORTD &= (etatLeds & MaskLeds);                                                                                         // Eteint la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Initialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = millis();                                                                                        // Démarre la temporisation à l'état bas en mode non bloquant
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD |= (etatLeds | MaskLeds);                                                                                         // Allume la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Initialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = 0ul;                                                                                             // Initialise la temporisation à l'état bas en mode non bloquant
      }
      
      if (t1DebutTempo_EtatHaut == 0ul && t1DebutTempo_EtatBas == 0ul) // ----------------------------------------------------- // Si les temporisations à l'état haut et à l'état bas sont arrêtées
      {
        etatLeds = (etatLeds | MaskEtatLeds) & ~(MaskEtatLeds >> 1);                                                            // Calcule la nouvelle valeur de l'état courant des LED
        MaskEtatLeds >>= 1;                                                                                                     // Décale l'état courant des LED à droite de 1 position
        if (etatLeds == MaskLeds) // .......................................................................................... // Si le masque de l'état courant des LED a été décalé 6 fois à droite de 1 position
        {
          MaskEtatLedsInitial = 0b00000100;                                                                                     // Définit le masque initial de l'état courant des LED
          MaskEtatLeds = MaskEtatLedsInitial;                                                                                   // Transfère le masque initial de l'état courant des LED dans le masque de l'état courant des LED
          EtatInitial = 0b11111000;                                                                                             // Définit l'état initial
          etatLeds = EtatInitial;                                                                                               // Transfère l'état initial des LED dans l'état courant des LED
          Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                           // Inverse l'indicateur du sens de décalage du PORTD
          
          compteurAffichageModeCourant++;                                                                                       // Incrémente le compteur d'affichages du mode courant en mode automatique
        }
        
        t1DebutTempo_EtatHaut = millis();                                                                                       // Démarre la temporisation à l'état haut en mode non bloquant
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 6 LED remplissage de gauche à droite et de droite à gauche non bloquant ***********************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 32) // ********************************************************************************************** // Si le mode courant "32" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 2;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      MaskLedsInitial = 0b11111100;                                                                                             // Définit le masque initial des LED pour déterminer l'état des LED
      MaskLeds = MaskLedsInitial;                                                                                               // Définit le masque des LED égal au masque initial des LED pour déterminer l'état des LED
      EtatInitial = 0b00000100;                                                                                                 // Définit l'état initial
      etatLeds = EtatInitial;                                                                                                   // Définit l'état courant des LED égal à l'état initial des LED
      CompteurDeplacementsComplets = 0;                                                                                         // Initialise le compteur de déplacements complets
      Sens_decalage_PORTD = false;                                                                                              // Définit l'indicateur du sens de décalage du PORTD
      DureeTempo_EtatBas = 200ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état bas
      DureeTempo_EtatHaut = 10ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état haut
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Initialise la temporisation à l'état bas en mode non bloquant
      t1DebutTempo_EtatHaut = millis();                                                                                         // Démarre la temporisation à l'état haut en mode non bloquant
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (!Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de gauche à droite
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Initialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = millis();                                                                                        // Démarre la temporisation à l'état bas en mode non bloquant
        
        if ((etatLeds << 1 & MaskLeds) == 0) // ............................................................................... // Si le déplacement est arrivée à sa position finale
        {
          CompteurDeplacementsComplets++;                                                                                       // Incrémente le compteur de déplacements complets
          
          if (CompteurDeplacementsComplets == 6)                                                                                // Si le remplissage est terminé
          {
            unsigned long TimeOut = millis();                                                                                   // Démarre la temporisation
            while (millis() - TimeOut <= 500ul) // ---------------------------------------------------------------------------- // Tant que la temporisation n'est pas écoulée
            {
              if (SortieModeCourant) // ....................................................................................... // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs "BPModePlus" ou "BPModeMoins"
              {
                SortieModeCourant = false;                                                                                      // Réinitialise l'indicateur de sortie du mode courant
                return;                                                                                                         // Retour début loop()
              }
            }
            
            PORTD &= 0b00000011;                                                                                                // Eteint les 6 LED du PORTD
            PORTB &= 0b11000000;                                                                                                // Eteint les 6 LED du PORTB
            MaskLeds = MaskLedsInitial;                                                                                         // Réinitialise le masque des LED
            EtatInitial = 0b10000000;                                                                                           // Définit l'état initial
            etatLeds = EtatInitial;                                                                                             // Réinitialise l'état courant des LED égal à l'état initial des LED
            CompteurDeplacementsComplets = 0;                                                                                   // Réinitialise le compteur de déplacements complets
            Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                         // Inverse l'indicateur du sens de décalage du PORTD
          }
          else                                                                                                                  // Si l'état courant des LED est différent de l'état initial des LED => Initialise les variables pour le prochain déplacement
          {
            etatLeds = EtatInitial;                                                                                             // Réinitialise l'état courant des LED égal à l'état initial des LED
            MaskLeds >>= 1;                                                                                                     // Décale le masque des LED à droite de 1 position
          }
          
          t1DebutTempo_EtatBas = 0ul;                                                                                           // Initialise la temporisation à l'état bas en mode non bloquant
          t1DebutTempo_EtatHaut = millis();                                                                                     // Démarre la temporisation à l'état haut en mode non bloquant
        }
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Initialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = 0ul;                                                                                             // Initialise la temporisation à l'état bas en mode non bloquant
      }
      
      if (t1DebutTempo_EtatHaut == 0ul && t1DebutTempo_EtatBas == 0ul) // ----------------------------------------------------- // Si les temporisations à l'état haut et à l'état bas sont arrêtées
      {
        etatLeds <<= 1;                                                                                                         // Décale l'état courant des LED à gauche de 1 position
        
        t1DebutTempo_EtatHaut = millis();                                                                                       // Démarre la temporisation à l'état haut en mode non bloquant
      }
    }
    else if (Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de droite à gauche
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Initialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = millis();                                                                                        // Démarre la temporisation à l'état bas en mode non bloquant
        
        if ((etatLeds >> 1 & MaskLeds) == 0) // ............................................................................... // Si le déplacement est arrivée à sa position finale
        {
          CompteurDeplacementsComplets++;                                                                                       // Incrémente le compteur de déplacements complets
          
          if (CompteurDeplacementsComplets == 6)                                                                                // Si le remplissage est terminé
          {
            unsigned long TimeOut = millis();                                                                                   // Démarre la temporisation
            while (millis() - TimeOut <= 500ul) // ---------------------------------------------------------------------------- // Tant que la temporisation n'est pas écoulée
            {
              if (SortieModeCourant) // ....................................................................................... // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs "BPModePlus" ou "BPModeMoins"
              {
                SortieModeCourant = false;                                                                                      // Réinitialise l'indicateur de sortie du mode courant
                return;                                                                                                         // Retour début loop()
              }
            }
            
            PORTD &= 0b00000011;                                                                                                // Eteint les 6 LED du PORTD
            PORTB &= 0b11000000;                                                                                                // Eteint les 6 LED du PORTB
            MaskLeds = MaskLedsInitial;                                                                                         // Réinitialise le masque des LED
            EtatInitial = 0b00000100;                                                                                           // Définit l'état initial
            etatLeds = EtatInitial;                                                                                             // Réinitialise l'état courant des LED égal à l'état initial des LED
            CompteurDeplacementsComplets = 0;                                                                                   // Réinitialise le compteur de déplacements complets
            Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                         // Inverse l'indicateur du sens de décalage du PORTD
            
            compteurAffichageModeCourant++;                                                                                     // Incrémente le compteur d'affichages du mode courant en mode automatique
          }
          else                                                                                                                  // Si l'état courant des LED est différent de l'état initial des LED => Initialise les variables pour le prochain déplacement
          {
            etatLeds = EtatInitial;                                                                                             // Réinitialise l'état courant des LED égal à l'état initial des LED
            MaskLeds <<= 1;                                                                                                     // Décale le masque des LED à gauche de 1 position
          }
          
          t1DebutTempo_EtatBas = 0ul;                                                                                           // Initialise la temporisation à l'état bas en mode non bloquant
          t1DebutTempo_EtatHaut = millis();                                                                                     // Démarre la temporisation à l'état haut en mode non bloquant
        }
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Initialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = 0ul;                                                                                             // Initialise la temporisation à l'état bas en mode non bloquant
      }
      
      if (t1DebutTempo_EtatHaut == 0ul && t1DebutTempo_EtatBas == 0ul) // ----------------------------------------------------- // Si les temporisations à l'état haut et à l'état bas sont arrêtées
      {
        etatLeds >>= 1;                                                                                                         // Décale l'état courant des LED à droite de 1 position
        
        t1DebutTempo_EtatHaut = millis();                                                                                       // Démarre la temporisation à l'état haut en mode non bloquant
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 6 LED passage de gauche à droite et de droite à gauche non bloquant ***************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 33) // ********************************************************************************************** // Si le mode courant "33" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      MaskLeds = 0b11111100;                                                                                                    // Définit le masque des LED pour déterminer la LED allumée
      EtatInitial = 0b00000100;                                                                                                 // Définit l'état initial
      etatLeds = EtatInitial;                                                                                                   // Définit l'état courant des LED égal à l'état initial des LED
      Sens_decalage_PORTD = false;                                                                                              // Définit l'indicateur du sens de décalage du PORTD
      DureeTempo_EtatBas = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état bas
      DureeTempo_EtatHaut = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état haut
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Initialise la temporisation à l'état bas en mode non bloquant
      t1DebutTempo_EtatHaut = millis();                                                                                         // Démarre la temporisation à l'état haut en mode non bloquant
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (!Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de gauche à droite
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatBas = 0ul;                                                                                             // Initialise la temporisation à l'état bas en mode non bloquant
        
        etatLeds <<= 1;                                                                                                         // Décale l'état courant des LED à gauche de 1 position
        if ((etatLeds & MaskLeds) == 0) // .................................................................................... // Si l'état courant des LED a été décalé 6 fois à gauche de 1 position
        {
          etatLeds = 0b00000100;                                                                                                // Définit l'état initial des LED
          t1DebutTempo_EtatHaut = 0ul;                                                                                          // Initialise la temporisation à l'état bas en mode non bloquant
          t1DebutTempo_EtatBas = millis();                                                                                      // Démarre la temporisation à l'état bas en mode non bloquant
        }
        else {t1DebutTempo_EtatHaut = millis();}                                                                                // Démarre la temporisation à l'état haut en mode non bloquant
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Initialise la temporisation à l'état haut en mode non bloquant
        
        etatLeds <<= 1;                                                                                                         // Décale l'état courant des LED à gauche de 1 position
        if ((etatLeds & MaskLeds) == 0) // .................................................................................... // Si l'état courant des LED a été décalé 6 fois à gauche de 1 position
        {
          etatLeds = 0b10000000;                                                                                                // Définit l'état initial des LED
          Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                           // Inverse l'indicateur du sens de décalage du PORTD
          t1DebutTempo_EtatBas = 0ul;                                                                                           // Initialise la temporisation à l'état bas en mode non bloquant
          t1DebutTempo_EtatHaut = millis();                                                                                     // Démarre la temporisation à l'état haut en mode non bloquant
        }
        else {t1DebutTempo_EtatBas = millis();}                                                                                 // Démarre la temporisation à l'état bas en mode non bloquant
      }
    }
    else if (Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de droite à gauche
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatBas = 0ul;                                                                                             // Initialise la temporisation à l'état bas en mode non bloquant
        
        etatLeds >>= 1;                                                                                                         // Décale l'état courant des LED à droite de 1 position
        if ((etatLeds & MaskLeds) == 0) // .................................................................................... // Si l'état courant des LED a été décalé 6 fois à droite de 1 position
        {
          etatLeds = 0b10000000;                                                                                                // Définit l'état initial des LED
          t1DebutTempo_EtatHaut = 0ul;                                                                                          // Initialise la temporisation à l'état bas en mode non bloquant
          t1DebutTempo_EtatBas = millis();                                                                                      // Démarre la temporisation à l'état bas en mode non bloquant
        }
        else {t1DebutTempo_EtatHaut = millis();}                                                                                // Démarre la temporisation à l'état haut en mode non bloquant
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Initialise la temporisation à l'état haut en mode non bloquant
        
        etatLeds >>= 1;                                                                                                         // Décale l'état courant des LED à droite de 1 position
        if ((etatLeds & MaskLeds) == 0) // .................................................................................... // Si l'état courant des LED a été décalé 6 fois à droite de 1 position
        {
          etatLeds = 0b00000100;                                                                                                // Définit l'état initial des LED
          Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                           // Inverse l'indicateur du sens de décalage du PORTD
          
          compteurAffichageModeCourant++;                                                                                       // Incrémente le compteur d'affichages du mode courant en mode automatique
          
          t1DebutTempo_EtatBas = 0ul;                                                                                           // Initialise la temporisation à l'état bas en mode non bloquant
          t1DebutTempo_EtatHaut = millis();                                                                                     // Démarre la temporisation à l'état haut en mode non bloquant
        }
        else {t1DebutTempo_EtatBas = millis();}                                                                                 // Démarre la temporisation à l'état bas en mode non bloquant
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 6 LED remplissage de gauche à droite et vidage de droite à gauche et remplissage de droite à gauche et vidage de gauche à droite non bloquant *************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 34) // ********************************************************************************************** // Si le mode courant "34" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      MaskLeds = 0b11111100;                                                                                                    // Définit le masque des LED pour déterminer la LED allumée
      EtatInitial = 0b00000100;                                                                                                 // Définit l'état initial
      etatLeds = EtatInitial;                                                                                                   // Définit l'état courant des LED égal à l'état initial des LED
      Sens_decalage_PORTD = false;                                                                                              // Définit l'indicateur du sens de décalage du PORTD
      Mute = false; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit l'indicateur d'activation du buzzer
      DureeTempo_EtatBas = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état bas
      DureeTempo_EtatHaut = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état haut
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Initialise la temporisation à l'état bas en mode non bloquant
      t1DebutTempo_EtatHaut = millis();                                                                                         // Démarre la temporisation à l'état haut en mode non bloquant
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (!Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de gauche à droite
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        if (Mute) {Buzzer(1, 0, 1);} // ....................................................................................... // Active le buzzer 1ms
        
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatBas = 0ul;                                                                                             // Initialise la temporisation à l'état bas en mode non bloquant
        
        etatLeds <<= 1;                                                                                                         // Décale l'état courant des LED à gauche de 1 position
        if ((etatLeds & MaskLeds) == 0) // .................................................................................... // Si l'état courant des LED a été décalé 6 fois à gauche de 1 position
        {
          etatLeds = 0b10000000;                                                                                                // Définit l'état initial des LED
          t1DebutTempo_EtatHaut = 0ul;                                                                                          // Initialise la temporisation à l'état haut en mode non bloquant
          t1DebutTempo_EtatBas = millis();                                                                                      // Démarre la temporisation à l'état bas en mode non bloquant
        }
        else {t1DebutTempo_EtatHaut = millis();}                                                                                // Démarre la temporisation à l'état haut en mode non bloquant
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Initialise la temporisation à l'état haut en mode non bloquant
        
        etatLeds >>= 1;                                                                                                         // Décale l'état courant des LED à droite de 1 position
        if ((etatLeds & MaskLeds) == 0) // .................................................................................... // Si l'état courant des LED a été décalé 6 fois à droite de 1 position
        {
          etatLeds = 0b10000000;                                                                                                // Définit l'état initial des LED
          Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                           // Inverse l'indicateur du sens de décalage
          t1DebutTempo_EtatBas = 0ul;                                                                                           // Initialise la temporisation à l'état bas en mode non bloquant
          t1DebutTempo_EtatHaut = millis();                                                                                     // Démarre la temporisation à l'état haut en mode non bloquant
        }
        else {t1DebutTempo_EtatBas = millis();}                                                                                 // Démarre la temporisation à l'état bas en mode non bloquant
      }
    }
    else if (Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de droite à gauche
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        if (Mute) {Buzzer(1, 0, 1);} // ....................................................................................... // Active le buzzer 1ms
        
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatBas = 0ul;                                                                                             // Initialise la temporisation à l'état bas en mode non bloquant
        
        etatLeds >>= 1;                                                                                                         // Décale l'état courant des LED à droite de 1 position
        if ((etatLeds & MaskLeds) == 0) // .................................................................................... // Si l'état courant des LED a été décalé 5 fois à droite de 1 position
        {
          etatLeds = 0b00000100;                                                                                                // Définit l'état initial des LED
          t1DebutTempo_EtatHaut = 0ul;                                                                                          // Initialise la temporisation à l'état haut en mode non bloquant
          t1DebutTempo_EtatBas = millis();                                                                                      // Démarre la temporisation à l'état bas en mode non bloquant
        }
        else {t1DebutTempo_EtatHaut = millis();}                                                                                // Démarre la temporisation à l'état haut en mode non bloquant
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint la LED courante
        
        PORTB &= 0b11000000;                                                                                                    // Réinitialise le PORTB sans modifier les bits "6" et "7"
        PORTB |= PORTD >> 2;                                                                                                    // Calcule la valeur du PORTB sans modifier les bits "6" et "7"
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Initialise la temporisation à l'état haut en mode non bloquant
        
        etatLeds <<= 1;                                                                                                         // Décale l'état courant des LED à gauche de 1 position
        if ((etatLeds & MaskLeds) == 0) // .................................................................................... // Si l'état courant des LED a été décalé 5 fois à gauche de 1 position
        {
          etatLeds = 0b00000100;                                                                                                // Définit l'état initial des LED
          Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                           // Inverse l'indicateur du sens de décalage
          
          compteurAffichageModeCourant++;                                                                                       // Incrémente le compteur d'affichages du mode courant en mode automatique
          
          t1DebutTempo_EtatBas = 0ul;                                                                                           // Initialise la temporisation à l'état bas en mode non bloquant
          t1DebutTempo_EtatHaut = millis();                                                                                     // Démarre la temporisation à l'état haut en mode non bloquant
        }
        else {t1DebutTempo_EtatBas = millis();}                                                                                 // Démarre la temporisation à l'état bas en mode non bloquant
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
  
  if (ModeAuto && ModeCourant != 10) // *************************************************************************************** // Si l'indicateur du mode automatique est activé et si le mode courant est différent de "10"
  {
    compteurAffichageModeCourant = 0;                                                                                           // Initialise le compteur d'affichages du mode courant en mode automatique
  }
  
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
