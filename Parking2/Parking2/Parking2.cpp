// Parking2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "parking2.h"

#define ACERA 80
#define COCHES 10000
//Punteros a las funciones
//Creamos el puntero
int (*PARKING2_inicio)(TIPO_FUNCION_LLEGADA *, TIPO_FUNCION_SALIDA *, LONG, BOOL);
int (*PARKING2_fin)();
int (*PARKING2_aparcar)(HCoche, void *datos, TIPO_FUNCION_APARCAR_COMMIT,TIPO_FUNCION_PERMISO_AVANCE,TIPO_FUNCION_PERMISO_AVANCE_COMMIT);
int (*PARKING2_desaparcar)(HCoche, void *datos,TIPO_FUNCION_PERMISO_AVANCE,TIPO_FUNCION_PERMISO_AVANCE_COMMIT);
int (*PARKING2_getNUmero)(HCoche);
int (*PARKING2_getLongitud)(HCoche);
int (*PARKING2_getX)(HCoche);
int (*PARKING2_getY)(HCoche);
int (*PARKING2_getX2)(HCoche);
int (*PARKING2_getY2)(HCoche);
int (*PARKING2_getAlgoritmo)(HCoche);

//Prototipos.
int creaPunteros();
int compruebaArgs(int, char *[], BOOL *);
void updateCarreteraVertical(int algoritmo, int posInicial, int posFinal, int s, int posVertical, int longitud);
void updateCarreteraHorizontal(int algoritmo, int posInicial, int longitud);
void updateAcera(HCoche hc, int algoritmo, int posInicial, int longitud);
int crearSemaforos();
TIPO_FUNCION_LLEGADA mejorAjusteIN(HCoche hc);
TIPO_FUNCION_LLEGADA peorAjusteIN(HCoche hc);
TIPO_FUNCION_LLEGADA siguienteAjusteIN(HCoche hc);
TIPO_FUNCION_LLEGADA primerAjusteIN(HCoche hc);
TIPO_FUNCION_SALIDA mejorAjusteOUT(HCoche hc);
TIPO_FUNCION_SALIDA peorAjusteOUT(HCoche hc);
TIPO_FUNCION_SALIDA siguienteAjusteOUT(HCoche hc);
TIPO_FUNCION_SALIDA primerAjusteOUT(HCoche hc);
TIPO_FUNCION_APARCAR_COMMIT aparcarCommit(HCoche hc);
TIPO_FUNCION_PERMISO_AVANCE permisoAvanzar(HCoche hc);
TIPO_FUNCION_PERMISO_AVANCE_COMMIT permisoAvanzarCommit(HCoche hc);
DWORD WINAPI hiloAparcarMejor(LPVOID val);
DWORD WINAPI hiloAparcarPeor(LPVOID val);
DWORD WINAPI hiloAparcarPrimer(LPVOID val);
DWORD WINAPI hiloAparcarSiguiente(LPVOID val);

DWORD WINAPI hiloDesaparcar(LPVOID val);

typedef struct Data {
	int primerAjusteAcera[ACERA], peorAjusteAcera[ACERA],
		mejorAjusteAcera[ACERA], siguienteAjusteAcera[ACERA];
	int SIGUIENTEPOSICION = -3;
	HANDLE semaforosPrimer[ACERA], semaforosPeor[ACERA], semaforosMejor[ACERA],
		semaforosSiguiente[ACERA],ordenMejor[COCHES], ordenPeor[COCHES],
		ordenPrimer[COCHES], ordenSiguiente[COCHES];
	HANDLE semMejor,semPeor,semPrimer,semSiguiente;
};
Data data;

int main(int argc, char * argv[]){
	BOOL debug = false;
	if (creaPunteros() == 1) return 1;
	if (compruebaArgs(argc, argv, &debug)==1) return 1;
	if (crearSemaforos() == 1) return 1;
	TIPO_FUNCION_LLEGADA f_llegadas[] = { (TIPO_FUNCION_LLEGADA)primerAjusteIN,
		(TIPO_FUNCION_LLEGADA)siguienteAjusteIN,
		(TIPO_FUNCION_LLEGADA)mejorAjusteIN,
		(TIPO_FUNCION_LLEGADA)peorAjusteIN };
	TIPO_FUNCION_SALIDA f_salidas[] = { (TIPO_FUNCION_SALIDA)primerAjusteOUT,
		(TIPO_FUNCION_SALIDA)siguienteAjusteOUT,
		(TIPO_FUNCION_SALIDA)mejorAjusteOUT,
		(TIPO_FUNCION_SALIDA)peorAjusteOUT };
	
	//Cargamos la carretera
	if (PARKING2_inicio(f_llegadas, f_salidas, atoi(argv[1]), debug) == -1) {
		PERROR("Error Parking2_inicio");
		system("pause");
		return 1;
	}
	Sleep(30000);
	PARKING2_fin();
	system("cls");
    return 0;
}

