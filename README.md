### TD8-Arena ###

* Yann Cortial
* Basile Fraboni
* Bastien Thouverez
* Peter Vystavel

![alt tag](http://bthouverez.fr/docs/TD8ARENA2.png)

## Le projet ##

Notre projet consiste en un mini-jeu où il s'agit de controller un vaisseau et de détruire des astéroides. 
Le projet utilise une caméra et à l'aide d'une mire de calibrage fixée au sol, elle décrit le repère du monde 3D dans lequel évolue le vaisseau.
Nous avons placé ce vaisseau au niveau de la mire, il peut être controlé grâce à un LeapMotion.

## Contrôle du vaisseau ##
 
Le vaisseau est donc contrôlable à l'aide du LeapMotion: en mettant la main droite à plat au dessus, le vaisseau adopte une position stationnaire. Il est alors possible de le faire bouger selon trois contrôles:
* la hauteur: gérée grâce à la hauteur de la main par rapport au LeapMotion (lever la main fera prendre de l'altitude au vaisseau et inversement).
* la vitesse: le vaisseau ne peut qu'avancer grâce à une inclinaison de la main vers l'avant (incliner vers l'arrière ou revenir en position stationaire arrêtera le vaisseau).
* la direction: le vaisseau peut tourner à droite et à gauche grâce à une rotation du poignet (par rapport à l'axe de l'avant-bras).

## Compilation et exécution du projet ##

Le projet a été développé sur des machines dotées d'Ubuntu (16.04). Les paquets suivants sont nécessaires pour compiler le projet:

* libglew-dev
* libglfw3-dev
* libopencv-dev 
* libcv-dev
* libglm-dev
* libsdl2-dev
* libsdl2-image-dev
* freeglut3-dev

Il faut aussi installer le SDK du LeapMotion: télécharger le SDK (https://developer-archive.leapmotion.com/downloads/external/skeletal-beta/linux?version=2.3.1.31549), extraire l'archive et installer le .deb avec dpkg:

* sudo dpkg --install Leap-*-x64.deb

Le projet contient différents programmes de test qui sont tous construits lorsque la commande 'make' est appelée depuis la racine du projet. On retrouve notamment un test_calib permettant de calibrer une caméra à l'aide d'une mire et d'enregistrer les paramètres intrinsèques relatifs à cette caméra dans un fichier texte.

Le programme principal s'appelle TD8_ARENA.


## En résumé ##

Pour une bonne exécution du programme, il est conseillé d'effectuer une calibration de la caméra puis de lancer le programme. Il est aussi nécessaire de lancer le service leapd nécessaire au bon 
fonctionnement du LeapMotion.
Voici un résumé des commandes utiles:

Dans une console à part ou en tâche de fond:
* sudo leapd


* git clone https://github.com/bthouverez/TD8-Arena
* cd TD8-Arena
* premake4 gmake
* make test_calib TD8_ARENA config=release64 -j8
* ./bin/test_calib # affiche les paramètres à donner au programme
* ./bin/test_calib 1920 1080 60 10 7 25 40 data/camera/intrinsics_guillou.txt 1 
* ./bin/TD8_ARENA 1920 1080 60 10 7 25 data/camera/intrinsics_guillou.txt 1

![alt tag](http://bthouverez.fr/docs/HEADSHOT_guillou.png)


