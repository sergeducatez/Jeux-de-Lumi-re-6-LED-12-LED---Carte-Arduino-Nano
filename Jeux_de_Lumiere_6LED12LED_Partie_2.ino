//**************************************************************************************************************************************************************************
//*********************************************** Jeux de Lumière 6 LED 12 LED - Partie 2 - Carte Nano (com49) *************************************************************
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
volatile const int NombreModesMax = 21; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de modes maximum

volatile bool SortieModeCourant = false;                                                                                        // Indicateur de sortie du mode courant

volatile bool ModeAuto = false;                                                                                                 // Indicateur du mode automatique
volatile long compteurAffichageModeCourant = 0;                                                                                 // Compteur d'affichages du mode courant en mode automatique
const long NombreAffichageModeCourant = 3;                                                                                      // Nombre d'affichages du mode courant en mode automatique
volatile long MultipleNombreAffichageModeCourant = 1;                                                                           // Multiple du nombre d'affichages du mode courant en mode automatique
bool AffichageModeManuel = true;                                                                                                // Indicateur d'affichage du mode manuel sur l'écran OLED
bool AffichageModeAuto = false;                                                                                                 // Indicateur d'affichage du mode auto sur l'écran OLED

const unsigned long DureeAntiRebond = 5ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de l'anti-rebonds des boutons poussoirs "BPModePlus" et "BPModeMoins" en ms

unsigned long t1DebutTempo_EtatHaut;                                                                                            // Début de la temporisation à l'état haut en mode non bloquant
unsigned long t1DebutTempo_EtatBas;                                                                                             // Début de la temporisation à l'état bas en mode non bloquant
unsigned long DureeTempo_EtatBas;                                                                                               // Durée de la temporisation à l'état bas en mode non bloquant
unsigned long DureeTempo_EtatHaut;                                                                                              // Durée de la temporisation à l'état haut en mode non bloquant
int DeltaTempo_EtatHaut;                                                                                                        // Incrément ou décrément de la valeur de la temporisation à l'état haut en mode non bloquant
int DeltaTempo_EtatBas;                                                                                                         // Incrément ou décrément de la valeur de la temporisation à l'état bas en mode non bloquant

volatile int CompteurFlash;                                                                                                     // Compteur de demi-périodes de clignotement

int PointeurTableauBrochesLED;                                                                                                  // Pointeur des tableaux des broches de 6 LED
const int TableauBrochesLED_D2D7 [] = {2, 3, 4, 5, 6, 7}; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des broches des 6 LED de (D2 à D7)
const int TableauBrochesLED_D8D13 [] = {8, 9, 10, 11, 12, 13}; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des broches des 6 LED de (D8 à D13)
int PointeurTableauBroches12LED;                                                                                                // Pointeur du tableau des broches 12 LED
int PointeurTableauBroches12LEDTemp;                                                                                            // Pointeur de fin temporaire du tableau des broches 12 LED
const int TableauBroches12LED [] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des broches des 12 LED

const int Nombre_Sequences = 16; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Nombre de séquences d'affichage
const byte Tableau_Sequenceur [Nombre_Sequences] = {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0}; // >>>>>>>>>>>>>>>>>>>>>>> // Tableau de séquences d'affichage
byte pointeur_TableauSequenceur;                                                                                                // Pointeur du tableau de séquences d'affichage
unsigned long Duree_EntreSequences;                                                                                             // Temporisation entre les séquences d'affichage

volatile bool Sens_decalage_PORTD;                                                                                              // Sens de décalage du PORTD

byte MaskLedsInitial;                                                                                                           // Masque initial des LED
volatile byte MaskLeds = MaskLedsInitial;                                                                                       // Masque des LED
byte EtatInitial;                                                                                                               // Etat initial des LED
byte etatLeds = EtatInitial;                                                                                                    // Etat courant des LED
byte Dernier_etatLeds;                                                                                                          // Dernier état courant des LED
byte masquePORTD;                                                                                                               // Masque du PORTD
byte masquePORTB;                                                                                                               // Masque du PORTB

bool Mute;                                                                                                                      // Indicateur d'activation du buzzer

int CompteurDecalageEtatLeds;                                                                                                   // Compteur de décalage de l'état courant des LED

volatile int CompteurBinaire;                                                                                                   // Compteur binaire

unsigned long DureeHIGH_LOW;                                                                                                    // Temporisation de la LED activée ou désactivée

const byte TableauSequencesLED1 [7][6] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des séquences d'affichage des LED 1
  {0, 0, 0, 0, 0, 0},
  {1, 0, 0, 0, 0, 0},
  {1, 0, 0, 0, 0, 1},
  {1, 1, 0, 0, 0, 1},
  {1, 1, 0, 0, 1, 1},
  {1, 1, 1, 0, 1, 1},
  {1, 1, 1, 1, 1, 1}
};

int pointeurLigneTableauSequences;                                                                                              // Pointeur des lignes du tableau des séquences d'affichage des LED 2
int pointeurColonnesTableauSequences;                                                                                           // Pointeur des colonnes du tableau des séquences d'affichage des LED 2
byte Masque0;                                                                                                                   // Masque pour les valeurs du tableau des séquences d'affichage des LED 2 égales à "0"
byte Masque1;                                                                                                                   // Masque pour les valeurs du tableau des séquences d'affichage des LED 2 égales à "1"
const byte TableauSequencesLED2 [24][6] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des séquences d'affichage des LED 2
  {0, 0, 1, 1, 0, 0},
  {0, 1, 1, 1, 1, 0},
  {1, 1, 1, 1, 1, 1},
  {0, 1, 1, 1, 1, 0},
  {0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 0, 0},
  {1, 0, 0, 0, 0, 1},
  {1, 1, 0, 0, 1, 1},
  {1, 1, 1, 1, 1, 1},
  {0, 1, 1, 1, 1, 0},
  {0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 0, 0},
  {1, 1, 0, 0, 1, 1},
  {1, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0},
  {1, 0, 0, 0, 0, 1},
  {1, 1, 0, 0, 1, 1},
  {1, 1, 1, 1, 1, 1},
  {0, 1, 1, 1, 1, 0},
  {0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 0, 0},
  {1, 0, 0, 0, 0, 1},
  {1, 1, 0, 0, 1, 1},
  {1, 1, 1, 1, 1, 1}
};