int crearSemaforos() {
	int i;
	for (i = 0; i < COCHES; i++) {
		data.ordenPeor[i] = CreateSemaphore(NULL, 0, 1, NULL);
		data.ordenPrimer[i] = CreateSemaphore(NULL, 0, 1, NULL);
		data.ordenMejor[i] = CreateSemaphore(NULL, 0, 1, NULL);
		data.ordenSiguiente[i] = CreateSemaphore(NULL, 0, 1, NULL);
		if (data.ordenPrimer[i] == NULL) {
			PERROR("Error CreateSemaphore.");
			return 1;
		}
		if (data.ordenPeor[i] == NULL) {
			PERROR("Error CreateSemaphore.");
			return 1;
		}
		if (data.ordenMejor[i] == NULL) {
			PERROR("Error CreateSemaphore.");
			return 1;
		}
		if (data.ordenSiguiente[i] == NULL) {
			PERROR("Error CreateSemaphore.");
			return 1;
		}
	}
	for (i = 0; i < ACERA; i++) {
		data.semaforosPrimer[i] = CreateSemaphore(NULL, 1, 1, NULL);
		data.semaforosPeor[i] = CreateSemaphore(NULL, 1, 1, NULL);
		data.semaforosSiguiente[i] = CreateSemaphore(NULL, 1, 1, NULL);
		data.semaforosMejor[i] = CreateSemaphore(NULL, 1, 1, NULL);
		if (data.semaforosPrimer[i] == NULL) {
			PERROR("Error CreateSemaphore.");
			return 1;
		}
		if (data.semaforosMejor[i] == NULL) {
			PERROR("Error CreateSemaphore.");
			return 1;
		}
		if (data.semaforosPeor[i] == NULL) {
			PERROR("Error CreateSemaphore.");
			return 1;
		}
		if (data.semaforosSiguiente[i] == NULL) {
			PERROR("Error CreateSemaphore.");
			return 1;
		}
	}
	data.semPrimer = CreateSemaphore(NULL, 1, 1, NULL);
	data.semPeor = CreateSemaphore(NULL, 1, 1, NULL);
	data.semMejor = CreateSemaphore(NULL, 1, 1, NULL);
	data.semSiguiente = CreateSemaphore(NULL, 1, 1, NULL);
	if (data.semMejor == NULL) {
		PERROR("Error CreateSemaphore");
		return 1;
	}
	if (data.semPeor == NULL) {
		PERROR("Error CreateSemaphore");
		return 1;
	}
	if (data.semSiguiente == NULL) {
		PERROR("Error CreateSemaphore");
		return 1;
	}
	if (data.semPrimer == NULL) {
		PERROR("Error CreateSemaphore");
		return 1;
	}
	return 0;
}

