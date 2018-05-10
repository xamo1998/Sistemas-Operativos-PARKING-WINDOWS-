// Parking2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "parking2.h"

#define ACERA 80
#define PRIMERAJUSTEACERA 0
#define SIGUIENTEAJUSTEACERA PRIMERAJUSTEACERA+ACERA
#define MEJORAJUSTEACERA SIGUIENTEAJUSTEACERA+ACERA
#define PEORAJUSTEACERA MEJORAJUSTEACERA+ACERA
//Punteros a las funciones
//Creamos el puntero
int (*PARKING2_inicio)(TIPO_FUNCION_LLEGADA *, TIPO_FUNCION_SALIDA *, LONG, BOOL);
int (*PARKING2_fin)();
int (*PARKING2_aparcar)(HCoche, void *datos, TIPO_FUNCION_APARCAR_COMMIT,TIPO_FUNCION_PERMISO_AVANCE,TIPO_FUNCION_PERMISO_AVANCE_COMMIT);
int (*PARKING2_desaparcar)(HCoche, void *datos,TIPO_FUNCION_PERMISO_AVANCE,TIPO_FUNCION_PERMISO_AVANCE_COMMIT);
int (*PARKING2_getNUmero)(HCoche);
int (*PARKING2_getLongitud)(HCoche);
int (*PARKING2_getPosiciOnEnAcera)(HCoche);
unsigned long (*PARKING2_getTServ)(HCoche);
int (*PARKING2_getColor)(HCoche);
void *(*PARKING2_getDatos)(HCoche);
int (*PARKING2_getX)(HCoche);
int (*PARKING2_getY)(HCoche);
int (*PARKING2_getX2)(HCoche);
int (*PARKING2_getY2)(HCoche);
int (*PARKING2_getAlgoritmo)(HCoche);
//Para el tiempo utilizar un parametro de la funcion timeOut

//Prototipos.
int creaPunteros();
int compruebaArgs(int, char *[], BOOL *);
void inicializaMemoria();
void updateCarreteraVertical(int algoritmo, int posInicial, int posFinal, int s, int posVertical, int longitud);
void updateCarreteraHorizontal(int algoritmo, int posInicial, int longitud);
void updateAcera(int algoritmo, int posInicial, int longitud);
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
DWORD WINAPI hiloAparcar(LPVOID val);
void imprimeSems();

typedef struct Data {
	int primerAjusteAcera[ACERA];
	char *dataMem;
	HANDLE semaforosPrimerCrear[ACERA],semaforosPrimer[ACERA],
		   semaforosPeorCrear[ACERA], semaforosPeor[ACERA];
	HANDLE FLAGSEMAFOROMEMORIA,FLAGSEMAFOROMEMORIA1;
};
Data data;

FILE *fp;

int main(int argc, char * argv[]){
	int i;
	//fp = fopen("data.txt", "w");
	BOOL debug = false;
	if (creaPunteros() == 1) return 1;
	if (compruebaArgs(argc, argv, &debug)==1) return 1;
	
	TIPO_FUNCION_LLEGADA f_llegadas[] = { (TIPO_FUNCION_LLEGADA)primerAjusteIN,
		(TIPO_FUNCION_LLEGADA)siguienteAjusteIN,
		(TIPO_FUNCION_LLEGADA)mejorAjusteIN,
		(TIPO_FUNCION_LLEGADA)peorAjusteIN };
	TIPO_FUNCION_SALIDA f_salidas[] = { (TIPO_FUNCION_SALIDA)primerAjusteOUT,
		(TIPO_FUNCION_SALIDA)siguienteAjusteOUT,
		(TIPO_FUNCION_SALIDA)mejorAjusteOUT,
		(TIPO_FUNCION_SALIDA)peorAjusteOUT };
	for (i = 0; i < ACERA; i++) {
		data.semaforosPrimerCrear[i] = CreateSemaphore(NULL, 0, 1, "SEM"+i);
		data.semaforosPeorCrear[i] = CreateSemaphore(NULL, 0, 1, "SEMP" + i);
		if (data.semaforosPrimerCrear[i] == NULL) {
			PERROR("Error CreateSemaphore.");
			return 1;
		}
		data.semaforosPeor[i] = OpenSemaphore(SEMAPHORE_MODIFY_STATE, true, "SEMP" + i);
		data.semaforosPrimer[i] = OpenSemaphore(SEMAPHORE_MODIFY_STATE, true, "SEM"+i);
		ReleaseSemaphore(data.semaforosPrimer[i], 1, NULL);
	}
	data.FLAGSEMAFOROMEMORIA1 = CreateSemaphore(NULL, 0, 1, "FLAGSEMAFOROMEMORIA");
	HANDLE sem;
	data.FLAGSEMAFOROMEMORIA = OpenSemaphore(SEMAPHORE_MODIFY_STATE, true, "FLAGSEMAFOROMEMORIA");
	ReleaseSemaphore(data.FLAGSEMAFOROMEMORIA, 1, NULL);
	//Cargamos la carretera
	if (PARKING2_inicio(f_llegadas, f_salidas, atoi(argv[1]), debug) == -1) {
		PERROR("Error Parking2_inicio");
		system("pause");
		return 1;
	}

	system("pause");
    return 0;
}