const byte TableauSequencesLED3 [14][12] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des séquences d'affichage des LED 3
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
  {0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0},
  {0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0},
  {0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0},
  {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const byte TableauSequencesLED4 [20][12] = { // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Tableau des séquences d'affichage des LED 4
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0},
  {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0},
  {0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0},
  {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0},
  {0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
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
      compteurAffichageModeCourant = 0;                                                                                         // Initialise le compteur d'affichages du mode courant en mode automatique
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
      if (ModeCourant != 0 && ModeCourant != 1 && ModeCourant != 2 && ModeCourant != 3 && ModeCourant != 6 && ModeCourant != 9 && ModeCourant != 14 && ModeCourant != 15 && ModeCourant != 19 && ModeCourant != 20 && ModeCourant != 21)
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
//*** Chenillard 12 LED séquentiel de gauche à droite et de droite à gauche non bloquant ***********************************************************************************
//**************************************************************************************************************************************************************************
  if (ModeCourant == 0) // **************************************************************************************************** // Si le mode courant "0" est sélectionné
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
      CompteurDecalageEtatLeds = 6;                                                                                             // Définit le compteur de décalage de l'état courant des LED
      Sens_decalage_PORTD = false;                                                                                              // Définit l'indicateur du sens de décalage du PORTD
      Mute = false; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit l'indicateur d'activation du buzzer
      DureeTempo_EtatBas = 150ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état bas
      DureeTempo_EtatHaut = 20ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état haut
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Initialise la temporisation à l'état bas en mode non bloquant
      t1DebutTempo_EtatHaut = millis();                                                                                         // Démarre la temporisation à l'état haut en mode non bloquant
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (!Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de gauche à droite
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        if (Mute) {Buzzer(1, 0, 1);} // ....................................................................................... // Active le buzzer 1ms
        
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume la LED courante du PORTD
        PORTB |= (etatLeds & MaskLeds) >> 2;                                                                                    // Allume la LED courante du PORTB
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Réinitialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = millis();                                                                                        // Démarre la temporisation à l'état bas en mode non bloquant
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint la LED courante du PORTD
        PORTB &= ~(etatLeds & MaskLeds) >> 2;                                                                                   // Eteint la LED courante du PORTB
        
        t1DebutTempo_EtatBas = 0;                                                                                               // Réinitialise la temporisation à l'état bas en mode non bloquant
        t1DebutTempo_EtatHaut = millis();                                                                                       // Démarre la temporisation à l'état haut en mode non bloquant
        
        CompteurDecalageEtatLeds--;                                                                                             // Décrémente le compteur de décalage de l'état courant des LED
        
        if (CompteurDecalageEtatLeds != 0) // ................................................................................. // Si le compteur de décalage de l'état courant des LED est différent de "0"
        {
          if (CompteurDecalageEtatLeds % 2 != 0) {etatLeds <<= CompteurDecalageEtatLeds;}                                       // Si le compteur de décalage de l'état courant des LED est impair
          else {etatLeds >>= CompteurDecalageEtatLeds;}                                                                         // Si le compteur de décalage de l'état courant des LED est pair
        }
        else if (CompteurDecalageEtatLeds == 0) // ............................................................................ // Si le compteur de décalage de l'état courant des LED est égal à "0"
        {
          etatLeds = 0b00010000;                                                                                                // Définit l'état initial des LED
          Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                           // Inverse l'indicateur du sens de décalage
          t1DebutTempo_EtatBas = 0ul;                                                                                           // Réinitialise la temporisation à l'état bas en mode non bloquant
        }
      }
    }
    else if (Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de droite à gauche
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        if (Mute) {Buzzer(1, 0, 1);} // ....................................................................................... // Active le buzzer 1ms
        
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume la LED courante du PORTD
        PORTB |= (etatLeds & MaskLeds) >> 2;                                                                                    // Allume la LED courante du PORTB
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Réinitialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = millis();                                                                                        // Démarre la temporisation à l'état bas en mode non bloquant
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint la LED courante du PORTD
        PORTB &= ~(etatLeds & MaskLeds) >> 2;                                                                                   // Eteint la LED courante du PORTB
        
        t1DebutTempo_EtatBas = 0ul;                                                                                             // Réinitialise la temporisation à l'état bas en mode non bloquant
        t1DebutTempo_EtatHaut = millis();                                                                                       // Démarre la temporisation à l'état haut en mode non bloquant
        
        CompteurDecalageEtatLeds++;                                                                                             // Incrémente le compteur de décalage de l'état courant des LED
        
        if (CompteurDecalageEtatLeds != 6) // ................................................................................. // Si le compteur de décalage de l'état courant des LED est différent de "6"
        {
          if (CompteurDecalageEtatLeds % 2 != 0) {etatLeds <<= CompteurDecalageEtatLeds;}                                       // Si le compteur de décalage de l'état courant des LED est impair
          else {etatLeds >>= CompteurDecalageEtatLeds;}                                                                         // Si le compteur de décalage de l'état courant des LED est pair
        }
        else if (CompteurDecalageEtatLeds == 6) // ............................................................................ // Si le compteur de décalage de l'état courant des LED est égal à "6"
        {
          etatLeds = 0b00000100;                                                                                                // Définit l'état initial des LED
          Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                           // Inverse l'indicateur du sens de décalage
          t1DebutTempo_EtatBas = 0ul;                                                                                           // Réinitialise la temporisation à l'état bas en mode non bloquant
          
          compteurAffichageModeCourant++;                                                                                       // Incrémente le compteur d'affichages du mode courant en mode automatique
        }
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Compteur binaire de aller-retour non bloquant ************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 1) // *********************************************************************************************** // Si le mode courant "1" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      Sens_decalage_PORTD = false;                                                                                              // Définit l'indicateur du sens de décalage
      MaskLeds = 0b00000000;                                                                                                    // Initialise le masque des LED
      CompteurBinaire = 0;                                                                                                      // Initialise le compteur binaire
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (!Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de gauche à droite
    {
      PORTD = PORTD & 0b00000011;                                                                                               // Eteint toutes les LED du PORTD
      PORTB = PORTB & 0b11000000;                                                                                               // Eteint toutes les LED du PORTB
      
      MaskLeds = (CompteurBinaire << 2);                                                                                        // Calcule le masque des LED en décalant de 2 bits le compteur binaire pour ne pas modifier RX et TX
      
      PORTD |= MaskLeds;                                                                                                        // Allume la LED courante du PORTD
      PORTB &= 0b11000000;                                                                                                      // Réinitialise le PORTB sans modifier les bits "6" et "7"
      PORTB |= PORTD >> 2;                                                                                                      // Allume la LED courante du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      CompteurBinaire++;                                                                                                        // Incrémente le compteur binaire
      if (CompteurBinaire > 64) // -------------------------------------------------------------------------------------------- // Si le compteur binaire est supérieur à "64"
      {
        CompteurBinaire = 0b00000001;                                                                                           // Borne le compteur binaire
        Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                             // Inverse l'indicateur du sens de décalage
      }
    }
    else if (Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de droite à gauche
    {
      PORTD = PORTD & 0b00000011;                                                                                               // Eteint toutes les LED du PORTD
      PORTB = PORTB & 0b11000000;                                                                                               // Eteint toutes les LED du PORTB
      
      PORTD |= FonctionMiroir_PortD(CompteurBinaire << 2);                                                                      // Allume la LED courante de l'octet inversé
      PORTB &= 0b11000000;                                                                                                      // Réinitialise le PORTB sans modifier les bits "6" et "7"
      PORTB |= PORTD >> 2;                                                                                                      // Allume la LED courante du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      CompteurBinaire++;                                                                                                        // Incrémente le compteur binaire
      if (CompteurBinaire > 0b00111111) // ------------------------------------------------------------------------------------ // Si le compteur binaire est supérieur à "0b00111111"
      {
        CompteurBinaire = 0b00000000;                                                                                           // Borne le compteur binaire
        Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                             // Inverse l'indicateur du sens de décalage
        MaskLeds = MaskLeds = 0b00000000;;                                                                                      // Réinitialise le masque des LED
        
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED Double LED espacées de gauche à droite et de droite à gauche non bloquant **************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 2) // *********************************************************************************************** // Si le mode courant "2" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 5;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      MaskLeds = 0b11111100;                                                                                                    // Définit le masque des LED pour déterminer la LED allumée
      EtatInitial = 0b00010100;                                                                                                 // Définit l'état initial des LED
      etatLeds = EtatInitial;                                                                                                   // Définit l'état courant des LED égal à l'état initial des LED
      Sens_decalage_PORTD = false;                                                                                              // Définit l'indicateur du sens de décalage du PORTD
      DureeTempo_EtatBas = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état bas
      DureeTempo_EtatHaut = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état haut
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Initialise la temporisation à l'état bas en mode non bloquant
      t1DebutTempo_EtatHaut = millis();                                                                                         // Démarre la temporisation à l'état haut en mode non bloquant
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (!Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de gauche à droite
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume les LED courantes du PORTD
        PORTB |= (etatLeds & MaskLeds) >> 2;                                                                                    // Allume les LED courantes du PORTB
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Réinitialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = millis();                                                                                        // Démarre la temporisation à l'état bas en mode non bloquant
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint les LED courantes du PORTD
        PORTB &= ~(etatLeds & MaskLeds) >> 2;                                                                                   // Eteint les LED courantes du PORTB
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Réinitialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = 0ul;                                                                                             // Réinitialise la temporisation à l'état bas en mode non bloquant
      }
      
      if (t1DebutTempo_EtatHaut == 0ul && t1DebutTempo_EtatBas == 0ul) // ----------------------------------------------------- // Si les temporisations à l'état haut et à l'état bas sont arrêtées
      {
        etatLeds <<= 1;                                                                                                         // Décale l'état courant des LED à gauche de 1 position
        if ((etatLeds << 2 & MaskLeds) == 0) // ............................................................................... // Si l'état courant des LED a été décalé 3 fois à gauche de 1 position
        {
          etatLeds = 0b10100000;                                                                                                // Transfère l'état initial des LED dans l'état courant des LED
          Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                           // Inverse l'indicateur du sens de décalage
        }
        
        t1DebutTempo_EtatHaut = millis();                                                                                       // Démarre la temporisation à l'état haut en mode non bloquant
      }
    }
    else if (Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de droite à gauche
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume les LED courantes du PORTD
        PORTB |= (etatLeds & MaskLeds) >> 2;                                                                                    // Allume les LED courantes du PORTB
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Réinitialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = millis();                                                                                        // Démarre la temporisation à l'état bas en mode non bloquant
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint les LED courantes du PORTD
        PORTB &= ~(etatLeds & MaskLeds) >> 2;                                                                                   // Eteint les LED courantes du PORTB
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Réinitialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = 0ul;                                                                                             // Réinitialise la temporisation à l'état bas en mode non bloquant
      }
      
      if (t1DebutTempo_EtatHaut == 0ul && t1DebutTempo_EtatBas == 0ul) // ----------------------------------------------------- // Si les temporisations à l'état haut et à l'état bas sont arrêtées
      {
        etatLeds >>= 1;                                                                                                         // Décale l'état courant des LED à droite de 1 position
        if ((etatLeds >> 2 & MaskLeds) == 0) // ............................................................................... // Si l'état courant des LED a été décalé 3 fois à droite de 1 position
        {
          etatLeds = 0b00010100;                                                                                                // Transfère l'état initial des LED dans l'état courant des LED
          Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                           // Inverse l'indicateur du sens de décalage
          
          compteurAffichageModeCourant++;                                                                                       // Incrémente le compteur d'affichages du mode courant en mode automatique
        }
        
        t1DebutTempo_EtatHaut = millis();                                                                                       // Démarre la temporisation à l'état haut en mode non bloquant
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED Triple LED de gauche à droite et de droite à gauche non bloquant ***********************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 3) // *********************************************************************************************** // Si le mode courant "3" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 5;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      MaskLeds = 0b11111100;                                                                                                    // Définit le masque des LED pour déterminer la LED allumée
      EtatInitial = 0b00011100;                                                                                                 // Définit l'état initial des LED
      etatLeds = EtatInitial;                                                                                                   // Définit l'état courant des LED égal à l'état initial des LED
      masquePORTB = 0b00111000;                                                                                                 // Définit le masque du PORTB
      Sens_decalage_PORTD = false;                                                                                              // Définit l'indicateur du sens de décalage du PORTD
      DureeTempo_EtatBas = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état bas
      DureeTempo_EtatHaut = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état haut
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Initialise la temporisation à l'état bas en mode non bloquant
      t1DebutTempo_EtatHaut = millis();                                                                                         // Démarre la temporisation à l'état haut en mode non bloquant
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (!Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de gauche à droite
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume les LED courantes du PORTD
        PORTB = ((PORTD) | masquePORTB) & masquePORTB;                                                                          // Allume les LED courantes du PORTB
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Réinitialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = millis();                                                                                        // Démarre la temporisation à l'état bas en mode non bloquant
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint les LED courantes du PORTD
        PORTB = ~((PORTD) | masquePORTB) & masquePORTB;                                                                         // Eteint les LED courantes du PORTB
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Réinitialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = 0ul;                                                                                             // Réinitialise la temporisation à l'état bas en mode non bloquant
      }
      
      if (t1DebutTempo_EtatHaut == 0ul && t1DebutTempo_EtatBas == 0ul) // ----------------------------------------------------- // Si les temporisations à l'état haut et à l'état bas sont arrêtées
      {
        etatLeds <<= 1;                                                                                                         // Décale l'état courant des LED à gauche de 1 position
        masquePORTB >>= 1;                                                                                                      // Décale le masque du PORTB à droite de 1 position
        if ((etatLeds << 2 & MaskLeds) == 0) // ............................................................................... // Si l'état courant des LED a été décalé 3 fois à gauche de 1 position
        {
          etatLeds = 0b11100000;                                                                                                // Transfère l'état initial des LED dans l'état courant des LED
          masquePORTB = 0b00000111;                                                                                             // Redéfinit le masque du PORTB
          Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                           // Inverse l'indicateur du sens de décalage
        }
        
        t1DebutTempo_EtatHaut = millis();                                                                                       // Démarre la temporisation à l'état haut en mode non bloquant
      }
    }
    else if (Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de droite à gauche
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume les LED courantes du PORTD
        PORTB = ((PORTD) | masquePORTB) & masquePORTB;                                                                          // Allume les LED courantes du PORTB
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Réinitialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = millis();                                                                                        // Démarre la temporisation à l'état bas en mode non bloquant
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint les LED courantes du PORTD
        PORTB = ~((PORTD) | masquePORTB) & masquePORTB;                                                                         // Eteint les LED courantes du PORTB
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Réinitialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = 0ul;                                                                                             // Réinitialise la temporisation à l'état bas en mode non bloquant
      }
      
      if (t1DebutTempo_EtatHaut == 0ul && t1DebutTempo_EtatBas == 0ul) // ----------------------------------------------------- // Si les temporisations à l'état haut et à l'état bas sont arrêtées
      {
        etatLeds >>= 1;                                                                                                         // Décale l'état courant des LED à droite de 1 position
        masquePORTB <<= 1;                                                                                                      // Décale le masque du PORTB à gauche de 1 position
        if ((etatLeds >> 2 & MaskLeds) == 0) // ............................................................................... // Si l'état courant des LED a été décalé 3 fois à droite de 1 position
        {
          etatLeds = 0b00011100;                                                                                                // Transfère l'état initial des LED dans l'état courant des LED
          masquePORTB = 0b00111000;                                                                                             // Réinitialise le masque du PORTB
          Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                           // Inverse l'indicateur du sens de décalage
          
          compteurAffichageModeCourant++;                                                                                       // Incrémente le compteur d'affichages du mode courant en mode automatique
        }
        
        t1DebutTempo_EtatHaut = millis();                                                                                       // Démarre la temporisation à l'état haut en mode non bloquant
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED non bloquante ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 4) // *********************************************************************************************** // Si le mode courant "4" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 1;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      MaskLeds = 0b00000000;                                                                                                    // Définit le masque des LED pour déterminer la LED allumée
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    MaskLeds = 0b00000011;                                                                                                      // Définit le masque des LED initial => Allume les LED du bit b2 au bit b7
    for (int compteur_Decalage = 0; compteur_Decalage < 9; compteur_Decalage++) // ++++++++++++++++++++++++++++++++++++++++++++ // Boucle 9 fois
    {
      if (compteur_Decalage > 1) // ------------------------------------------------------------------------------------------- // Si le compteur de décalage est supérieur à "1" pour ne pas modifier RX et TX
      {
        PORTD = PORTD | MaskLeds;                                                                                               // Allume la LED courante définit par le masque des LED
        PORTB = PORTD >> 2;                                                                                                     // Allume la LED courante du PORTB
        //Serial.print(MaskLeds, BIN); Serial.print(" compteur_Decalage = "); Serial.println(compteur_Decalage);
        MaskLeds = MaskLeds | int(MaskLeds * 2 + 1);                                                                            // Calcule le masque des LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    MaskLeds = 0b01111111;                                                                                                      // Définit le masque des LED initial => Eteint les LED du bit b7 au bit b2
    for (int compteur_Decalage = 0; compteur_Decalage < 6; compteur_Decalage++) // ++++++++++++++++++++++++++++++++++++++++++++ // Boucle 5 fois
    {
      PORTD = PORTD & MaskLeds;                                                                                                 // Eteint la LED courante définit par le masque des LED
      PORTB = PORTD >> 2;                                                                                                       // Eteint la LED courante du PORTB
      //Serial.print(MaskLeds, BIN); Serial.print(" compteur_Decalage = "); Serial.println(compteur_Decalage); // Débug
      MaskLeds = MaskLeds & int(MaskLeds / 2);                                                                                  // Calcule le masque des LED
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    MaskLeds = 0b00000000;                                                                                                      // Définit le masque des LED initial => Allume les LED du bit b7 au bit b2
    int a = 1;                                                                                                                  // Déclare et définit le dénominateur pour le calcul du masque des LED
    for (int compteur_Decalage = 0; compteur_Decalage < 9; compteur_Decalage++) // ++++++++++++++++++++++++++++++++++++++++++++ // Boucle 6 fois
    {
      if (compteur_Decalage > 1) // ------------------------------------------------------------------------------------------- // Si le compteur de décalage est supérieur à "1" pour ne pas modifier RX et TX
      {
        PORTD = PORTD | MaskLeds;                                                                                               // Allume la LED courante définit par le masque des LED
        PORTB = PORTD >> 2;                                                                                                     // Allume la LED courante du PORTB
        //Serial.print(MaskLeds, BIN); Serial.print(" compteur_Decalage = "); Serial.println(compteur_Decalage); // Débug
        MaskLeds = MaskLeds | int(128 / a);                                                                                     // Calcule le masque des LED
        a = a * 2;                                                                                                              // Calcule le dénominateur pour le calcul du masque des LED
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    MaskLeds = 0b11111011;                                                                                                      // Définit le masque des LED initial => Eteint les LED du bit b2 au bit b7
    for (int compteur_Decalage = 0; compteur_Decalage < 6; compteur_Decalage++) // ++++++++++++++++++++++++++++++++++++++++++++ // Boucle 5 fois
    {
      PORTD = PORTD & MaskLeds;                                                                                                 // Eteint la LED courante définit par le masque des LED
      PORTB = PORTD >> 2;                                                                                                       // Eteint la LED courante du PORTB
      MaskLeds = MaskLeds << 1;                                                                                                 // Décale le masque des LED d'un bit vers la gauche
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    MaskLeds = 0b11111100;                                                                                                      // Définit le masque des LED initial => Clignotement des LED du bit b2 au bit b7
    for (int Compteur = 0; Compteur < 10; Compteur++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 10 fois
    {
      PORTD = PORTD ^ MaskLeds;                                                                                                 // Inverse les bits b2 à b7 du PORTD
      PORTB = PORTD >> 2;                                                                                                       // Inverse les bits b0 à b5 du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    MaskLeds = 0b00000000;                                                                                                      // Définit le masque des LED initial
    for (int ligne = 0; ligne < 7; ligne++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt les lignes du tableau "TableauSequencesLED1"
    {
      for (int colonne = 0; colonne < 6; colonne++) // ------------------------------------------------------------------------ // Parcourt les colonnes du tableau "TableauSequencesLED1"
      {
        if (TableauSequencesLED1[ligne][colonne] == 0) // ..................................................................... // Si la valeur du tableau de rangs "ligne" et "colonne" est égale à "0"
        {
               if (colonne == 0) {MaskLeds = MaskLeds & 0b11111011;}                                                            // Si la colonne courante est égale à 0 => Calcule le masque des LED
          else if (colonne == 1) {MaskLeds = MaskLeds & 0b11110111;}                                                            // Si la colonne courante est égale à 1 => Calcule le masque des LED
          else if (colonne == 2) {MaskLeds = MaskLeds & 0b11101111;}                                                            // Si la colonne courante est égale à 2 => Calcule le masque des LED
          else if (colonne == 3) {MaskLeds = MaskLeds & 0b11011111;}                                                            // Si la colonne courante est égale à 3 => Calcule le masque des LED
          else if (colonne == 4) {MaskLeds = MaskLeds & 0b10111111;}                                                            // Si la colonne courante est égale à 4 => Calcule le masque des LED
          else if (colonne == 5) {MaskLeds = MaskLeds & 0b01111111;}                                                            // Si la colonne courante est égale à 5 => Calcule le masque des LED
        }
        else if (TableauSequencesLED1[ligne][colonne] == 1) // ................................................................ // Si la valeur du tableau de rangs "ligne" et "colonne" est égale à "1"
        {
               if (colonne == 0) {MaskLeds = MaskLeds | 0b00000100;}                                                            // Si la colonne courante est égale à 0 => Calcule le masque des LED
          else if (colonne == 1) {MaskLeds = MaskLeds | 0b00001000;}                                                            // Si la colonne courante est égale à 1 => Calcule le masque des LED
          else if (colonne == 2) {MaskLeds = MaskLeds | 0b00010000;}                                                            // Si la colonne courante est égale à 2 => Calcule le masque des LED
          else if (colonne == 3) {MaskLeds = MaskLeds | 0b00100000;}                                                            // Si la colonne courante est égale à 3 => Calcule le masque des LED
          else if (colonne == 4) {MaskLeds = MaskLeds | 0b01000000;}                                                            // Si la colonne courante est égale à 4 => Calcule le masque des LED
          else if (colonne == 5) {MaskLeds = MaskLeds | 0b10000000;}                                                            // Si la colonne courante est égale à 5 => Calcule le masque des LED
        }
      }
      
      PORTD = MaskLeds;                                                                                                         // Transfère le masque des LED dans le PORTD
      PORTB = MaskLeds >> 2;                                                                                                    // Transfère le masque des LED dans le PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    MaskLeds = 0b10000100;                                                                                                      // Définit le masque des LED initial => Clignotement des LED
    for (int Compteur = 0; Compteur < 10; Compteur++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 10 fois
    {
      PORTD = PORTD ^ MaskLeds;                                                                                                 // Inverse les bits b2 et b7 du PORTD
      PORTB = PORTD >> 2;                                                                                                       // Inverse les bits b0 et b5 du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    MaskLeds = 0b01001000;                                                                                                      // Définit le masque des LED initial => Clignotement des LED
    for (int Compteur = 0; Compteur < 10; Compteur++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 10 fois
    {
      PORTD = PORTD ^ MaskLeds;                                                                                                 // Inverse les bits b3 et b6 du PORTD
      PORTB = PORTD >> 2;                                                                                                       // Inverse les bits b1 et b4 du PORTB
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    MaskLeds = 0b00110000;                                                                                                      // Définit le masque des LED initial => Clignotement des LED
    for (int Compteur = 0; Compteur < 10; Compteur++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 10 fois
    {
      PORTD = PORTD ^ MaskLeds;                                                                                                 // Inverse les bits b4 et b5 du PORTD
      PORTB = PORTD >> 2;                                                                                                       // Inverse les bits b2 et b3 du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    MaskLeds = 0b11001100;                                                                                                      // Définit le masque des LED initial => Clignotement des LED
    for (int Compteur = 0; Compteur < 10; Compteur++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 10 fois
    {
      PORTD = PORTD ^ MaskLeds;                                                                                                 // Inverse les bits b2, b3, b6, b7 du PORTD
      PORTB = PORTD >> 2;                                                                                                       // Inverse les bits b0, b1, b4, b5 du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    MaskLeds = 0b10000100;                                                                                                      // Définit le masque des LED initial => Clignotement des LED
    for (int Compteur = 0; Compteur < 10; Compteur++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 10 fois
    {
      PORTD = PORTD ^ MaskLeds;                                                                                                 // Inverse les bits b2 et b7 du PORTD
      PORTB ^= PORTD >> 2;                                                                                                      // Inverse les bits b0 et b5 du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    MaskLeds = 0b11111100;                                                                                                      // Définit le masque des LED initial => Clignotement des LED du bit b2 au bit b7
    for (int Compteur = 0; Compteur < 51; Compteur++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 51 fois
    {
      PORTD = PORTD ^ MaskLeds;                                                                                                 // Inverse les bits b2 à b7 du PORTD
      PORTB = (PORTD ^ MaskLeds) >> 2;                                                                                          // Inverse les bits b0 à b5 du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED aller-retour et flashs non bloquant ****************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 5) // *********************************************************************************************** // Si le mode courant "5" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      DureeTempo_EtatBas = 10ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état bas
      DureeTempo_EtatHaut = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état haut
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (float exposant = 2; exposant < 8; exposant++) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt l'exposant de "2" à "7"
    {
      double Decalage_Type_double = (pow(2.0, exposant));                                                                       // Déclare et calcule la variable "Decalage_Type_double"
      Decalage_Type_double = (int)(round(Decalage_Type_double));                                                                // Caste et arrondit à l'entier le plus proche la variable "Decalage_Type_double"
      
      int Decalage_Type_int = Decalage_Type_double;                                                                             // Transfère la variable "Decalage_Type_double" dans la variable de type int "Decalage_Type_int"
      
      PORTD = PORTD | (0b00000000 | Decalage_Type_int);                                                                         // Active le bit de rang "Decalage_Type_int" du PORTD
      PORTB = PORTD >> 2;                                                                                                       // Active le bit de rang "Decalage_Type_int" du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD = PORTD & (0b00000011 & Decalage_Type_int);                                                                         // Désactive le bit de rang "Decalage_Type_int" du PORTD
      PORTB = PORTD >> 2;                                                                                                       // Désactive le bit de rang "Decalage_Type_int" du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    MaskLeds = 0b11111100;                                                                                                      // Définit le masque des LED initial => Clignotement des LED du bit b2 au bit b7
    for (int Compteur = 0; Compteur < 10; Compteur++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 10 fois
    {
      PORTD = PORTD ^ MaskLeds;                                                                                                 // Inverse les bits b2 à b7 du PORTD
      PORTB = PORTD >> 2;                                                                                                       // Inverse les bits b0 à b5 du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (float exposant = 7; exposant >= 2; exposant--) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Parcourt l'exposant de "6" à "3"
    {
      double Decalage_Type_double = (pow(2.0, exposant));                                                                       // Déclare et calcule la variable "Decalage_Type_double"
      Decalage_Type_double = (int)(round(Decalage_Type_double));                                                                // Caste et arrondit à l'entier le plus proche la variable "Decalage_Type_double"
      
      int Decalage_Type_int = Decalage_Type_double;                                                                             // Transfère la variable "Decalage_Type_double" dans la variable de type int "Decalage_Type_int"
      
      PORTD = PORTD | (0b00000000 | Decalage_Type_int);                                                                         // Active le bit (LED) de rang "Decalage_Type_int" du PORTD
      PORTB = PORTD >> 2;                                                                                                       // Active le bit (LED) de rang "Decalage_Type_int" du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD = PORTD & (0b00000011 & Decalage_Type_int);                                                                         // Désactive le bit (LED) de rang "NbBit" du PORTD
      PORTB = PORTD >> 2;                                                                                                       // Désactive le bit (LED) de rang "NbBit" du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    MaskLeds = 0b11111100;                                                                                                      // Définit le masque des LED initial => Clignotement des LED du bit b2 au bit b7
    for (int Compteur = 0; Compteur < 20; Compteur++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 20 fois
    {
      PORTD = PORTD ^ MaskLeds;                                                                                                 // Inverse les bits b2 à b7 du PORTD
      PORTB ^= (PORTD >> 2);                                                                                                    // Inverse les bits b0 à b5 du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées non bloquante ****************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 6) // *********************************************************************************************** // Si le mode courant "6" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      pointeurLigneTableauSequences = 0;                                                                                        // Initialise le pointeur des lignes du tableau des séquences d'affichage des LED 2
      pointeurColonnesTableauSequences = 0;                                                                                     // Initialise le pointeur des colonnes du tableau des séquences d'affichage des LED 2
      Masque0 = 0b01111111;                                                                                                     // Définit le masque pour les valeurs du tableau des séquences d'affichage des LED 2 égales à "0"
      Masque1 = 0b10000000;                                                                                                     // Définit le masque pour les valeurs du tableau des séquences d'affichage des LED 2 égales à "1"
      MaskLeds = 0b00000000;                                                                                                    // Définit le masque des LED
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (TableauSequencesLED2[pointeurLigneTableauSequences][pointeurColonnesTableauSequences] == 0) // ++++++++++++++++++++++++ // Si la valeur courante du tableau des séquences d'affichage des LED 2 est égale à "0"
    {
      MaskLeds &= Masque0;                                                                                                      // Calcule le masque pour les valeurs du tableau des séquences d'affichage des LED 2 égales à "0"
      Masque0 = ~((~Masque0) >> 1);
      Masque1 = Masque1 >> 1;
    }
    else if (TableauSequencesLED2[pointeurLigneTableauSequences][pointeurColonnesTableauSequences] == 1) // +++++++++++++++++++ // Si la valeur courante du tableau des séquences d'affichage des LED 2 est égale à "1"
    {
      MaskLeds |= Masque1;                                                                                                      // Calcule le masque pour les valeurs du tableau des séquences d'affichage des LED 2 égales à "1"
      Masque0 = ~((~Masque0) >> 1);
      Masque1 = Masque1 >> 1;
    }
    
    PORTD = MaskLeds;                                                                                                           // Transfère le masque des LED vers le PORTD
    PORTB = MaskLeds >> 2;                                                                                                      // Transfère le masque inversé des LED vers le PORTB
    
    pointeurColonnesTableauSequences++;                                                                                         // Incrémente le pointeur des colonnes du tableau des séquences d'affichage des LED 2
    if (pointeurColonnesTableauSequences > 5) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Borne le pointeur des colonnes du tableau des séquences d'affichage des LED 2
    {
      pointeurColonnesTableauSequences = 0;                                                                                     // Réinitialise le pointeur des colonnes du tableau des séquences d'affichage des LED 2
      Masque0 = 0b01111111;                                                                                                     // Réinitialise le masque pour les valeurs du tableau des séquences d'affichage des LED 2 égales à "0"
      Masque1 = 0b10000000;                                                                                                     // Réinitialise le masque pour les valeurs du tableau des séquences d'affichage des LED 2 égales à "1"
      pointeurLigneTableauSequences++;                                                                                          // Incrémente le pointeur des lignes du tableau des séquences d'affichage des LED 2
      if (pointeurLigneTableauSequences > 23) // ------------------------------------------------------------------------------ // Borne le pointeur des lignes du tableau des séquences d'affichage des LED 2
      {
        pointeurColonnesTableauSequences = 0;                                                                                   // Réinitialise le pointeur des colonnes du tableau des séquences d'affichage des LED 2
        pointeurLigneTableauSequences = 0;                                                                                      // Réinitialise le pointeur des lignes du tableau des séquences d'affichage des LED 2
        
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED effet queue de comète non bloquant *****************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 7) // *********************************************************************************************** // Si le mode courant "7" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 5;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBrochesLED = 0;                                                                                            // Initialise le pointeur du tableau des broches LED
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 5; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], HIGH);                                                    // Allume la LED de rang "PointeurTableauBrochesLED_D2D7" du PORTD
      digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED - 1], HIGH);                                           // Allume la LED de rang "6 - PointeurTableauBrochesLED_D8D13 - 1" du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED <= 5; PointeurTableauBrochesLED++) // +++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], LOW);                                                     // Eteint la LED de rang "PointeurTableauBrochesLED_D2D7" du PORTD
      digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED - 1], LOW);                                            // Eteint la LED de rang "6 - PointeurTableauBrochesLED_D8D13 - 1" du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 5; PointeurTableauBrochesLED >= 0; PointeurTableauBrochesLED--) // +++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], HIGH);                                                    // Allume la LED de rang "PointeurTableauBrochesLED_D2D7" du PORTD
      digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED - 1], HIGH);                                           // Allume la LED de rang "6 - PointeurTableauBrochesLED_D8D13 - 1" du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 5; PointeurTableauBrochesLED >= 0; PointeurTableauBrochesLED--) // +++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], LOW);                                                     // Eteint la LED de rang "PointeurTableauBrochesLED_D2D7" du PORTD
      digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED - 1], LOW);                                            // Eteint la LED de rang "6 - PointeurTableauBrochesLED_D8D13 - 1" du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED non bloquante ***************************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 8) // *********************************************************************************************** // Si le mode courant "8" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 1;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      PointeurTableauBroches12LED = 0;                                                                                          // Initialise le pointeur du tableau des broches 12 LED
      PointeurTableauBroches12LEDTemp = 10;                                                                                     // Définit le pointeur de fin temporaire du tableau des broches 12 LED
      DureeHIGH_LOW = 80ul;                                                                                                     // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    DureeHIGH_LOW = 80ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (int compteurEffet = 0; compteurEffet < 2; compteurEffet++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 2 fois
    {
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED++) // --------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED++) // --------------- // Parcours le tableau des broches des 12 LED
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
    
    for (int compteurEffet = 0; compteurEffet < 2; compteurEffet++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 2 fois
    {
      for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // -------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // -------------- // Parcours le tableau des broches des 12 LED
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
    
    DureeHIGH_LOW = 80ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (int compteurEffet = 0; compteurEffet < 2; compteurEffet++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 2 fois
    {
      digitalWrite(TableauBroches12LED[0], HIGH);                                                                               // Allume la LED D2
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 11; PointeurTableauBroches12LED++) // --------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);                                               // Allume la LED de rang "PointeurTableauBroches12LED + 1"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      digitalWrite(TableauBroches12LED[11], LOW);                                                                               // Eteint la LED D13
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (int compteurEffet = 0; compteurEffet < 2; compteurEffet++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 2 fois
    {
      digitalWrite(TableauBroches12LED[11], HIGH);                                                                              // Allume la LED D13
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // -------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);                                               // Allume la LED de rang "PointeurTableauBroches12LED + 1"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      digitalWrite(TableauBroches12LED[0], LOW);                                                                                // Eteint la LED D2
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    DureeHIGH_LOW = 150ul;                                                                                                      // Définit la temporisation de la LED activée ou désactivée
    for (int compteurEffet = 0; compteurEffet < 9; compteurEffet++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 10 fois
    {
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 11; PointeurTableauBroches12LED += 2) // ------------ // Parcours le tableau des broches paires des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
      }
      for (PointeurTableauBroches12LED = 1; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED += 2) // ------------ // Parcours le tableau des broches impaires des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 11; PointeurTableauBroches12LED += 2) // ------------ // Parcours le tableau des broches paires des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
      }
      for (PointeurTableauBroches12LED = 1; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED += 2) // ------------ // Parcours le tableau des broches impaires des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      PORTD &= 0b00000011;                                                                                                      // Eteint toutes les LED du PORTD
      PORTB &= 0b11000000;                                                                                                      // Eteint toutes les LED du PORTB
    }
    
    DureeHIGH_LOW = 80ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (int compteurEffet = 0; compteurEffet < 2; compteurEffet++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 2 fois
    {
      digitalWrite(TableauBroches12LED[0], HIGH);                                                                               // Allume la LED D2
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[1], HIGH);                                                                               // Allume la LED D3
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 10; PointeurTableauBroches12LED++) // --------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], HIGH);                                               // Allume la LED de rang "PointeurTableauBroches12LED + 2"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      digitalWrite(TableauBroches12LED[10], LOW);                                                                               // Eteint la LED D12
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[11], LOW);                                                                               // Eteint la LED D13
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (int compteurEffet = 0; compteurEffet < 2; compteurEffet++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 2 fois
    {
      digitalWrite(TableauBroches12LED[11], HIGH);                                                                              // Allume la LED D13
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[10], HIGH);                                                                              // Allume la LED D12
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= 2; PointeurTableauBroches12LED--) // -------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 2], HIGH);                                               // Allume la LED de rang "PointeurTableauBroches12LED + 2"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      digitalWrite(TableauBroches12LED[1], LOW);                                                                                // Eteint la LED D3
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      digitalWrite(TableauBroches12LED[0], LOW);                                                                                // Eteint la LED D2
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    PointeurTableauBroches12LEDTemp = 10;                                                                                       // Réinitialise le pointeur de fin temporaire du tableau des broches 12 LED
    DureeHIGH_LOW = 70ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (int NombreLED = 0; NombreLED < 12; NombreLED++) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 12 fois (Nombre de LED)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcours le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = -1; PointeurTableauBroches12LED < PointeurTableauBroches12LEDTemp + 1; PointeurTableauBroches12LED++)
      {
        if (PointeurTableauBroches12LED == -1) // ............................................................................. // Si le pointeur du tableau des broches 12 LED est égal à "-1" (1ère boucle)
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);                                             // Allume la LED de rang "PointeurTableauBroches12LED + 1" (1ère LED)
        }
        else // ............................................................................................................... // Si le pointeur du tableau des broches 12 LED est supérieur à "-1" (1ère boucle effectuée)
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED"
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);                                             // Allume la LED de rang "PointeurTableauBroches12LED + 1"
        }
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      PointeurTableauBroches12LEDTemp--;                                                                                        // Décrémente le pointeur de fin temporaire du tableau des broches 12 LED
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    PORTD &= 0b00000011;                                                                                                        // Eteint toutes les LED du PORTD
    PORTB &= 0b11000000;                                                                                                        // Eteint toutes les LED du PORTB
    
    PointeurTableauBroches12LEDTemp = -1;                                                                                       // Réinitialise le pointeur de fin temporaire du tableau des broches 12 LED
    DureeHIGH_LOW = 70ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (int NombreLED = 0; NombreLED < 12; NombreLED++) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 12 fois (Nombre de LED)
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcours le tableau des broches des 12 LED
      for (PointeurTableauBroches12LED = 12; PointeurTableauBroches12LED > PointeurTableauBroches12LEDTemp + 1; PointeurTableauBroches12LED--)
      {
        if (PointeurTableauBroches12LED == 12) // ............................................................................. // Si le pointeur du tableau des broches 12 LED est égal à "12" (1ère boucle)
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);                                             // Allume la LED de rang "PointeurTableauBroches12LED - 1" (1ère LED)
        }
        else // ............................................................................................................... // Si le pointeur du tableau des broches 12 LED est inférieur à "12" (1ère boucle effectuée)
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED"
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);                                             // Allume la LED de rang "PointeurTableauBroches12LED - 1"
        }
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      PointeurTableauBroches12LEDTemp++;                                                                                        // Incrémente le pointeur de fin temporaire du tableau des broches 12 LED
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
    
    PORTD &= 0b00000011;                                                                                                        // Eteint toutes les LED du PORTD
    PORTB &= 0b11000000;                                                                                                        // Eteint toutes les LED du PORTB
    
    DureeHIGH_LOW = 70ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (int compteurEffet = 0; compteurEffet < 4; compteurEffet++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 4 fois
    {
      for (PointeurTableauBroches12LED = -1; PointeurTableauBroches12LED < 11; PointeurTableauBroches12LED++) // -------------- // Parcours le tableau des broches des 12 LED
      {
        if (PointeurTableauBroches12LED == -1) // ............................................................................. // Si le pointeur du tableau des broches 12 LED est égal à "-1" (1ère boucle)
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);                                             // Allume la LED de rang "PointeurTableauBroches12LED" (1ère LED)
        }
        else // ............................................................................................................... // Si le pointeur du tableau des broches 12 LED est supérieur à "-1" (1ère boucle effectuée)
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED"
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 1], HIGH);                                             // Allume la LED de rang "PointeurTableauBroches12LED + 1"
        }
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED > 0; PointeurTableauBroches12LED--) // --------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], HIGH);                                               // Allume la LED de rang "PointeurTableauBroches12LED - 1"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      PORTD &= 0b00000011;                                                                                                      // Eteint toutes les LED du PORTD
      PORTB &= 0b11000000;                                                                                                      // Eteint toutes les LED du PORTB
    }
    
    DureeHIGH_LOW = 60ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (int compteurEffet = 0; compteurEffet < 4; compteurEffet++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 4 fois
    {
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED++) // --------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED++) // --------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // -------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= 0; PointeurTableauBroches12LED--) // -------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Allume la LED de rang "PointeurTableauBroches12LED"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      PORTD &= 0b00000011;                                                                                                      // Eteint toutes les LED du PORTD
      PORTB &= 0b11000000;                                                                                                      // Eteint toutes les LED du PORTB
    }
    
    DureeHIGH_LOW = 70ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (int compteurEffet = 0; compteurEffet < 4; compteurEffet++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 4 fois
    {
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 6; PointeurTableauBroches12LED++) // ---------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[(6 - PointeurTableauBroches12LED) + 5], HIGH);                                         // Allume la LED de rang "(6 - PointeurTableauBroches12LED) + 5"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 6; PointeurTableauBroches12LED++) // ---------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[(6 - PointeurTableauBroches12LED) + 5], LOW);                                          // Eteint la LED de rang "(6 - PointeurTableauBroches12LED) + 5"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    DureeHIGH_LOW = 70ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (int compteurEffet = 0; compteurEffet < 4; compteurEffet++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 4 fois
    {
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 6; PointeurTableauBroches12LED++) // ---------------- // Parcours les 6 premières adresses du tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[(6 - PointeurTableauBroches12LED) + 5], HIGH);                                         // Allume la LED de rang "(6 - PointeurTableauBroches12LED) + 5"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      for (PointeurTableauBroches12LED = 6; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED++) // --------------- // Parcours les 6 dernières adresses du tableau des broches des 12 LED
      {
        if (PointeurTableauBroches12LED == 6) // .............................................................................. // Si le pointeur du tableau des broches 12 LED est égal à "6" (1ère boucle)
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED"
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1], LOW);                                              // Eteint la LED de rang "PointeurTableauBroches12LED - 1"
        }
        else // ............................................................................................................... // Si le pointeur du tableau des broches 12 LED est supérieur "6" (1ère boucle effectuée)
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED"
          digitalWrite(TableauBroches12LED[(5 - PointeurTableauBroches12LED) + 6], LOW);                                        // Eteint la LED de rang "(5 - PointeurTableauBroches12LED) + 6"
        }
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    DureeHIGH_LOW = 100ul;                                                                                                      // Définit la temporisation de la LED activée ou désactivée
    for (int compteurEffet = 0; compteurEffet < 16; compteurEffet++) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 16 fois
    {
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED++) // --------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED++) // --------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    DureeHIGH_LOW = 80ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (int compteurEffet = 0; compteurEffet < 8; compteurEffet++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 8 fois
    {
      for (PointeurTableauBroches12LED = -2; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED++) // -------------- // Parcours le tableau des broches des 12 LED
      {
        if (PointeurTableauBroches12LED == -2) // ............................................................................. // Si le pointeur du tableau des broches 12 LED est égal à "-2" (1ère boucle)
        {
          digitalWrite(TableauBroches12LED[0], HIGH);                                                                           // Allume la LED D2
        }
        else if (PointeurTableauBroches12LED == -1) // ........................................................................ // Si le pointeur du tableau des broches 12 LED est égal à "-1" (2ème boucle)
        {
          digitalWrite(TableauBroches12LED[1], HIGH);                                                                           // Allume la LED D3
        }
        else if (PointeurTableauBroches12LED == 10) // ........................................................................ // Si le pointeur du tableau des broches 12 LED est égal à "10" (10ème boucle)
        {
          digitalWrite(TableauBroches12LED[10], LOW);                                                                           // Eteint la LED D12
        }
        else if (PointeurTableauBroches12LED == 11) // ........................................................................ // Si le pointeur du tableau des broches 12 LED est égal à "11" (11ème boucle)
        {
          digitalWrite(TableauBroches12LED[11], LOW);                                                                           // Eteint la LED D13
        }
        else // ................................................................................................................// Si le pointeur du tableau des broches 12 LED est compris entre "0" et "9"
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], HIGH);                                             // Allume la LED de rang "PointeurTableauBroches12LED + 2"
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                  // Eteint la LED de rang "PointeurTableauBroches12LED"
        }
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      for (PointeurTableauBroches12LED = 11; PointeurTableauBroches12LED >= -2; PointeurTableauBroches12LED--) // ------------- // Parcours le tableau des broches des 12 LED
      {
        if (PointeurTableauBroches12LED == 11) // ............................................................................. // Si le pointeur du tableau des broches 12 LED est égal à "11" (1ère boucle)
        {
          digitalWrite(TableauBroches12LED[11], HIGH);                                                                          // Allume la LED D13
        }
        else if (PointeurTableauBroches12LED == 10) // ........................................................................ // Si le pointeur du tableau des broches 12 LED est égal à "10" (2ème boucle)
        {
          digitalWrite(TableauBroches12LED[10], HIGH);                                                                          // Allume la LED D12
        }
        else if (PointeurTableauBroches12LED == -1) // ........................................................................ // Si le pointeur du tableau des broches 12 LED est égal à "-1" (10ème boucle)
        {
          digitalWrite(TableauBroches12LED[1], LOW);                                                                            // Eteint la LED D3
        }
        else if (PointeurTableauBroches12LED == -2) // ........................................................................ // Si le pointeur du tableau des broches 12 LED est égal à "-2" (11ème boucle)
        {
          digitalWrite(TableauBroches12LED[0], LOW);                                                                            // Eteint la LED D2
        }
        else // ............................................................................................................... // Si le pointeur du tableau des broches 12 LED est compris entre "9" et "0"
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                 // Allume la LED de rang "PointeurTableauBroches12LED"
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 2], LOW);                                              // Eteint la LED de rang "PointeurTableauBroches12LED + 2"
        }
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    DureeHIGH_LOW = 90ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (int compteurEffet = 0; compteurEffet < 8; compteurEffet++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 8 fois
    {
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 6; PointeurTableauBroches12LED++) // ---------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], HIGH);                                           // Allume la LED de rang "6 - PointeurTableauBroches12LED + 5"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], LOW);                                            // Eteint la LED de rang "6 - PointeurTableauBroches12LED + 5"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      for (PointeurTableauBroches12LED = 7; PointeurTableauBroches12LED < 11; PointeurTableauBroches12LED++) // --------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[4 - PointeurTableauBroches12LED + 7], HIGH);                                           // Allume la LED de rang "4 - PointeurTableauBroches12LED + 7"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
        
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[4 - PointeurTableauBroches12LED + 7], LOW);                                            // Allume la LED de rang "4 - PointeurTableauBroches12LED + 7"
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    DureeHIGH_LOW = 80ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (int compteurEffet = 0; compteurEffet < 8; compteurEffet++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 8 fois
    {
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 6; PointeurTableauBroches12LED++) // ---------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], HIGH);                                           // Allume la LED de rang "6 - PointeurTableauBroches12LED + 5"
        
        if (PointeurTableauBroches12LED > 0) // ............................................................................... // Si le pointeur du tableau des broches 12 LED est supérieur à "0"
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1 ], LOW);                                             // Eteint la LED de rang "PointeurTableauBroches12LED - 1 "
          digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5 + 1], LOW);                                      // Eteint la LED de rang "6 - PointeurTableauBroches12LED + 5 + 1"
        }
        else if (PointeurTableauBroches12LED == 0) // ......................................................................... // Si le pointeur du tableau des broches 12 LED est égal à "0"
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5 ], LOW);                                             // Eteint la LED de rang "PointeurTableauBroches12LED + 1"
          digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED], LOW);                                              // Eteint la LED de rang "6 - PointeurTableauBroches12LED"
        }
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    DureeHIGH_LOW = 80ul;                                                                                                       // Définit la temporisation de la LED activée ou désactivée
    for (int compteurEffet = 0; compteurEffet < 8; compteurEffet++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 8 fois
    {
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 6; PointeurTableauBroches12LED++) // ---------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], HIGH);                                           // Allume la LED de rang "6 - PointeurTableauBroches12LED + 5"
        
        if (PointeurTableauBroches12LED > 0) // ............................................................................... // Si le pointeur du tableau des broches 12 LED est supérieur à "0"
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1 ], LOW);                                             // Eteint la LED de rang "PointeurTableauBroches12LED - 1 "
          digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5 + 1], LOW);                                      // Eteint la LED de rang "6 - PointeurTableauBroches12LED + 5 + 1"
        }
        else if (PointeurTableauBroches12LED == 0) // ......................................................................... // Si le pointeur du tableau des broches 12 LED est égal à "0"
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED + 5 ], LOW);                                             // Eteint la LED de rang "PointeurTableauBroches12LED + 1"
          digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED], LOW);                                              // Eteint la LED de rang "6 - PointeurTableauBroches12LED"
        }
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
      
      for (PointeurTableauBroches12LED = 6; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED++) // --------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
        digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5], HIGH);                                           // Allume la LED de rang "6 - PointeurTableauBroches12LED + 5"
        
        if (PointeurTableauBroches12LED == 11) // ............................................................................. // Si le pointeur du tableau des broches 12 LED est égal à "11"
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1 ], LOW);                                             // Eteint la LED de rang "PointeurTableauBroches12LED - 1"
          digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 6], LOW);                                          // Eteint la LED de rang "6 - PointeurTableauBroches12LED + 6"
        }
        else if (PointeurTableauBroches12LED > 6) // .......................................................................... // Si le pointeur du tableau des broches 12 LED est supérieur à "6"
        {
          digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED - 1 ], LOW);                                             // Eteint la LED de rang "PointeurTableauBroches12LED - 1 "
          digitalWrite(TableauBroches12LED[6 - PointeurTableauBroches12LED + 5 + 1], LOW);                                      // Eteint la LED de rang "6 - PointeurTableauBroches12LED + 5 + 1"
        }
        
        Fonction_Temporisation(DureeHIGH_LOW);                                                                                  // Appelle la fonction de temporisation
        if (SortieModeCourant) // ............................................................................................. // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
        {
          SortieModeCourant = false;                                                                                            // Réinitialise l'indicateur de sortie du mode courant
          return;                                                                                                               // Retour début loop()
        }
      }
    }
    
    DureeHIGH_LOW = 100ul;                                                                                                      // Définit la temporisation de la LED activée ou désactivée
    for (int compteurEffet = 0; compteurEffet < 16; compteurEffet++) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 16 fois
    {
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 6; PointeurTableauBroches12LED++) // ---------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (PointeurTableauBroches12LED = 0; PointeurTableauBroches12LED < 6; PointeurTableauBroches12LED++) // ---------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
      }
      
      for (PointeurTableauBroches12LED = 6; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED++) // --------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], HIGH);                                                   // Allume la LED de rang "PointeurTableauBroches12LED"
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      for (PointeurTableauBroches12LED = 6; PointeurTableauBroches12LED < 12; PointeurTableauBroches12LED++) // --------------- // Parcours le tableau des broches des 12 LED
      {
        digitalWrite(TableauBroches12LED[PointeurTableauBroches12LED], LOW);                                                    // Eteint la LED de rang "PointeurTableauBroches12LED"
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencée non bloquante *****************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 9) // *********************************************************************************************** // Si le mode courant "9" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 5;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      pointeurLigneTableauSequences = 0;                                                                                        // Initialise le pointeur des lignes du tableau des séquences d'affichage des LED 2
      pointeurColonnesTableauSequences = 0;                                                                                     // Initialise le pointeur des colonnes du tableau des séquences d'affichage des LED 2
      MaskLeds = 0b00000000;                                                                                                    // Définit le masque des LED
      DureeHIGH_LOW = 100ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (pointeurColonnesTableauSequences = 5; pointeurColonnesTableauSequences >= 0; pointeurColonnesTableauSequences--) // ++ // Parcours le tableau des séquences d'affichage des LED 2
    {
      if (TableauSequencesLED2[pointeurLigneTableauSequences][pointeurColonnesTableauSequences] == 0) // ---------------------- // Si la valeur courante du tableau des séquences d'affichage des LED 2 est égale à "0"
      {
        bitClear(MaskLeds, pointeurColonnesTableauSequences);                                                                   // Positionne le bit de rang "pointeurColonnesTableauSequences" du masque des LED à "0"
      }
      else if (TableauSequencesLED2[pointeurLigneTableauSequences][pointeurColonnesTableauSequences] == 1) // ----------------- // Si la valeur courante du tableau des séquences d'affichage des LED 2 est égale à "1"
      {
        bitSet(MaskLeds, pointeurColonnesTableauSequences);                                                                     // Positionne le bit de rang "pointeurColonnesTableauSequences" du masque des LED à "1"
      }
    }
    
    PORTD &= 0b00000011;                                                                                                        // Eteint toutes les LED du PORTD
    PORTD |= MaskLeds << 2;                                                                                                     // Transfère le masque des LED vers le PORTD sans modifier RX et TX
    
    PORTB &= 0b11000000;                                                                                                        // Eteint toutes les LED du PORTB
    PORTB |= ~(MaskLeds);                                                                                                       // Transfère le masque inversé des LED vers le PORTB sans modifier b6 et b7
    
    pointeurLigneTableauSequences++;                                                                                            // Incrémente le pointeur des lignes du tableau des séquences d'affichage des LED 2
    if (pointeurLigneTableauSequences > 11) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Borne le pointeur des lignes du tableau des séquences d'affichage des LED 2
    {
      pointeurLigneTableauSequences = 0;                                                                                        // Réinitialise le pointeur des lignes du tableau des séquences d'affichage des LED 2
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED séquencée non bloquant *****************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 10) // ********************************************************************************************** // Si le mode courant "10" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 5;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      pointeurLigneTableauSequences = 0;                                                                                        // Initialise le pointeur des lignes du tableau des séquences d'affichage des LED 2
      pointeurColonnesTableauSequences = 0;                                                                                     // Initialise le pointeur des colonnes du tableau des séquences d'affichage des LED 2
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (pointeurLigneTableauSequences = 0; pointeurLigneTableauSequences < 15; pointeurLigneTableauSequences++) // +++++++++++ // Parcourt les lignes du tableau "TableauSequencesLED3"
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt les colonnes du tableau "TableauSequencesLED3"
      for (pointeurColonnesTableauSequences = 0; pointeurColonnesTableauSequences < 12; pointeurColonnesTableauSequences++)
      {
        if (TableauSequencesLED3[pointeurLigneTableauSequences][pointeurColonnesTableauSequences] == 0) // .................... // Si la valeur courante du tableau "TableauSequencesLED3" est égale à "0"
        {
          digitalWrite(TableauBroches12LED[pointeurColonnesTableauSequences], LOW);                                             // Eteint la LED courante
        }
        else if (TableauSequencesLED3[pointeurLigneTableauSequences][pointeurColonnesTableauSequences] == 1) // ............... // Si la valeur courante du tableau "TableauSequencesLED3" est égale à "1"
        {
          digitalWrite(TableauBroches12LED[pointeurColonnesTableauSequences], HIGH);                                            // Allume la LED courante
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
//*** Chenillard 12 LED séquencée non bloquant *****************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 11) // ********************************************************************************************** // Si le mode courant "11" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 5;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      pointeurLigneTableauSequences = 0;                                                                                        // Initialise le pointeur des lignes du tableau des séquences d'affichage des LED 2
      pointeurColonnesTableauSequences = 0;                                                                                     // Initialise le pointeur des colonnes du tableau des séquences d'affichage des LED 2
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (pointeurLigneTableauSequences = 0; pointeurLigneTableauSequences < 15; pointeurLigneTableauSequences++) // +++++++++++ // Parcourt les lignes du tableau "TableauSequencesLED3"
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt les colonnes du tableau "TableauSequencesLED3"
      for (pointeurColonnesTableauSequences = 0; pointeurColonnesTableauSequences < 12; pointeurColonnesTableauSequences++)
      {
        if (TableauSequencesLED3[pointeurLigneTableauSequences][pointeurColonnesTableauSequences] == 0) // .................... // Si la valeur courante du tableau "TableauSequencesLED3" est égale à "0"
        {
          digitalWrite(TableauBroches12LED[pointeurColonnesTableauSequences], HIGH);                                            // Allume la LED courante
        }
        else if (TableauSequencesLED3[pointeurLigneTableauSequences][pointeurColonnesTableauSequences] == 1) // ............... // Si la valeur courante du tableau "TableauSequencesLED3" est égale à "1"
        {
          digitalWrite(TableauBroches12LED[pointeurColonnesTableauSequences], LOW);                                             // Eteint la LED courante
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
//*** Chenillard 12 LED séquencée non bloquant *****************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 12) // ********************************************************************************************** // Si le mode courant "12" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 5;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      pointeurLigneTableauSequences = 0;                                                                                        // Initialise le pointeur des lignes du tableau des séquences d'affichage des LED 2
      pointeurColonnesTableauSequences = 0;                                                                                     // Initialise le pointeur des colonnes du tableau des séquences d'affichage des LED 2
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (pointeurLigneTableauSequences = 0; pointeurLigneTableauSequences < 20; pointeurLigneTableauSequences++) // +++++++++++ // Parcourt les lignes du tableau "TableauSequencesLED4"
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt les colonnes du tableau "TableauSequencesLED4"
      for (pointeurColonnesTableauSequences = 0; pointeurColonnesTableauSequences < 12; pointeurColonnesTableauSequences++)
      {
        if (TableauSequencesLED4[pointeurLigneTableauSequences][pointeurColonnesTableauSequences] == 0) // .................... // Si la valeur courante du tableau "TableauSequencesLED4" est égale à "0"
        {
          digitalWrite(TableauBroches12LED[pointeurColonnesTableauSequences], LOW);                                             // Eteint la LED courante
        }
        else if (TableauSequencesLED4[pointeurLigneTableauSequences][pointeurColonnesTableauSequences] == 1) // ............... // Si la valeur courante du tableau "TableauSequencesLED4" est égale à "1"
        {
          digitalWrite(TableauBroches12LED[pointeurColonnesTableauSequences], HIGH);                                            // Allume la LED courante
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
//*** Chenillard 12 LED séquencée non bloquant *****************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 13) // ********************************************************************************************** // Si le mode courant "13" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 5;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      pointeurLigneTableauSequences = 0;                                                                                        // Initialise le pointeur des lignes du tableau des séquences d'affichage des LED 2
      pointeurColonnesTableauSequences = 0;                                                                                     // Initialise le pointeur des colonnes du tableau des séquences d'affichage des LED 2
      DureeHIGH_LOW = 80ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (pointeurLigneTableauSequences = 0; pointeurLigneTableauSequences < 20; pointeurLigneTableauSequences++) // +++++++++++ // Parcourt les lignes du tableau "TableauSequencesLED4"
    {
      // ---------------------------------------------------------------------------------------------------------------------- // Parcourt les colonnes du tableau "TableauSequencesLED4"
      for (pointeurColonnesTableauSequences = 0; pointeurColonnesTableauSequences < 12; pointeurColonnesTableauSequences++)
      {
        if (TableauSequencesLED4[pointeurLigneTableauSequences][pointeurColonnesTableauSequences] == 0) // .................... // Si la valeur courante du tableau "TableauSequencesLED4" est égale à "0"
        {
          digitalWrite(TableauBroches12LED[pointeurColonnesTableauSequences], HIGH);                                            // Allume la LED courante
        }
        else if (TableauSequencesLED4[pointeurLigneTableauSequences][pointeurColonnesTableauSequences] == 1) // ............... // Si la valeur courante du tableau "TableauSequencesLED4" est égale à "1"
        {
          digitalWrite(TableauBroches12LED[pointeurColonnesTableauSequences], LOW);                                             // Eteint la LED courante
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
//*** Clignotement 12 LED simultané de 4 LED inversées avec 8 autres LED avec un tableau de séquences de LED non bloquant **************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 14) // ********************************************************************************************** // Si le mode courant "14" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 100;                                                                                 // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      pointeur_TableauSequenceur = 0;                                                                                           // Initialise le pointeur du tableau de séquences
      Duree_EntreSequences = 0;                                                                                                 // Initialise la temporisation entre les séquences
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() > Duree_EntreSequences) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation entre les séquences est écoulée
    {
      Duree_EntreSequences = millis() + DureeHIGH_LOW;                                                                          // Redémarre la temporisation entre les séquences
      
      digitalWrite(BrocheLED_D2, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D2 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D3, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                               // Allume ou éteint la LED D3 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D4, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D4 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D5, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D5 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D6, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                               // Allume ou éteint la LED D6 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D7, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D7 en fonction de l'état du contenu du pointeur du tableau de séquences
      
      digitalWrite(BrocheLED_D8,  !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                             // Allume ou éteint la LED D8 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D9,  Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D9 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D10, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                             // Allume ou éteint la LED D10 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D11, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                             // Allume ou éteint la LED D11 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D12, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D12 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D13, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                             // Allume ou éteint la LED D13 en fonction de l'état du contenu du pointeur du tableau de séquences
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
      
      pointeur_TableauSequenceur = (pointeur_TableauSequenceur + 1) % Nombre_Sequences;                                         // Borne le tableau de séquences
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED simultané de 6 LED inversées avec 6 autres LED avec un tableau de séquences de LED non bloquant **************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 15) // ********************************************************************************************** // Si le mode courant "15" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 100;                                                                                 // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      pointeur_TableauSequenceur = 0;                                                                                           // Initialise le pointeur du tableau de séquences
      Duree_EntreSequences = 0;                                                                                                 // Initialise la temporisation entre les séquences
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() > Duree_EntreSequences) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation entre les séquences est écoulée
    {
      Duree_EntreSequences = millis() + DureeHIGH_LOW;                                                                          // Redémarre la temporisation entre les séquences
      
      digitalWrite(BrocheLED_D2, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                               // Allume ou éteint la LED D2 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D3, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                               // Allume ou éteint la LED D3 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D4, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                               // Allume ou éteint la LED D4 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D5, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D5 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D6, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D6 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D7, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D7 en fonction de l'état du contenu du pointeur du tableau de séquences
      
      digitalWrite(BrocheLED_D8,  !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                             // Allume ou éteint la LED D8 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D9,  !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                             // Allume ou éteint la LED D9 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D10, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                             // Allume ou éteint la LED D10 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D11, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D11 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D12, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D12 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D13, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D13 en fonction de l'état du contenu du pointeur du tableau de séquences
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
      
      pointeur_TableauSequenceur = (pointeur_TableauSequenceur + 1) % Nombre_Sequences;                                         // Borne le tableau de séquences
    }
  }
//**************************************************************************************************************************************************************************
//***  Animation 12 LED Flashs alternés entre le PORTD et le PORTB non bloquante ******************************************************************************************* 
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
      MaskLeds = 0b11111100;                                                                                                    // Définit le masque des LED pour déterminer la LED allumée
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (CompteurFlash = 0; CompteurFlash < 2; CompteurFlash++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 2 fois
    {
      MaskLeds = 0b11111100;                                                                                                    // Définit le masque des LED
      
      PORTD |= MaskLeds;                                                                                                        // Eteint toutes les LED du PORTD
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      MaskLeds = 0b00000011;                                                                                                    // Définit le masque des LED
      
      PORTD &= MaskLeds;                                                                                                        // Eteint toutes les LED du PORTD
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (CompteurFlash = 0; CompteurFlash < 2; CompteurFlash++) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 2 fois
    {
      MaskLeds = 0b00111111;                                                                                                    // Définit le masque des LED
      
      PORTB |= MaskLeds;                                                                                                        // Eteint toutes les LED du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
      
      MaskLeds = 0b11000000;                                                                                                    // Définit le masque des LED
      
      PORTB &= MaskLeds;                                                                                                        // Eteint toutes les LED du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED Triple LED de gauche à droite et de droite à gauche non bloquant ***********************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 17) // ********************************************************************************************** // Si le mode courant "17" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      masquePORTD = 0b10000000;                                                                                                 // Définit le masque du PORTD
      masquePORTB = 0b00000001;                                                                                                 // Définit le masque du PORTB
      CompteurDecalageEtatLeds = 0;                                                                                             // Initialise le compteur de décalage de l'état courant des LED
      DureeHIGH_LOW = 120ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (int compteurEffet = 0; compteurEffet < 13; compteurEffet++) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 13 fois
    {
      PORTD |= masquePORTD;                                                                                                     // Allume la LED courante du PORTD
      masquePORTD >>= 1;                                                                                                        // Décale le masque du PORTD d'une position à droite
      
      CompteurDecalageEtatLeds++;                                                                                               // Incrémente le compteur de décalage de l'état courant des LED
      if (CompteurDecalageEtatLeds > 6) // ------------------------------------------------------------------------------------ // Si le masque du PORTD a été décalé 5 fois
      {
        PORTD &= 0b00000011;                                                                                                    // Eteint toutes les LED du PORTD
        masquePORTD = 0b10000000;                                                                                               // Réinitialise le masque du PORTD
        CompteurDecalageEtatLeds = 0;                                                                                           // Réinitialise le compteur de décalage de l'état courant des LED
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    PORTD &= 0b00000011;                                                                                                        // Eteint toutes les LED du PORTD
    masquePORTD = 0b10000000;                                                                                                   // Réinitialise le masque du PORTB
    CompteurDecalageEtatLeds = 0;                                                                                               // Réinitialise le compteur de décalage de l'état courant des LED
    
    for (int compteurEffet = 0; compteurEffet < 13; compteurEffet++) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 13 fois
    {
      PORTB |= masquePORTB;                                                                                                     // Allume la LED courante du PORTB
      masquePORTB <<= 1;                                                                                                        // Décale le masque du PORTB d'une position à droite
      
      CompteurDecalageEtatLeds++;                                                                                               // Incrémente le compteur de décalage de l'état courant des LED
      if (CompteurDecalageEtatLeds > 6) // ------------------------------------------------------------------------------------ // Si le masque du PORTD a été décalé 5 fois
      {
        PORTB &= 0b11000000;                                                                                                    // Eteint toutes les LED du PORTB
        masquePORTB = 0b00000001;                                                                                               // Réinitialise le masque du PORTB
        CompteurDecalageEtatLeds = 0;                                                                                           // Réinitialise le compteur de décalage de l'état courant des LED
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    PORTB &= 0b11000000;                                                                                                        // Eteint toutes les LED du PORTB
    masquePORTB = 0b00000001;                                                                                                   // Réinitialise le masque du PORTB
    CompteurDecalageEtatLeds = 0;                                                                                               // Réinitialise le compteur de décalage de l'état courant des LED
    
    for (int compteurEffet = 0; compteurEffet < 13; compteurEffet++) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Boucle 13 fois
    {
      PORTD |= masquePORTD;                                                                                                     // Allume la LED courante du PORTD
      masquePORTD >>= 1;                                                                                                        // Décale le masque du PORTD d'une position à droite
      PORTB |= masquePORTB;                                                                                                     // Allume la LED courante du PORTB
      masquePORTB <<= 1;                                                                                                        // Décale le masque du PORTB d'une position à droite
      
      CompteurDecalageEtatLeds++;                                                                                               // Incrémente le compteur de décalage de l'état courant des LED
      if (CompteurDecalageEtatLeds > 6) // ------------------------------------------------------------------------------------ // Si le masque du PORTD a été décalé 5 fois
      {
        PORTD &= 0b00000011;                                                                                                    // Eteint toutes les LED du PORTD
        masquePORTD = 0b10000000;                                                                                               // Réinitialise le masque du PORTD
        PORTB &= 0b11000000;                                                                                                    // Eteint toutes les LED du PORTB
        masquePORTB = 0b00000001;                                                                                               // Réinitialise le masque du PORTB
        CompteurDecalageEtatLeds = 0;                                                                                           // Réinitialise le compteur de décalage de l'état courant des LED
      }
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    PORTD &= 0b00000011;                                                                                                        // Eteint toutes les LED du PORTD
    masquePORTD = 0b10000000;                                                                                                   // Réinitialise le masque du PORTD
    CompteurDecalageEtatLeds = 0;                                                                                               // Réinitialise le compteur de décalage de l'état courant des LED
    PORTB &= 0b11000000;                                                                                                        // Eteint toutes les LED du PORTB
    masquePORTB = 0b00000001;                                                                                                   // Réinitialise le masque du PORTB
    CompteurDecalageEtatLeds = 0;                                                                                               // Réinitialise le compteur de décalage de l'état courant des LED
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED non bloquant ***************************************************************************************************************************************
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
      PointeurTableauBrochesLED = 0;                                                                                            // Initialise le pointeur du tableau des broches LED
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED < 3; PointeurTableauBrochesLED++) // ++++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], HIGH);                                                    // Allume la LED de rang "PointeurTableauBrochesLED_D2D7" du PORTD
      digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED - 4], HIGH);                                           // Allume la LED de rang "6 - PointeurTableauBrochesLED_D8D13 - 4" du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 0; PointeurTableauBrochesLED < 3; PointeurTableauBrochesLED++) // ++++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], LOW);                                                     // Eteint la LED de rang "PointeurTableauBrochesLED_D2D7" du PORTD
      digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED - 4], LOW);                                            // Eteint la LED de rang "6 - PointeurTableauBrochesLED_D8D13 - 4" du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 3; PointeurTableauBrochesLED < 6; PointeurTableauBrochesLED++) // ++++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], HIGH);                                                    // Allume la LED de rang "PointeurTableauBrochesLED_D2D7" du PORTD
      digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED - 4], HIGH);                                           // Allume la LED de rang "6 - PointeurTableauBrochesLED - 4" du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
    
    for (PointeurTableauBrochesLED = 3; PointeurTableauBrochesLED < 6; PointeurTableauBrochesLED++) // ++++++++++++++++++++++++ // Parcourt le tableau des broches
    {
      digitalWrite(TableauBrochesLED_D2D7[PointeurTableauBrochesLED], LOW);                                                     // Eteint la LED de rang "PointeurTableauBrochesLED_D2D7" du PORTD
      digitalWrite(TableauBrochesLED_D8D13[6 - PointeurTableauBrochesLED - 4], LOW);                                            // Eteint la LED de rang "6 - PointeurTableauBrochesLED - 4" du PORTB
      
      Fonction_Temporisation(DureeHIGH_LOW);                                                                                    // Appelle la fonction de temporisation
      if (SortieModeCourant) // ----------------------------------------------------------------------------------------------- // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
      {
        SortieModeCourant = false;                                                                                              // Réinitialise l'indicateur de sortie du mode courant
        return;                                                                                                                 // Retour début loop()
      }
    }
  }
//**************************************************************************************************************************************************************************
//*** Animation 12 LED séquencées non bloquante ****************************************************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 19) // ********************************************************************************************** // Si le mode courant "19" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 3;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      pointeurLigneTableauSequences = 0;                                                                                        // Initialise le pointeur des lignes du tableau des séquences d'affichage des LED 2
      pointeurColonnesTableauSequences = 0;                                                                                     // Initialise le pointeur des colonnes du tableau des séquences d'affichage des LED 2
      Masque0 = 0b01111111;                                                                                                     // Définit le masque pour les valeurs du tableau des séquences d'affichage des LED 2 égales à "0"
      Masque1 = 0b10000000;                                                                                                     // Définit le masque pour les valeurs du tableau des séquences d'affichage des LED 2 égales à "1"
      MaskLeds = 0b00000000;                                                                                                    // Définit le masque des LED
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (TableauSequencesLED2[pointeurLigneTableauSequences][pointeurColonnesTableauSequences] == 0) // ++++++++++++++++++++++++ // Si la valeur courante du tableau des séquences d'affichage des LED 2 est égale à "0"
    {
      MaskLeds &= Masque0;                                                                                                      // Calcule le masque pour les valeurs du tableau des séquences d'affichage des LED 2 égales à "0"
      Masque0 = ~((~Masque0) >> 1);
      Masque1 = Masque1 >> 1;
    }
    else if (TableauSequencesLED2[pointeurLigneTableauSequences][pointeurColonnesTableauSequences] == 1) // +++++++++++++++++++ // Si la valeur courante du tableau des séquences d'affichage des LED 2 est égale à "1"
    {
      MaskLeds |= Masque1;                                                                                                      // Calcule le masque pour les valeurs du tableau des séquences d'affichage des LED 2 égales à "1"
      Masque0 = ~((~Masque0) >> 1);
      Masque1 = Masque1 >> 1;
    }
    
    PORTD = MaskLeds;                                                                                                           // Transfère le masque des LED vers le PORTD
    PORTB = ~MaskLeds >> 2;                                                                                                     // Transfère le masque inversé des LED vers le PORTB
    
    pointeurColonnesTableauSequences++;                                                                                         // Incrémente le pointeur des colonnes du tableau des séquences d'affichage des LED 2
    if (pointeurColonnesTableauSequences > 5) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Borne le pointeur des colonnes du tableau des séquences d'affichage des LED 2
    {
      pointeurColonnesTableauSequences = 0;                                                                                     // Réinitialise le pointeur des colonnes du tableau des séquences d'affichage des LED 2
      Masque0 = 0b01111111;                                                                                                     // Réinitialise le masque pour les valeurs du tableau des séquences d'affichage des LED 2 égales à "0"
      Masque1 = 0b10000000;                                                                                                     // Réinitialise le masque pour les valeurs du tableau des séquences d'affichage des LED 2 égales à "1"
      pointeurLigneTableauSequences++;                                                                                          // Incrémente le pointeur des lignes du tableau des séquences d'affichage des LED 2
      if (pointeurLigneTableauSequences > 23) // ------------------------------------------------------------------------------ // Borne le pointeur des lignes du tableau des séquences d'affichage des LED 2
      {
        pointeurColonnesTableauSequences = 0;                                                                                   // Réinitialise le pointeur des colonnes du tableau des séquences d'affichage des LED 2
        pointeurLigneTableauSequences = 0;                                                                                      // Réinitialise le pointeur des lignes du tableau des séquences d'affichage des LED 2
        
        compteurAffichageModeCourant++;                                                                                         // Incrémente le compteur d'affichages du mode courant en mode automatique
      }
    }
    
    Fonction_Temporisation(DureeHIGH_LOW);                                                                                      // Appelle la fonction de temporisation
    if (SortieModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si l'indicateur de sortie du mode courant est activé via une des fonctions d'interruption de changement de broche des boutons poussoirs
    {
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      return;                                                                                                                   // Retour début loop()
    }
  }
//**************************************************************************************************************************************************************************
//*** Clignotement 12 LED simultané de 6 LED inversées avec 6 autres LED avec un tableau de séquences de LED non bloquant **************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 20) // ********************************************************************************************** // Si le mode courant "20" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 100;                                                                                 // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      DureeHIGH_LOW = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Définit la temporisation de la LED activée ou désactivée
      pointeur_TableauSequenceur = 0;                                                                                           // Initialise le pointeur du tableau de séquences
      Duree_EntreSequences = 0;                                                                                                 // Initialise la temporisation entre les séquences
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (millis() > Duree_EntreSequences) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si la temporisation entre les séquences est écoulée
    {
      Duree_EntreSequences = millis() + DureeHIGH_LOW;                                                                          // Redémarre la temporisation entre les séquences
      
      digitalWrite(BrocheLED_D2, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D2 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D3, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                               // Allume ou éteint la LED D3 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D4, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D4 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D5, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                               // Allume ou éteint la LED D5 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D6, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D6 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D7, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                               // Allume ou éteint la LED D7 en fonction de l'état du contenu du pointeur du tableau de séquences
      
      digitalWrite(BrocheLED_D8,  Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D8 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D9,  !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                             // Allume ou éteint la LED D9 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D10, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D10 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D11, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                             // Allume ou éteint la LED D11 en fonction de l'état inverse du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D12, Tableau_Sequenceur[pointeur_TableauSequenceur]);                                              // Allume ou éteint la LED D12 en fonction de l'état du contenu du pointeur du tableau de séquences
      digitalWrite(BrocheLED_D13, !Tableau_Sequenceur[pointeur_TableauSequenceur]);                                             // Allume ou éteint la LED D13 en fonction de l'état du contenu du pointeur du tableau de séquences
      
      compteurAffichageModeCourant++;                                                                                           // Incrémente le compteur d'affichages du mode courant en mode automatique
      
      pointeur_TableauSequenceur = (pointeur_TableauSequenceur + 1) % Nombre_Sequences;                                         // Borne le tableau de séquences
    }
  }
//**************************************************************************************************************************************************************************
//*** Chenillard 12 LED 4 LED de gauche à droite et de droite à gauche non bloquant ****************************************************************************************
//**************************************************************************************************************************************************************************
  else if (ModeCourant == 21) // ********************************************************************************************** // Si le mode courant "21" est sélectionné
  {
    if (DernierModeCourant != ModeCourant) // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Si le mode courant a changé
    {
      //Serial.print(F("ModeCourant = ")); Serial.println(ModeCourant); // Débug
      FonctionReinitialisation();                                                                                               // Appelle la fonction de réinitialisation
      SortieModeCourant = false;                                                                                                // Réinitialise l'indicateur de sortie du mode courant
      MultipleNombreAffichageModeCourant = 5;                                                                                   // Définit le multiple du nombre d'affichages du mode courant en mode automatique
      Buzzer(3, 0, 1);                                                                                                          // Active le buzzer 3ms
      FonctionAffichageOLED();                                                                                                  // Appelle la fonction de gestion de l'affichage sur l'écran OLED 1.3"
      MaskLeds = 0b11111100;                                                                                                    // Définit le masque des LED pour déterminer la LED allumée
      EtatInitial = 0b00111100;                                                                                                 // Définit l'état initial des LED
      etatLeds = EtatInitial;                                                                                                   // Définit l'état courant des LED égal à l'état initial des LED
      Sens_decalage_PORTD = false;                                                                                              // Définit l'indicateur du sens de décalage du PORTD
      DureeTempo_EtatBas = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état bas
      DureeTempo_EtatHaut = 50ul; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> // Durée de la temporisation à l'état haut
      t1DebutTempo_EtatBas = 0ul;                                                                                               // Initialise la temporisation à l'état bas en mode non bloquant
      t1DebutTempo_EtatHaut = millis();                                                                                         // Démarre la temporisation à l'état haut en mode non bloquant
      
      DernierModeCourant = ModeCourant;                                                                                         // Mémorise le dernier mode courant
    }
    
    if (!Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de gauche à droite
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume les LED courantes du PORTD
        PORTB |= (etatLeds & MaskLeds) >> 2;                                                                                    // Allume les LED courantes du PORTB
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Réinitialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = millis();                                                                                        // Démarre la temporisation à l'état bas en mode non bloquant
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint les LED courantes du PORTD
        PORTB &= ~(etatLeds & MaskLeds) >> 2;                                                                                   // Eteint les LED courantes du PORTB
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Réinitialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = 0ul;                                                                                             // Réinitialise la temporisation à l'état bas en mode non bloquant
      }
      
      if (t1DebutTempo_EtatHaut == 0ul && t1DebutTempo_EtatBas == 0ul) // ----------------------------------------------------- // Si les temporisations à l'état haut et à l'état bas sont arrêtées
      {
        etatLeds <<= 1;                                                                                                         // Décale l'état courant des LED à gauche de 1 position
        if ((etatLeds << 2 & MaskLeds) == 0) // ............................................................................... // Si l'état courant des LED a été décalé 3 fois à gauche de 1 position
        {
          etatLeds = EtatInitial;                                                                                               // Transfère l'état initial des LED dans l'état courant des LED
          Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                           // Inverse l'indicateur du sens de décalage
        }
        
        t1DebutTempo_EtatHaut = millis();                                                                                       // Démarre la temporisation à l'état haut en mode non bloquant
      }
    }
    else if (Sens_decalage_PORTD) // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // Sens de droite à gauche
    {
      if (millis() - t1DebutTempo_EtatHaut >= DureeTempo_EtatHaut && t1DebutTempo_EtatHaut != 0ul) // ------------------------- // Si la temporisation à l'état haut en mode non bloquant est écoulée
      {
        PORTD |= (etatLeds & MaskLeds);                                                                                         // Allume les LED courantes du PORTD
        PORTB |= (etatLeds & MaskLeds) >> 2;                                                                                    // Allume les LED courantes du PORTB
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Réinitialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = millis();                                                                                        // Démarre la temporisation à l'état bas en mode non bloquant
      }
      else if (millis() - t1DebutTempo_EtatBas >= DureeTempo_EtatBas && t1DebutTempo_EtatBas != 0ul) // ----------------------- // Si la temporisation à l'état bas en mode non bloquant est écoulée
      {
        PORTD &= ~(etatLeds & MaskLeds);                                                                                        // Eteint les LED courantes du PORTD
        PORTB &= ~(etatLeds & MaskLeds) >> 2;                                                                                   // Eteint les LED courantes du PORTB
        
        t1DebutTempo_EtatHaut = 0ul;                                                                                            // Réinitialise la temporisation à l'état haut en mode non bloquant
        t1DebutTempo_EtatBas = 0ul;                                                                                             // Réinitialise la temporisation à l'état bas en mode non bloquant
      }
      
      if (t1DebutTempo_EtatHaut == 0ul && t1DebutTempo_EtatBas == 0ul) // ----------------------------------------------------- // Si les temporisations à l'état haut et à l'état bas sont arrêtées
      {
        etatLeds >>= 1;                                                                                                         // Décale l'état courant des LED à droite de 1 position
        if ((etatLeds >> 2 & MaskLeds) == 0) // ............................................................................... // Si l'état courant des LED a été décalé 3 fois à droite de 1 position
        {
          etatLeds = EtatInitial;                                                                                               // Transfère l'état initial des LED dans l'état courant des LED
          Sens_decalage_PORTD = !Sens_decalage_PORTD;                                                                           // Inverse l'indicateur du sens de décalage
          
          compteurAffichageModeCourant++;                                                                                       // Incrémente le compteur d'affichages du mode courant en mode automatique
        }
        
        t1DebutTempo_EtatHaut = millis();                                                                                       // Démarre la temporisation à l'état haut en mode non bloquant
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
  
  compteurAffichageModeCourant = 0;                                                                                             // Initialise le compteur d'affichages du mode courant en mode automatique
  
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
//*** Fonction pour inverser (effet miroir) le code binaire naturel sur le PORTD sans modifier D0 et D1 (RX et TX) *********************************************************
//**************************************************************************************************************************************************************************
uint8_t FonctionMiroir_PortD (uint8_t Octet_Original)
{
  uint8_t Octet_Miroir = 0b00000000;                                                                                            // Déclare et initialise l'octet pour calculer et stocker le résultat de l'octet inversé
  const uint8_t Masque1 = 0b00000100;                                                                                           // Déclare et initialise le masque pour le calcul du résultat de l'octet inversé
  
  for (int Bit_Octet_Original_n = 7; Bit_Octet_Original_n > 1; Bit_Octet_Original_n--) // ************************************* // Parcourt les bits D2 à D7 du compteur binaire passé en paramètre à la fonction
  {
    if ((Octet_Original & (1 << Bit_Octet_Original_n)) >> Bit_Octet_Original_n == 1) // +++++++++++++++++++++++++++++++++++++++ // Si le bit courant est égal à "1"
    {
      Octet_Miroir |= (Octet_Miroir | (Masque1 << ((6 - Bit_Octet_Original_n) + 1)));                                           // Calcule l'emplacement du bit de l'octet inversé
    }
    else if ((Octet_Original & (1 << Bit_Octet_Original_n)) >> Bit_Octet_Original_n == 0) // ********************************** // Si le bit courant est égal à "0"
    {
      Octet_Miroir |= (Octet_Miroir & (~Masque1 << ((6 - Bit_Octet_Original_n) + 1)));                                          // Calcule l'emplacement du bit de l'octet inversé
    }
  }
  
  return (Octet_Miroir);                                                                                                        // Retourne l'octet inversé
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
