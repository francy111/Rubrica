#include "./../include/log.h"
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

void formatMessage(logMessage *msg, char *_client, char *_operation, short int _success, char *_additionalMsg) {
    time_t t = time(NULL); // Restituisce i secondi passati da gennaio 1 1970 12AM
    struct tm *dT = localtime(&t); // Lo formatta in una struttura contenente giorno, mese, anno e ora

    msg->dateTime = asctime(dT); // Restituisce la struttura della data in una stringa direttamente formattata per la lettura umana
    msg->dateTime[strlen(msg->dateTime) - 1] = '\0'; // La stringa restituita finisce con \n, noi non lo vogliamo
    msg->client = _client;
    msg->operation = _operation;
    msg->success = _success;
    msg->additionalMsg = _additionalMsg;
}

void logF(logMessage msg) {
    /*
     * Apertura del file
     *  - Sola scrittura
     *  - Modalita' append, le scritture avvengono sempre in fondo al file
     *  - Il file viene creato se non esiste
     */
    int logFd = open("files/log.txt", O_WRONLY | O_APPEND | O_CREAT, 0664);

    // Formattazione della stringa da scrivere
    char *str = msg.dateTime;
    sprintf(str, "%s - Author: %s - %s", msg.dateTime, msg.client, msg.operation);

    if(msg.success != IGNORED) { 
        sprintf(str + strlen(str), msg.success == SUCCESS ? " - SUCCEEDED" : " - FAILED");
    }
    if(msg.additionalMsg != NULL) {
        sprintf(str + strlen(str), " - %s", msg.additionalMsg);
    }
    sprintf(str + strlen(str), "\n");

    // Scrittura e chiusura file
    write(logFd, str, strlen(str));
    close(logFd);
}