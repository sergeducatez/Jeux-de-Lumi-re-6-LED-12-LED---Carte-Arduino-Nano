//**************************************************************************************************************************************************************************
//*********************************************** Jeux de Lumière 6 LED 12 LED - Partie 4a - Carte Nano (com49) ************************************************************
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
volatile const int NombreModesMax = 30; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de modes maximum

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
unsigned long DureeTempo_EtatBas;                                                                                               // Durée de la temporisation à l'état haut en mode non bloquant
unsigned long DureeTempo_EtatHaut;                                                                                              // Durée de la temporisation à l'état bas en mode non bloquant
int DeltaTempo_EtatHaut;                                                                                                        // Incrément ou décrément de la valeur de la temporisation à l'état haut en mode non bloquant
int DeltaTempo_EtatBas;                                                                                                         // Incrément ou décrément de la valeur de la temporisation à l'état bas en mode non bloquant
unsigned long OffsetDureeTempo_EtatHaut;                                                                                        // Offset de la temporisation à l'état haut en mode non bloquant

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

const byte NombreLED = 12; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de LED
const byte NombreSequencesLED = 23; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
int SequenceCouranteLED = 0;                                                                                                    // Séquence courante des LED affichées
const bool TableauSequencesLED [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des séquences d'affichage des LED
   1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, // 0
   0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, // 1
   0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, // 2
   0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, // 3
   0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, // 4
   1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, // 5
   0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, // 6
   0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, // 7
   0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, // 8
   0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, // 9
   0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, // 10
   1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, // 11
   0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, // 12
   0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, // 13
   0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, // 14
   0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, // 15
   1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, // 16
   0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, // 17
   0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, // 18
   0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, // 19
   0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, // 20
   0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, // 21
   1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1  // 22
};

const byte NombreSequencesLED1 = 24; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool TableauSequencesLED1 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 1 des séquences d'affichage des LED 
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

