#include <Wire.h>//https://www.arduino.cc/en/reference/wire
#include <DS3231.h>//https://github.com/NorthernWidget/DS3231

//RTC
RTClib RTC;

int commande_haut = 4; // Déclaration broche Bouton fermeture
int commande_bas = 5; // Déclaration broche Bouton ouverture
int fin_de_course_haut = 6; // Capteur fin de course haut (porte fermée)
int enA = 10;                   // crée une variable de type "int", nommée "enA" et attachée à la Broche 10, qui permet de gérer la vitesse du moteur.
int in1 = 9;                    // crée une variable de type "int", nommée "in1" et attachée à la Broche 9, qui permet de gérer le sens de rotation.
int in2 = 8;                    // crée une variable de type "int", nommée "in2" et attachée à la Broche 8, qui permet de gérer l'autre sens de rotation.

unsigned long debut_fermeture; // timer pour debut de fermeture
unsigned long timeoutMoteur = 30000; // timeout ou on considere le moteur bloquee
int delayTime = 10; // Temps entre chaque pas 10ms
boolean fdch = false; // Déclaration variable Fin de Course Haut (porte fermee)
boolean etat_bp_h = false, etat_bp_b = false; // Déclaration des variables bas et haut
boolean mem_h = false, mem_b = false, mem_fdch = false; // Déclaration des mémoires


// tableau des horaires selon les quinzaines
// tableau des horaires marche 2 mois par 2 mois (janvier avec decembre, février avec novembre, ...)
// il fonctionne par 15aine
// janvier du 01 au 15 correspond a l'index 0, janvier du 16 au 31 correspond à l'index 1
// décembre du 01 au 15 correspond a l'index 1, décembre du 16 au 30 correspond a l'index 0
// ...
int LeveSoleilHeure[12]= {7,7, 7,7, 6,6, 5,5, 5,5, 5,5 };   
int LeveSoleilMinute[12]= {15,15, 0,0, 45,45, 30,30, 15,15, 0,0 };   
int CoucheSoleilHeure[12]= {19,19, 19,19, 20,20, 20,20, 21,21, 21,21 };   
int CoucheSoleilMinute[12]={15,15, 30,30, 0,0, 30,30, 0,0, 30,30 }; 
  
void setup() {
  Serial.begin(9600); // Ouverture du port série et debit de communication fixé à 9600 bauds
  Wire.begin();
  pinMode(commande_haut, INPUT_PULLUP); // Déclaration entrée pull-up sur entrée BP haut
  pinMode(commande_bas, INPUT_PULLUP); // Déclaration entrée pull-up sur entrée BP bas
  pinMode(fin_de_course_haut, INPUT_PULLUP); // Déclaration entrée pull-up sur entrée Fin de course haut
  pinMode(enA, OUTPUT);         // indique que la broche de la variable "enA" donc ici la PIN 10, est une sortie.
  pinMode(in1, OUTPUT);         // indique que la broche de la variable "in1" donc ici la PIN 9, est une sortie.
  pinMode(in2, OUTPUT);         // indique que la broche de la variable "in2" donc ici la PIN 8, est une sortie.
  etat_bp_h = !digitalRead(commande_haut); // Inverse de la lecture sur entrée BP haut
  etat_bp_b = !digitalRead(commande_bas); // Inverse de la lecture sur entrée BP bas
  fdch = !digitalRead(fin_de_course_haut); // Inverse de la lecture sur entrée Fin de course haut
  mem_fdch = fdch; // Mémorisation du nouvel état du fin de course bas
  mem_h = etat_bp_h; // Mémorisation du nouvel état du bouton haut
  mem_b = etat_bp_b; // Mémorisation du nouvel état du bouton bas
  DateTime now = RTC.now(); //  Utile au debug pour connaitre l'heure au debut du programme
  Serial.println(now.hour()); // heure
  Serial.println(now.minute()); // minute
}


void loop() {
  etat_bp_h = !digitalRead(commande_haut); // Inverse de la lecture sur entrée BP haut
  etat_bp_b = !digitalRead(commande_bas); // Inverse de la lecture sur entrée BP bas
  fdch = !digitalRead(fin_de_course_haut); // Inverse de la lecture sur entrée Fin de course haut
  DateTime now = RTC.now();   // recupere la date
  int index= getIndex(now); // calcul de l'index pour le tableau des horaires

  //debug
  if (fdch != mem_fdch) // Changement d'état du fin de course haut (front montant ou descendant)
  {
    if (fdch) 
    {
      Serial.println("Porte fermée !"); // Affichage sur le moniteur série du texte
    }
    if (!fdch)
    {
      Serial.println("Porte non fermée"); // Affichage sur le moniteur série du texte
    }
  }
  mem_fdch = fdch; // Mémorisation du nouvel état du fin de course bas
  
  if (etat_bp_h != mem_h) // Changement d'état du bouton poussoir haut (front montant ou descendant)
  {
    if (etat_bp_h && !fdch) // Appui sur BP haut et la porte n'est pas fermee
    {
      Serial.println("Fermer");
      Fermer_porte(); // ferme
      Ouvrir_1sec(); // ouverture d'1 sec (utile car dans certains cas le fin de course n'était pas atteint)
      Fermer_porte(); //ferme
    }
  }
  mem_h = etat_bp_h; // Mémorisation du nouvel état du bouton haut
  
  if (etat_bp_b != mem_b) // Changement d'état du bouton poussoir bas (front montant ou descendant)
  {
    if (etat_bp_b && fdch) // Appui sur BP bas mais pas sur le haut
    {
      Serial.println("Ouvrir");
        Ouvrir_porte();
    }
  }
  mem_b = etat_bp_b; // Mémorisation du nouvel état du bouton bas

  // ouverture en fonction de l'heure
  if (canOpen(now, index)) //en horaire d'ouverture
  {
    Ouvrir_porte();;
  }
  //fermeture en fonction de l'heure et si la porte n'est pas deja fermee
  if (!fdch && canClose(now,index))//(fdcb && canClose(now,index)) //Porte non fermée et en horaire de fermeture
  {
    Fermer_porte();
    Ouvrir_1sec();
    Fermer_porte();
  }
}

