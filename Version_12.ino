// Lecture et affichage de la température de la piscine et de dehors - OK
// Lecture et affichage de l'heure et de la date - OK
// Affichage de messtages texte avec gestion de la couleur - OK
// Avec 54 colonnes (9 caractères) au lieu de 32 à l'origine
// Optimisation et correction de la présentation
// Gestion de la luminosité
// Correction formule températures extérieure
// Commande de l'extinction de l'écran
// 9 couleurs de message

#include <SPI.h>                                   // Bibliothèque pour SPI
#include <Ethernet.h>                              // Bibliothèque pour Ethernet
#include <Adafruit_GFX.h>                          // Bibl1othèque pour caractères graphiques
#include <Adafruit_NeoMatrix.h>                    // Bibliothèque pour matrice de LED adressables
#include <Adafruit_NeoPixel.h>                     // Bibliothèque pour LED adressables

#define PIN 3                                      // Broche de commande des LED
#define LUM 1                                      // Broche de la LDR
#define LUMAX 80                                   // Luminosité maximum

// Ethernet shield attached to pins 10, 11, 12, 13 et 4 pour la carte SD

byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0xDF, 0xAA}; // Adresse mac de la carte Ethernet
byte ip[] = {192, 168, 0, 184};                    // Adresse IP
EthernetServer serveur(84);                        // Déclare l'objet serveur au port d'écoute 84

int tpn; float tp; int tpa;                        // Température
char h1, h2, m1, m2;                               // Caractères de l'heure
char j1, j2, a1, a2;                               // Caractères de la date
char p1, p2, p3, p4;                               // Caractères de la température

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(54, 8, PIN,             // Paramètres 1, 2 and 3
  NEO_MATRIX_BOTTOM    + NEO_MATRIX_RIGHT +                            // Paramètre 4
  NEO_MATRIX_COLUMNS   + NEO_MATRIX_ZIGZAG,                            // Paramètre 4 suite
  NEO_GRB              + NEO_KHZ800);                                  // Paramètre 5

const uint16_t colors[] = {                                            // 8 couleurs au total car pas le noir
matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(255, 255, 0),matrix.Color(0, 0, 255), matrix.Color(255, 0, 255),
matrix.Color(0, 255, 255), matrix.Color(255, 255, 255), matrix.Color(255, 128, 0), matrix.Color(255, 64, 64), matrix.Color(128, 255, 128) };
// 0 = rouge, 1 = vert, 2 = jaune, 3 = bleu foncé, 4 = rose, 5 = bleu clair, 6 = blanc, 7 = orange, 8 = rouge clair, 9 = vert clair
char newmessage[11] = "ABCDEFGHI\0";               // Nouvelle chaine de caractères à envoyer pour l'affichage
char oldmessage[11] = "ABCDEFGHI\0";               // Ancienne chaine de caractères à effacer pour l'affichage
int newcouleur = 0;                                // Nouvelle couleur
int oldcouleur = 0;                                // Ancienne couleur
int x;                                             // Position curseur
int analum;                                        // Lecture canal analogique LDR          
int lum = 1;                                       // Luminosité

void setup() {
  Serial.begin (9600);                             // Initialisation de communication série avec le moniteur
  Ethernet.begin (mac, ip);                        // Initialisation de la communication Ethernet
  Serial.print("Le serveur est à l'adresse : ");
  Serial.println(Ethernet.localIP());              // On affiche l'adresse IP de la connexion
  serveur.begin();                                 // On démarre l'écoute
  matrix.begin();
  matrix.setTextWrap(false);
}

