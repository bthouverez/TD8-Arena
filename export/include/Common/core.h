/**
@file core.h
@brief Fichier contenant les types/enumerations de la plate-forme
@author Simon Pouliot
@date 30 avril 2008
*/

#ifndef RA_CORE_H_
#define RA_CORE_H_

#include <iostream>
using namespace std;

#ifdef _WIN32
  #ifdef _DEBUG
	// The Winsock2.h header file internally includes core elements from the Windows.h header file, so there is not usually an #include line for the Windows.h header file in Winsock applications. If an #include line is needed for the Windows.h  header file, this should be preceded with the #define WIN32_LEAN_AND_MEAN macro. For historical reasons, the Windows.h header defaults to including the Winsock.h header file for Windows Sockets 1.1. The declarations in the Winsock.h header file will conflict with the declarations in the Winsock2.h header file required by Windows Sockets 2.0. The WIN32_LEAN_AND_MEAN macro prevents the Winsock.h from being included by the Windows.h  header.
    #define WIN32_LEAN_AND_MEAN	// to avoid windows.h and winsock2.h conflict
	#include <windows.h>
  #endif
#else
  #include <unistd.h>
  #include <stdint.h>	// for int8_t ...
  #include <stdio.h>	// for sprintf
  #ifdef _LINUX
  #include <linux/types.h>
  #endif
#endif

#include <string>

//76 Buttons : 26*2 letters (uppercase and lowercase) + esc + arrow keys + 12 joystick buttons
// + 5 Mouse buttons (LEFT, RIGHT, MIDDLE_PRESSED, MIDDLE_UP, MIDDLE_DOWN) 
// Organisation : lowercase letters first [0-25], then uppercase[26-51], then esc[52],
// then arrows[53-56], then joystick buttons[57-68], then mouse buttons[69-73].
#define NB_BUTTONS 74
#define B_UP 1
#define B_DOWN 2
#define B_INACTIVE 0
// Keys defines
#define K_a 0
#define K_A 26
#define K_ESC 52
#define K_LEFT 53
#define K_RIGHT 54
#define K_UP 55
#define K_DOWN 56
// Joystick Buttons
#define J_1 57
#define J_2 58
#define J_3 59
#define J_4 60
#define J_5 61
#define J_6 62
#define J_7 63
#define J_8 64
#define J_9 65
#define J_10 66
#define J_11 67
#define J_12 68
// Mouse Buttons defines
#define M_LEFT 69
#define M_MIDDLE 70
#define M_RIGHT 71
#define M_MIDDLE_DOWN 72
#define M_MIDDLE_UP 73
//6 moving axis : 4 for mouse (absolute and relative), 4 for the joystick
#define NB_AXIS 8
#define MOUSE_X 0
#define MOUSE_Y 1
#define MOUSE_X_REL 6
#define MOUSE_Y_REL 7
#define JOY_1_X 2
#define JOY_1_Y 3
#define JOY_2_X 4
#define JOY_2_Y 5
//2 Signals : QUIT and FULLSCREEN
#define NB_SIGNALS 2
#define S_FULLSCREEN 0
#define S_QUIT 1

#define RA_UNREFERENCED_PARAMETER(P)(P)

/************************************************************************/
/*           T Y P E   D E   B A S E   D E   L ' E N G I N              */
/************************************************************************/
//RA_TYPE == RealiteAugmente_TYPE

#ifdef _WIN32
	typedef __int8                  RA_CHAR;
	typedef unsigned __int8         RA_UCHAR;

	typedef __int16                 RA_SHORT;
	typedef unsigned __int16        RA_USHORT;

	typedef float                   RA_FLOAT;
	typedef double                  RA_DOUBLE;

	typedef __int32                 RA_INT;
	typedef unsigned __int32        RA_UINT;

	typedef long                    RA_LONG;
	typedef unsigned long           RA_ULONG;

	typedef RA_ULONG                RA_DWORD; 

	typedef bool                    RA_BOOL;

	typedef void                    RA_VOID;

	//typedef float *&              NULL_ptrref;
	#define NULL_PTR_REF *&0.0f;
#else
	typedef int8_t                  RA_CHAR;
	typedef uint8_t                RA_UCHAR;

	typedef int16_t                 RA_SHORT;
	typedef uint16_t               RA_USHORT;

	typedef float                   RA_FLOAT;
	typedef double                  RA_DOUBLE;

	typedef int32_t                 RA_INT;
	typedef uint32_t               RA_UINT;

	typedef long                    RA_LONG;
	typedef unsigned long           RA_ULONG;

	typedef RA_ULONG                RA_DWORD; 

	typedef bool                    RA_BOOL;

	typedef void                    RA_VOID;

	#ifndef ZeroMemory
	#define ZeroMemory bzero
	#endif

	#ifndef _strdup
	#define _strdup strdup
	#endif
