/**
@file valeursGlobales.h
@brief Fichier d'inclusion commun
@author Nicolas Theriault-L
@date 10 juin 2008
*/

#ifdef GLOBAL
//! Si GLOBAL a ete defini dans le ficier appelant, ce fichier considerera
//! les variables de ce fichier comme des variables internes.
//! Attention! ne definir qu'un seul fichier proprietaire pour eviter les conflits.
#define Global
#else
//! Si GLOBAL n'a pas ete defini dans le fichier appelant, ce fichier considerera
//! les variables de ce fichier comme des variables externes
#define Global extern
#endif

/************************************************************************/
/* Ensemble de fichiers d'inclusion pour le projet en entier            */
/************************************************************************/
// Possibilite d'enlever les fichiers d'inclusion dans chacun des fichiers
// du projet et tous les regrouper ici, il n'est alors necessaire d'inclure
// que ce fichier-ci afin d'avoir sous la main toutes les ressources necesaires.

// #include <assert.h>
// #include <cv.h>
// #include <cxcore.h>
// #include <fstream>
// #include <highgui.h>
// #include <iostream>
// #include <math.h>
// #include <memory.h>
// #include <pthread.h>
// #include <SDL.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string>
// #include <time.h>
// #include <vector>


// Librairies du projet
// #include "../Localisation/cameraTracker.h"
// #include "core.h"
// #include "../Localisation/cubeInertiel.h"
// #include "../Display/displayManager.h"
// #include "../ModelLoader/face.h"
// #include "../Localisation/gps.h"
// #include "../Localisation/gpsDevice.h"
// #include "../Localisation/gps_GSatBluetooth.h"
// #include "../Localisation/imageAnalyst.h"
// #include "../Acquisition/imageCaptor.h"
// #include "../Localisation/interSense.h"
// #include "../Localisation/isense.h"
// #include "../ModelLoader/modelLoader.h"
// #include "../ModelLoader/modelMatrice.h"
// #include "../Localisation/positionDevice.h"
// #include "../Localisation/positionManager.h"
// #include "../Display/scene.h"
// #include "../Display/sceneScreen.h"
// #include "vec.h"
// #include "../Localisation/wiimote.h"

// Librairies systeme
// #ifdef _WIN32
// #include <shellapi.h>
// #include <vld.h>//Detecteur de fuite de memoire
// #include <windows.h>
// #else
// #include <bluetooth/bluetooth.h>
// #include <unistd.h>
// #endif
// 
//#ifdef _OSX
//#include <OpenGL/glew.h>
//#include <OpenGL/glu.h>
//#else
//#include <GL/glew.h>
//#include <GL/glu.h>
//#endif

/************************************************************************/
/* Constantes                                                           */
/************************************************************************/

// Valeur de Pi
// #define PI    3.14159265358979323846
// Valeur de conversion des degres en radians
// #define RAD   0.0174532925
// Valeur de conversion des radians en degres
// #define DEG   57.2957796

/************************************************************************/
/* Structures                                                           */
/************************************************************************/

// Structure pour collecter les donnees de positionnement des wiimotes
/**
Structure pour collecter les donnees de positionnement des wiimotes
@struct image_Info
*/
typedef struct image_Info {
  //! Largeur de l'image.
  int width;
  //! Hauteur de l'image.
  int height;
  //! Donnees des pixels de l'image.
  void* imageData;
} image_Info;


/************************************************************************/
/* Variables Globales                                                   */
/************************************************************************/

//! Objet contenant le vecteur de webcams


//! Variable pour la texture contenant le frame, A deplacer a un plus bas
//! niveau car il n'est necessaire que dans Scene et SceneScreen
//Global GLuint camTexture;
Global unsigned int camTexture;
