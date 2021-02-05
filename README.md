# Porte de poulailler

## Fonctionnement
Pour fixer l'heure sur la rtc : https://github.com/NorthernWidget/DS3231

En fonction de la date et l'heure la porte du poulailer s'ouvre et se ferme.

La porte est un systeme pont levis avec une poulie

Pour la fermeture le moteur s'enclenche pour enrouler le fil.  
Dés que la porte touche le capteur de fin de course (positionné sur la butée de la porte) il doit s'arreter.  
Finalement j'ai choisi de lancer une premiere fermeture puis ouvrir pendant une seconde et relancer une fermeture.  
Ceci car je me suis rendu compte que parfois la porte se bloquait un peu avant le capteur de fin de course.  

Pour l'ouverture j'avais mis le même systeme que la fermeture avec le capteur de butée sous la porte.  
Mais comme les poules marchent régulièrement sur la porte le capteur a fini par se casser et il était détecté tout le temps ouvert.  
J'ai donc utilisée une tempo (mesure du temps moyen d'ouverture de la porte plus petite rallonge de fil de pêche) pour faire l'ouverture.  

## requirements
Logiciel
- arduino pour coder et téléverser sur l'arduino

Librairies 
- DS3231.h cf https://github.com/NorthernWidget/DS3231
- Wire.h cf https://www.arduino.cc/en/reference/wire

## Materiel
- un arduino nano (type elegoo)
- une rtc DS3231
- un moteur dc (aliexpress https://fr.aliexpress.com/item/4000295013680.html?spm=a2g0s.9042311.0.0.27426c37luoqCs)
- une alim 12v
- un pont L298N pour controller le moteur
- 2 boutons poussoirs
- 1 capteur fin de course (style relai)
- des cables du pont et autres
- un fer à souder et de quoi souder
- poulie
- fil de peche pour la traction de la porte
- charniere pour la porte
- crochet pour relier le fil au moteur
- une boite de derivation pour mettre tous les composants à l'abri de la pluie et des poules

## Cablage
Voir fritzing ou image

## Code
Voir fichier .ino

## Amélioration
remplacer les boutons poussoirs par un module wifi ESP8266 flashé tasmota pour pouvoir piloter la porte via un smartphone
cf https://github.com/R0d01ph3/Porte-Poulailler-wifi

## Sources
Je me suis inspiré de beaucoup de sites différents.  
J'en oublies surement beaucoup mais voilà quelques sources :

- https://electrotoile.eu/fabrication-diy-porte-poulailler-automatique-arduino.php
- http://raspi.projets.free.fr/automatisme-de-poulailler-connecte.html
- http://portepoulailler.unblog.fr/creer-votre-porte-de-poulailler-pour-12e/
- https://journaldunarchiviste.fr/2019/01/04/domotique-poulailler-connecte-partie-1/


