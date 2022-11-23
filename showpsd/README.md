**Installer la librairie oscpack (Necessaire pour la communication avec max)**

    sudo apt-get install liboscpack-dev

**Ajouter l'utilisateur au groupe dialout pour avoir acces au capteur :**

    sudo usermod -a -G dialout $USER


**Necessite GCC, Make et CMake**

**Pour compiler, executer les commandes suivantes:** 

    cmake CMakeLists.txt
    make
    
    
**Pour trouver le raspberry pi sur le reseau**

Si vous utilisez le raspberry pi sur un reseau, vous devrez trouver son ip, voici 3 pistes de solution pour le faire

1:IP statique

2:Trouver l'ip a l'aide de l'interface du router

3:utiliser bonjour/zeroconf et utiliser "raspberrypi.local"

