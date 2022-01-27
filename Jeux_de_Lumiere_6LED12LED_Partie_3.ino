//**************************************************************************************************************************************************************************
//*********************************************** Jeux de Lumière 6 LED 12 LED - Partie 3 - Carte Nano (com49) *************************************************************
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
int DernierModeCourant = -1;                                                                                                    // Dernier mode courant
volatile const int NombreModesMax = 18; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de modes maximum

volatile bool SortieModeCourant = false;                                                                                        // Indicateur de sortie du mode courant

volatile bool ModeAuto = false;                                                                                                 // Indicateur du mode automatique
volatile long compteurAffichageModeCourant = 0;                                                                                 // Compteur d'affichages du mode courant en mode automatique
const long NombreAffichageModeCourant = 3;                                                                                      // Nombre d'affichages du mode courant en mode automatique
volatile long MultipleNombreAffichageModeCourant = 1;                                                                           // Multiple du nombre d'affichages du mode courant en mode automatique
bool AffichageModeManuel = true;                                                                                                // Indicateur d'affichage du mode manuel sur l'écran OLED
bool AffichageModeAuto = false;                                                                                                 // Indicateur d'affichage du mode auto sur l'écran OLED

volatile const unsigned long DureeAntiRebond = 5ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de l'anti-rebonds des boutons poussoirs "BPModePlus" et "BPModeMoins" en ms

unsigned long t1DebutTempo_EtatHaut;                                                                                            // Début de la temporisation à l'état haut en mode non bloquant
unsigned long t1DebutTempo_EtatBas;                                                                                             // Début de la temporisation à l'état bas en mode non bloquant
unsigned long DureeTempo_EtatBas;                                                                                               // Durée de la temporisation à l'état bas en mode non bloquant
unsigned long DureeTempo_EtatHaut;                                                                                              // Durée de la temporisation à l'état haut en mode non bloquant
int DeltaTempo_EtatHaut;                                                                                                        // Incrément ou décrément de la valeur de la temporisation à l'état haut en mode non bloquant
int DeltaTempo_EtatBas;                                                                                                         // Incrément ou décrément de la valeur de la temporisation à l'état bas en mode non bloquant

int PointeurTableauBrochesLED;                                                                                                  // Pointeur des tableaux des broches de 6 LED
const int TableauBrochesLED_D2D7 [] = {2, 3, 4, 5, 6, 7}; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des broches des 6 LED de (D2 à D7)
const int TableauBrochesLED_D8D13 [] = {8, 9, 10, 11, 12, 13}; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des broches des 6 LED de (D8 à D13)
int PointeurTableauBroches12LED;                                                                                                // Pointeur du tableau des broches 12 LED
int PointeurTableauBroches12LEDTemp;                                                                                            // Pointeur de fin temporaire du tableau des broches 12 LED
const int TableauBroches12LED [] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des broches des 12 LED
int compteurOffsetPointeurTableauBroches12LED;                                                                                  // Compteur de décalage du pointeur du tableau des broches 12 LED
int OffsetPointeurTableauBroches12LED;                                                                                          // Valeur du décalage du pointeur du tableau des broches 12 LED
int CompteurPointeurTableauBroches12LEDMIN;                                                                                     // Valeur min du pointeur du tableau des broches 12 LED

const int Nombre_Sequences = 16; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage
const byte Tableau_Sequenceur [Nombre_Sequences] = {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0}; // >>>>>>>>>>>>>>>>>>>>>>> // Tableau de séquences d'affichage
byte pointeur_TableauSequenceur;                                                                                                // Pointeur du tableau de séquences d'affichage
unsigned long Duree_EntreSequences;                                                                                             // Temporisation entre les séquences d'affichage

unsigned long t1DebutTempoMultipleFlash_ON;                                                                                     // Début de la temporisation des multiples flashs ON
unsigned long t1DebutTempoMultipleFlash_OFF;                                                                                    // Début de la temporisation entre les multiples flashs
unsigned long DureeTempoMultipleFlash_ON;                                                                                       // Durée de la temporisation des multiples flashs ON
unsigned long DureeTempoMultipleFlash_OFF;                                                                                      // Durée de la temporisation entre les multiples flashs
int NombreFlashs;                                                                                                               // Valeur du nombre de flashs
int CompteurFlashs;                                                                                                             // Compteur de flashs

unsigned long DureeHIGH_LOW;                                                                                                    // Temporisation de la LED activée ou désactivée

uint8_t TableauTemporaire [12];                                                                                                 // Tableau temporaire de motifs
uint8_t TableauGaucheON [] = {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0};                                                              // Tableau du motif de gauche
uint8_t TableauDroiteON [] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1 ,1};                                                              // Tableau du motif de droite
unsigned long Tempo1;                                                                                                           // Temporisation des flashs
unsigned long Tempo2;                                                                                                           // Temporisation entre les motifs

int pointeurTableauSequencesOctets2;                                                                                            // Pointeur du tableau "TableauSequencesOctets2"
const byte TableauSequencesOctets2 [8] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des séquences d'affichage des LED 2
  0b01010100,
  0b00010101,
  0b10101000,
  0b00101010,
  0b11001100,
  0b00001100,
  0b00110000,
  0b00110011
};