TIPO_FUNCION_LLEGADA mejorAjusteIN(HCoche hc){
	return (TIPO_FUNCION_LLEGADA)-2;
}
TIPO_FUNCION_LLEGADA peorAjusteIN(HCoche hc){
	return (TIPO_FUNCION_LLEGADA)-2;
	fprintf(stderr, "\n\n\n\n\n\n\n\n\n\n\n\n");
	system("pause");
	int matrizHuecos[ACERA][2];
	int i, j, longitudCoche = PARKING2_getLongitud(hc), numHuecos = 0;
	int contador = 0, huecoMax = 0, posFinal = 0, posInicial;
	char *dataMem;

	//waitSem(FLAGSEMAFOROMEMORIA);

	//dataMem = shmat(data.idMemoria, (void *)0, 0);

	for (i = 0; i<ACERA; i++)
		for (j = 0; j<2; j++)
			matrizHuecos[i][j] = 0;

	for (i = 0; i<ACERA; i++) {
		if (data.dataMem[PEORAJUSTEACERA + i] != 1) {
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
			data.dataMem[PEORAJUSTEACERA + i] = 1;
		
		//signalSem(FLAGSEMAFOROMEMORIA);
		return (TIPO_FUNCION_LLEGADA)posInicial;
	}

	
	return (TIPO_FUNCION_LLEGADA)-2;
}
TIPO_FUNCION_LLEGADA siguienteAjusteIN(HCoche hc){
	return (TIPO_FUNCION_LLEGADA)-2;
}
TIPO_FUNCION_LLEGADA primerAjusteIN(HCoche hc){
	//return (TIPO_FUNCION_LLEGADA)-2;
	char *dataMem;
	DWORD valor;
	WaitForSingleObject(data.FLAGSEMAFOROMEMORIA, INFINITE);
	//waitSem(FLAGSEMAFOROMEMORIA);
	//dataMem = shmat(data.idMemoria, (void *)0, 0);
	//if (dataMem == (char *)(-1)) {
	//	perror("shmat");
	//	exit(1);
	//}
	int i, j, longitudCoche = PARKING2_getLongitud(hc), contador = 0;

	for (i = 0; i<ACERA; i++) {
		if (data.dataMem[PRIMERAJUSTEACERA+i] != 1) {
			contador++;
			if (contador == longitudCoche) {

				for (j = i - longitudCoche + 1; j <= i; j++) {
					data.dataMem[PRIMERAJUSTEACERA + j] = 1;
				}
				if (CreateThread(NULL, 0, hiloAparcar, (LPVOID)hc, NULL, NULL) == NULL) {
					PERROR("CreateThread");
				}
				ReleaseSemaphore(data.FLAGSEMAFOROMEMORIA, 1, NULL);
				return (TIPO_FUNCION_LLEGADA) (i - longitudCoche + 1);
			}

		}
		else {
			contador = 0;
		}

	}
	ReleaseSemaphore(data.FLAGSEMAFOROMEMORIA, 1, NULL);
	return (TIPO_FUNCION_LLEGADA)-1;

}

void imprimeSems() {
	int i;
	for (i = 0; i < ACERA; i++) {
		fprintf(fp, "jk,n");
	}
}

TIPO_FUNCION_SALIDA mejorAjusteOUT(HCoche hc){
	return (TIPO_FUNCION_SALIDA)-2;
}
TIPO_FUNCION_SALIDA peorAjusteOUT(HCoche hc){
	return (TIPO_FUNCION_SALIDA)-2;
}
TIPO_FUNCION_SALIDA siguienteAjusteOUT(HCoche hc){
	return (TIPO_FUNCION_SALIDA)-2;
}
TIPO_FUNCION_SALIDA primerAjusteOUT(HCoche hc){
	return (TIPO_FUNCION_SALIDA)-2;
}

TIPO_FUNCION_APARCAR_COMMIT aparcarCommit(HCoche hc) {
	return (TIPO_FUNCION_APARCAR_COMMIT)1;

}
TIPO_FUNCION_PERMISO_AVANCE permisoAvanzar(HCoche hc) {
	int i;
	
	switch (PARKING2_getAlgoritmo(hc)) {
	case 0: //Primer Ajuste...
		if (PARKING2_getX(hc) != PARKING2_getX2(hc)) {
			if (PARKING2_getX2(hc) >= 0) {
				WaitForSingleObject(data.semaforosPrimer[PARKING2_getX2(hc)],INFINITE);
			//	waitSem(POSSEMAFOROCARRETERAPRIMER + PARKING2_getX2(hc));
			}

		}
		if (PARKING2_getY2(hc) == 2 && PARKING2_getY(hc) == 1) {//Si estoy en la linea y voy a la carretera...
			for (i = PARKING2_getX(hc) + PARKING2_getLongitud(hc) - 1; i >= PARKING2_getX(hc); i--) {
				WaitForSingleObject(data.semaforosPrimer[i], INFINITE);

				//	waitSem(POSSEMAFOROCARRETERAPRIMER + i);//Wait sobre la ultima posicion por la derecha...
			}

		}
		return (TIPO_FUNCION_PERMISO_AVANCE)1; //Puede moverse
		break; //End of primer ajuste.
	}
	return (TIPO_FUNCION_PERMISO_AVANCE)1;
}
TIPO_FUNCION_PERMISO_AVANCE_COMMIT permisoAvanzarCommit(HCoche hc) {
	

	int i;
	char *dataMem;
	switch (PARKING2_getAlgoritmo(hc)) {
	case 0:

		if (PARKING2_getX(hc) == PARKING2_getX2(hc)) {//Ha sido un movimiento vertical.
			updateCarreteraVertical(PARKING2_getAlgoritmo(hc), PARKING2_getX(hc), PARKING2_getX2(hc), PARKING2_getY(hc), PARKING2_getY2(hc), PARKING2_getLongitud(hc));
		}
		else {
			updateCarreteraHorizontal(PARKING2_getAlgoritmo(hc), PARKING2_getX2(hc), PARKING2_getLongitud(hc));
		}


		if (PARKING2_getY2(hc) == 1 && PARKING2_getY(hc) == 2) {
			updateAcera(PARKING2_getAlgoritmo(hc), PARKING2_getX(hc), PARKING2_getLongitud(hc));
		}
		break;
	case 3:

		if (PARKING2_getX(hc) == PARKING2_getX2(hc)) {//Ha sido un movimiento vertical.
			updateCarreteraVertical(PARKING2_getAlgoritmo(hc), PARKING2_getX(hc), PARKING2_getX2(hc), PARKING2_getY(hc), PARKING2_getY2(hc), PARKING2_getLongitud(hc));
		}
		else {
			updateCarreteraHorizontal(PARKING2_getAlgoritmo(hc), PARKING2_getX2(hc), PARKING2_getLongitud(hc));
		}


		if (PARKING2_getY2(hc) == 1 && PARKING2_getY(hc) == 2) {
			updateAcera(PARKING2_getAlgoritmo(hc), PARKING2_getX(hc), PARKING2_getLongitud(hc));
		}
		break;
	
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
void updateAcera(int algoritmo, int posInicial, int longitud) {
	int i;
	//WaitForSingleObject(data.FLAGSEMAFOROMEMORIA, INFINITE);
	/*
	switch (algoritmo) {
	case 0: //Primer algoritmo.
		for (i = posInicial; i< (posInicial + longitud); i++) {
			data.dataMem[HUECOPRIMERPOS + i] = 0;//data.huecoMejor[0][i]=0;
		}
		if (shmdt(dataMem) == -1) {
			perror("Parking:shmdt");
		}
		signalSem(FLAGSEMAFOROMEMORIA);
		break;

	
	}*/
}



DWORD WINAPI hiloAparcar(LPVOID hCoche) {
	HCoche hc= (HCoche)hCoche;
	//fprintf(stderr,"\n\n\n\nLPVOID:%d=======HC:%d", hCoche, *hc);
	PARKING2_aparcar(hc,NULL, (TIPO_FUNCION_APARCAR_COMMIT) aparcarCommit,
		(TIPO_FUNCION_PERMISO_AVANCE) permisoAvanzar, (TIPO_FUNCION_PERMISO_AVANCE_COMMIT) permisoAvanzarCommit);
	return 1;
}


void inicializaMemoria() {
	int i;
	for (i = 0; i < ACERA; i++)
		data.primerAjusteAcera[i] = 0;
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
	//Variables
	HINSTANCE controladorDLL;
	HANDLE buffer;
	//Creamos la memoria compartida
	if ((buffer= CreateFileMapping(NULL, NULL, PAGE_READWRITE, 0, 1, "buffer")) == FALSE) {
		PERROR("CreateFileMapping");
		return 1;
	}
	if ((data.dataMem =(char *) MapViewOfFile(buffer, FILE_MAP_WRITE, 0, 0, 1)) == NULL){
		PERROR("MapViewOfFile");
		return 3;
	}

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

	PARKING2_getPosiciOnEnAcera = (int(*)(HCoche))
		GetProcAddress(controladorDLL, "PARKING2_getPosiciOnEnAcera");
	if (PARKING2_getPosiciOnEnAcera == NULL) {
		PERROR("Error GetProcAddres Parking2_getPosiciOnEnAcera.");
		system("pause");
		return 1;

	}

	PARKING2_getTServ = (unsigned long(*)(HCoche))
		GetProcAddress(controladorDLL, "PARKING2_getTServ");
	if (PARKING2_getTServ == NULL) {
		PERROR("Error GetProcAddres Parking2_getTServ.");
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

