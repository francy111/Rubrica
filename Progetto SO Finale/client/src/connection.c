#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "./../include/connection.h"

int readContact(int clientFD, Contact *toRead,int matchIndex, Contact *serverRead){
    int outcome = 0;
    // Creazione del messaggio da inviare al server con i parametri per la ricerca
    char message[PACKET_LENGTH];
    serverPacket toSend;
    buildEmptyPacket(&toSend);
    toSend.operation = READ;
    toSend.matchIndex = matchIndex;
    strcpy(toSend.name, toRead->name);
    strcpy(toSend.surname, toRead->surname);
    strcpy(toSend.phoneNumber, toRead->phoneNumber);
    
    buildMessage(message,toSend);

    // Invio del messaggio al server e controllo esito della scrittura
    ssize_t bytesWritten = write(clientFD,message,PACKET_LENGTH);
    if (bytesWritten != PACKET_LENGTH) {
        perror("Impossibile comunicare con il server");    
        exit(EXIT_FAILURE);
    }

    // Lettura della risposta del server e controllo esito della lettura
    char response[PACKET_LENGTH];
    ssize_t bytesRead = read(clientFD,response,PACKET_LENGTH);
    if (bytesRead != PACKET_LENGTH) {
        perror("Impossibile comunicare con il server");    
        exit(EXIT_FAILURE);
    }

    // Inizializzo il contatto serverRead con le informazioni del pacchetto letto
    serverPacket received;
    buildEmptyPacket(&received);
    parseMessage(response,&received);
    strcpy(serverRead->name,received.name);
    strcpy(serverRead->surname,received.surname);
    strcpy(serverRead->phoneNumber,received.phoneNumber);
    
    // Controllo l'esito dell'operazione
    if(received.outcome == OPERATION_SUCCESS)
        outcome = 1;
    else if(received.outcome == READ_CONTACT_MISSING)
        outcome = 2;
    return outcome;
}

int authenticate(int clientFD, char *username, char *password){
    int outcome = 0;
    // Creazione del messaggio da inviare al server con i parametri per la ricerca
    char message[PACKET_LENGTH];
    serverPacket toSend;
    buildEmptyPacket(&toSend);
    toSend.operation = AUTH;
    strcpy(toSend.username, username);
    strcpy(toSend.password, password);
    buildMessage(message,toSend);
    // Invio del messaggio al server e controllo esito della scrittura
    ssize_t bytesWritten = write(clientFD,message,PACKET_LENGTH);
    if (bytesWritten != PACKET_LENGTH) {
        perror("Impossibile comunicare con il server");    
        exit(EXIT_FAILURE);
    }
    // Lettura della risposta del server e controllo esito della lettura
    char response[PACKET_LENGTH];
    ssize_t bytesRead = read(clientFD,response,PACKET_LENGTH);
    if (bytesRead != PACKET_LENGTH) {
        perror("Impossibile comunicare con il server");    
        exit(EXIT_FAILURE);
    }

    // Inizializzo il pacchetto received con il messaggio letto
    serverPacket received;
    buildEmptyPacket(&received);
    parseMessage(response,&received);
    
    // Controllo se il server ha accettato le credenziali
    if(received.outcome == OPERATION_SUCCESS)
        outcome = 1;
    return outcome;
}

int addContact(int clientFD, char *username, char *password, Contact *toAdd){
    int outcome = 0;
    // Creazione del messaggio da inviare al server con i parametri per la ricerca
    char message[PACKET_LENGTH];
    serverPacket toSend;
    buildEmptyPacket(&toSend);
    toSend.operation = ADD;
    strcpy(toSend.username, username);
    strcpy(toSend.password, password);
    strcpy(toSend.name, toAdd->name);
    strcpy(toSend.surname, toAdd->surname);
    strcpy(toSend.phoneNumber, toAdd->phoneNumber);
    buildMessage(message,toSend);

    // Invio del messaggio al server e controllo esito della scrittura
    ssize_t bytesWritten = write(clientFD,message,PACKET_LENGTH);
    if (bytesWritten != PACKET_LENGTH) {
        perror("Impossibile comunicare con il server");    
        exit(EXIT_FAILURE);
    }

    // Lettura della risposta del server e controllo esito della lettura
    char response[PACKET_LENGTH];
    ssize_t bytesRead = read(clientFD,response,PACKET_LENGTH);
    if (bytesRead != PACKET_LENGTH) {
        perror("Impossibile comunicare con il server");    
        exit(EXIT_FAILURE);
    }

    // Inizializzo il pacchetto received con il messaggio letto
    serverPacket received;
    buildEmptyPacket(&received);
    parseMessage(response,&received);

    // Controllo l'esito dell'operazione
     if(received.outcome == OPERATION_SUCCESS)
        outcome = 1;
    else if(received.outcome == CREDENTIALS_EXPIRED)
        outcome = 3;
    else if(received.outcome == CONTACT_ALREADY_EXISTS)
        outcome = 5;
    return outcome;
}