void loop() {  
  char c;
  EthernetClient client = serveur.available();     // On écoute le port
  if (client) {                                    // Si client connecté
    Serial.println("\n\nClient en ligne");         // On le dit
    if (client.connected()) {                      // Si le client est en connecté
      while (client.available()) {                 // Tant qu'il a des infos à transmettre
        c = client.read();                         // On lit le caractère
        Serial.write(c);                           // On écrit sur le moniteur série
        delay(1);                                  // Délai de lecture
        if (c == 'Z') {                            // On s'intéresse à la suite
          c = client.read();                       // On lit le caractère qui indique quelle donnée
          Serial.write(c);                         // On écrit sur le moniteur série
          delay(1);                                // Délai de lecture     
          switch (c)
          {
            case 'H':                              // L'heure va suivre
            newcouleur = 8;                        // Rouge clair
            heure();
            break;
            case 'J':                              // La date va suivre
            newcouleur = 2;                        // Jaune
            date();
            break;
            case 'P':                              // La température de la piscine va suivre
            newcouleur = 5;                        // Bleu clair
            temperature();
            break;
            case 'D':                              // La température de dehors va suivre
            newcouleur = 1;                        // Vert
            temperature();
            break;
            case 'M':                              // Un message va suivre après la couleur
            newcouleur = client.read() - 48;       // Couleur du message
            message();
            break;
            case 'E':                              // Extinction de l'affichage
            matrix.setBrightness(1);               // Luminosité au minimum
            matrix.fillScreen(0);                  // Eteint toutes les LED
          }       
        }
      }     
      entete(client);                              // Réponse au client
      client.println("<a href=?valeur=100>Envoi</a><br>");
      client.println("<a href=?>Refresh</a><br>");
      client.println("<br><hr></body></html>");    // Ligne horizontale et fermeture des balises    
      client.stop();                               // On déconnecte le client
      Serial.println("Fin de communication avec le client");
      Serial.println (lum);
      affiche();
    }
  }
}
// Fonction d'affichage de l'entête HTML
void entete(EthernetClient cl) {
  cl.println("<!DOCTYPE HTML>");
  cl.println("<html>");
  cl.println("<head><title>Esssai</title></head>");
  cl.println("<body><h1>Essai</h1><hr><br>");
}

void temperature() {
  int ii;
  float jj;
  char c;
  tpn = 0;
  EthernetClient client = serveur.available();     // On écoute le port
  for (ii = 5 ; ii >= 1 ; ii--) {                  // 5 digits à lire
    c = client.read();                             // On lit le caractère
    Serial.write(c);                               // On l'écrit sur le moniteur série
    delay(1);                                      // Délai de lecture
    tpn = tpn*10 + c - 48;                         // Calcul de la température en numérique, le code code ASII du 0 est 48
    if (newcouleur == 5) tp = ((tpn * 0.000050354) - 0.25) / 0.028;     // Calcul de la température de la piscine en analogique
    else tp = (tpn * 0.0050354) - 48;              // Calcull de la température de dehors en analogique
    tp = 10 * tp;                                  // Pour avoir un chiffre après la virgule
    jj = tp - int(tp);                             // Arrondissement au degré entier
    if (jj >= 0.5) tpa = int (tp) + 1;
    else tpa = int (tp); 
  }
  Serial.print("\nTempérature arrondie: ");
  Serial.print(tpa);
  p1 = int(tpa / 100);                             // Dizaines
  p2 = int((tpa - p1 * 100) / 10);                 // Unités
  p3 = tpa - p1 * 100 - p2 * 10;                   // Dizièmes
  ii = 0;                                          // Prépare le début de la chaine
  if (tpa < 0) newmessage[ii++] = '-';             // Température négative, signe - tout à gauche
  else newmessage[ii++] = ' ';                     // Espace à la place du -
  if (p1) newmessage [ii++] = p1 + 48;             // Dizaines, pas de 0 non significatif
  newmessage[ii++] = p2 + 48;                      // Unités
  newmessage[ii++] = '.';                          // Virgule
  newmessage[ii++] = p3 + 48;                      // Dizièmes
  newmessage[ii++] = ' ';                          // Espace
  newmessage[ii++] = '\367';                       // °
  newmessage[ii++] = 'C';                          // C
  newmessage[ii++] = '\0';                         // Fin de chaine
  Serial.println(newmessage);                      // Visualise sur le moniteur
}

void heure() {
  char c = 0;
  int ii = 0;
  newmessage[ii++] = ' ';                          // On commence par un espace
  EthernetClient client = serveur.available();     // On écoute le port
  while (c  != 'T') {                              // Attente du caractère T
    c = client.read();                             // On lit le caractère
    delay(1);                                      // Délai de lecture
  }
  h1 = client.read();                              // On lit les heures 1
  delay(1);                                        // Délai de lecture
  h2 = client.read();                              // On lit les heures 2
  delay(1);                                        // Délai de lecture
  m1 = client.read();                              // On lit les :
  delay(1);                                        // Délai de lecture
  m1 = client.read();                              // On lit les minutes 1
  delay(1);                                        // Délai de lecture
  m2 = client.read();                              // On lit les minutes 2
  delay(1);                                        // Délai de lecture
  newmessage[ii++] = ' ';                          // Espace
  if (!h1) newmessage[ii++] = ' ';                 // Espace si 0 non significatif
  else newmessage[ii++] = h1;
  newmessage[ii++] = h2;
  newmessage[ii++] = ':';
  newmessage[ii++] = m1;
  newmessage[ii++] = m2;
  newmessage[ii++] = ' ';                          // Espace pour effacer la fin des messages plus longs
  newmessage[ii++] = ' ';                          // Espace pour effacer la fin des messages plus longs
  newmessage[ii++] = '\0';                         // Fin de chaine
  Serial.print("Heure: ");
  Serial.print(h1);                                // Affichage de l'heure  
  Serial.print(h2);
  Serial.print(":");
  Serial.print(m1);
  Serial.print(m2);
}