TIPO_FUNCION_LLEGADA mejorAjusteIN(HCoche hc){
	int matrizHuecos[ACERA][2];
	int i, j, longitudCoche = PARKING2_getLongitud(hc), numHuecos = 0;
	int contador = 0, huecoMin = 0, posFinal = 0, posInicial;
	HANDLE thread;
	WaitForSingleObject(data.semMejor, INFINITE);
	for (i = 0; i<ACERA; i++)
		for (j = 0; j<2; j++)
			matrizHuecos[i][j] = 0;

	for (i = 0; i<ACERA; i++) {
		if (data.mejorAjusteAcera[i] != 1) {
			contador++;
			if (contador == longitudCoche) {
				matrizHuecos[numHuecos][0] = contador; //Tamaño
				matrizHuecos[numHuecos][1] = i - longitudCoche + 1;
				numHuecos++;
			}
			else if (contador>longitudCoche) { //Me sobran huecos
				matrizHuecos[numHuecos - 1][0] = contador;//Tamaño
				matrizHuecos[numHuecos - 1][1] = matrizHuecos[numHuecos - 1][1];
			}
		}
		else {
			contador = 0;
		}
	}
	huecoMin = 100;
	for (i = 0; i<ACERA; i++) {
		if (huecoMin>matrizHuecos[i][0] && matrizHuecos[i][0] != 0) {
			huecoMin = matrizHuecos[i][0];
			posInicial = matrizHuecos[i][1];
		}
	}

	if (huecoMin >= longitudCoche && huecoMin != 100) {
		for (i = posInicial; i<posInicial + longitudCoche; i++)
			data.mejorAjusteAcera[i] = 1;
		thread = CreateThread(NULL, 0, hiloAparcarMejor, (LPVOID)hc, NULL, NULL);
		if (thread == NULL) {
			PERROR("CreateThread");
		}
		CloseHandle(thread);
		ReleaseSemaphore(data.semMejor, 1, NULL);
		return (TIPO_FUNCION_LLEGADA)posInicial;
	}

	ReleaseSemaphore(data.semMejor, 1, NULL);
	return (TIPO_FUNCION_LLEGADA)-1;


}
TIPO_FUNCION_LLEGADA peorAjusteIN(HCoche hc){
	WaitForSingleObject(data.semPeor, INFINITE);
	int matrizHuecos[ACERA][2];
	int i, j, longitudCoche = PARKING2_getLongitud(hc), numHuecos = 0;
	int contador = 0, huecoMax = 0, posFinal = 0, posInicial;
	HANDLE thread;

	for (i = 0; i<ACERA; i++)
		for (j = 0; j<2; j++)
			matrizHuecos[i][j] = 0;

	for (i = 0; i<ACERA; i++) {
		if (data.peorAjusteAcera[i] != 1) {
			contador++;
			if (contador == longitudCoche) {
				matrizHuecos[numHuecos][0] = contador; //Tamaño
				matrizHuecos[numHuecos][1] = i - longitudCoche + 1;
				numHuecos++;
			}
			else if (contador>longitudCoche) { //Me sobran huecos
				matrizHuecos[numHuecos][0] = contador;//Tamaño
				matrizHuecos[numHuecos][1] = matrizHuecos[numHuecos - 1][1];
				numHuecos++;
			}
		}
		else {
			contador = 0;
		}


	}
	for (i = 0; i<ACERA; i++) {
		if (huecoMax<matrizHuecos[i][0]) {
			huecoMax = matrizHuecos[i][0];
			posInicial = matrizHuecos[i][1];
		}
	}
	if (huecoMax >= longitudCoche) {
		for (i = posInicial; i<posInicial + longitudCoche; i++)
			data.peorAjusteAcera[i] = 1;
		thread = CreateThread(NULL, 0, hiloAparcarPeor, (LPVOID)hc, NULL, NULL);
		if ( thread == NULL) {
			PERROR("CreateThread");
		}
		CloseHandle(thread);
		ReleaseSemaphore(data.semPeor, 1, NULL);
		return (TIPO_FUNCION_LLEGADA)posInicial;
	}
	ReleaseSemaphore(data.semPeor, 1, NULL);
	return (TIPO_FUNCION_LLEGADA)-1;
}
TIPO_FUNCION_LLEGADA siguienteAjusteIN(HCoche hc){
	int lastPosition = -5, flag = 0;
	HANDLE thread;
	WaitForSingleObject(data.semSiguiente, INFINITE);
	int i, j, longitudCoche = PARKING2_getLongitud(hc), contador = 0;
	lastPosition = data.SIGUIENTEPOSICION;
	if (lastPosition == -3) {

		lastPosition = 0;//Para el bucle.
	}
	if (data.siguienteAjusteAcera[lastPosition] != 1) {
		for (i = lastPosition; i >= 0; i--) {
			if (data.siguienteAjusteAcera[i] == 1 && flag == 0) {
				lastPosition = i;
				flag = 1;
			}
			if (i == 0 && flag == 0)
				lastPosition = 0;
		}
	}

	for (i = lastPosition; i<ACERA; i++) {
		if (data.siguienteAjusteAcera[i] == 0) {
			contador++;
			if (contador == longitudCoche) {
				for (j = i - longitudCoche + 1; j <= i; j++) {
					data.siguienteAjusteAcera[j] = 1;
				}
				data.SIGUIENTEPOSICION = i;
				thread = CreateThread(NULL, 0, hiloAparcarSiguiente, (LPVOID)hc, NULL, NULL);
				if (thread== NULL) {
					PERROR("CreateThread");
				}
				CloseHandle(thread);
				ReleaseSemaphore(data.semSiguiente, 1, NULL);
				return (TIPO_FUNCION_LLEGADA)(i - longitudCoche + 1);
			}
		}
		else {
			contador = 0;
		}
	}
	contador = 0;
	for (i = 0; i< lastPosition; i++) {
		if (data.siguienteAjusteAcera[i] != 1) {
			contador++;
			if (contador == longitudCoche) {
				for (j = i - longitudCoche + 1; j <= i; j++) {
					data.siguienteAjusteAcera[j] = 1;
				}
				data.SIGUIENTEPOSICION = i;
				thread = CreateThread(NULL, 0, hiloAparcarSiguiente, (LPVOID)hc, NULL, NULL);
				if (thread== NULL) {
					PERROR("CreateThread");
				}
				CloseHandle(thread);
				ReleaseSemaphore(data.semSiguiente, 1, NULL);
				return (TIPO_FUNCION_LLEGADA)(i - longitudCoche + 1);
			}
		}
		else {
			contador = 0;
		}
	}
	ReleaseSemaphore(data.semSiguiente, 1, NULL);
	return (TIPO_FUNCION_LLEGADA)-1;
}
TIPO_FUNCION_LLEGADA primerAjusteIN(HCoche hc){
	WaitForSingleObject(data.semPrimer, INFINITE);
	HANDLE thread;
	int i, j, longitudCoche = PARKING2_getLongitud(hc), contador = 0;
	for (i = 0; i<ACERA; i++) {
		if (data.primerAjusteAcera[i] != 1) {
			contador++;
			if (contador == longitudCoche) {
				for (j = i - longitudCoche + 1; j <= i; j++) {
					data.primerAjusteAcera[j] = 1;
				}
				thread = CreateThread(NULL, 0, hiloAparcarPrimer, (LPVOID)hc, NULL, NULL);
				if (thread== NULL) {
					PERROR("CreateThread");
				}
				CloseHandle(thread);
				ReleaseSemaphore(data.semPrimer, 1, NULL);
				return (TIPO_FUNCION_LLEGADA) (i - longitudCoche + 1);
			}
		}
		else {
			contador = 0;
		}
	}
	ReleaseSemaphore(data.semPrimer, 1, NULL);
	return (TIPO_FUNCION_LLEGADA)-1;

}