int deleteContact(int clientFD, char *username, char *password, Contact *toDelete){
    int outcome = 0;
    // Creazione del messaggio da inviare al server con i parametri per la ricerca
    char message[PACKET_LENGTH];
    serverPacket toSend;
    buildEmptyPacket(&toSend);
    toSend.operation = DEL;
    strcpy(toSend.username, username);
    strcpy(toSend.password, password);
    strcpy(toSend.name, toDelete->name);
    strcpy(toSend.surname, toDelete->surname);
    strcpy(toSend.phoneNumber, toDelete->phoneNumber);
    buildMessage(message,toSend);

    // Invio del messaggio al server e controllo esito della scrittura
    ssize_t bytesWritten = write(clientFD,message,PACKET_LENGTH);
    if (bytesWritten != PACKET_LENGTH) {
        perror("Impossibile comunicare con il server");    
        exit(EXIT_FAILURE);
    }

    // Lettura della risposta del server e controllo esito della lettura
    char response[PACKET_LENGTH];
    ssize_t bytesRead = read(clientFD,response,PACKET_LENGTH);
    if (bytesRead != PACKET_LENGTH) {
        perror("Impossibile comunicare con il server");    
        exit(EXIT_FAILURE);
    }
    
    // Inizializzo il pacchetto received con il messaggio letto
    serverPacket received;
    buildEmptyPacket(&received);
    parseMessage(response,&received);

    // Controllo l'esito dell'operazione
    if(received.outcome == OPERATION_SUCCESS)
        outcome = 1;
    else if(received.outcome == CREDENTIALS_EXPIRED)
        outcome = 3;
    else if(received.outcome == CONTACT_ALREADY_MODIFIED)
        outcome = 4;
    return outcome;
}

int modifyContact(int clientFD, char *username, char *password, Contact *toModify, Contact *modifiedContact){
    int outcome = 0;
    // Creazione del messaggio da inviare al server con i parametri per la ricerca
    char message[PACKET_LENGTH];
    serverPacket toSend;
    buildEmptyPacket(&toSend);
    toSend.operation = MODIFY;
    strcpy(toSend.username, username);
    strcpy(toSend.password, password);
    strcpy(toSend.name, toModify->name);
    strcpy(toSend.surname, toModify->surname);
    strcpy(toSend.phoneNumber, toModify->phoneNumber);
    strcpy(toSend.newName, modifiedContact->name);
    strcpy(toSend.newSurname, modifiedContact->surname);
    strcpy(toSend.newPhoneNumber, modifiedContact->phoneNumber);
    buildMessage(message,toSend);

    // Invio del messaggio al server e controllo esito della scrittura
    ssize_t bytesWritten = write(clientFD,message,PACKET_LENGTH);
    if (bytesWritten != PACKET_LENGTH) {
        perror("Impossibile comunicare con il server");    
        exit(EXIT_FAILURE);
    }

    // Lettura della risposta del server e controllo esito della lettura
    char response[PACKET_LENGTH];
    ssize_t bytesRead = read(clientFD,response,PACKET_LENGTH);
    if (bytesRead != PACKET_LENGTH) {
        perror("Impossibile comunicare con il server");    
        exit(EXIT_FAILURE);
    }
    
    // Inizializzo il pacchetto received con il messaggio letto
    serverPacket received;
    buildEmptyPacket(&received);
    parseMessage(response,&received);

    // Controllo l'esito dell'operazione
     if(received.outcome == OPERATION_SUCCESS)
        outcome = 1;
    else if(received.outcome == CREDENTIALS_EXPIRED)
        outcome = 3;
    else if(received.outcome == CONTACT_ALREADY_MODIFIED)
        outcome = 4;
    return outcome;
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
    if(c == SERVER_ERROR || c == OPERATION_SUCCESS || c == READ_CONTACT_MISSING || c == CREDENTIALS_EXPIRED || c == CONTACT_ALREADY_MODIFIED || c == CONTACT_ALREADY_EXISTS)
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
    
}

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

// per DEBUG
void printMessage(char *message){
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

int closeConnection(int socketFd) {
    char buf[PACKET_LENGTH];
    memset(buf, '\0', PACKET_LENGTH);
    buf[0] = 'x';

    // Invia il pacchetto di chiusura al server
    if(write(socketFd, buf, PACKET_LENGTH) <= 0) exit(EXIT_FAILURE);

    // Il server leggera' il pacchetto di chiusura e rispondera'
    if(read(socketFd, buf, PACKET_LENGTH <= 0)) exit(EXIT_FAILURE); 

    // Ora siamo sicuri che il server è a conoscenza dell'interruzione della comunicazione
    close(socketFd);

    // Stampa messaggio di chiusura
    printf(GREEN "\nSessione chiusa con successo\n\n" RESET_COLOR);
    return 0;
}