#endif

#ifndef SUCCEEDED
	#define SUCCEEDED(b) (b>=0)
#endif

/************************************************************************/
/*           e T A T S   D E S   P e R I P H e R I Q U E S              */
/************************************************************************/
/**
@enum DEVICE_STATE
@brief Liste de tous les etats possibles pour un peripherique.
*/
enum DEVICE_STATE
{
  //! .
  RA_NON_INITIALISE =0,
  //! .
  RA_DISPONIBLE,
  //! .
  RA_NON_DISPONIBLE,
  //! .
  RA_EN_MARCHE
};


/************************************************************************/
/*             G E S T I O N      D E S     E R R E U R S               */
/************************************************************************/

/**
@enum RA_ERROR
@brief Les types d'erreurs declarees
*/
enum RA_ERROR
{
  // "FAILED"
  //! .
  RA_E_NO_VIDEO_DEVICE_FOUND = -10,
  //! .
  RA_E_INVALID_ARGUMENT,  
  //! .
  RA_E_FILE_NOT_FOUND,  
  //! .
  RA_E_OUT_OF_MEMORY,  
  //! .
  RA_NOT_ALLOCATED,
  //! .
  RA_E_NOT_LOADED,
  //! .
  RA_E_NOT_FOUND,
  //! .
  RA_CANCEL, 
  //! .
  RA_FAIL,
  //! .
  RA_NO,

  // "SUCCEEDED"
  //! .
  RA_OK,
  //! .
  RA_YES,   
  //! .
  RA_FOUND  
};

#define RA_FAILED(b) (b<0)

//Permet une evaluation rapide des code d'erreur
//S'il y a une erreur, un message est affiche dans la fenetre de debogage
#ifdef _DEBUG
#define RA_SUCCEEDED(b) V((b),__LINE__, __FILE__)
#else
#define RA_SUCCEEDED(b) (b>=0)
#endif // _DEBUG

/**
Affiche un message d'erreur dans le ::OutputDebugString() quand la condition est fausse pour
trouver rapiement d'ou viennent les erreurs.
*/
#ifdef _WIN32
inline RA_BOOL V(const RA_ERROR _error, const RA_LONG _line, const std::string _file)
{
  #ifdef _DEBUG
    if(RA_FAILED(_error))
    {
      //Il y a une erreur, un message est affiche dans la fenetre de bebogage
      //EX:
      //  * * * *
      //  * RA_ERROR Detected at (Line=   251 c:\fichier.cpp
      //  * * * *

      static TCHAR temp[1024];
      static TCHAR stars[10];

      wsprintf(stars,__TEXT("* * * *\n"));
      wsprintf(temp,__TEXT("* RA_ERROR Detected at (Line=%6d %s\n"),_line,_file.c_str());
      wsprintf(stars,__TEXT("* * * *\n"));
      
      //Affiche dans le fenetre de debogage
      ::OutputDebugString(stars);
      ::OutputDebugString(temp);
      ::OutputDebugString(stars);
    }
  #else
    RA_UNREFERENCED_PARAMETER(_error);
    RA_UNREFERENCED_PARAMETER(_line);
    RA_UNREFERENCED_PARAMETER(_file);
  #endif // _DEBUG  
    return (_error>=0);  
}
#else 	// LINUX
inline RA_BOOL V(const RA_ERROR _error, const RA_LONG _line, const std::string _file)
{
  #ifdef _DEBUG
    if(RA_FAILED(_error))
    {
      //Il y a une erreur, un message est affiche dans la fenetre de bebogage
      //EX:
      //  * * * *
      //  * RA_ERROR Detected at (Line=   251 c:\fichier.cpp
      //  * * * *

      static char temp[1024];
      static char stars[10];

      sprintf( stars, "* * * *\n");
      sprintf( temp, "* RA_ERROR Detected at (Line=%6ld %s\n", _line, _file.c_str());
      
      //Affiche dans le fenetre de debogage
      fprintf( stderr, "%s", stars);
      fprintf( stderr, "%s", temp);
      fprintf( stderr, "%s", stars);
    }
  #else
    RA_UNREFERENCED_PARAMETER(_error);
    RA_UNREFERENCED_PARAMETER(_line);
    RA_UNREFERENCED_PARAMETER(_file);
  #endif // _DEBUG  
    return (_error>=0);  
}

#endif

#ifdef _DEBUGDISPLAY
inline void debug_display(char* debug_explanation)
{
	fprintf(stderr,debug_explanation);
}
#else
inline void debug_display(char* debug_explanation)
{
    cout << debug_explanation << " line : " << __FILE__ << endl;
}
#endif

#endif //RACORE_H_