void date() {
  char c;
  int ii;
  EthernetClient client = serveur.available();     // On écoute le port
  for (ii = 0 ; ii < 5 ; ii++) {                   // 5 digits à lire
    c = client.read();                             // On lit le caractère
    delay(1);                                      // Délai de lecture
  }
  a1 = client.read();                              // On lit le mois 1
  delay(1);                                        // Délai de lecture
  a2 = client.read();                              // On lit le mois 2
  delay(1);                                        // Délai de lecture
  j1 = client.read();                              // On lit le -
  delay(1);                                        // Délai de lecture
  j1 = client.read();                              // On lit le jour 1
  delay(1);                                        // Délai de lecture
  j2 = client.read();                              // On lit le jour 2
  delay(1);                                        // Délai de lecture
  ii = 0;
  newmessage[ii++] = ' ';                          // On commence par 1 espace
  if (!j1) newmessage[ii++] = ' ';                 // Espace si 0 non significatif
  else newmessage[ii++] = j1;
  newmessage[ii++] = j2;
  newmessage[ii++] = ' ';                          // Espace
  newmessage[ii++] = '/';
  newmessage[ii++] = ' ';                          // Espace
  newmessage[ii++] = a1;
  newmessage[ii++] = a2;
  newmessage[ii++] = ' ';                          // Espace pour effacer la fin des messages plus longs
  newmessage[ii++] = '\0';                         // Fin de chaine
  Serial.print("Date: ");
  Serial.print(j1);                                // Affichage du mois
  Serial.print(j2);
  Serial.print("/");
  Serial.print(a1);
  Serial.print(a2);
}

void message() {
  char c;
  int ii;
  EthernetClient client = serveur.available();     // On écoute le port
  for (ii = 0 ; ii < 9 ; ii++) {                   // 9 caractères à traiter
    c = client.read();                             // On lit le caractère
    delay(1);                                      // Délai de lecture
    Serial.print(c);
    newmessage[ii] = c;                            // On met les caractères lus
  }
  newmessage[++ii] = '\0';                         // Fin de chaine
}

void affiche() {
  analum = analogRead(LUM);                        // Lecture canal analogique LDR
  lum = map(analum, 1024, 0, 1, LUMAX);            // Entrée, min entrée, max entrée, min sortie, max sortie
  matrix.setBrightness(lum);                       // Luminosité                               
// Scroll pour effacer l'ancien message
  x = matrix.width();                              // Curseur position démarrage à droite
  matrix.setTextColor(colors[oldcouleur]);         // Sélectionne la couleur 
  while (x-- > 0) {
    matrix.fillScreen(0);                          // Eteint toutes les LED
    matrix.setCursor(x - matrix.width(), 0);       // Place le curseur. Colonne, ligne
    matrix.print(oldmessage);                                   
    matrix.show();                                 // Valide pour affichage
    delay (30);
  }
  oldcouleur = newcouleur;                         // Sauve la couleur d'origine
  strcpy(oldmessage, newmessage);                  // Sauve le mesage pour l'effacer plus tard
// Scroll pour afficher le nouveau message
  x = matrix.width();                              // Curseur position démarrage à droite
  matrix.setTextColor(colors[newcouleur]);         // Sélectionne la couleur 
  while (x-- > 0) {
    matrix.fillScreen(0);                          // Eteint toutes les LED
    matrix.setCursor(x, 0);                        // Place le curseur. Colonne, ligne
    matrix.print(newmessage);                                   
    matrix.show();                                 // Valide pour affichage
    delay (30);
  }
  strcpy(newmessage, "        \0");                // Efface toute la chaine
}
