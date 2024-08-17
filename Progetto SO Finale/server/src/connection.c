#include "./../include/connection.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void buildEmptyPacket(serverPacket *packet) {

    // Impostazione a 0 di ogni parametro, per ottenere un pacchetto del tutto vuoto
    packet->operation = '\0';
    packet->outcome = '\0';
    memset(packet->username, '\0', AUTH_PARAM_LENGTH);
    memset(packet->password, '\0', AUTH_PARAM_LENGTH);
    packet->matchIndex = 0;
    memset(packet->name, '\0', CONTACT_PARAM_LENGTH);
    memset(packet->surname, '\0', CONTACT_PARAM_LENGTH);
    memset(packet->phoneNumber, '\0', CONTACT_PARAM_LENGTH);
    memset(packet->newName, '\0', CONTACT_PARAM_LENGTH);
    memset(packet->newSurname, '\0', CONTACT_PARAM_LENGTH);
    memset(packet->newPhoneNumber, '\0', CONTACT_PARAM_LENGTH);
}

void buildMessage(char *message, serverPacket packet){

    // Svuotiamo la stringa message da eventuali impurita'
    memset(message,'\0',PACKET_LENGTH);

    // Impostiamo un'operazione solo se corretta
    char c = packet.operation;
    if(c == READ || c == AUTH || c == ADD || c == DEL|| c == MODIFY)
        message[OPERATION_INDEX]= c;

    // Impostiamo l'esito solo se valido
    c = packet.outcome;
    if(c == SERVER_ERROR || c == OPERATION_SUCCESS || c == READ_CONTACT_MISSING || c == CREDENTIALS_EXPIRED || c == CONTACT_ALREADY_MODIFIED || c == CONTACT_ALREADY_EXISTS || c == INVALID_PACKET)
        message[OUTCOME_INDEX]= c;

    // Impostiamo username
    if(strlen(packet.username) > 0){
        for(int i = 0; i < AUTH_PARAM_LENGTH; i++) {
            message[i + USERNAME_INDEX] = packet.username[i];
        }
    }

    // Impostiamo password
    if(strlen(packet.password) > 0){
        for(int i = 0; i < AUTH_PARAM_LENGTH; i++) {
            message[i + PASSWORD_INDEX] = packet.password[i];
        }
    }
    
    // Impostiamo match index
    if(packet.matchIndex > 0){
        // Trasformiamo match index in stringa
        char mIndex[CONTACT_PARAM_LENGTH + 1];
        sprintf(mIndex, "%d", packet.matchIndex);
        for(int i = 0; i <  CONTACT_PARAM_LENGTH; i++) {
            message[i + MATCHINDEX_INDEX] = mIndex[i];
        }
    }

    // Impostiamo il nome
    if(strlen(packet.name) > 0){
        for(int i = 0; i < CONTACT_PARAM_LENGTH; i++) {
            message[i + NAME_INDEX] = packet.name[i];
        }
    }

    // Impostiamo il cognome
    if(strlen(packet.surname) > 0){
        for(int i = 0; i < CONTACT_PARAM_LENGTH; i++) {
            message[i + SURNAME_INDEX] = packet.surname[i];
        }
    }

    // Impostiamo il numero di telefono
    if(strlen(packet.phoneNumber) > 0){
        for(int i = 0; i < CONTACT_PARAM_LENGTH; i++) {
            message[i + PHONE_NUMBER_INDEX] = packet.phoneNumber[i];
        }
    }

    // Impostiamo il nome modificato
    if(strlen(packet.newName) > 0){
        for(int i = 0; i < CONTACT_PARAM_LENGTH; i++) {
            message[i + NEW_NAME_INDEX] = packet.newName[i];
        }
    }

    // Impostiamo il cognome modificato
    if(strlen(packet.newSurname) > 0){
        for(int i = 0; i < CONTACT_PARAM_LENGTH; i++) {
            message[i + NEW_SURNAME_INDEX] = packet.newSurname[i];
        }
    }

    // Impostiamo il numero di telefono modificato
    if(strlen(packet.newPhoneNumber) > 0){
        for(int i = 0; i < CONTACT_PARAM_LENGTH; i++) {
            message[i + NEW_PHONE_NUMBER_INDEX] = packet.newPhoneNumber[i];
        }
    }

}

