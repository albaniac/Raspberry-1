/*
 * GestionAD5220.cpp
 *
 *  Created on: 17 octobre 2016
 *      Author: totof
 * Controle un AD5220 avec la librairie wiringPi sur Raspberry Pi
*/
 
// *************************************************
// Les circuits peuvent �tre cabl�s avec ou sans CS
// Les broches U/D et CLK peuvent �tre communes avec
// s�lection par la broche CS
// *************************************************

#include "GestionAD5220.h"

namespace std {

// ***************************************************
// Constructeur
// @param pPinCLK numero de la pin GPIO horloge
// @param pPinUD numero de la pin GPIO Up/Dn
// @param pValueOhm valeur de la r�sistance
// Constructeur a utiliser si la pin CS est � la masse
// ***************************************************
GestionAD5220::GestionAD5220(PinGPIO pPinCLK, PinGPIO pPinUD, unsigned int pValueOhm) {
	valueOhm = pValueOhm;
	init(pPinCLK, pPinUD);
	utilCS = false;
}

// ************************************************
// Constructeur
// @param pPinCLK numero de la pin GPIO horloge
// @param pPinUD numero de la pin GPIO Up/Dn
// @param pPinCS numero de la pin GPIO CS
// @param pValueOhm valeur de la r�sistance
// Constructeur a utiliser si la pin CS est utilise
// ************************************************
GestionAD5220::GestionAD5220(PinGPIO pPinCLK, PinGPIO pPinUD, PinGPIO pPinCS, unsigned int pValueOhm) {
	valueOhm = pValueOhm;
	CS.init(pPinCS);
	CS.out();
	CS.on();
	init(pPinCLK, pPinUD);
	utilCS = true;
}

// ***************************************************
// Methode de construction generique
// @param pPinCLK numero de la pin GPIO horloge
// @param pPinUD numero de la pin GPIO Up/Dn
// Initialise les pins clock et Up/Dn
// Positionne la valeur � la valeur de depart
// ***************************************************
void GestionAD5220::init(PinGPIO pPinCLK, PinGPIO pPinUD) {
	CLK.init(pPinCLK);
	CLK.out();
	CLK.on();
	UD.init(pPinUD);
	UD.out();
	UD.off();
	value = START_VALUE;
}

// ******************************
// Active la broche CS si utilis�
// ******************************
void GestionAD5220::activeCircuit(void) {
	if(utilCS) {
		CS.off();
	}
}

// ********************************
// Deactive la broche CS si utilis�
// ********************************
void GestionAD5220::deActiveCircuit(void) {
	if(utilCS) {
		CS.on();
	}
}

// ***********************
// Une impulsion d'horloge
// ***********************
void GestionAD5220::pulseClock(void) {
	CLK.on();
	CLK.off();	
}

// ****************************************
// Incremente la valeur du potentiometres
// @return true si commande ok, sinon false
// ****************************************
bool GestionAD5220::increment(void) {
	// Potentiometre deja au maximum
	if(value >= MAX_VALUE) {
		return false;
	}
	activeCircuit();
	// Passe en mode UP
	UD.on();
	pulseClock();
	deActiveCircuit();
	// Incremente la valeur
	value++;
	return true;
}

// ********************************************
// Decremente la valeur d'un des potentiometres
// @return true si commande ok, sinon false
// ********************************************
bool GestionAD5220::decrement(void) {
	// Potentiometre deja au minimum
	if(value <= MIN_VALUE) {
		return false;
	}
	activeCircuit();
	// Passe en mode DOWN
	UD.off();
	pulseClock();
	deActiveCircuit();
	// Decremente la valeur
	value--;
	return true;
}

// ***************************************************
// Positionne le potentiometre a la valeur START_VALUE
// Cette fonction force la valeur si la valeur du 
// potentiom�tre n'est plus sur
// ***************************************************
void GestionAD5220::reInit(void) {
	activeCircuit();
	UD.off();
	for(int compteur = 0; compteur != MAX_VALUE; compteur++) {
		pulseClock();
	}
	deActiveCircuit();
	value = MIN_VALUE;
	write(START_VALUE);
}

// *************************************************
// Positionne le potentiometre a une valeur
// @return false si pValue ne peut pas �tre atteinte
//         sinon true
// *************************************************
bool GestionAD5220::write(unsigned int pValue) {
	if(value > pValue) {
		while(value != pValue) {
			if(!decrement()) {
				return false;
			} 
		}
	} else {
		while(value != pValue) {
			if(!increment()) {
				return false;
			}
		}
	}	
	return true;
}

// ********************************
// Donne la valeur du potentiometre
// @return la valeur
// ********************************
unsigned int GestionAD5220::read(void) {
	return value;
}

// *************************************************
// Positionne le potentiometre a une valeur en ohm
// @return false si pValue ne peut pas �tre atteinte
//         sinon true
// *************************************************
bool GestionAD5220::writeOhm(unsigned int pValue) {
	return write(((float)pValue / (float)valueOhm) * MAX_VALUE);
}

// ********************************
// Donne la valeur du potentiometre
// @return la valeur
// ********************************
unsigned int GestionAD5220::readOhm(void) {
	return ((float)value / (float)MAX_VALUE) * valueOhm;
}

// ************************************
// Donne la valeur min du potentiometre
// @return la valeur
// ************************************
unsigned int GestionAD5220::getMinValue(void) {
	return MIN_VALUE;
}

// ************************************
// Donne la valeur max du potentiometre
// @return la valeur
// ************************************
unsigned int GestionAD5220::getMaxValue(void) {
	return MAX_VALUE;
}

// ******************************************
// Demande si le destructeur sera appliqu�
// return destroyFlag, le flag de destructeur
// ******************************************
bool GestionAD5220::isToDesactivate(void) {
	return desactivateFlag;
}

// ****************************************
// Indique que le destructeur sera appliqu�
// ****************************************
void GestionAD5220::toDesactivate(void) {
	setToDesactivate(true);
}

// ***********************************************
// Indique que le destructeur ne sera pas appliqu�
// ***********************************************
void GestionAD5220::noDesactivate(void) {
	setToDesactivate(false);
}

// ************************************
// Positionne le flag de destructeur
// @param pValue le flag de destructeur
// ************************************
void GestionAD5220::setToDesactivate(bool pValue) {
	desactivateFlag = pValue;
}

// *******************************************
// Destructeur
// Repositionne la valeur � la valeur initiale
// *******************************************
GestionAD5220::~GestionAD5220() {
 	if(isToDesactivate()) {
		write(START_VALUE);
	} else {
		CS.noDesactivate();
		UD.noDesactivate();
		CLK.noDesactivate();		
	}
}

} /* namespace std */