const byte NombreLED = 6; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de LED
const byte NombreSequencesLED = 22; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
int SequenceCouranteLED = 0;                                                                                                    // Séquence courante des LED affichées
const bool TableauSequencesLED [NombreSequencesLED][NombreLED] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des séquences d'affichage des LED
  {1, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 0, 1, 0, 0, 0},
  {0, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 1, 0}, {0, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0},
  {1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0},
  {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 1, 1}, {0, 0, 0, 0, 0, 0},
  {1, 0, 0, 0, 0, 1}, {0, 1, 0, 0, 1, 0}, {0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 0, 0},
  {1, 0, 1, 0, 1, 0}, {0, 1, 0, 1, 0, 1}, {1, 0, 1, 0, 1, 0},
  {0, 0, 0, 0, 0, 0},
  {1, 1, 1, 1, 1, 1}
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
//*** Chenillard 12 LED simple aller de 2 x 6 LED en partant du centre *****************************************************************************************************
//**************************************************************************************************************************************************************************
  if (ModeCourant == 0) // **************************************************************************************************** // Si le mode courant "0" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 5;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBrochesLED = 0;                                                                                            // Initialise le pointeur du tableau des broches LED
      DureeHIGH_LOW = 150ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], HIGH);                                            // Allume la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 + PointeurTableauBrochesLED], HIGH);                                                // Allume la LED de rang "3 + PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED - 1], HIGH);                                           // Allume la LED de rang "3 - PointeurTableauBrochesLED_D8D13 - 1" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 + PointeurTableauBrochesLED], HIGH);                                               // Allume la LED de rang "3 + PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], LOW);                                             // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 + PointeurTableauBrochesLED], LOW);                                                 // Eteint la LED de rang "3 + PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED - 1], LOW);                                            // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D8D13 - 1" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 + PointeurTableauBrochesLED], LOW);                                                // Eteint la LED de rang "3 + PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED flashs simulation police, ambulance, pompier ********************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 1) // *********************************************************************************************** // Si le mode courant "1" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 5;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      Tempo1 = 300; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation des flashs
      Tempo2 = 30; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation entre les motifs
      NombreFlashs = 4; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit le nombre de flashs pour chaque motif
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    memcpy(TableauTemporaire, TableauGaucheON, sizeof(TableauTemporaire));                                                      // Copie le tableau du motif de gauche dans le tableau temporaire de motifs
    
    for (int compteurFlashs = 1; compteurFlashs <= NombreFlashs; compteurFlashs++) // +++++++++++++++++++++++++++++++++++++++++ // Boucle en fonction du nombres de flashs
    {
      for (int compteurNombreLED = 0; compteurNombreLED < NombreLED * 2; compteurNombreLED++) // ------------------------------ // Parcourt le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[compteurNombreLED], TableauTemporaire[compteurNombreLED]);                             // Allume ou éteint la LED de rang "compteurNombreLED"
      }
      
      Fonction_Temporisation(Tempo2);                                                                                           // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (int compteurNombreLED = 0; compteurNombreLED < NombreLED * 2; compteurNombreLED++) // ------------------------------ // Parcourt le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[compteurNombreLED], LOW);                                                              // Eteint toutes les LED
      }
      
      Fonction_Temporisation(Tempo2);                                                                                           // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(Tempo2);                                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    memcpy(TableauTemporaire, TableauDroiteON, sizeof(TableauTemporaire));                                                      // Copie le tableau du motif de droite dans le tableau temporaire de motifs
    
    for (int compteurFlashs = 1; compteurFlashs <= NombreFlashs; compteurFlashs++) // +++++++++++++++++++++++++++++++++++++++++ // Boucle en fonction du nombres de flashs
    {
      for (int compteurNombreLED = 0; compteurNombreLED < NombreLED * 2; compteurNombreLED++) // ------------------------------ // Parcourt le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[compteurNombreLED], TableauTemporaire[compteurNombreLED]);                             // Allume ou éteint la LED de rang "compteurNombreLED"
      }
      
      Fonction_Temporisation(Tempo2);                                                                                           // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (int compteurNombreLED = 0; compteurNombreLED < NombreLED * 2; compteurNombreLED++) // ------------------------------ // Parcourt le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[compteurNombreLED], LOW);                                                              // Eteint toutes les LED
      }
      
      Fonction_Temporisation(Tempo2);                                                                                           // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(Tempo2);                                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED aller-retour de 2 x 6 LED en partant du centre *****************************************************************************************************
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
      PointeurTableauBrochesLED = 0;                                                                                            // Initialise le pointeur du tableau des broches LED
      DureeHIGH_LOW = 150ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], HIGH);                                            // Allume la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 + PointeurTableauBrochesLED], HIGH);                                                // Allume la LED de rang "3 + PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED - 1], HIGH);                                           // Allume la LED de rang "3 - PointeurTableauBrochesLED_D8D13 - 1" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 + PointeurTableauBrochesLED], HIGH);                                               // Allume la LED de rang "3 + PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], LOW);                                             // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 + PointeurTableauBrochesLED], LOW);                                                 // Eteint la LED de rang "3 + PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED - 1], LOW);                                            // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D8D13 - 1" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 + PointeurTableauBrochesLED], LOW);                                                // Eteint la LED de rang "3 + PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], HIGH);                                                    // Allume la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED + 2], HIGH);                                            // Allume la LED de rang "3 - PointeurTableauBrochesLED_D2D7 + 2" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED + 2], HIGH);                                           // Allume la LED de rang "3 - PointeurTableauBrochesLED_D8D13 + 2" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], LOW);                                                     // Eteint la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED + 2], LOW);                                             // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D2D7 + 2" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED + 2], LOW);                                            // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D8D13 + 2" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED aller-retour de 2 x 6 LED en partant du centre inversé *********************************************************************************************
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
      PointeurTableauBrochesLED = 0;                                                                                            // Initialise le pointeur du tableau des broches LED
      DureeHIGH_LOW = 150ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], HIGH);                                            // Allume la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 + PointeurTableauBrochesLED], HIGH);                                                // Allume la LED de rang "3 + PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED - 1], LOW);                                            // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D8D13 - 1" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 + PointeurTableauBrochesLED], LOW);                                                // Eteint la LED de rang "3 + PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], LOW);                                             // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 + PointeurTableauBrochesLED], LOW);                                                 // Eteint la LED de rang "3 + PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED - 1], HIGH);                                           // Allume la LED de rang "3 - PointeurTableauBrochesLED_D8D13 - 1" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 + PointeurTableauBrochesLED], HIGH);                                               // Allume la LED de rang "3 + PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], HIGH);                                                    // Allume la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED + 2], HIGH);                                            // Allume la LED de rang "3 - PointeurTableauBrochesLED_D2D7 + 2" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED + 2], LOW);                                            // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D8D13 + 2" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], LOW);                                                     // Eteint la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED + 2], LOW);                                             // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D2D7 + 2" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED + 2], HIGH);                                           // Allume la LED de rang "3 - PointeurTableauBrochesLED_D8D13 + 2" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED aller-retour de 2 x 6 LED en partant du centre inversé *********************************************************************************************
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
      PointeurTableauBrochesLED = 0;                                                                                            // Initialise le pointeur du tableau des broches LED
      DureeHIGH_LOW = 150ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], HIGH);                                            // Allume la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED + 2], HIGH);                                            // Allume la LED de rang "3 - PointeurTableauBrochesLED_D2D7 + 2" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED - 1], HIGH);                                           // Allume la LED de rang "3 - PointeurTableauBrochesLED_D8D13 - 1" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED + 2], HIGH);                                           // Allume la LED de rang "3 - PointeurTableauBrochesLED_D8D13 + 2" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], LOW);                                             // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 + PointeurTableauBrochesLED], LOW);                                                 // Eteint la LED de rang "3 + PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED - 1], LOW);                                            // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D8D13 - 1" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 + PointeurTableauBrochesLED], LOW);                                                // Eteint la LED de rang "3 + PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], HIGH);                                                    // Allume la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 + PointeurTableauBrochesLED], HIGH);                                                // Allume la LED de rang "3 + PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 + PointeurTableauBrochesLED], HIGH);                                               // Allume la LED de rang "3 + PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], LOW);                                                     // Eteint la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED + 2], LOW);                                             // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D2D7 + 2" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED + 2], LOW);                                            // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D8D13 + 2" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED 4 flashs alternés *******************************+++++++++++++++++++++++++++++**********************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 5) // *********************************************************************************************** // Si le mode courant "5" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 5;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBrochesLED = 0;                                                                                            // Initialise le pointeur du tableau des broches LED
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], HIGH);                                            // Allume la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], LOW);                                             // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[3 + PointeurTableauBrochesLED], HIGH);                                                // Allume la LED de rang "3 + PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D2D7[3 + PointeurTableauBrochesLED], LOW);                                                 // Eteint la LED de rang "3 + PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED + 2], HIGH);                                           // Allume la LED de rang "3 - PointeurTableauBrochesLED_D8D13 + 2" du tableau des broches des 6 LED de (D8 à D13)
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED + 2], LOW);                                            // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D8D13 + 2" du tableau des broches des 6 LED de (D8 à D13)
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED 4 multi-flashs alternés *************************+++++++++++++++++++++++++++++**********************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 6) // *********************************************************************************************** // Si le mode courant "6" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 5;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBrochesLED = 0;                                                                                            // Initialise le pointeur du tableau des broches LED
      NombreFlashs = 3;                                                                                                         // Définit la valeur du nombre de flashs
      CompteurFlashs = 0;                                                                                                       // Initialise le compteur de flashs
      DureeTempo_EtatBas = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la durée de la temporisation à l'état bas
      DureeTempo_EtatHaut = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit le durée de la temporisation à l'état haut
      Duree_EntreSequences = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation entre les séquences d'affichage
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (CompteurFlashs = 0; CompteurFlashs < NombreFlashs; CompteurFlashs++) // ++++++++++++++++++++++++++++++++++++++++++++++ // Boucle en fonction du nombre de flashs
    {
      for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // --------------------- // Parcourt le tableau des broches de droite à gauche
      {
        digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], HIGH);                                          // Allume la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
      }
      
      Fonction_Temporisation(DureeTempo_EtatHaut);                                                                              // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // --------------------- // Parcourt le tableau des broches de droite à gauche
      {
        digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], LOW);                                           // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
      }
      
      Fonction_Temporisation(DureeTempo_EtatBas);                                                                               // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(Duree_EntreSequences);                                                                               // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (CompteurFlashs = 0; CompteurFlashs < NombreFlashs; CompteurFlashs++) // ++++++++++++++++++++++++++++++++++++++++++++++ // Boucle en fonction du nombre de flashs
    {
      for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // --------------------- // Parcourt le tableau des broches de droite à gauche
      {
        digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], HIGH);                                                 // Allume la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      }
      
      Fonction_Temporisation(DureeTempo_EtatHaut);                                                                              // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // --------------------- // Parcourt le tableau des broches de droite à gauche
      {
        digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], LOW);                                                  // Eteint la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      }
      
      Fonction_Temporisation(DureeTempo_EtatBas);                                                                               // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(Duree_EntreSequences);                                                                               // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (CompteurFlashs = 0; CompteurFlashs < NombreFlashs; CompteurFlashs++) // ++++++++++++++++++++++++++++++++++++++++++++++ // Boucle en fonction du nombre de flashs
    {
      for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // --------------------- // Parcourt le tableau des broches de droite à gauche
      {
        digitalWrite(TableauBrochesLED_D2D7[3 + PointeurTableauBrochesLED], HIGH);                                              // Allume la LED de rang "3 + PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      }
      
      Fonction_Temporisation(DureeTempo_EtatHaut);                                                                              // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // --------------------- // Parcourt le tableau des broches de droite à gauche
      {
        digitalWrite(TableauBrochesLED_D2D7[3 + PointeurTableauBrochesLED], LOW);                                               // Eteint la LED de rang "3 + PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      }
      
      Fonction_Temporisation(DureeTempo_EtatBas);                                                                               // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(Duree_EntreSequences);                                                                               // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (CompteurFlashs = 0; CompteurFlashs < NombreFlashs; CompteurFlashs++) // ++++++++++++++++++++++++++++++++++++++++++++++ // Boucle en fonction du nombre de flashs
    {
      for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // --------------------- // Parcourt le tableau des broches de droite à gauche
      {
        digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED + 2], HIGH);                                         // Allume la LED de rang "3 - PointeurTableauBrochesLED_D8D13 + 2" du tableau des broches des 6 LED de (D8 à D13)
      }
      
      Fonction_Temporisation(DureeTempo_EtatHaut);                                                                              // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // --------------------- // Parcourt le tableau des broches de droite à gauche
      {
        digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED + 2], LOW);                                          // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D8D13 + 2" du tableau des broches des 6 LED de (D8 à D13)
      }
      
      Fonction_Temporisation(DureeTempo_EtatBas);                                                                               // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    Fonction_Temporisation(Duree_EntreSequences);                                                                               // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée 1 *****************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 7) // *********************************************************************************************** // Si le mode courant "7" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 1;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      pointeurTableauSequencesOctets2 = 0;                                                                                      // Initialise le pointeur du tableau "TableauSequencesOctets2"
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    pointeurTableauSequencesOctets2 = 0;                                                                                        // Initialise le pointeur du tableau "TableauSequencesOctets2"
    for (int compteurEffet = 0; compteurEffet < 40; compteurEffet++) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 40 fois
    {
      PORTD = TableauSequencesOctets2[pointeurTableauSequencesOctets2];                                                         // Transfère la valeur du tableau TableauSequencesOctets2 de rang "pointeurTableauSequencesOctets2" vers le PORTD
      PORTB = TableauSequencesOctets2[pointeurTableauSequencesOctets2 + 1];                                                     // Transfère la valeur du tableau TableauSequencesOctets2 de rang "pointeurTableauSequencesOctets2 + 1" vers le PORTB
      
      pointeurTableauSequencesOctets2 += 2;                                                                                     // Incrémente le pointeur du tableau "TableauSequencesOctets2"
      if (pointeurTableauSequencesOctets2 > 2) // ----------------------------------------------------------------------------- // Borne le pointeur du tableau "TableauSequencesOctets2"
      {
        pointeurTableauSequencesOctets2 = 0;                                                                                    // Réinitialise le pointeur du tableau "TableauSequencesOctets2"
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    pointeurTableauSequencesOctets2 = 4;                                                                                        // Initialise le pointeur du tableau "TableauSequencesOctets2"
    for (int compteurEffet = 0; compteurEffet < 40; compteurEffet++) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 40 fois
    {
      PORTD = TableauSequencesOctets2[pointeurTableauSequencesOctets2];                                                         // Transfère la valeur du tableau TableauSequencesOctets2 de rang "pointeurTableauSequencesOctets2" vers le PORTD
      PORTB = TableauSequencesOctets2[pointeurTableauSequencesOctets2 + 1];                                                     // Transfère la valeur du tableau TableauSequencesOctets2 de rang "pointeurTableauSequencesOctets2 + 1" vers le PORTB
      
      pointeurTableauSequencesOctets2 += 2;                                                                                     // Incrémente le pointeur du tableau "TableauSequencesOctets2"
      if (pointeurTableauSequencesOctets2 > 6) // ----------------------------------------------------------------------------- // Borne le pointeur du tableau "TableauSequencesOctets2"
      {
        pointeurTableauSequencesOctets2 = 4;                                                                                    // Réinitialise le pointeur du tableau "TableauSequencesOctets2"
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
//*** Chenillard 12 LED aller-retour avec incrémentation du nombre de LED **************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 8) // *********************************************************************************************** // Si le mode courant "8" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 1;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= -1; PointeurTableauBroches12LED--) // +++++++++++++++ // Parcourt le tableau des broches de gauche à droite
    {
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], LOW);}
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 14; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED > 0 && PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);}
      if (PointeurTableauBroches12LED > 1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], LOW);}
      if (PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], LOW);}
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= -3; PointeurTableauBroches12LED--) // +++++++++++++++ // Parcourt le tableau des broches de gauche à droite
    {
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], HIGH);}
      if (PointeurTableauBroches12LED < 9) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], LOW);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], LOW);}
      if (PointeurTableauBroches12LED < 9) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], LOW);}
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 17; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED > 0 && PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);}
      if (PointeurTableauBroches12LED > 1 && PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], HIGH);}
      if (PointeurTableauBroches12LED > 2 && PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], HIGH);}
      if (PointeurTableauBroches12LED > 3 && PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], LOW);}
      if (PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], LOW);}
      if (PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], LOW);}
      if (PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], LOW);}
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= -5; PointeurTableauBroches12LED--) // +++++++++++++++ // Parcourt le tableau des broches de gauche à droite
    {
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], HIGH);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], HIGH);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], HIGH);}
      if (PointeurTableauBroches12LED < 7) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], LOW);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], LOW);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], LOW);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], LOW);}
      if (PointeurTableauBroches12LED < 7) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], LOW);}
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 19; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED > 0 && PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);}
      if (PointeurTableauBroches12LED > 1 && PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], HIGH);}
      if (PointeurTableauBroches12LED > 2 && PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], HIGH);}
      if (PointeurTableauBroches12LED > 3 && PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], HIGH);}
      if (PointeurTableauBroches12LED > 4 && PointeurTableauBroches12LED < 17) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 5], HIGH);}
      if (PointeurTableauBroches12LED > 5 && PointeurTableauBroches12LED < 18) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 6], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], LOW);}
      if (PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], LOW);}
      if (PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], LOW);}
      if (PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], LOW);}
      if (PointeurTableauBroches12LED < 17) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 5], LOW);}
      if (PointeurTableauBroches12LED < 18) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 6], LOW);}
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= -7; PointeurTableauBroches12LED--) // +++++++++++++++ // Parcourt le tableau des broches de gauche à droite
    {
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], HIGH);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], HIGH);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], HIGH);}
      if (PointeurTableauBroches12LED < 7 && PointeurTableauBroches12LED > -6)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], HIGH);}
      if (PointeurTableauBroches12LED < 6 && PointeurTableauBroches12LED > -7)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 6], HIGH);}
      if (PointeurTableauBroches12LED < 5) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 7], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], LOW);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], LOW);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], LOW);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], LOW);}
      if (PointeurTableauBroches12LED < 7 && PointeurTableauBroches12LED > -6)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], LOW);}
      if (PointeurTableauBroches12LED < 6 && PointeurTableauBroches12LED > -7)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 6], LOW);}
      if (PointeurTableauBroches12LED < 5) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 7], LOW);}
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 21; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED > 0 && PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);}
      if (PointeurTableauBroches12LED > 1 && PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], HIGH);}
      if (PointeurTableauBroches12LED > 2 && PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], HIGH);}
      if (PointeurTableauBroches12LED > 3 && PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], HIGH);}
      if (PointeurTableauBroches12LED > 4 && PointeurTableauBroches12LED < 17) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 5], HIGH);}
      if (PointeurTableauBroches12LED > 5 && PointeurTableauBroches12LED < 18) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 6], HIGH);}
      if (PointeurTableauBroches12LED > 6 && PointeurTableauBroches12LED < 19) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 7], HIGH);}
      if (PointeurTableauBroches12LED > 7 && PointeurTableauBroches12LED < 20) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 8], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], LOW);}
      if (PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], LOW);}
      if (PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], LOW);}
      if (PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], LOW);}
      if (PointeurTableauBroches12LED < 17) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 5], LOW);}
      if (PointeurTableauBroches12LED < 18) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 6], LOW);}
      if (PointeurTableauBroches12LED < 19) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 7], LOW);}
      if (PointeurTableauBroches12LED < 20) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 8], LOW);}
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= -9; PointeurTableauBroches12LED--) // +++++++++++++++ // Parcourt le tableau des broches de gauche à droite
    {
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], HIGH);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], HIGH);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], HIGH);}
      if (PointeurTableauBroches12LED < 7 && PointeurTableauBroches12LED > -6)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], HIGH);}
      if (PointeurTableauBroches12LED < 6 && PointeurTableauBroches12LED > -7)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 6], HIGH);}
      if (PointeurTableauBroches12LED < 5 && PointeurTableauBroches12LED > -8)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 7], HIGH);}
      if (PointeurTableauBroches12LED < 4 && PointeurTableauBroches12LED > -9)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 8], HIGH);}
      if (PointeurTableauBroches12LED < 3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 9], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], LOW);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], LOW);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], LOW);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], LOW);}
      if (PointeurTableauBroches12LED < 7 && PointeurTableauBroches12LED > -6)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], LOW);}
      if (PointeurTableauBroches12LED < 6 && PointeurTableauBroches12LED > -7)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 6], LOW);}
      if (PointeurTableauBroches12LED < 5 && PointeurTableauBroches12LED > -8)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 7], LOW);}
      if (PointeurTableauBroches12LED < 4 && PointeurTableauBroches12LED > -9)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 8], LOW);}
      if (PointeurTableauBroches12LED < 3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 9], LOW);}
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 23; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED > 0 && PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);}
      if (PointeurTableauBroches12LED > 1 && PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], HIGH);}
      if (PointeurTableauBroches12LED > 2 && PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], HIGH);}
      if (PointeurTableauBroches12LED > 3 && PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], HIGH);}
      if (PointeurTableauBroches12LED > 4 && PointeurTableauBroches12LED < 17) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 5], HIGH);}
      if (PointeurTableauBroches12LED > 5 && PointeurTableauBroches12LED < 18) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 6], HIGH);}
      if (PointeurTableauBroches12LED > 6 && PointeurTableauBroches12LED < 19) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 7], HIGH);}
      if (PointeurTableauBroches12LED > 7 && PointeurTableauBroches12LED < 20) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 8], HIGH);}
      if (PointeurTableauBroches12LED > 8 && PointeurTableauBroches12LED < 21) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 9], HIGH);}
      if (PointeurTableauBroches12LED > 9 && PointeurTableauBroches12LED < 22) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 10], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], LOW);}
      if (PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], LOW);}
      if (PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], LOW);}
      if (PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], LOW);}
      if (PointeurTableauBroches12LED < 17) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 5], LOW);}
      if (PointeurTableauBroches12LED < 18) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 6], LOW);}
      if (PointeurTableauBroches12LED < 19) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 7], LOW);}
      if (PointeurTableauBroches12LED < 20) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 8], LOW);}
      if (PointeurTableauBroches12LED < 21) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 9], LOW);}
      if (PointeurTableauBroches12LED < 22) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 10], LOW);}
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= -11; PointeurTableauBroches12LED--) // +++++++++++++++ // Parcourt le tableau des broches de gauche à droite
    {
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}              // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], HIGH);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], HIGH);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], HIGH);}
      if (PointeurTableauBroches12LED < 7 && PointeurTableauBroches12LED > -6)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], HIGH);}
      if (PointeurTableauBroches12LED < 6 && PointeurTableauBroches12LED > -7)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 6], HIGH);}
      if (PointeurTableauBroches12LED < 5 && PointeurTableauBroches12LED > -8)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 7], HIGH);}
      if (PointeurTableauBroches12LED < 4 && PointeurTableauBroches12LED > -9)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 8], HIGH);}
      if (PointeurTableauBroches12LED < 3 && PointeurTableauBroches12LED > -10) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 9], HIGH);}
      if (PointeurTableauBroches12LED < 2 && PointeurTableauBroches12LED > -11) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 10], HIGH);}
      if (PointeurTableauBroches12LED < 1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 11], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], LOW);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], LOW);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], LOW);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], LOW);}
      if (PointeurTableauBroches12LED < 7 && PointeurTableauBroches12LED > -6)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], LOW);}
      if (PointeurTableauBroches12LED < 6 && PointeurTableauBroches12LED > -7)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 6], LOW);}
      if (PointeurTableauBroches12LED < 5 && PointeurTableauBroches12LED > -8)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 7], LOW);}
      if (PointeurTableauBroches12LED < 4 && PointeurTableauBroches12LED > -9)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 8], LOW);}
      if (PointeurTableauBroches12LED < 3 && PointeurTableauBroches12LED > -10) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 9], LOW);}
      if (PointeurTableauBroches12LED < 2 && PointeurTableauBroches12LED > -11) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 10], LOW);}
      if (PointeurTableauBroches12LED < 1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 11], LOW);}
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED effet ressort avec modification de la temporisation *************************************************************************************************
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
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de gauche à droite
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < sizeof(TableauBroches12LED) / sizeof(int); PointeurTableauBroches12LED++)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      if (PointeurTableauBroches12LED > 7) {DureeHIGH_LOW = 120ul;} // -------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est supérieur à "7"
      else {DureeHIGH_LOW = 50ul;} // ----------------------------------------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est inférieur ou égal à "7"
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de droite à gauche
    for (PointeurTableauBroches12LED = sizeof(TableauBroches12LED) / sizeof(int); PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      if (PointeurTableauBroches12LED < 4) {DureeHIGH_LOW = 120ul;} // -------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est inférieur à "4"
      else {DureeHIGH_LOW = 50ul;} // ----------------------------------------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est supérieur ou égal à "4"
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de droite à gauche
    for (PointeurTableauBroches12LED = sizeof(TableauBroches12LED) / sizeof(int); PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      if (PointeurTableauBroches12LED < 4) {DureeHIGH_LOW = 120ul;} // -------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est inférieur à "4"
      else {DureeHIGH_LOW = 50ul;} // ----------------------------------------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est supérieur ou égal à "4"
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de gauche à droite
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < sizeof(TableauBroches12LED) / sizeof(int); PointeurTableauBroches12LED++)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      if (PointeurTableauBroches12LED > 7) {DureeHIGH_LOW = 120ul;} // -------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est supérieur à "7"
      else {DureeHIGH_LOW = 50ul;} // ----------------------------------------------------------------------------------------- // Si le pointeur du tableau des broches 12 LED est inférieur ou égal à "7"
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED alternées ******************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 10) // ********************************************************************************************** // Si le mode courant "10" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 10;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de gauche à droite
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < sizeof(TableauBroches12LED) / sizeof(int); PointeurTableauBroches12LED = PointeurTableauBroches12LED + 2)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED + 1" du tableau des broches des 12 LED
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de gauche à droite
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < sizeof(TableauBroches12LED) / sizeof(int); PointeurTableauBroches12LED = PointeurTableauBroches12LED + 2)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);                                                 // Allume la LED de rang "PointeurTableauBroches12LED + 1" du tableau des broches des 12 LED
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED alternées 1 LED sur 2 ******************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 11) // ********************************************************************************************** // Si le mode courant "11" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de gauche à droite
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= sizeof(TableauBroches12LED) / sizeof(int); PointeurTableauBroches12LED++)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PointeurTableauBroches12LED = PointeurTableauBroches12LED + 1;                                                            // Incrémente le pointeur du tableau des broches 12 LED
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de gauche à droite
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= sizeof(TableauBroches12LED) / sizeof(int); PointeurTableauBroches12LED++)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de droite à gauche
    for (PointeurTableauBroches12LED = sizeof(TableauBroches12LED) / sizeof(int) + 1; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PointeurTableauBroches12LED = PointeurTableauBroches12LED - 1;                                                            // Décrémente le pointeur du tableau des broches 12 LED
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de droite à gauche
    for (PointeurTableauBroches12LED = sizeof(TableauBroches12LED) / sizeof(int); PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED aller-retour ***************************************************************************************************************************************
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
      DureeHIGH_LOW = 60ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de gauche à droite
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= sizeof(TableauBroches12LED) / sizeof(int); PointeurTableauBroches12LED++)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de droite à gauche
    for (PointeurTableauBroches12LED = sizeof(TableauBroches12LED) / sizeof(int) - 2; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED clignotement 3 LED aller-retour ********************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 13) // ********************************************************************************************** // Si le mode courant "13" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED > -1; PointeurTableauBroches12LED -= 3) // +++++++++++++ // Parcourt le tableau des broches des 12 LED de droite à gauche
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 0], HIGH);                                                 // Allume la LED de rang "PointeurTableauBroches12LED - 0" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);                                                 // Allume la LED de rang "PointeurTableauBroches12LED - 1" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], HIGH);                                                 // Allume la LED de rang "PointeurTableauBroches12LED - 2" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED - 2" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED - 1" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 0], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED - 0" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED += 3) // ++++++++++++++ // Parcourt le tableau des broches des 12 LED de gauche à droite
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 0], HIGH);                                                 // Allume la LED de rang "PointeurTableauBroches12LED + 0" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);                                                 // Allume la LED de rang "PointeurTableauBroches12LED + 1" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], HIGH);                                                 // Allume la LED de rang "PointeurTableauBroches12LED + 2" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED + 2" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED + 1" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 0], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED + 0" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED effet aller-retour allumage 3 LED par 3 LED dans un sens effacement dans l'autre sens avec un pas de 3 *********************************************
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
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= 2; PointeurTableauBroches12LED -= 3) // +++++++++++++ // Parcourt le tableau des broches des 12 LED de droite à gauche
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Compte la valeur du décalage du pointeur du tableau des broches 12 LED
      for (compteurOffsetPointeurTableauBroches12LED = 0; compteurOffsetPointeurTableauBroches12LED < 3; compteurOffsetPointeurTableauBroches12LED++)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - compteurOffsetPointeurTableauBroches12LED], HIGH);       // Allume la LED de rang "PointeurTableauBroches12LED - compteurOffsetPointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Compte la valeur du décalage du pointeur du tableau des broches 12 LED
      for (compteurOffsetPointeurTableauBroches12LED = 2; compteurOffsetPointeurTableauBroches12LED >= 0; compteurOffsetPointeurTableauBroches12LED--)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - compteurOffsetPointeurTableauBroches12LED], LOW);        // Eteint la LED de rang "PointeurTableauBroches12LED - compteurOffsetPointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= 9; PointeurTableauBroches12LED += 3) // ++++++++++++++ // Parcourt le tableau des broches des 12 LED de gauche à droite
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Compte la valeur du décalage du pointeur du tableau des broches 12 LED
      for (compteurOffsetPointeurTableauBroches12LED = 0; compteurOffsetPointeurTableauBroches12LED < 3; compteurOffsetPointeurTableauBroches12LED++)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + compteurOffsetPointeurTableauBroches12LED], HIGH);       // Allume la LED de rang "PointeurTableauBroches12LED - compteurOffsetPointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Compte la valeur du décalage du pointeur du tableau des broches 12 LED
      for (compteurOffsetPointeurTableauBroches12LED = 2; compteurOffsetPointeurTableauBroches12LED >= 0; compteurOffsetPointeurTableauBroches12LED--)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + compteurOffsetPointeurTableauBroches12LED], LOW);        // Eteint la LED de rang "PointeurTableauBroches12LED - compteurOffsetPointeurTableauBroches12LED" du tableau des broches des 12 LED
        
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
//*** Chenillard 12 LED effet aller-retour allumage 3 LED par 3 LED dans un sens effacement dans l'autre sens avec un pas de 1 *********************************************
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
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 45ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= 2; PointeurTableauBroches12LED -= 1) // +++++++++++++ // Parcourt le tableau des broches des 12 LED de droite à gauche
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Compte la valeur du décalage du pointeur du tableau des broches 12 LED
      for (compteurOffsetPointeurTableauBroches12LED = 0; compteurOffsetPointeurTableauBroches12LED < 3; compteurOffsetPointeurTableauBroches12LED++)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - compteurOffsetPointeurTableauBroches12LED], HIGH);       // Allume la LED de rang "PointeurTableauBroches12LED - compteurOffsetPointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Compte la valeur du décalage du pointeur du tableau des broches 12 LED
      for (compteurOffsetPointeurTableauBroches12LED = 2; compteurOffsetPointeurTableauBroches12LED >= 0; compteurOffsetPointeurTableauBroches12LED--)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - compteurOffsetPointeurTableauBroches12LED], LOW);        // Eteint la LED de rang "PointeurTableauBroches12LED - compteurOffsetPointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= 9; PointeurTableauBroches12LED += 1) // ++++++++++++++ // Parcourt le tableau des broches des 12 LED de gauche à droite
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Compte la valeur du décalage du pointeur du tableau des broches 12 LED
      for (compteurOffsetPointeurTableauBroches12LED = 0; compteurOffsetPointeurTableauBroches12LED < 3; compteurOffsetPointeurTableauBroches12LED++)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + compteurOffsetPointeurTableauBroches12LED], HIGH);       // Allume la LED de rang "PointeurTableauBroches12LED - compteurOffsetPointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Compte la valeur du décalage du pointeur du tableau des broches 12 LED
      for (compteurOffsetPointeurTableauBroches12LED = 2; compteurOffsetPointeurTableauBroches12LED >= 0; compteurOffsetPointeurTableauBroches12LED--)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + compteurOffsetPointeurTableauBroches12LED], LOW);        // Eteint la LED de rang "PointeurTableauBroches12LED - compteurOffsetPointeurTableauBroches12LED" du tableau des broches des 12 LED
        
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
//*** Chenillard 12 LED simultané déplacement vers le centre avec décrémentation d'une LED à chaque boucle *****************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 16) // ********************************************************************************************** // Si le mode courant "16" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 10;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      compteurOffsetPointeurTableauBroches12LED = 5;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 45ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de gauche à droite
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 1)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], HIGH);                                             // Allume la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de gauche à droite
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 1)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], LOW);                                              // Eteint la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    compteurOffsetPointeurTableauBroches12LED--;                                                                                // Décrémente le compteur de décalage du pointeur du tableau des broches 12 LED
    if (compteurOffsetPointeurTableauBroches12LED < 0) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le compteur de décalage du pointeur du tableau des broches 12 LED est inférieur à "0"
    {
      compteurOffsetPointeurTableauBroches12LED = 5;                                                                            // Réinitialise le compteur de décalage du pointeur du tableau des broches 12 LED
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED simultané déplacement vers l'extérieur avec décrémentation d'une LED à chaque boucle ***************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 17) // ********************************************************************************************** // Si le mode courant "17" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 10;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 45ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de droite à gauche
    for (PointeurTableauBroches12LED = 5; PointeurTableauBroches12LED >= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED -= 1)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], HIGH);                                             // Allume la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED de droite à gauche
    for (PointeurTableauBroches12LED = 5; PointeurTableauBroches12LED >= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED -= 1)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], LOW);                                              // Eteint la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    compteurOffsetPointeurTableauBroches12LED++;                                                                                // Incrémente le compteur de décalage du pointeur du tableau des broches 12 LED
    if (compteurOffsetPointeurTableauBroches12LED > 5) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le compteur de décalage du pointeur du tableau des broches 12 LED est supérieur à "5"
    {
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Réinitialise le compteur de décalage du pointeur du tableau des broches 12 LED
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED aller-retour avec incrémentation du nombre de LED **************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 18) // ********************************************************************************************** // Si le mode courant "18" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 1;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= -1; PointeurTableauBroches12LED--) // +++++++++++++++ // Parcourt le tableau des broches de gauche à droite
    {
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], LOW);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 14; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED > 0 && PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);}
      if (PointeurTableauBroches12LED > 1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], LOW);}
      if (PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], LOW);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= -3; PointeurTableauBroches12LED--) // +++++++++++++++ // Parcourt le tableau des broches de gauche à droite
    {
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], HIGH);}
      if (PointeurTableauBroches12LED < 9) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], LOW);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], LOW);}
      if (PointeurTableauBroches12LED < 9) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], LOW);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 17; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED > 0 && PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);}
      if (PointeurTableauBroches12LED > 1 && PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], HIGH);}
      if (PointeurTableauBroches12LED > 2 && PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], HIGH);}
      if (PointeurTableauBroches12LED > 3 && PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], LOW);}
      if (PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], LOW);}
      if (PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], LOW);}
      if (PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], LOW);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= -5; PointeurTableauBroches12LED--) // +++++++++++++++ // Parcourt le tableau des broches de gauche à droite
    {
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], HIGH);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], HIGH);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], HIGH);}
      if (PointeurTableauBroches12LED < 7) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], LOW);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], LOW);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], LOW);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], LOW);}
      if (PointeurTableauBroches12LED < 7) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], LOW);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 19; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED > 0 && PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);}
      if (PointeurTableauBroches12LED > 1 && PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], HIGH);}
      if (PointeurTableauBroches12LED > 2 && PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], HIGH);}
      if (PointeurTableauBroches12LED > 3 && PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], HIGH);}
      if (PointeurTableauBroches12LED > 4 && PointeurTableauBroches12LED < 17) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 5], HIGH);}
      if (PointeurTableauBroches12LED > 5 && PointeurTableauBroches12LED < 18) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 6], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], LOW);}
      if (PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], LOW);}
      if (PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], LOW);}
      if (PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], LOW);}
      if (PointeurTableauBroches12LED < 17) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 5], LOW);}
      if (PointeurTableauBroches12LED < 18) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 6], LOW);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= -7; PointeurTableauBroches12LED--) // +++++++++++++++ // Parcourt le tableau des broches de gauche à droite
    {
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], HIGH);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], HIGH);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], HIGH);}
      if (PointeurTableauBroches12LED < 7 && PointeurTableauBroches12LED > -6)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], HIGH);}
      if (PointeurTableauBroches12LED < 6 && PointeurTableauBroches12LED > -7)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 6], HIGH);}
      if (PointeurTableauBroches12LED < 5) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 7], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], LOW);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], LOW);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], LOW);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], LOW);}
      if (PointeurTableauBroches12LED < 7 && PointeurTableauBroches12LED > -6)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], LOW);}
      if (PointeurTableauBroches12LED < 6 && PointeurTableauBroches12LED > -7)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 6], LOW);}
      if (PointeurTableauBroches12LED < 5) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 7], LOW);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 21; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED > 0 && PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);}
      if (PointeurTableauBroches12LED > 1 && PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], HIGH);}
      if (PointeurTableauBroches12LED > 2 && PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], HIGH);}
      if (PointeurTableauBroches12LED > 3 && PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], HIGH);}
      if (PointeurTableauBroches12LED > 4 && PointeurTableauBroches12LED < 17) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 5], HIGH);}
      if (PointeurTableauBroches12LED > 5 && PointeurTableauBroches12LED < 18) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 6], HIGH);}
      if (PointeurTableauBroches12LED > 6 && PointeurTableauBroches12LED < 19) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 7], HIGH);}
      if (PointeurTableauBroches12LED > 7 && PointeurTableauBroches12LED < 20) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 8], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], LOW);}
      if (PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], LOW);}
      if (PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], LOW);}
      if (PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], LOW);}
      if (PointeurTableauBroches12LED < 17) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 5], LOW);}
      if (PointeurTableauBroches12LED < 18) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 6], LOW);}
      if (PointeurTableauBroches12LED < 19) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 7], LOW);}
      if (PointeurTableauBroches12LED < 20) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 8], LOW);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= -9; PointeurTableauBroches12LED--) // +++++++++++++++ // Parcourt le tableau des broches de gauche à droite
    {
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], HIGH);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], HIGH);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], HIGH);}
      if (PointeurTableauBroches12LED < 7 && PointeurTableauBroches12LED > -6)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], HIGH);}
      if (PointeurTableauBroches12LED < 6 && PointeurTableauBroches12LED > -7)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 6], HIGH);}
      if (PointeurTableauBroches12LED < 5 && PointeurTableauBroches12LED > -8)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 7], HIGH);}
      if (PointeurTableauBroches12LED < 4 && PointeurTableauBroches12LED > -9)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 8], HIGH);}
      if (PointeurTableauBroches12LED < 3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 9], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], LOW);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], LOW);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], LOW);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], LOW);}
      if (PointeurTableauBroches12LED < 7 && PointeurTableauBroches12LED > -6)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], LOW);}
      if (PointeurTableauBroches12LED < 6 && PointeurTableauBroches12LED > -7)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 6], LOW);}
      if (PointeurTableauBroches12LED < 5 && PointeurTableauBroches12LED > -8)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 7], LOW);}
      if (PointeurTableauBroches12LED < 4 && PointeurTableauBroches12LED > -9)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 8], LOW);}
      if (PointeurTableauBroches12LED < 3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 9], LOW);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 23; PointeurTableauBroches12LED++) // +++++++++++++++++ // Parcourt le tableau des broches de droite à gauche
    {
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}             // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED > 0 && PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);}
      if (PointeurTableauBroches12LED > 1 && PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], HIGH);}
      if (PointeurTableauBroches12LED > 2 && PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], HIGH);}
      if (PointeurTableauBroches12LED > 3 && PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], HIGH);}
      if (PointeurTableauBroches12LED > 4 && PointeurTableauBroches12LED < 17) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 5], HIGH);}
      if (PointeurTableauBroches12LED > 5 && PointeurTableauBroches12LED < 18) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 6], HIGH);}
      if (PointeurTableauBroches12LED > 6 && PointeurTableauBroches12LED < 19) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 7], HIGH);}
      if (PointeurTableauBroches12LED > 7 && PointeurTableauBroches12LED < 20) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 8], HIGH);}
      if (PointeurTableauBroches12LED > 8 && PointeurTableauBroches12LED < 21) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 9], HIGH);}
      if (PointeurTableauBroches12LED > 9 && PointeurTableauBroches12LED < 22) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 10], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED < 12) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 13) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], LOW);}
      if (PointeurTableauBroches12LED < 14) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], LOW);}
      if (PointeurTableauBroches12LED < 15) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], LOW);}
      if (PointeurTableauBroches12LED < 16) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 4], LOW);}
      if (PointeurTableauBroches12LED < 17) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 5], LOW);}
      if (PointeurTableauBroches12LED < 18) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 6], LOW);}
      if (PointeurTableauBroches12LED < 19) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 7], LOW);}
      if (PointeurTableauBroches12LED < 20) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 8], LOW);}
      if (PointeurTableauBroches12LED < 21) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 9], LOW);}
      if (PointeurTableauBroches12LED < 22) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 10], LOW);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= -11; PointeurTableauBroches12LED--) // +++++++++++++++ // Parcourt le tableau des broches de gauche à droite
    {
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);}              // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], HIGH);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], HIGH);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], HIGH);}
      if (PointeurTableauBroches12LED < 7 && PointeurTableauBroches12LED > -6)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], HIGH);}
      if (PointeurTableauBroches12LED < 6 && PointeurTableauBroches12LED > -7)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 6], HIGH);}
      if (PointeurTableauBroches12LED < 5 && PointeurTableauBroches12LED > -8)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 7], HIGH);}
      if (PointeurTableauBroches12LED < 4 && PointeurTableauBroches12LED > -9)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 8], HIGH);}
      if (PointeurTableauBroches12LED < 3 && PointeurTableauBroches12LED > -10) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 9], HIGH);}
      if (PointeurTableauBroches12LED < 2 && PointeurTableauBroches12LED > -11) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 10], HIGH);}
      if (PointeurTableauBroches12LED < 1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 11], HIGH);}
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (PointeurTableauBroches12LED > -1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);}              // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches 12 LED
      if (PointeurTableauBroches12LED < 11 && PointeurTableauBroches12LED > -2) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], LOW);}
      if (PointeurTableauBroches12LED < 10 && PointeurTableauBroches12LED > -3) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], LOW);}
      if (PointeurTableauBroches12LED < 9 && PointeurTableauBroches12LED > -4)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 3], LOW);}
      if (PointeurTableauBroches12LED < 8 && PointeurTableauBroches12LED > -5)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 4], LOW);}
      if (PointeurTableauBroches12LED < 7 && PointeurTableauBroches12LED > -6)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5], LOW);}
      if (PointeurTableauBroches12LED < 6 && PointeurTableauBroches12LED > -7)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 6], LOW);}
      if (PointeurTableauBroches12LED < 5 && PointeurTableauBroches12LED > -8)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 7], LOW);}
      if (PointeurTableauBroches12LED < 4 && PointeurTableauBroches12LED > -9)  {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 8], LOW);}
      if (PointeurTableauBroches12LED < 3 && PointeurTableauBroches12LED > -10) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 9], LOW);}
      if (PointeurTableauBroches12LED < 2 && PointeurTableauBroches12LED > -11) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 10], LOW);}
      if (PointeurTableauBroches12LED < 1) {digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 11], LOW);}
      
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