void parseMessage(char *message, serverPacket *packet){

    // Controlliamo se il messaggio fornito è corretto
    int valid = 1;
    for(int i = 0; i < PACKET_LENGTH; i++)
        if(message[i] == ',')
            valid = 0;

    // Se è valido
    if(valid) {

        // Formattiamo i campi di packet con le informazioni contenute in message
        packet->operation = message[OPERATION_INDEX];
        packet->outcome = message[OUTCOME_INDEX];

        // Trascriviamo l'username
        for(int i = 0; i < AUTH_PARAM_LENGTH; i++) {
            packet->username[i] = message[i + USERNAME_INDEX];
        }
        packet->username[AUTH_PARAM_LENGTH] = '\0';

        // Trascriviamo la password
        for(int i = 0; i < AUTH_PARAM_LENGTH; i++) {
            packet->password[i] = message[i + PASSWORD_INDEX];
        }
        packet->password[AUTH_PARAM_LENGTH] = '\0';

        // Trascriviamo match index
        char mIndex[CONTACT_PARAM_LENGTH + 1];
        for(int i = 0; i < CONTACT_PARAM_LENGTH; i++) {
            mIndex[i] = message[i + MATCHINDEX_INDEX];
        }
        mIndex[CONTACT_PARAM_LENGTH] = '\0';
        packet->matchIndex = atoi(mIndex);

        // Trascriviamo il nome
        for(int i = 0; i < CONTACT_PARAM_LENGTH; i++) {
            packet->name[i] = message[i + NAME_INDEX];
        }
        packet->name[CONTACT_PARAM_LENGTH] = '\0';

        // Trascriviamo il cognome
        for(int i = 0; i < CONTACT_PARAM_LENGTH; i++) {
            packet->surname[i] = message[i + SURNAME_INDEX];
        }
        packet->surname[CONTACT_PARAM_LENGTH] = '\0';

        // Trascriviamo il numero di telefono
        for(int i = 0; i < CONTACT_PARAM_LENGTH; i++) {
            packet->phoneNumber[i] = message[i + PHONE_NUMBER_INDEX];
        }
        packet->phoneNumber[CONTACT_PARAM_LENGTH] = '\0';

        // Trascriviamo il nome modificato
        for(int i = 0; i < CONTACT_PARAM_LENGTH; i++) {
            packet->newName[i] = message[i + NEW_NAME_INDEX];
        }
        packet->newName[CONTACT_PARAM_LENGTH] = '\0';

        // Trascriviamo il cognome modificato
        for(int i = 0; i < CONTACT_PARAM_LENGTH; i++) {
            packet->newSurname[i] = message[i + NEW_SURNAME_INDEX];
        }
        packet->newSurname[CONTACT_PARAM_LENGTH] = '\0';

        // Trascriviamo il numero di telefono modificato
        for(int i = 0; i < CONTACT_PARAM_LENGTH; i++) {
            packet->newPhoneNumber[i] = message[i + NEW_PHONE_NUMBER_INDEX];
        }
        packet->newPhoneNumber[CONTACT_PARAM_LENGTH] = '\0';
    } else { // Se il pacchetto non è valido
        packet->operation = INVALID_PACKET;
    }
}

void printMessage(char *message){

    // Eseguiamo il parse del messaggio e stampiamo le informazioni contenute
    serverPacket packet;
    buildEmptyPacket(&packet);
    parseMessage(message,&packet);
    printf("operazione: %c\n",packet.operation);
    printf("esito: %c\n",packet.outcome);
    printf("nome utente: %s\n",packet.username);
    printf("password: %s\n",packet.password);
    printf("indice: %d\n",packet.matchIndex);
    printf("nome: %s\n",packet.name);
    printf("cognome: %s\n",packet.surname);
    printf("numero di telefono: %s\n",packet.phoneNumber);
    printf("nuovo nome: %s\n",packet.newName);
    printf("nuovo cognome: %s\n",packet.newSurname);
    printf("nuovo numero di telefono: %s\n",packet.newPhoneNumber);
}