TIPO_FUNCION_SALIDA mejorAjusteOUT(HCoche hc){
	HANDLE thread;
	thread = CreateThread(NULL, 0, hiloDesaparcar, (LPVOID)hc, NULL, NULL);
	if (thread == NULL) {
		PERROR("CreateThread");
	}
	CloseHandle(thread);
	return (TIPO_FUNCION_SALIDA)1;
}
TIPO_FUNCION_SALIDA peorAjusteOUT(HCoche hc){
	HANDLE thread;
	thread = CreateThread(NULL, 0, hiloDesaparcar, (LPVOID)hc, NULL, NULL);
	if (thread == NULL) {
		PERROR("CreateThread");
	}
	CloseHandle(thread);
	return (TIPO_FUNCION_SALIDA)1;
}
TIPO_FUNCION_SALIDA siguienteAjusteOUT(HCoche hc){
	HANDLE thread;
	thread = CreateThread(NULL, 0, hiloDesaparcar, (LPVOID)hc, NULL, NULL);
	if (thread == NULL) {
		PERROR("CreateThread");
	}
	CloseHandle(thread);
	return (TIPO_FUNCION_SALIDA)1;
}
TIPO_FUNCION_SALIDA primerAjusteOUT(HCoche hc){
	HANDLE thread;
	thread = CreateThread(NULL, 0, hiloDesaparcar, (LPVOID)hc, NULL, NULL);
	if (thread == NULL) {
		PERROR("CreateThread");
	}
	CloseHandle(thread);
	return (TIPO_FUNCION_SALIDA)1;
}