void Ouvrir_1sec() {
    digitalWrite(in1,HIGH);       // met la broche in1 à l'état bas, la carte n'injecte pas d'électricité, le moteur ne tourne pas dans le sens de in1.
    digitalWrite(in2,LOW);      // envoie l'électricité dans la broche in2 (donc la PIN 8)ce qui fait donc tourner le moteur dans le sens de rotation de in2.
    analogWrite(enA,200);        // défini la vitesse du moteur, ici 200 sur un maximum de 255 (échélle comprise entre 0 et 255, car cela correspond à 8 bits soit 1 octet).
    delay(1000); // delay d'une seconde
    fdch = !digitalRead(fin_de_course_haut); // récuperation du fin de coursehaut
    Arret(); // arret de moteurs
}


void Fermer_porte() {
  debut_fermeture = millis(); // calcul du temps de fermeture
  while (!fdch) {
      digitalWrite(in1,LOW);       // met la broche in1 à l'état bas, la carte n'injecte pas d'électricité, le moteur ne tourne pas dans le sens de in1.
      digitalWrite(in2,HIGH);      // envoie l'électricité dans la broche in2 (donc la PIN 8)ce qui fait donc tourner le moteur dans le sens de rotation de in2.
      analogWrite(enA,200);        // défini la vitesse du moteur, ici 200 sur un maximum de 255 (échélle comprise entre 0 et 255, car cela correspond à 8 bits soit 1 octet).
      delay(delayTime);   // pas de 10ms

      fdch = !digitalRead(fin_de_course_haut); // récupere l'etat du fin de course
      etat_bp_b = !digitalRead(commande_bas); // Inverse de la lecture sur entrée BP bas
      if (fdch) // si porte fermee
      {
        Serial.println("Porte en haut"); // Affichage sur le moniteur série du texte
        Arret(); // arret des moteurs
        break; // fin
      }
      if ((millis() - debut_fermeture) > timeoutMoteur) {
        Serial.println("Porte bloquee");
          Arret(); // arret carte timeout atteint
          break;
        }
  }
}

void Ouvrir_porte() {
    digitalWrite(in1,HIGH);     // met la broche in1 à l'état bas, la carte n'injecte pas d'électricité, le moteur ne tourne pas dans le sens de in1.
    digitalWrite(in2,LOW);      // envoie l'électricité dans la broche in2 (donc la PIN 8)ce qui fait donc tourner le moteur dans le sens de rotation de in2.
    analogWrite(enA,200);       // défini la vitesse du moteur, ici 200 sur un maximum de 255 (échélle comprise entre 0 et 255, car cela correspond à 8 bits soit 1 octet).
    delay(16000);               // delay 16sec le temps d'ouverture de la porte
    fdch = !digitalRead(fin_de_course_haut); // récupère l'etat du relais porte fermée
    Arret();                    // arret du moteur
}


void Arret() {
  digitalWrite(in1,LOW);       // en mettant la broche in1 à l'état bas, la carte n'injecte pas d'électricité, le moteur est donc à l'arrêt.
  digitalWrite(in2,LOW);       // en mettant la broche in2 à l'état bas, la carte n'injecte pas d'électricité, le moteur est donc à l'arrêt.
}

int getIndex(DateTime t){
  int Index=t.month(); // on récupere le mois
  if (t.month()>6)     // si le mois est après Juin
  {
    Index=13-t.month();    //on récupère le 13 moins le mois (décembre est avec janvier, nomvembre avec fevrier , ...) 
    Index=(Index-1)*2;     // on fait -1 x 2 car le tableau est par quinzaine
    if( t.day()<15)       // si on est dans la quinzaine du début du moins on augmente l'index
    {
      Index++;   
    }
  }
  else   // mois avant juin
  {  
    Index=(Index-1)*2;     // on fait -1 x 2 car le tableau est par quinzaine
    if( t.day()>15)       // si on est dans la quinzaine de fin du moins on augmente l'index
    {
      Index++;   
    }
  }   
  return Index;   
}

boolean canOpen(DateTime t,int index)   
{ 
  if (t.hour()== LeveSoleilHeure[index] && t.minute()== LeveSoleilMinute[index] && t.second()==0 )
  {
    Serial.println("Can open");
    return true;
  }
  else    
  {
    return false;   
  }
}   

boolean canClose(DateTime t,int index)
{  
  if (t.hour()== CoucheSoleilHeure[index] && t.minute()== CoucheSoleilMinute[index] && t.second()==0 )
  {
    Serial.println("Can close");
    return true;   
  }
  else    
  {
    return false;   
  }
}   