const byte NombreSequencesLED2 = 24; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage des LED
const bool TableauSequencesLED2 [] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau 2 des séquences d'affichage des LED
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
//*** Chenillard 12 LED simultané déplacement dans les deux sens avec décrémentation d'une LED sur 2 à chaque boucle *******************************************************
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
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 10ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 12 fois
    for (compteurOffsetPointeurTableauBroches12LED = 11; compteurOffsetPointeurTableauBroches12LED >= 0; compteurOffsetPointeurTableauBroches12LED -= 1)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 12 fois
    for (compteurOffsetPointeurTableauBroches12LED = 0; compteurOffsetPointeurTableauBroches12LED <= 11; compteurOffsetPointeurTableauBroches12LED += 1)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 12 fois
    for (compteurOffsetPointeurTableauBroches12LED = 11; compteurOffsetPointeurTableauBroches12LED >= 0; compteurOffsetPointeurTableauBroches12LED -= 1)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[12 - PointeurTableauBroches12LED - 1], HIGH);                                          // Allume la LED de rang "12 - PointeurTableauBroches12LED - 1" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[12 - PointeurTableauBroches12LED - 1], LOW);                                           // Eteint la LED de rang "12 - PointeurTableauBroches12LED - 1" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    for (compteurOffsetPointeurTableauBroches12LED = 0; compteurOffsetPointeurTableauBroches12LED <= 11; compteurOffsetPointeurTableauBroches12LED += 1)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[12 - PointeurTableauBroches12LED - 1], HIGH);                                          // Allume la LED de rang "12 - PointeurTableauBroches12LED - 1" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[12 - PointeurTableauBroches12LED - 1], LOW);                                           // Eteint la LED de rang "12 - PointeurTableauBroches12LED - 1" du tableau des broches des 12 LED
        
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
//*** Chenillard 12 LED simultané déplacement vers le centre avec décrémentation d'une LED sur 2 à chaque boucle ***********************************************************
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
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 6 fois
    for (compteurOffsetPointeurTableauBroches12LED = 5; compteurOffsetPointeurTableauBroches12LED >= 0; compteurOffsetPointeurTableauBroches12LED -= 1)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], HIGH);                                           // Allume la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], LOW);                                            // Eteint la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
        
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
//*** Chenillard 12 LED simultané déplacement vers la droite avec décrémentation d'une LED sur 2 à chaque boucle ***********************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 2) // *********************************************************************************************** // Si le mode courant "2" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 12;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED], HIGH);                                                 // Allume la LED de rang "6 + PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED], LOW);                                                  // Eteint la LED de rang "6 + PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
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
//*** Chenillard 12 LED simultané déplacement vers la gauche avec décrémentation d'une LED sur 2 à chaque boucle ***********************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 3) // *********************************************************************************************** // Si le mode courant "3" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 12;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    for (PointeurTableauBroches12LED = 5; PointeurTableauBroches12LED >= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED -= 2)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED], HIGH);                                                 // Allume la LED de rang "6 + PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    for (PointeurTableauBroches12LED = 5; PointeurTableauBroches12LED >= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED -= 2)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED], LOW);                                                  // Eteint la LED de rang "6 + PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
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
//*** Chenillard 12 LED simultané déplacement vers la gauche avec incrémentation d'une LED sur 2 à chaque boucle ***********************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 4) // *********************************************************************************************** // Si le mode courant "4" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 12;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    for (PointeurTableauBroches12LED = 5; PointeurTableauBroches12LED >= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED -= 2)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED], LOW);                                                  // Eteint la LED de rang "6 + PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    for (PointeurTableauBroches12LED = 5; PointeurTableauBroches12LED >= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED -= 2)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED], HIGH);                                                 // Allume la LED de rang "6 + PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
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
//*** Chenillard 12 LED simultané déplacement vers le centre avec incrémentation d'une LED sur 2 à chaque boucle ***********************************************************
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
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 6 fois
    for (compteurOffsetPointeurTableauBroches12LED = 5; compteurOffsetPointeurTableauBroches12LED >= 0; compteurOffsetPointeurTableauBroches12LED -= 1)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], LOW);                                            // Eteint la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], HIGH);                                           // Allume la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
        
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
//*** Chenillard 12 LED simultané déplacement de 2 x 3 LED vers la droite avec décrémentation d'une LED sur 2 à chaque boucle **********************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 6) // *********************************************************************************************** // Si le mode courant "6" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 12;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 120ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    for (PointeurTableauBroches12LED = 3; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], HIGH);                                                 // Allume la LED de rang "PointeurTableauBroches12LED - 3" du tableau des broches des 12 
      
      if (6 + PointeurTableauBroches12LED < 12) // ---------------------------------------------------------------------------- // Si "6 + PointeurTableauBroches12LED" est inférieur à "11"
      {
        digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED], HIGH);                                               // Allume la LED de rang "6 + PointeurTableauBroches12LED" du tableau des broches des 12 LED
      }
      
      digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED - 3], HIGH);                                             // Allume la LED de rang "6 + PointeurTableauBroches12LED - 3" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    for (PointeurTableauBroches12LED = 3; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 3], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED - 3" du tableau des broches des 12 LED
      
      if (6 + PointeurTableauBroches12LED < 12) // ---------------------------------------------------------------------------- // Si "6 + PointeurTableauBroches12LED" est inférieur à "11"
      {
        digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED], LOW);                                                // Eteint la LED de rang "6 - PointeurTableauBroches12LED" du tableau des broches des 12 LED
      }
      
      digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED - 3], LOW);                                              // Eteint la LED de rang "6 - PointeurTableauBroches12LED - 3" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    compteurOffsetPointeurTableauBroches12LED--;                                                                                // Décrémente le compteur de décalage du pointeur du tableau des broches 12 LED
    if (compteurOffsetPointeurTableauBroches12LED < 3) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le compteur de décalage du pointeur du tableau des broches 12 LED est inférieur à "3"
    {
      compteurOffsetPointeurTableauBroches12LED = 5;                                                                            // Réinitialise le compteur de décalage du pointeur du tableau des broches 12 LED
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED simultané déplacement de 2 x 3 LED vers le centre avec décrémentation d'une LED à chaque boucle ****************************************************
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
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 3 fois
    for (compteurOffsetPointeurTableauBroches12LED = 2; compteurOffsetPointeurTableauBroches12LED >= 0; compteurOffsetPointeurTableauBroches12LED -= 1)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 1)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[3 - PointeurTableauBroches12LED + 2], HIGH);                                           // Allume la LED de rang "3 - PointeurTableauBroches12LED + 2" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], HIGH);                                           // Allume la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED], LOW);                                                // Eteint la LED de rang "6 + PointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 1)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[3 - PointeurTableauBroches12LED + 2], LOW);                                            // Eteint la LED de rang "3 - PointeurTableauBroches12LED + 2" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], LOW);                                            // Eteint la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED], HIGH);                                               // Allume la LED de rang "6 + PointeurTableauBroches12LED" du tableau des broches des 12 LED
        
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
//*** Chenillard 12 LED simultané déplacement 2 x 3 LED vers l'extérieur centre avec décrémentation d'une LED à chaque boucle **********************************************
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
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 3 fois
    for (compteurOffsetPointeurTableauBroches12LED = 2; compteurOffsetPointeurTableauBroches12LED >= 0; compteurOffsetPointeurTableauBroches12LED -= 1)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED -= 1)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[3 - PointeurTableauBroches12LED + 2], HIGH);                                           // Allume la LED de rang "3 - PointeurTableauBroches12LED + 2" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], HIGH);                                           // Allume la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED], LOW);                                                // Eteint la LED de rang "6 + PointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED -= 1)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[3 - PointeurTableauBroches12LED + 2], LOW);                                            // Eteint la LED de rang "3 - PointeurTableauBroches12LED + 2" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], LOW);                                            // Eteint la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED], HIGH);                                               // Allume la LED de rang "6 + PointeurTableauBroches12LED" du tableau des broches des 12 LED
        
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
//*** Chenillard 12 LED simultané déplacement dans les deux sens sans décrémentation de LED ********************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 9) // *********************************************************************************************** // Si le mode courant "9" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 1;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 20ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 12 fois
    for (compteurOffsetPointeurTableauBroches12LED = 11; compteurOffsetPointeurTableauBroches12LED >= 0; compteurOffsetPointeurTableauBroches12LED -= 1)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 1)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 12 fois
    for (compteurOffsetPointeurTableauBroches12LED = 0; compteurOffsetPointeurTableauBroches12LED <= 11; compteurOffsetPointeurTableauBroches12LED += 1)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 1)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 12 fois
    for (compteurOffsetPointeurTableauBroches12LED = 11; compteurOffsetPointeurTableauBroches12LED >= 0; compteurOffsetPointeurTableauBroches12LED -= 1)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 1)
      {
        digitalWrite(TableauBroches12LED[12 - PointeurTableauBroches12LED - 1], LOW);                                           // Eteint la LED de rang "12 - PointeurTableauBroches12LED - 1" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[12 - PointeurTableauBroches12LED - 1], HIGH);                                          // Allume la LED de rang "12 - PointeurTableauBroches12LED - 1" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    for (compteurOffsetPointeurTableauBroches12LED = 0; compteurOffsetPointeurTableauBroches12LED <= 11; compteurOffsetPointeurTableauBroches12LED += 1)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 2)
      {
        digitalWrite(TableauBroches12LED[12 - PointeurTableauBroches12LED - 1], LOW);                                           // Eteint la LED de rang "12 - PointeurTableauBroches12LED - 1" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 1)
      {
        digitalWrite(TableauBroches12LED[12 - PointeurTableauBroches12LED - 1], HIGH);                                          // Allume la LED de rang "12 - PointeurTableauBroches12LED - 1" du tableau des broches des 12 LED
        
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
//*** Chenillard 12 LED effet aller-retour allumage 3 LED par 3 LED dans un sens effacement dans l'autre sens avec un pas de 3 *********************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 10) // ********************************************************************************************** // Si le mode courant "10" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 1;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= 2; PointeurTableauBroches12LED -= 3) // +++++++++++++ // Parcourt le tableau des broches des 12 LED
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
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= 9; PointeurTableauBroches12LED += 3) // ++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Compte la valeur du décalage du pointeur du tableau des broches 12 LED
      for (compteurOffsetPointeurTableauBroches12LED = 0; compteurOffsetPointeurTableauBroches12LED < 3; compteurOffsetPointeurTableauBroches12LED++)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + compteurOffsetPointeurTableauBroches12LED], LOW);        // Eteint la LED de rang "PointeurTableauBroches12LED - compteurOffsetPointeurTableauBroches12LED" du tableau des broches des 12 LED
        
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
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + compteurOffsetPointeurTableauBroches12LED], HIGH);       // Allume la LED de rang "PointeurTableauBroches12LED - compteurOffsetPointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    for (PointeurTableauBroches12LED = 2; PointeurTableauBroches12LED <= 11; PointeurTableauBroches12LED += 3) // +++++++++++++ // Parcourt le tableau des broches des 12 LED
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
    
    for (PointeurTableauBroches12LED = 9; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED -= 3) // ++++++++++++++ // Parcourt le tableau des broches des 12 LED
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
//*** Chenillard 12 LED simultané déplacement vers le centre avec incrémentation d'une LED à chaque boucle *****************************************************************
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
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 6 fois
    for (compteurOffsetPointeurTableauBroches12LED = 5; compteurOffsetPointeurTableauBroches12LED >= 0; compteurOffsetPointeurTableauBroches12LED -= 1)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 1)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], LOW);                                            // Eteint la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 1)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], HIGH);                                           // Allume la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
        
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
//*** Chenillard 12 LED simultané déplacement 2 x 3 LED vers l'extérieur centre avec incrémentation d'une LED à chaque boucle **********************************************
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
      compteurOffsetPointeurTableauBroches12LED = 0;                                                                            // Initialise le compteur de décalage du pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 3 fois
    for (compteurOffsetPointeurTableauBroches12LED = 2; compteurOffsetPointeurTableauBroches12LED >= 0; compteurOffsetPointeurTableauBroches12LED -= 1)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED -= 1)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[3 - PointeurTableauBroches12LED + 2], HIGH);                                           // Allume la LED de rang "3 - PointeurTableauBroches12LED + 2" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], LOW);                                            // Eteint la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED], LOW);                                                // Eteint la LED de rang "6 + PointeurTableauBroches12LED" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED -= 1)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[3 - PointeurTableauBroches12LED + 2], LOW);                                            // Eteint la LED de rang "3 - PointeurTableauBroches12LED + 2" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], HIGH);                                           // Allume la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 + PointeurTableauBroches12LED], HIGH);                                               // Allume la LED de rang "6 + PointeurTableauBroches12LED" du tableau des broches des 12 LED
        
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
//*** Chenillard 12 LED aller-retour de 2 x 6 LED en partant du centre et aller-retour 1 LED *******************************************************************************
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
      PointeurTableauBrochesLED = 0;                                                                                            // Initialise le pointeur du tableau des broches LED
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], LOW);                                             // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 + PointeurTableauBrochesLED], LOW);                                                 // Eteint la LED de rang "3 + PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED - 1], HIGH);                                           // Allume la LED de rang "3 - PointeurTableauBrochesLED_D8D13 - 1" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 + PointeurTableauBrochesLED], LOW);                                                // Eteint la LED de rang "3 + PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], HIGH);                                            // Allume la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 + PointeurTableauBrochesLED], HIGH);                                                // Allume la LED de rang "3 + PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED - 1], LOW);                                            // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D8D13 - 1" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 + PointeurTableauBrochesLED], HIGH);                                               // Allume la LED de rang "3 + PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], HIGH);                                                    // Allume la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
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
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], LOW);                                                     // Eteint la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
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
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 11; PointeurTableauBrochesLED++) // ++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      if (6 - PointeurTableauBrochesLED + 5 < 6) // --------------------------------------------------------------------------- // Si "6 - PointeurTableauBrochesLED + 5" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[6 - PointeurTableauBrochesLED + 5], HIGH);                                          // Allume la LED de rang "6 - PointeurTableauBrochesLED_D2D7 + 5" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED + 5], HIGH);                                         // Allume la LED de rang "6 - PointeurTableauBrochesLED_D8D13 + 5" du tableau des broches des 6 LED de (D8 à D13)
      }
      if (PointeurTableauBrochesLED < 6) // ----------------------------------------------------------------------------------- // Si "PointeurTableauBrochesLED" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], HIGH);                                                  // Allume la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], HIGH);                                                 // Allume la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (6 - PointeurTableauBrochesLED + 5 < 6) // --------------------------------------------------------------------------- // Si "6 - PointeurTableauBrochesLED + 5" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[6 - PointeurTableauBrochesLED + 5], LOW);                                           // Eteint la LED de rang "6 - PointeurTableauBrochesLED_D2D7 + 5" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED + 5], LOW);                                          // Eteint la LED de rang "6 - PointeurTableauBrochesLED_D8D13 + 5" du tableau des broches des 6 LED de (D8 à D13)
      }
      if (PointeurTableauBrochesLED < 6) // ----------------------------------------------------------------------------------- // Si "PointeurTableauBrochesLED" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], LOW);                                                   // Eteint la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], LOW);                                                  // Eteint la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      }
    }
    
    for (PointeurTableauBrochesLED = 11; PointeurTableauBrochesLED >= 0; PointeurTableauBrochesLED--) // ++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      if (6 - PointeurTableauBrochesLED + 5 < 6) // --------------------------------------------------------------------------- // Si "6 - PointeurTableauBrochesLED + 5" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[6 - PointeurTableauBrochesLED + 5], HIGH);                                          // Allume la LED de rang "6 - PointeurTableauBrochesLED_D2D7 + 5" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED + 5], HIGH);                                         // Allume la LED de rang "6 - PointeurTableauBrochesLED_D8D13 + 5" du tableau des broches des 6 LED de (D8 à D13)
      }
      if (PointeurTableauBrochesLED < 6) // ----------------------------------------------------------------------------------- // Si "PointeurTableauBrochesLED" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], HIGH);                                                  // Allume la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], HIGH);                                                 // Allume la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (6 - PointeurTableauBrochesLED + 5 < 6) // --------------------------------------------------------------------------- // Si "6 - PointeurTableauBrochesLED + 5" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[6 - PointeurTableauBrochesLED + 5], LOW);                                           // Eteint la LED de rang "6 - PointeurTableauBrochesLED_D2D7 + 5" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED + 5], LOW);                                          // Eteint la LED de rang "6 - PointeurTableauBrochesLED_D8D13 + 5" du tableau des broches des 6 LED de (D8 à D13)
      }
      if (PointeurTableauBrochesLED < 6) // ----------------------------------------------------------------------------------- // Si "PointeurTableauBrochesLED" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], LOW);                                                   // Eteint la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], LOW);                                                  // Eteint la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED aller-retour de 2 x 6 LED en partant de droite et de gauche et aller-retour 1 LED ******************************************************************
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
      PointeurTableauBrochesLED = 0;                                                                                            // Initialise le pointeur du tableau des broches LED
      DureeHIGH_LOW = 60ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }

    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], HIGH);                                                    // Allume la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED + 2], LOW);                                             // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D2D7 + 2" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED + 2], LOW);                                            // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D8D13 + 2" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                           // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], LOW);                                                     // Eteint la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED + 2], HIGH);                                            // Allume la LED de rang "3 - PointeurTableauBrochesLED_D2D7 + 2" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED + 2], HIGH);                                           // Allume la LED de rang "3 - PointeurTableauBrochesLED_D8D13 + 2" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                           // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 11; PointeurTableauBrochesLED++) // ++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      if (6 - PointeurTableauBrochesLED + 5 < 6) // --------------------------------------------------------------------------- // Si "6 - PointeurTableauBrochesLED + 5" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[6 - PointeurTableauBrochesLED + 5], LOW);                                           // Eteint la LED de rang "6 - PointeurTableauBrochesLED_D2D7 + 5" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED + 5], HIGH);                                         // Allume la LED de rang "6 - PointeurTableauBrochesLED_D8D13 + 5" du tableau des broches des 6 LED de (D8 à D13)
      }
      if (PointeurTableauBrochesLED < 6) // ----------------------------------------------------------------------------------- // Si "PointeurTableauBrochesLED" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], HIGH);                                                  // Allume la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], LOW);                                                  // Eteint la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (6 - PointeurTableauBrochesLED + 5 < 6) // --------------------------------------------------------------------------- // Si "6 - PointeurTableauBrochesLED + 5" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[6 - PointeurTableauBrochesLED + 5], HIGH);                                          // Allume la LED de rang "6 - PointeurTableauBrochesLED_D2D7 + 5" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED + 5], LOW);                                          // Eteint la LED de rang "6 - PointeurTableauBrochesLED + 5" du tableau des broches des 6 LED de (D8 à D13)
      }
      if (PointeurTableauBrochesLED < 6) // ----------------------------------------------------------------------------------- // Si "PointeurTableauBrochesLED" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], LOW);                                                   // Eteint la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], HIGH);                                                 // Allume la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      }
    }
    
    PORTD &= 0b00000011;                                                                                                        // Eteint toutes les LED du PORTD
    PORTB &= 0b11000000;                                                                                                        // Eteint toutes les LED du PORTB
    
    for (PointeurTableauBrochesLED = 11; PointeurTableauBrochesLED >= 0; PointeurTableauBrochesLED--) // ++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      if (6 - PointeurTableauBrochesLED + 5 < 6) // --------------------------------------------------------------------------- // Si "6 - PointeurTableauBrochesLED + 5" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[6 - PointeurTableauBrochesLED + 5], HIGH);                                          // Allume la LED de rang "6 - PointeurTableauBrochesLED_D2D7 + 5" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED + 5], LOW);                                          // Eteint la LED de rang "6 - PointeurTableauBrochesLED_D8D13 + 5" du tableau des broches des 6 LED de (D8 à D13)
      }
      if (PointeurTableauBrochesLED < 6) // ----------------------------------------------------------------------------------- // Si "PointeurTableauBrochesLED" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], HIGH);                                                  // Allume la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], LOW);                                                  // Eteint la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      if (6 - PointeurTableauBrochesLED + 5 < 6) // --------------------------------------------------------------------------- // Si "6 - PointeurTableauBrochesLED + 5" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[6 - PointeurTableauBrochesLED + 5], LOW);                                           // Eteint la LED de rang "6 - PointeurTableauBrochesLED_D2D7 + 5" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED + 5], HIGH);                                         // Allume la LED de rang "6 - PointeurTableauBrochesLED_D8D13 + 5" du tableau des broches des 6 LED de (D8 à D13)
      }
      if (PointeurTableauBrochesLED < 6) // ----------------------------------------------------------------------------------- // Si "PointeurTableauBrochesLED" est inférieur à "6"
      {
        digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], LOW);                                                   // Eteint la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
        digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], HIGH);                                                 // Allume la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      }
    }
    
    PORTD &= 0b00000011;                                                                                                        // Eteint toutes les LED du PORTD
    PORTB &= 0b11000000;                                                                                                        // Eteint toutes les LED du PORTB
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 4));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED aller-retour de 2 x 6 LED **************************************************************************************************************************
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
      PointeurTableauBrochesLED = 0;                                                                                            // Initialise le pointeur du tableau des broches LED
      DureeHIGH_LOW = 120ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], HIGH);                                                    // Allume la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED + 2], HIGH);                                            // Allume la LED de rang "3 - PointeurTableauBrochesLED_D2D7 + 2" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED + 2], LOW);                                            // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D8D13 + 2" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], LOW);                                                     // Eteint la LED de rang "PointeurTableauBrochesLED_D2D7" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED + 2], LOW);                                             // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D2D7 + 2" du tableau des broches des 6 LED de (D2 à D7)
      digitalWrite(TableauBrochesLED_D8D13[PointeurTableauBrochesLED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBrochesLED_D8D13" du tableau des broches des 6 LED de (D8 à D13)
      digitalWrite(TableauBrochesLED_D8D13[3 - PointeurTableauBrochesLED + 2], HIGH);                                           // Allume la LED de rang "3 - PointeurTableauBrochesLED_D8D13 + 2" du tableau des broches des 6 LED de (D8 à D13)
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], LOW);                                             // Eteint la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
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
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 2; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[3 - PointeurTableauBrochesLED - 1], HIGH);                                            // Allume la LED de rang "3 - PointeurTableauBrochesLED_D2D7 - 1" du tableau des broches des 6 LED de (D2 à D7)
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
//*** Chenillard 12 LED aller-retour 1 LED et changement de vitesse ********************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 16) // ********************************************************************************************** // Si le mode courant "16" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 1;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= 11; PointeurTableauBroches12LED++) // ++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // ++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // ++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= 11; PointeurTableauBroches12LED++) // ++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
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
  else if (ModeCourant == 17) // ********************************************************************************************** // Si le mode courant "17" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 12;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      DeltaTempo_EtatHaut = 10; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit l'incrément ou le décrément pour le calcul de la valeur de la temporisation de la LED activée ou désactivée
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= 11; PointeurTableauBroches12LED += 2) // +++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= 11; PointeurTableauBroches12LED += 2) // +++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED -= 2) // +++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED -= 2) // +++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    if (DureeHIGH_LOW > 50ul) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation de la LED activée ou désactivée est supérieure à "50"
    {
      DeltaTempo_EtatHaut = -DeltaTempo_EtatHaut;                                                                               // Inverse l'incrément ou le décrément
    }
    else if (DureeHIGH_LOW < 10ul) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation de la LED activée ou désactivée est inférieure à "10"
    {
      DeltaTempo_EtatHaut = -DeltaTempo_EtatHaut;                                                                               // Inverse l'incrément ou le décrément
      
    }
    
    DureeHIGH_LOW += DeltaTempo_EtatHaut;                                                                                       // Incrémente ou décremente la temporisation de la LED activée ou désactivée
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED affichage alterné de 1 LED sur 2 avec variation de vitesse *****************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 18) // ********************************************************************************************** // Si le mode courant "18" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 25;                                                                                  // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      DureeTempo_EtatHaut = 5ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état haut en mode non bloquant
      OffsetDureeTempo_EtatHaut = 5ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Initialise l'offset de la temporisation à l'état haut en mode non bloquant
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= 10; PointeurTableauBroches12LED += 2) // +++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeTempo_EtatHaut);                                                                              // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      DureeTempo_EtatHaut += OffsetDureeTempo_EtatHaut;                                                                         // Incrémente la durée de la temporisation à l'état haut en mode non bloquant
    }
    
    DureeTempo_EtatHaut = 5;                                                                                                    // Réinitialise la durée de la temporisation à l'état haut en mode non bloquant
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= 10; PointeurTableauBroches12LED += 2) // +++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], LOW);                                              // Eteint la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= 1; PointeurTableauBroches12LED -= 2) // +++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED aller-retour 3 LED *********************************************************************************************************************************
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
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = -2; PointeurTableauBroches12LED < 13; PointeurTableauBroches12LED++) // ++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      PORTD &= 0b00000011;                                                                                                      // Eteint toutes les LED du PORTD
      PORTB &= 0b11000000;                                                                                                      // Eteint toutes les LED du PORTB
      
      Fonction_Temporisation(int(DureeHIGH_LOW / 50.0));                                                                        // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Incrémente 3 fois l'offset du pointeur du tableau des broches 12 LED
      for (int OffsetPointeurTableauBroches12LED = 0; OffsetPointeurTableauBroches12LED < 3; OffsetPointeurTableauBroches12LED++)
      {
        // .................................................................................................................... // Si "PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED" est supérieur à "-1" && "PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED" est inférieur à "12"
        if (PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED > -1 && PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED < 12)
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED], LOW);              // Eteint les LED courantes du tableau des broches des 12 LED
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED], HIGH);             // Allume les LED courantes du tableau des broches des 12 LED
        }
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
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBroches12LED = 13; PointeurTableauBroches12LED > -2; PointeurTableauBroches12LED--) // ++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      PORTD &= 0b00000011;                                                                                                      // Eteint toutes les LED du PORTD
      PORTB &= 0b11000000;                                                                                                      // Eteint toutes les LED du PORTB
      
      Fonction_Temporisation(int(DureeHIGH_LOW / 50.0));                                                                        // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Incrémente 3 fois l'offset du pointeur du tableau des broches 12 LED
      for (int OffsetPointeurTableauBroches12LED = 0; OffsetPointeurTableauBroches12LED < 3; OffsetPointeurTableauBroches12LED++)
      {
        // .................................................................................................................... // Si "PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED" est supérieur à "-1" && "PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED" est inférieur à "12"
        if (PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED > -1 && PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED < 12)
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED], LOW);              // Eteint les LED courantes du tableau des broches des 12 LED
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED], HIGH);             // Allume les LED courantes du tableau des broches des 12 LED
        }
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
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED aller 4 LED retour 3 LED et changement de vitesse au retour ****************************************************************************************
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
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = -3; PointeurTableauBroches12LED < 14; PointeurTableauBroches12LED++) // ++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      PORTD &= 0b00000011;                                                                                                      // Eteint toutes les LED du PORTD
      PORTB &= 0b11000000;                                                                                                      // Eteint toutes les LED du PORTB
      
      Fonction_Temporisation(int(DureeHIGH_LOW / 50.0));                                                                        // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Incrémente 4 fois l'offset du pointeur du tableau des broches 12 LED
      for (int OffsetPointeurTableauBroches12LED = 0; OffsetPointeurTableauBroches12LED < 4; OffsetPointeurTableauBroches12LED++)
      {
        // .................................................................................................................... // Si "PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED" est supérieur à "-1" && "PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED" est inférieur à "12"
        if (PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED > -1 && PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED < 12)
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED], LOW);              // Eteint les LED courantes du tableau des broches des 12 LED
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED], HIGH);             // Allume les LED courantes du tableau des broches des 12 LED
        }
      }
      
      Fonction_Temporisation(DureeHIGH_LOW + int(DureeHIGH_LOW / 2.0));                                                         // Appelle la fonction de temporisation
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
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBroches12LED = 13; PointeurTableauBroches12LED > -2; PointeurTableauBroches12LED--) // ++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      PORTD &= 0b00000011;                                                                                                      // Eteint toutes les LED du PORTD
      PORTB &= 0b11000000;                                                                                                      // Eteint toutes les LED du PORTB
      
      Fonction_Temporisation(int(DureeHIGH_LOW / 50.0));                                                                        // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      // ---------------------------------------------------------------------------------------------------------------------- // Incrémente 3 fois l'offset du pointeur du tableau des broches 12 LED
      for (int OffsetPointeurTableauBroches12LED = 0; OffsetPointeurTableauBroches12LED < 2; OffsetPointeurTableauBroches12LED++)
      {
        // .................................................................................................................... // Si "PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED" est supérieur à "-1" && "PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED" est inférieur à "12"
        if (PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED > -1 && PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED < 12)
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED], LOW);              // Eteint les LED courantes du tableau des broches des 12 LED
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED], HIGH);             // Allume les LED courantes du tableau des broches des 12 LED
        }
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
    
    Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                             // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée *******************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 21) // ********************************************************************************************** // Si le mode courant "21" est sélectionné
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
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    int ADRDepartTableauSequencesLED = 0;                                                                                       // Déclare et initialise l'adresse de départ de chaque séquence du tableau des séquences d'affichage des LED
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED1 - 1; SequenceCouranteLED++) // ++++++++++++++++++++ // Parcourt les séquences du tableau 1 des séquences d'affichage des LED
    {
      ADRDepartTableauSequencesLED = NombreLED * SequenceCouranteLED;                                                           // Calcule l'adresse de départ de la séquence courante du tableau des séquences d'affichage des LED
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "ADRDepartTableauSequencesLED + PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], TableauSequencesLED1[ADRDepartTableauSequencesLED + (NombreLED - 1) - PointeurTableauBroches12LED]);
        
        Fonction_Temporisation(int(DureeHIGH_LOW / 4.0));                                                                       // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTD &= 0b00000011;                                                                                                    // Eteint toutes les LED du PORTD
        PORTB &= 0b11000000;                                                                                                    // Eteint toutes les LED du PORTB
      }
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED1 - 1; SequenceCouranteLED++) // ++++++++++++++++++++ // Parcourt les séquences du tableau 1 des séquences d'affichage des LED
    {
      ADRDepartTableauSequencesLED = NombreLED * SequenceCouranteLED;                                                           // Calcule l'adresse de départ de la séquence courante du tableau des séquences d'affichage des LED
      
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
        
        PORTD &= 0b00000011;                                                                                                    // Eteint toutes les LED du PORTD
        PORTB &= 0b11000000;                                                                                                    // Eteint toutes les LED du PORTB
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée *******************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 22) // ********************************************************************************************** // Si le mode courant "22" est sélectionné
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
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    int ADRDepartTableauSequencesLED = 0;                                                                                       // Déclare et initialise l'adresse de départ de chaque séquence du tableau des séquences d'affichage des LED
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED2 - 1; SequenceCouranteLED++) // ++++++++++++++++++++ // Parcourt les séquences du tableau 2 des séquences d'affichage des LED
    {
      ADRDepartTableauSequencesLED = NombreLED * SequenceCouranteLED;                                                           // Calcule l'adresse de départ de la séquence courante du tableau des séquences d'affichage des LED
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "ADRDepartTableauSequencesLED + PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], TableauSequencesLED2[ADRDepartTableauSequencesLED + (NombreLED - 1) - PointeurTableauBroches12LED]);
        
        Fonction_Temporisation(int(DureeHIGH_LOW / 4.0));                                                                       // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTD &= 0b00000011;                                                                                                    // Eteint toutes les LED du PORTD
        PORTB &= 0b11000000;                                                                                                    // Eteint toutes les LED du PORTB
      }
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED2 - 1; SequenceCouranteLED++) // ++++++++++++++++++++ // Parcourt les séquences du tableau 2 des séquences d'affichage des LED
    {
      ADRDepartTableauSequencesLED = NombreLED * SequenceCouranteLED;                                                           // Calcule l'adresse de départ de la séquence courante du tableau des séquences d'affichage des LED
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "ADRDepartTableauSequencesLED + PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], TableauSequencesLED2[ADRDepartTableauSequencesLED + PointeurTableauBroches12LED]);
        
        Fonction_Temporisation(int(DureeHIGH_LOW / 4.0));                                                                       // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTD &= 0b00000011;                                                                                                    // Eteint toutes les LED du PORTD
        PORTB &= 0b11000000;                                                                                                    // Eteint toutes les LED du PORTB
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED alterné aller de gauche à droite et retour en clignotant *******************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 23) // ********************************************************************************************** // Si le mode courant "23" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      DureeHIGH_LOW = 70ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 6; PointeurTableauBroches12LED++) // ++++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED courante du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], HIGH);                                             // Allume la LED courante du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 6; PointeurTableauBroches12LED++) // ++++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED courante du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], LOW);                                              // Eteint la LED courante du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 6; PointeurTableauBroches12LED <= 11; PointeurTableauBroches12LED++) // ++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      for (int OSC = 0; OSC < 5; OSC++) // ------------------------------------------------------------------------------------ // Boucle 5 fois
      {
                                                                                                                                // Allume ou éteint la LED courante du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !digitalRead(TableauBroches12LED[PointeurTableauBroches12LED]));
                                                                                                                                // Allume ou éteint la LED courante du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], !digitalRead(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5]));
        
        Fonction_Temporisation(int(DureeHIGH_LOW / 3));                                                                         // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    PORTD &= 0b00000011;                                                                                                        // Eteint toutes les LED du PORTD
    PORTB &= 0b11000000;                                                                                                        // Eteint toutes les LED du PORTB
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    for (PointeurTableauBroches12LED = 5; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // +++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED courante du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], HIGH);                                             // Allume la LED courante du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 5; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // +++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED courante du tableau des broches des 12 LED
      digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], LOW);                                              // Eteint la LED courante du tableau des broches des 12 LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 6; PointeurTableauBroches12LED++) // ++++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      for (int OSC = 0; OSC < 5; OSC++) // ------------------------------------------------------------------------------------ // Boucle 5 fois
      {
                                                                                                                                // Allume ou éteint la LED courante du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], !digitalRead(TableauBroches12LED[PointeurTableauBroches12LED]));
                                                                                                                                // Allume ou éteint la LED courante du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], !digitalRead(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5]));
        
        Fonction_Temporisation(int(DureeHIGH_LOW / 3));                                                                         // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    PORTD &= 0b00000011;                                                                                                        // Eteint toutes les LED du PORTD
    PORTB &= 0b11000000;                                                                                                        // Eteint toutes les LED du PORTB
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée *******************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 24) // ********************************************************************************************** // Si le mode courant "24" est sélectionné
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
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    int ADRDepartTableauSequencesLED = 0;                                                                                       // Déclare et initialise l'adresse de départ de chaque séquence du tableau des séquences d'affichage des LED
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED - 1; SequenceCouranteLED++) // +++++++++++++++++++++ // Parcourt les séquences du tableau des séquences d'affichage des LED
    {
      ADRDepartTableauSequencesLED = NombreLED * SequenceCouranteLED;                                                           // Calcule l'adresse de départ de la séquence courante du tableau des séquences d'affichage des LED
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "ADRDepartTableauSequencesLED + PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], TableauSequencesLED[ADRDepartTableauSequencesLED + PointeurTableauBroches12LED]);
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
//*** Animation 12 LED séquencée inversée **********************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 25) // ********************************************************************************************** // Si le mode courant "25" est sélectionné
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
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    int ADRDepartTableauSequencesLED = 0;                                                                                       // Déclare et initialise l'adresse de départ de chaque séquence du tableau des séquences d'affichage des LED
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED - 1; SequenceCouranteLED++) // +++++++++++++++++++++ // Parcourt les séquences du tableau des séquences d'affichage des LED
    {
      ADRDepartTableauSequencesLED = NombreLED * SequenceCouranteLED;                                                           // Calcule l'adresse de départ de la séquence courante du tableau des séquences d'affichage des LED
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "ADRDepartTableauSequencesLED + PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], TableauSequencesLED[ADRDepartTableauSequencesLED + (NombreLED - 1) - PointeurTableauBroches12LED]);
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
//*** Chenillard 12 LED ****************************************************************************************************************************************************
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
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBroches12LED = -4; PointeurTableauBroches12LED < 15; PointeurTableauBroches12LED++) // ++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      PORTD |= 0b11111100;                                                                                                      // Allume toutes les LED du PORTD
      PORTB |= 0b00111111;                                                                                                      // Allume toutes les LED du PORTB
      
      // ---------------------------------------------------------------------------------------------------------------------- // Incrémente 5 fois l'offset du pointeur du tableau des broches 12 LED
      for (int OffsetPointeurTableauBroches12LED = 0; OffsetPointeurTableauBroches12LED < 5; OffsetPointeurTableauBroches12LED++)
      {
        // .................................................................................................................... // Si "PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED" est supérieur à "-1" && "PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED" est inférieur à "12"
        if (PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED > -1 && PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED < 12)
        {
          if (OffsetPointeurTableauBroches12LED == 2)                                                                           // Si l'offset du pointeur du tableau des broches 12 LED est égal à "2"
          {
            digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], LOW);                                            // Eteint les LED courantes du tableau des broches des 12 LED
            
            Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                     // Appelle la fonction de temporisation
            if (SortieModeCourant)                                                                                              // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
            {
              SortieModeCourant = false;                                                                                        // Réinitialise l'indicateur de sortie du mode courant
              return;                                                                                                           // Retour début loop()
            }
            
            digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], HIGH);                                           // Allume les LED courantes du tableau des broches des 12 LED
            
            Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                     // Appelle la fonction de temporisation
            if (SortieModeCourant)                                                                                              // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
            {
              SortieModeCourant = false;                                                                                        // Réinitialise l'indicateur de sortie du mode courant
              return;                                                                                                           // Retour début loop()
            }
          }
          
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED], LOW);              // Eteint les LED courantes du tableau des broches des 12 LED
        }
      }
      
      Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                           // Appelle la fonction de temporisation
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
    
    for (PointeurTableauBroches12LED = 15; PointeurTableauBroches12LED > -4; PointeurTableauBroches12LED--) // ++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      PORTD |= 0b11111100;                                                                                                      // Allume toutes les LED du PORTD
      PORTB |= 0b00111111;                                                                                                      // Allume toutes les LED du PORTB
      
      // ---------------------------------------------------------------------------------------------------------------------- // Incrémente 5 fois l'offset du pointeur du tableau des broches 12 LED
      for (int OffsetPointeurTableauBroches12LED = 0; OffsetPointeurTableauBroches12LED < 5; OffsetPointeurTableauBroches12LED++)
      {
        // .................................................................................................................... // Si "PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED" est supérieur à "-1" && "PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED" est inférieur à "12"
        if (PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED > -1 && PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED < 12)
        {
          if (OffsetPointeurTableauBroches12LED == 2)                                                                           // Si l'offset du pointeur du tableau des broches 12 LED est égal à "2"
          {
            digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], LOW);                                            // Eteint les LED courantes du tableau des broches des 12 LED
            
            Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                     // Appelle la fonction de temporisation
            if (SortieModeCourant)                                                                                              // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
            {
              SortieModeCourant = false;                                                                                        // Réinitialise l'indicateur de sortie du mode courant
              return;                                                                                                           // Retour début loop()
            }
            
            digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], HIGH);                                           // Allume les LED courantes du tableau des broches des 12 LED
            
            Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                     // Appelle la fonction de temporisation
            if (SortieModeCourant)                                                                                              // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
            {
              SortieModeCourant = false;                                                                                        // Réinitialise l'indicateur de sortie du mode courant
              return;                                                                                                           // Retour début loop()
            }
          }
          
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED], LOW);              // Eteint les LED courantes du tableau des broches des 12 LED
        }
      }
      
      Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                           // Appelle la fonction de temporisation
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
//*** Chenillard 12 LED ****************************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 27) // ********************************************************************************************** // Si le mode courant "27" est sélectionné
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
    
    for (PointeurTableauBroches12LED = -4; PointeurTableauBroches12LED < 15; PointeurTableauBroches12LED++) // ++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      PORTD |= 0b11111100;                                                                                                      // Allume toutes les LED du PORTD
      PORTB |= 0b00111111;                                                                                                      // Allume toutes les LED du PORTB
      
      // ---------------------------------------------------------------------------------------------------------------------- // Incrémente 5 fois l'offset du pointeur du tableau des broches 12 LED
      for (int OffsetPointeurTableauBroches12LED = 0; OffsetPointeurTableauBroches12LED < 5; OffsetPointeurTableauBroches12LED++)
      {
        // .................................................................................................................... // Si "PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED" est supérieur à "-1" && "PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED" est inférieur à "12"
        if (PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED > -1 && PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED < 12)
        {
          if (OffsetPointeurTableauBroches12LED == 2)                                                                           // Si l'offset du pointeur du tableau des broches 12 LED est égal à "2"
          {
            digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], LOW);                                            // Eteint les LED courantes du tableau des broches des 12 LED
            
            Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                     // Appelle la fonction de temporisation
            if (SortieModeCourant)                                                                                              // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
            {
              SortieModeCourant = false;                                                                                        // Réinitialise l'indicateur de sortie du mode courant
              return;                                                                                                           // Retour début loop()
            }
            
            digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], HIGH);                                           // Allume les LED courantes du tableau des broches des 12 LED
            
            Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                     // Appelle la fonction de temporisation
            if (SortieModeCourant)                                                                                              // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
            {
              SortieModeCourant = false;                                                                                        // Réinitialise l'indicateur de sortie du mode courant
              return;                                                                                                           // Retour début loop()
            }
          }
          
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED], LOW);              // Eteint les LED courantes du tableau des broches des 12 LED
          
          Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                       // Appelle la fonction de temporisation
          if (SortieModeCourant)                                                                                                // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
          {
            SortieModeCourant = false;                                                                                          // Réinitialise l'indicateur de sortie du mode courant
            return;                                                                                                             // Retour début loop()
          }
          
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + OffsetPointeurTableauBroches12LED], HIGH);             // Allume les LED courantes du tableau des broches des 12 LED
        }
      }
      
      Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                           // Appelle la fonction de temporisation
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
    
    for (PointeurTableauBroches12LED = 15; PointeurTableauBroches12LED > -4; PointeurTableauBroches12LED--) // ++++++++++++++++ // Parcourt le tableau des broches des 12 LED
    {
      PORTD |= 0b11111100;                                                                                                      // Allume toutes les LED du PORTD
      PORTB |= 0b00111111;                                                                                                      // Allume toutes les LED du PORTB
      
      // ---------------------------------------------------------------------------------------------------------------------- // Incrémente 5 fois l'offset du pointeur du tableau des broches 12 LED
      for (int OffsetPointeurTableauBroches12LED = 0; OffsetPointeurTableauBroches12LED < 5; OffsetPointeurTableauBroches12LED++)
      {
        // .................................................................................................................... // Si "PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED" est supérieur à "-1" && "PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED" est inférieur à "12"
        if (PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED > -1 && PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED < 12)
        {
          if (OffsetPointeurTableauBroches12LED == 2)                                                                           // Si l'offset du pointeur du tableau des broches 12 LED est égal à "2"
          {
            digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], LOW);                                            // Eteint les LED courantes du tableau des broches des 12 LED
            
            Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                     // Appelle la fonction de temporisation
            if (SortieModeCourant)                                                                                              // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
            {
              SortieModeCourant = false;                                                                                        // Réinitialise l'indicateur de sortie du mode courant
              return;                                                                                                           // Retour début loop()
            }
            
            digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], HIGH);                                           // Allume les LED courantes du tableau des broches des 12 LED
            
            Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                     // Appelle la fonction de temporisation
            if (SortieModeCourant)                                                                                              // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
            {
              SortieModeCourant = false;                                                                                        // Réinitialise l'indicateur de sortie du mode courant
              return;                                                                                                           // Retour début loop()
            }
          }
          
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED], LOW);              // Eteint les LED courantes du tableau des broches des 12 LED
          
          Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                       // Appelle la fonction de temporisation
          if (SortieModeCourant)                                                                                                // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
          {
            SortieModeCourant = false;                                                                                          // Réinitialise l'indicateur de sortie du mode courant
            return;                                                                                                             // Retour début loop()
          }
          
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - OffsetPointeurTableauBroches12LED], HIGH);             // Allume les LED courantes du tableau des broches des 12 LED
        }
      }
      
      Fonction_Temporisation(int(DureeHIGH_LOW * 2));                                                                           // Appelle la fonction de temporisation
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
//*** Animation 12 LED séquencée *******************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 28) // ********************************************************************************************** // Si le mode courant "28" est sélectionné
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
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    int ADRDepartTableauSequencesLED = 0;                                                                                       // Déclare et initialise l'adresse de départ de chaque séquence du tableau des séquences d'affichage des LED
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED - 1; SequenceCouranteLED++) // +++++++++++++++++++++ // Parcourt les séquences du tableau des séquences d'affichage des LED
    {
      ADRDepartTableauSequencesLED = NombreLED * SequenceCouranteLED;                                                           // Calcule l'adresse de départ de la séquence courante du tableau des séquences d'affichage des LED
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "ADRDepartTableauSequencesLED + PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], TableauSequencesLED[ADRDepartTableauSequencesLED + (NombreLED - 1) - PointeurTableauBroches12LED]);
        
        Fonction_Temporisation(int(DureeHIGH_LOW / 2.0));                                                                       // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTD &= 0b00000011;                                                                                                    // Eteint toutes les LED du PORTD
        PORTB &= 0b11000000;                                                                                                    // Eteint toutes les LED du PORTB
      }
    }
    
    for (SequenceCouranteLED = 0; SequenceCouranteLED < NombreSequencesLED - 1; SequenceCouranteLED++) // +++++++++++++++++++++ // Parcourt les séquences du tableau des séquences d'affichage des LED
    {
      ADRDepartTableauSequencesLED = NombreLED * SequenceCouranteLED;                                                           // Calcule l'adresse de départ de la séquence courante du tableau des séquences d'affichage des LED
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < NombreLED; PointeurTableauBroches12LED++) // -------- // Parcourt le tableau des broches des 12 LED
      {
                                                                                                                                // Allume ou éteint la LED courante de rang "ADRDepartTableauSequencesLED + PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], TableauSequencesLED[ADRDepartTableauSequencesLED + PointeurTableauBroches12LED]);
        
        Fonction_Temporisation(int(DureeHIGH_LOW / 2.0));                                                                       // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTD &= 0b00000011;                                                                                                    // Eteint toutes les LED du PORTD
        PORTB &= 0b11000000;                                                                                                    // Eteint toutes les LED du PORTB
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED simulation vu-mètre + pic inversée ******************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 29) // ********************************************************************************************** // Si le mode courant "29" est sélectionné
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
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    long Amplitude = random(0, 12);                                                                                             // Déclare et sélectionne une amplitude aléatoirement
    static long Pic = 0;                                                                                                        // Déclare et Initialise l'amplitude max
    
    if (Amplitude > Pic) {Pic = Amplitude;} // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'amplitude courante est supérieure à l'amplitude max
    
    for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= Amplitude; PointeurTableauBroches12LED++) // +++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                      // Eteint la LED courante de rang "PointeurTableauBroches12LED"
      
      Fonction_Temporisation(int(DureeHIGH_LOW / 4.0));                                                                         // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    if (Amplitude < Pic) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'amplitude courante est inférieure à l'amplitude max
    {
      digitalWrite(TableauBroches12LED[Pic], HIGH);                                                                             // Allume la LED courante de rang "Pic"
      
      if (Pic > 0) {Pic = Pic - 1;} else {Pic = 0;} // ------------------------------------------------------------------------ // Si l'amplitude max est supérieure à "0" => Décrémente l'amplitude max
      
      digitalWrite(TableauBroches12LED[Pic], LOW);                                                                              // Eteint la LED courante de rang "Pic"
    }
    
    if (Amplitude > 0 && Amplitude == Pic) {Amplitude = Amplitude - 1;} // ++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'amplitude courante est supérieure à "0" et égale à l'amplitude max => Décrémente l'amplitude courante
    
    for (PointeurTableauBroches12LED = Amplitude; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // +++++++++ // Parcourt le tableau des broches des 12 LED
    {
      digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                     // Allume la LED courante de rang "PointeurTableauBroches12LED"
      
      Fonction_Temporisation(int(DureeHIGH_LOW / 4.0));                                                                         // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED simultané inversé déplacement vers le centre avec incrémentation d'une LED à chaque boucle *********************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 30) // ********************************************************************************************** // Si le mode courant "30" est sélectionné
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
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 6 fois
    for (compteurOffsetPointeurTableauBroches12LED = 5; compteurOffsetPointeurTableauBroches12LED >= 0; compteurOffsetPointeurTableauBroches12LED -= 1)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED <= compteurOffsetPointeurTableauBroches12LED; PointeurTableauBroches12LED += 1)
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED" du tableau des broches des 12 LED
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], LOW);                                            // Eteint la LED de rang "6 - PointeurTableauBroches12LED + 5" du tableau des broches des 12 LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        PORTD |= 0b11111100;                                                                                                    // Allume toutes les LED du PORTD
        PORTB |= 0b00111111;                                                                                                    // Allume toutes les LED du PORTB
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