TIPO_FUNCION_APARCAR_COMMIT aparcarCommit(HCoche hc) {
	switch (PARKING2_getAlgoritmo(hc)) {
	case 0:
		ReleaseSemaphore(data.ordenPrimer[PARKING2_getNUmero(hc) + 1], 1, NULL);
		break;
	case 1:
		ReleaseSemaphore(data.ordenSiguiente[PARKING2_getNUmero(hc) + 1], 1, NULL);
		break;
	case 2:
		ReleaseSemaphore(data.ordenMejor[PARKING2_getNUmero(hc) + 1], 1, NULL);
		break;
	case 3:
		ReleaseSemaphore(data.ordenPeor[PARKING2_getNUmero(hc) + 1], 1, NULL);
		break;
	}
	return (TIPO_FUNCION_APARCAR_COMMIT)1;
}
TIPO_FUNCION_PERMISO_AVANCE permisoAvanzar(HCoche hc) {
	int i;
	switch (PARKING2_getAlgoritmo(hc)) {
	case 0: //Primer Ajuste...
		if (PARKING2_getX(hc) != PARKING2_getX2(hc)) {
			if (PARKING2_getX2(hc) >= 0) {
				WaitForSingleObject(data.semaforosPrimer[PARKING2_getX2(hc)],INFINITE);
			}
		}
		if (PARKING2_getY2(hc) == 2 && PARKING2_getY(hc) == 1) {//Si estoy en la linea y voy a la carretera...
			for (i = PARKING2_getX(hc) + PARKING2_getLongitud(hc) - 1; i >= PARKING2_getX(hc); i--) {
				WaitForSingleObject(data.semaforosPrimer[i], INFINITE);
			}
		}
		return (TIPO_FUNCION_PERMISO_AVANCE)1; //Puede moverse
		break; //End of primer ajuste.
	case 1: //Primer Ajuste...
		if (PARKING2_getX(hc) != PARKING2_getX2(hc)) {
			if (PARKING2_getX2(hc) >= 0) {
				WaitForSingleObject(data.semaforosSiguiente[PARKING2_getX2(hc)], INFINITE);
			}
		}
		if (PARKING2_getY2(hc) == 2 && PARKING2_getY(hc) == 1) {//Si estoy en la linea y voy a la carretera...
			for (i = PARKING2_getX(hc) + PARKING2_getLongitud(hc) - 1; i >= PARKING2_getX(hc); i--) {
				WaitForSingleObject(data.semaforosSiguiente[i], INFINITE);
			}
		}
		return (TIPO_FUNCION_PERMISO_AVANCE)1; //Puede moverse
		break; //End of primer ajuste.
	case 2: //Primer Ajuste...
		if (PARKING2_getX(hc) != PARKING2_getX2(hc)) {
			if (PARKING2_getX2(hc) >= 0) {
				WaitForSingleObject(data.semaforosMejor[PARKING2_getX2(hc)], INFINITE);
			}
		}
		if (PARKING2_getY2(hc) == 2 && PARKING2_getY(hc) == 1) {//Si estoy en la linea y voy a la carretera...
			for (i = PARKING2_getX(hc) + PARKING2_getLongitud(hc) - 1; i >= PARKING2_getX(hc); i--) {
				WaitForSingleObject(data.semaforosMejor[i], INFINITE);
			}
		}
		return (TIPO_FUNCION_PERMISO_AVANCE)1; //Puede moverse
		break; //End of primer ajuste.
	case 3: //Primer Ajuste...
		if (PARKING2_getX(hc) != PARKING2_getX2(hc)) {
			if (PARKING2_getX2(hc) >= 0) {
				WaitForSingleObject(data.semaforosPeor[PARKING2_getX2(hc)], INFINITE);
			}
		}
		if (PARKING2_getY2(hc) == 2 && PARKING2_getY(hc) == 1) {//Si estoy en la linea y voy a la carretera...
			for (i = PARKING2_getX(hc) + PARKING2_getLongitud(hc) - 1; i >= PARKING2_getX(hc); i--) {
				WaitForSingleObject(data.semaforosPeor[i], INFINITE);
			}
		}
		return (TIPO_FUNCION_PERMISO_AVANCE)1; //Puede moverse
		break; //End of primer ajuste.
	}
	return (TIPO_FUNCION_PERMISO_AVANCE)1;
}
TIPO_FUNCION_PERMISO_AVANCE_COMMIT permisoAvanzarCommit(HCoche hc) {	
	int i;
		if (PARKING2_getX(hc) == PARKING2_getX2(hc)) {//Ha sido un movimiento vertical.
			updateCarreteraVertical(PARKING2_getAlgoritmo(hc), PARKING2_getX(hc), PARKING2_getX2(hc), PARKING2_getY(hc), PARKING2_getY2(hc), PARKING2_getLongitud(hc));
		}
		else {
			updateCarreteraHorizontal(PARKING2_getAlgoritmo(hc), PARKING2_getX2(hc), PARKING2_getLongitud(hc));
		}
		if (PARKING2_getY2(hc) == 1 && PARKING2_getY(hc) == 2) {
			updateAcera(hc,PARKING2_getAlgoritmo(hc), PARKING2_getX(hc), PARKING2_getLongitud(hc));
		}
	return (TIPO_FUNCION_PERMISO_AVANCE_COMMIT)1;
}

