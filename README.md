# Proto-Pulmo
## Description
Ce repo github contient le code du projet de l'équipe Proto-Pulmo dans le cadre du cours ELE400

Le code est conçu pour être interfacer le capteur SLMX4 de SensorLogic sur une machine linux, détecter la respiration à partir des données du capteur et envoyer ces données vers le logiciel musical MaxMSP.

Documentation module capteur de SensorLogic:

https://github.com/SensorLogicInc/modules

Documentation plus poussée sur le circuit intégré du capteur

https://github.com/novelda/Legacy-Documentation

 
## Contenu
Le repo est separe en 3 dossiers principaux :
- **Boitier** 
	 -  Fichiers sources pour le boitier du capteur
	 -	Fichiers exportés pret à l'impression 3D
 - **Firmware_Capteur** 
	 - Code pour le logiciel embarqué qui roule sur le capteur 
	 - Viens de SensorLogic
 - **Matlab** 
	 - Scripts Matlab qui ont servi au développement de l'algorithme de detection
 - **SW_Repartisseur**
	 - Code C pour permettre l'interface avec le capteur, la détection de respiration et l'envoi des données vers MaxMSP

## Usage
Pour commencer, si vous avez un nouveau repartisseur, vous devrez installer les librairies qui sont nécessaires puis compiler le code (Voir le readme de sw_repartisseur).

Verrifiez que le binaire se lance bien et que l'utilisateur est bel et bien ajouté au groupe dialout. Ensuite, vous devrez ajouter le programme au crontab du système d'exploitation pour que celui-ci se lance automatiquement lors du démarage du système. Pour ce faire, utilisez la commande crontab -e et ajoutez à la fin du fichier la ligne suivante : 

@reboot sleep 15 && /home/pi/proto-pulmo/sw_repartisseur/bin/sw_repartisseur

 remplacez bien sur /home/pi/proto-pulmo/sw_repartisseur/bin/sw_repartisseur par le chemin au binaire compilé
 
 
 
Ensuite, lorsque le repartisseur est pret, vous devez envoyer vottre adresse IP de MAXmsp au répartisseur, pour ce faire, vous devrez envoyer un message de format @192.168.2.1 via sendUDP. Cela indiquera au répartisseur ou envoyer les données. Le répartisseur envoiera ensuite les données par udp sur le port déterminé