void updateCarreteraVertical(int algoritmo, int posActual, int posPrevia, int posVerticalActual, int posVerticalPrevia, int longitud) {
	int i;
	switch (algoritmo) {
	case 0:
		if (posVerticalPrevia == 2)
			for (i = posActual; i < posActual + longitud; i++)
				ReleaseSemaphore(data.semaforosPrimer[i], 1, NULL);
		break;
	case 1:
		if (posVerticalPrevia == 2)
			for (i = posActual; i < posActual + longitud; i++)
				ReleaseSemaphore(data.semaforosSiguiente[i], 1, NULL);
		break;
	case 2:
		if (posVerticalPrevia == 2)
			for (i = posActual; i < posActual + longitud; i++)
				ReleaseSemaphore(data.semaforosMejor[i], 1, NULL);
		break;
	case 3:
		if (posVerticalPrevia == 2)
			for (i = posActual; i < posActual + longitud; i++)
				ReleaseSemaphore(data.semaforosPeor[i], 1, NULL);
		break;
	}
}

void updateCarreteraHorizontal(int algoritmo, int posInicial, int longitud) {
	int i;
	switch (algoritmo) {
	case 0:
		if (posInicial<0) { //Estamos fuera de la pantalla
			ReleaseSemaphore(data.semaforosPrimer[posInicial + longitud - 1], 1, NULL);
		}
		else if (posInicial == 0) {
			ReleaseSemaphore(data.semaforosPrimer[longitud - 1], 1, NULL);
		}
		else if (posInicial + longitud - 1<ACERA) {
			ReleaseSemaphore(data.semaforosPrimer[posInicial + longitud - 1], 1, NULL);
		}
		break;
	case 1:
		if (posInicial<0) { //Estamos fuera de la pantalla
			ReleaseSemaphore(data.semaforosSiguiente[posInicial + longitud - 1], 1, NULL);
		}
		else if (posInicial == 0) {
			ReleaseSemaphore(data.semaforosSiguiente[longitud - 1], 1, NULL);
		}
		else if (posInicial + longitud - 1<ACERA) {
			ReleaseSemaphore(data.semaforosSiguiente[posInicial + longitud - 1], 1, NULL);
		}
		break;
	case 2:
		if (posInicial<0) { //Estamos fuera de la pantalla
			ReleaseSemaphore(data.semaforosMejor[posInicial + longitud - 1], 1, NULL);
		}
		else if (posInicial == 0) {
			ReleaseSemaphore(data.semaforosMejor[longitud - 1], 1, NULL);
		}
		else if (posInicial + longitud - 1<ACERA) {
			ReleaseSemaphore(data.semaforosMejor[posInicial + longitud - 1], 1, NULL);
		}
		break;
	case 3:
		if (posInicial<0) { //Estamos fuera de la pantalla
			ReleaseSemaphore(data.semaforosPeor[posInicial + longitud - 1], 1, NULL);
		}
		else if (posInicial == 0) {
			ReleaseSemaphore(data.semaforosPeor[longitud - 1], 1, NULL);
		}
		else if (posInicial + longitud - 1<ACERA) {
			ReleaseSemaphore(data.semaforosPeor[posInicial + longitud - 1], 1, NULL);
		}
		break;
	}
}
void updateAcera(HCoche hc, int algoritmo, int posInicial, int longitud) {
	int i;	
	switch (algoritmo) {
	case 0: //Primer algoritmo.
		WaitForSingleObject(data.semPrimer, INFINITE);
		for (i = posInicial; i< (posInicial + longitud); i++) {
			data.primerAjusteAcera[i] = 0;
		}
		ReleaseSemaphore(data.semPrimer, 1, NULL);
		break;
	case 1: //Primer algoritmo.
		WaitForSingleObject(data.semSiguiente, INFINITE);
		for (i = posInicial; i< (posInicial + longitud); i++) {
			data.siguienteAjusteAcera[i] = 0;
		}
		ReleaseSemaphore(data.semSiguiente, 1, NULL);
		break;
	case 2: //Primer algoritmo.
		WaitForSingleObject(data.semMejor, INFINITE);
		for (i = posInicial; i< (posInicial + longitud); i++) {
			data.mejorAjusteAcera[i] = 0;
		}
		ReleaseSemaphore(data.semMejor, 1, NULL);
		break;
	case 3: //Primer algoritmo.
		WaitForSingleObject(data.semPeor, INFINITE);
		for (i = posInicial; i< (posInicial + longitud); i++) {
			data.peorAjusteAcera[i] = 0;
		}
		ReleaseSemaphore(data.semPeor, 1, NULL);
		break;
	}
}
DWORD WINAPI hiloDesaparcar(LPVOID hCoche) {
	PARKING2_desaparcar((HCoche)hCoche, NULL,(TIPO_FUNCION_PERMISO_AVANCE)permisoAvanzar, (TIPO_FUNCION_PERMISO_AVANCE_COMMIT)permisoAvanzarCommit);
	return 1;
}
DWORD WINAPI hiloAparcarMejor(LPVOID hCoche) {
	HCoche hc= (HCoche)hCoche;
	if (PARKING2_getNUmero(hc) > 1) {
		WaitForSingleObject(data.ordenMejor[PARKING2_getNUmero(hc)], INFINITE);
	}
	PARKING2_aparcar(hc,NULL, (TIPO_FUNCION_APARCAR_COMMIT) aparcarCommit,
		(TIPO_FUNCION_PERMISO_AVANCE) permisoAvanzar, (TIPO_FUNCION_PERMISO_AVANCE_COMMIT) permisoAvanzarCommit);
	return 1;
}
DWORD WINAPI hiloAparcarSiguiente(LPVOID hCoche) {
	HCoche hc = (HCoche)hCoche;
	if (PARKING2_getNUmero(hc) > 1) {
		WaitForSingleObject(data.ordenSiguiente[PARKING2_getNUmero(hc)], INFINITE);
	}
	PARKING2_aparcar(hc, NULL, (TIPO_FUNCION_APARCAR_COMMIT)aparcarCommit,
		(TIPO_FUNCION_PERMISO_AVANCE)permisoAvanzar, (TIPO_FUNCION_PERMISO_AVANCE_COMMIT)permisoAvanzarCommit);
	return 1;
}
DWORD WINAPI hiloAparcarPrimer(LPVOID hCoche) {
	HCoche hc = (HCoche)hCoche;
	if (PARKING2_getNUmero(hc) > 1) {
		WaitForSingleObject(data.ordenPrimer[PARKING2_getNUmero(hc)], INFINITE);
	}
	PARKING2_aparcar(hc,NULL, (TIPO_FUNCION_APARCAR_COMMIT)aparcarCommit,
		(TIPO_FUNCION_PERMISO_AVANCE)permisoAvanzar, (TIPO_FUNCION_PERMISO_AVANCE_COMMIT)permisoAvanzarCommit);
	return 1;
}
DWORD WINAPI hiloAparcarPeor(LPVOID hCoche) {
	HCoche hc = (HCoche)hCoche;
	if (PARKING2_getNUmero(hc) > 1) {
		WaitForSingleObject(data.ordenPeor[PARKING2_getNUmero(hc)], INFINITE);
	}
	PARKING2_aparcar(hc, NULL, (TIPO_FUNCION_APARCAR_COMMIT)aparcarCommit,
		(TIPO_FUNCION_PERMISO_AVANCE)permisoAvanzar, (TIPO_FUNCION_PERMISO_AVANCE_COMMIT)permisoAvanzarCommit);
	return 1;
}
int compruebaArgs(int argc, char *argv[], BOOL *debug) {
	switch (argc) {
		case 2:
			if (atoi(argv[1])<0) { //<0
				fprintf(stderr, "Error: Velocidad invalida. Esta debe ser mayor o igual que 0.\n");
				return 1;
			}
			else
				return 0;
		case 3:
			if (atoi(argv[1])<0) { //<0
				fprintf(stderr, "Error: Velocidad invalida. Esta debe ser mayor o igual que 0.\n");
				return 1;
			}
			if (strcmp(argv[2], "D"))
				return 1;
			*debug= true;
			return 0;
		default:
			fprintf(stderr, "Error: Argumentos invalidos.\nEl modo de uso es el siguiente: parking [velocidad][numChoferes][PX][D]\nX puede ser 'D' (Prioridad para desaparcar) o 'A' (Prioridad para aparcar))\n");
			return 1;
	}
}
int creaPunteros() {
	HINSTANCE controladorDLL;
	//Cargar biblioteca
	controladorDLL = LoadLibrary("parking2.dll");
	if (controladorDLL == NULL) {
		PERROR("Error LoadLibrary.");
		system("pause");
		return 1;
	}
	//Obtención de punteros a las funciones.
	PARKING2_inicio = (int(*)(TIPO_FUNCION_LLEGADA *, TIPO_FUNCION_SALIDA *, LONG, BOOL))
		GetProcAddress(controladorDLL, "PARKING2_inicio");
	if (PARKING2_inicio == NULL) {
		PERROR("Error GetProcAddres Parking2_inicio.");
		system("pause");
		return 1;
	}
	PARKING2_fin = (int(*)())GetProcAddress(controladorDLL, "PARKING2_fin");
	if (PARKING2_fin == NULL) {
		PERROR("Error GetProcAddres Parking2_fin.");
		system("pause");
		return 1;
	}
	PARKING2_aparcar = (int(*)(HCoche, void *datos, TIPO_FUNCION_APARCAR_COMMIT,
		TIPO_FUNCION_PERMISO_AVANCE,
		TIPO_FUNCION_PERMISO_AVANCE_COMMIT))
		GetProcAddress(controladorDLL, "PARKING2_aparcar");
	if (PARKING2_aparcar == NULL) {
		PERROR("Error GetProcAddres Parking2_aparcar.");
		system("pause");
		return 1;
	}
	PARKING2_desaparcar = (int(*)(HCoche, void *datos,
		TIPO_FUNCION_PERMISO_AVANCE,
		TIPO_FUNCION_PERMISO_AVANCE_COMMIT))
		GetProcAddress(controladorDLL, "PARKING2_desaparcar");
	if (PARKING2_desaparcar == NULL) {
		PERROR("Error GetProcAddres Parking2_desaparcar.");
		system("pause");
		return 1;
	}

	PARKING2_getNUmero = (int(*)(HCoche))
		GetProcAddress(controladorDLL, "PARKING2_getNUmero");
	if (PARKING2_getNUmero == NULL) {
		PERROR("Error GetProcAddres Parking2_getNUmero.");
		system("pause");
		return 1;

	}
	PARKING2_getLongitud = (int(*)(HCoche))
		GetProcAddress(controladorDLL, "PARKING2_getLongitud");
	if (PARKING2_getLongitud == NULL) {
		PERROR("Error GetProcAddres Parking2_getLongitud.");
		system("pause");
		return 1;

	}
	PARKING2_getX = (int(*)(HCoche))
		GetProcAddress(controladorDLL, "PARKING2_getX");
	if (PARKING2_getX == NULL) {
		PERROR("Error GetProcAddres Parking2_getX.");
		system("pause");
		return 1;

	}
	PARKING2_getY = (int(*)(HCoche))
		GetProcAddress(controladorDLL, "PARKING2_getY");
	if (PARKING2_getY == NULL) {
		PERROR("Error GetProcAddres Parking2_getY.");
		system("pause");
		return 1;

	}
	PARKING2_getX2 = (int(*)(HCoche))
		GetProcAddress(controladorDLL, "PARKING2_getX2");
	if (PARKING2_getX2 == NULL) {
		PERROR("Error GetProcAddres Parking2_getX2.");
		system("pause");
		return 1;

	}
	PARKING2_getY2 = (int(*)(HCoche))
		GetProcAddress(controladorDLL, "PARKING2_getY2");
	if (PARKING2_getY2 == NULL) {
		PERROR("Error GetProcAddres Parking2_getY2.");
		system("pause");
		return 1;

	}
	PARKING2_getAlgoritmo = (int(*)(HCoche))
		GetProcAddress(controladorDLL, "PARKING2_getAlgoritmo");
	if (PARKING2_getAlgoritmo == NULL) {
		PERROR("Error GetProcAddres Parking2_getAlgoritmo.");
		system("pause");
		return 1;

	}
	return 0;

}