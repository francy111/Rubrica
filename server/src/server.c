#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "./../include/log.h"
#include "./../include/utility.h"
#include "./../include/connection.h"
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>

#define DEFAULT_PORT 50000
#define MAX_REQUESTS 10

/**
 * serverFd - FD della 'server socket', ovvero quella che si occupa di accettare connessioni
 * clientFd - FD della socket usata per comunicare con il client, ottenuta tramite accept
 * portNumber - Numero di porta su cui è aperto il server
 * operationAuthor - Stringa che identifica chi esegue un operazione 
 * 
 * Sono variabili globali in quanto la gestione delle socket e il logging avviene anche
 * a livello di gestione dei segnali (ctrl-c), è quindi necessario oltre che utile
 * poter accedere ai file descriptor fuori dal main
 */
int serverFd, clientFd, portNumber;
char operationAuthor[22];
short int running = 1;

/**
 * Gestiamo il segnale SIGPIPE
 * Vogliamo gestire la chiusura improvvisa della socket del client
 * Chiudendo correttamente la socket e facendo il log di tutto
 * Questo segnale accade quando cerchiamo di scrivere su un pipe o
 * socket chiuso
 */
void sigpipe_handler(int sig) {
    logMessage toBeLogged;
    close(clientFd);
    formatMessage(&toBeLogged, operationAuthor, "Connection terminated", FAILURE, "Error during client response, closing socket");
    logF(toBeLogged);
    exit(EXIT_FAILURE);
}

/**
 * Il processo figlio, che comunica con il client è immune a CTRL-C
 * Per il padre invece, questo era l'unico modo per potersi interrompere
 * 
 * Gestiamo il CTRL-C incaricando un figlio (manager) di gestire la chiusura della socket
 * server e di altre operazioni di utility per il server
 */
void ctrlcHandler(int sig) {
    
    // Disabilitiamo momentaneamente l'handler, cosi' da non spawnare tanti figli manager
    signal(SIGINT, SIG_IGN);
    
    // Incarichiamo il figlio, il padre non fara' l'if e tornera' ad accettare connessioni
    if(fork() == 0) {

        // Questo processo sara' immune a CTRL-C, l'unico modo per chiuderlo sara' uscire dal suo menu'
        signal(SIGUSR1, SIG_IGN);
        signal(SIGUSR2, SIG_IGN);

        /*
        * Passiamo informazioni aggiuntive al figli per la gestione
        *  Passiamo il pid del padre perchè perso durante execl, per poter inviare un segnale al padre
        *  Una stringa autore, per poter scrivere "Server:porta" per il logging, questo per capire anche
        *  se ci fossero piu' server aperti su piu' porte, quale server ha fatto l'operazione
        */
        char ppidStr[10], author[20];
        memset(ppidStr, '\0', 10);
        memset(author, '\0', 8);
        sprintf(ppidStr, "%d", getppid());
        sprintf(author, "Server:%d", portNumber);

        // Eseguiremo il programma /serverManager
        execl("./utility/serverManager", "serverManager", ppidStr, author, NULL);

        // Queste righe non dovrebbero essere eseguite
        perror("execl");
        exit(EXIT_FAILURE);
    }
}

/**
 * Il processo serverManager eseguira' le operazioni di utility
 * nel suo scope, senza interferire con il server (padre), ma,
 * tra le varie operazioni vi è quella di chiudere il menu
 * 
 * Il manager inviera' un segnale di tipo SIGUSR2 al padre, quindi, 
 * quando quest'ultimo riceve il segnale e sapra' che non ci sono
 * manager aperti
 */
void sigusr1Handler(int sig) {

    printf(GREEN "In attesa di richieste...\n" RESET_COLOR);

    // Reimpostiamo l'handler che richiama il manager
    signal(SIGINT, ctrlcHandler);

}

/**
 * Il processo serverManager eseguira' le operazioni di utility
 * nel suo scope, senza interferire con il server (padre), ma,
 * tra le varie operazioni vi è quella di chiusura della socket, 
 * per smettere di accettare connessioni, eventualmente per un 
 * restart del server
 * 
 * Il manager, una volta confermata l'operazione di chiusura, inviera'
 * un segnale di tipo SIGUSR1 al padre, quindi, quando quest'ultimo
 * riceve il segnale, sapra' di dover chiudere la server socket
 */
void sigusr2Handler(int sig) {

    //Chiusura della socket
    close(serverFd);
    logMessage toBeLogged;

    // Facciamo log dell'operazione
    sprintf(operationAuthor, "Server:%d", portNumber);
    formatMessage(&toBeLogged, operationAuthor, "Server socket closing", IGNORED, "Socket successfully closed");
    logF(toBeLogged);

    printf(GREEN "Server chiuso con successo\n" RESET_COLOR);

    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {

    // Dichiarazione strutture dati necessarie
    int serverLength, clientLength;
    struct sockaddr_in serverAddress, clientAddress;
    struct sockaddr *serverFdAddressPtr, *clientFdAddressPtr;
    logMessage toBeLogged;

    // Il server puo' essere avviato specificando una porta specifica sulla quale accettare connessioni
    portNumber = (argc > 1) ? (atoi(argv[1]) ? atoi(argv[1]) : DEFAULT_PORT) : DEFAULT_PORT;

    /*
     * Gestione dei segnali
     *  Vogliamo gestire SIGPIPE chiudendo correttamente la socket di comunicazione con il client
     *  Vogliamo ignorare i segnali SIGCHLD (death-of-child), cosi' i figli possono interrompersi e non aspetteranno la wait del padre (evitiamo i processi zombie)
     *  Vogliamo gestire il CTRL-C (solo del processo padre) per gestire, oltre a una corretta terminazione, alcune operazioni del server
     */
    signal(SIGPIPE, sigpipe_handler);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGINT, ctrlcHandler);
    signal(SIGUSR1, sigusr1Handler);
    signal(SIGUSR2, sigusr2Handler);

    // Inizializzazione della socket, dominio AF_INET, bidirezionale e protocollo scelto automaticamente
    serverFdAddressPtr = (struct sockaddr*) &serverAddress;
    serverLength = sizeof(serverAddress);
    clientFdAddressPtr = (struct sockaddr*) &clientAddress;
    clientLength = sizeof(clientAddress);
    serverFd = socket(AF_INET, SOCK_STREAM, 0);

    /*
     * Quando una socket viene chiusa, la porta che utilizzava viene liberata dal sistema operativo
     * Ma non è detto che quella porta possa essere riassegnata subito. Chiudere un server (con socket compresa)
     * e riaprirlo puo' quindi portare ad avere la porta non subito riassegnata
     */
    int opt = 1;
    if(setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    // Inizializziamo la porta
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // Ascoltiamo richieste provenienti da qualsiasi rete

    bind(serverFd, serverFdAddressPtr, serverLength);

    // Controlliamo se il bind ha successo o meno
    if(serverFd < 0) {
        formatMessage(&toBeLogged, "Server", "Socket creation", FAILURE, "Couldn't create socket");
        printf(RED "Server non avviato, errore creazione socket\n" RESET_COLOR);
        exit(EXIT_FAILURE);
    } else {
        sprintf(operationAuthor, "Server:%d", portNumber);
        formatMessage(&toBeLogged, operationAuthor, "Socket creation", SUCCESS, "Socket successfully created");
        printf(GREEN "Server avviato con successo\nIn attesa di richieste...\n" RESET_COLOR);
    }
    logF(toBeLogged);

    // Prepariamo la socket per accettare richieste
    listen(serverFd, MAX_REQUESTS);
    while(running) {

        // Accettiamo una richiesta di connessione e incarichiamo un processo figlio di gestirla, il padre tornera' ad accettare richieste
        clientFd = accept(serverFd, clientFdAddressPtr, &clientLength);
        pid_t pid = fork();
        if(pid == 0) {

            // Processo figlio
            close(serverFd);

            // Detach-iamo il processo figlio dal terminale
            int nullFd = open("/dev/null", O_RDWR);
            dup2(nullFd, STDIN_FILENO);
            dup2(nullFd, STDOUT_FILENO);
            dup2(nullFd, STDERR_FILENO);
            close(nullFd);

            // Processo figlio, ascolta le richieste della sessione con il client
            serverPacket packetReceived, packetToSend;

            // Nel client gestiamo le interruzioni di sospensione e interruzione disabilitandole
            signal(SIGINT, SIG_IGN);
            signal(SIGUSR1, SIG_IGN);
            signal(SIGUSR2, SIG_IGN);

            char clientInfo[INET_ADDRSTRLEN], requestMsg[350], additionalMsg[200];
            char operationAuthor[22];
            int status, a = getpeername(clientFd, clientFdAddressPtr, &clientLength);
            inet_ntop(AF_INET, &(clientAddress.sin_addr), clientInfo, INET_ADDRSTRLEN);
            memset(operationAuthor, '\0', 22);
            sprintf(operationAuthor, "%s:%d", clientInfo, clientAddress.sin_port);

            // Facciamo il log del collegamento del client
            formatMessage(&toBeLogged, operationAuthor, "Connection established", IGNORED, "Session started");
            logF(toBeLogged);

            char socketBuffer[PACKET_LENGTH];
            int connected = 1;

            // Sessione di comunicazione con il client
            while(connected) {

                // Leggiamo il messaggio inviato dal client
                memset(socketBuffer, '\0', PACKET_LENGTH);
                memset(requestMsg, '\0', 350);
                memset(additionalMsg, '\0', 200);
                if(read(clientFd, socketBuffer, PACKET_LENGTH) != PACKET_LENGTH) { // Controlliamo che sia la lunghezza giusta (quella del pacchetto)
                    logMessage toBeLogged;
                    close(clientFd);
                    formatMessage(&toBeLogged, operationAuthor, "Connection terminated", FAILURE, "Error during client request, closing socket");
                    logF(toBeLogged);
                    exit(EXIT_FAILURE);
                }      

                // Scomponiamo il messaggio formattando il pacchetto
                buildEmptyPacket(&packetReceived);         
                parseMessage(socketBuffer, &packetReceived);

                // Controlliamo l'operazione
                switch(packetReceived.operation) {

                    /*
                     * Il client ha richiesto un'operazione di lettura dalla rubrica
                     * Specifica i parametri per la ricerca del contatto e quale istanza
                     * vuole.
                     * matchIndex di packet rappresenta appunto il numero di istanza
                     */
                    case READ:

                        /*
                         * Inizializziamo una struct contact con le informazioni per la ricerca
                         * le informazioni sono contenute nel pacchetto
                         */
                        Contact toSearch, found;
                        createEmptyContact(&toSearch);
                        strncpy(toSearch.name, packetReceived.name, strlen(packetReceived.name));
                        strncpy(toSearch.surname, packetReceived.surname, strlen(packetReceived.surname));
                        strncpy(toSearch.phoneNumber, packetReceived.phoneNumber, strlen(packetReceived.phoneNumber));
                        packetToSend.operation = packetReceived.operation;

                        int searchIndex = 0, foundIndex = 0, contactRetrived = 1;
                        buildEmptyPacket(&packetToSend);

                        /*
                         * Leggiamo la rubrica un contatto alla volta tramite getContact
                         * Controlliamo se il parametro trovato combacia con i criteri stabiliti
                         * dal client. In tal caso controlliamo se tale contatto è l'n-esima (matchIndex)
                         * corrispondenza trovata, in tal caso lo invieremo al client, in  caso contrario 
                         * cercheremo il successivo corrispondente
                         * 
                         * searchIndex - Indice di scorrimento della rubrica, rappresenta quale contatto prendere dalla rubrica
                         * foundIndex - Rappresenta quanti contatti sono stati trovati che corrispondono ai criteri di ricerca
                         * (se ci fosse un array solo di contatti che combaciano, sarebbe l'indice di tale array)
                         */
                        while(foundIndex < packetReceived.matchIndex && contactRetrived) {

                            // Leggo un contatto dalla rubrica
                            createEmptyContact(&found);
                            contactRetrived = getContact(&found, searchIndex);

                            // Se get restituisce 0 la rubrica non aveva ulteriori contatti (è stata letta tutta)
                            if(contactRetrived) { 

                                // Controllo se il contatto trovato corrisponde a quello cercato in base ai criteri stabiliti
                                if(matchesParameters(toSearch, found)) {
                                    foundIndex ++; // Abbiamo trovato l'i-esimo (foundIndex) contatto che corrisponde ai criteri di ricerca
                                }
                            }
                            searchIndex ++; // Mi preparo a leggere il successivo
                        }

                        /*
                         * Abbiamo due possibili cause per l'uscita dal ciclo
                         *   è stato trovato il contatto che cercavamo
                         *   Sono finiti i contatti
                         */
                        if(foundIndex == packetReceived.matchIndex) { // Se il contatto è stato trovato

                            // Inizializziamo il pacchetto di risposta da inviare al client, indicando il successo e il contatto trovato
                            packetToSend.outcome = OPERATION_SUCCESS;
                            packetToSend.matchIndex = foundIndex;
                            strncpy(packetToSend.name, found.name, strlen(found.name));
                            strncpy(packetToSend.surname, found.surname, strlen(found.surname));
                            strncpy(packetToSend.phoneNumber, found.phoneNumber, strlen(found.phoneNumber));

                            // Per il logging
                            status = SUCCESS;
                            sprintf(additionalMsg, "Found matching contact: [%s, %s, %s]", found.name, found.surname, found.phoneNumber);

                        } else { // Abbiamo letto tutta la rubrica senza trovare il contatto che cercavamo

                            // Inizializziamo il pacchetto di risposta da inviare al client, indicando il fallimento
                            packetToSend.outcome = READ_CONTACT_MISSING;
                            
                            // Per il logging
                            status = FAILURE;
                            sprintf(additionalMsg, "Finished file without any contacts");
                        }

                        /*
                         * Formattiamo il messaggio da stampare sul file di log in modo tale
                         * da indicare solamente i parametri richiesti per la ricerca
                         */
                        if(isContactEmpty(toSearch)) {
                            sprintf(requestMsg, "Requested search for contact number %d", packetReceived.matchIndex);
                        } else {
                            sprintf(requestMsg, "Requested search for contact number %d that matches [", packetReceived.matchIndex);
                            if(toSearch.name[0] != '\0') sprintf(requestMsg + strlen(requestMsg), "-Name: %s ", toSearch.name);
                            if(toSearch.surname[0] != '\0') sprintf(requestMsg + strlen(requestMsg), "-Surname: %s ", toSearch.surname);
                            if(toSearch.phoneNumber[0] != '\0') sprintf(requestMsg + strlen(requestMsg), "-Phone number: %s", toSearch.phoneNumber);
                            sprintf(requestMsg + strlen(requestMsg), "]");
                        }

                        // Facciamo il log dell' operazione
                        formatMessage(&toBeLogged, operationAuthor, requestMsg, status, additionalMsg);
                        logF(toBeLogged);

                        // Inviamo la risposta al client
                        memset(socketBuffer, '\0', PACKET_LENGTH);
                        buildMessage(socketBuffer, packetToSend);
                        if(write(clientFd, socketBuffer, PACKET_LENGTH) != PACKET_LENGTH) {
                            logMessage toBeLogged;
                            close(clientFd);
                            formatMessage(&toBeLogged, operationAuthor, "Connection terminated", FAILURE, "Error during client response, closing socket");
                            logF(toBeLogged);
                            exit(EXIT_FAILURE);
                        }
                        break;

                    /*
                     * Il client ha richiesto un'operazione di autenticazione
                     * Invia nome utente e password e controlla la sua validita'
                     * Inviando l'esito al client
                     */
                    case AUTH:

                        // Leggiamo username e password e li controlliamo
                        buildEmptyPacket(&packetToSend);
                        packetToSend.operation = packetReceived.operation;

                        /*
                         * Il nome utente non puo' essere vuoto
                         * Quindi procediamo solo in caso sia corretto
                         */
                        if(packetReceived.username[0] != '\0') {

                            // Controlliamo la validita' delle credenziali
                            if(checkCredentials(packetReceived.username, packetReceived.password)) { // Se sono corrette
                            
                                // Inizializziamo il pacchetto di risposta da inviare al client, indicando il successo
                                packetToSend.outcome = OPERATION_SUCCESS;

                                // Per logging
                                status = SUCCESS;
                                sprintf(additionalMsg, "User identified as [%s]", packetReceived.username);
                            } else {

                                // Inizializziamo il pacchetto di risposta da inviare al client, indicando il fallimento
                                packetToSend.outcome = SERVER_ERROR;

                                // Per logging
                                status = FAILURE;
                                sprintf(additionalMsg, "Credentials not present");
                            }

                        } else { // Le credenziali non sono state inviate correttamente, indichiamo quindi fallimento dell'operazione
                            packetToSend.outcome = SERVER_ERROR;
                            status = FAILURE;
                            sprintf(additionalMsg, "Invalid credentials");
                        }
                        
                        // Facciamo log sul file
                        formatMessage(&toBeLogged, operationAuthor, "Authentication attempt", status, additionalMsg);
                        logF(toBeLogged);

                        // Inviamo la risposta al client
                        memset(socketBuffer, '\0', PACKET_LENGTH);
                        buildMessage(socketBuffer, packetToSend);
                        if(write(clientFd, socketBuffer, PACKET_LENGTH) != PACKET_LENGTH) { // Controlliamo che scriva correttamente tutto il pacchetto
                            logMessage toBeLogged;
                            close(clientFd);
                            formatMessage(&toBeLogged, operationAuthor, "Connection terminated", FAILURE, "Error during client response, closing socket");
                            logF(toBeLogged);
                            exit(EXIT_FAILURE);
                        }
                        break;

                    /*
                     * Il client ha richiesto un'operazione di aggiunta di un contatto
                     * Invia nome utente e password, per verificarne la validita' (l'utente potrebbe essere stato modificato o eliminato)
                     * Invia inoltre un contatto da aggiungere alla rubrica, se non è gia' presente
                     * Invia al client un pacchetto contenente l'esito
                     */
                    case ADD:
                    
                        // Inizializziamo il pacchetto da inviare
                        buildEmptyPacket(&packetToSend);
                        packetToSend.operation = packetReceived.operation;

                        // Controlliamo se l'utente è autorizzato
                        if(checkCredentials(packetReceived.username, packetReceived.password)) {

                            // Inizializziamo il contatto da aggiungere
                            Contact toAdd;
                            createEmptyContact(&toAdd);
                            strncpy(toAdd.name, packetReceived.name, strlen(packetReceived.name));
                            strncpy(toAdd.surname, packetReceived.surname, strlen(packetReceived.surname));
                            strncpy(toAdd.phoneNumber, packetReceived.phoneNumber, strlen(packetReceived.phoneNumber));

                            // Proviamo ad aggiungerlo
                            int addRes = addContact(toAdd);
                            
                            if(addRes == 1) { 

                                // è stato aggiunto, impostiamo quindi success come esito
                                packetToSend.outcome = OPERATION_SUCCESS;

                                // Per logging
                                status = SUCCESS;
                                sprintf(additionalMsg, "Contact added at end of list");
                            } else if (addRes == 2) {

                                // Non è stato aggiunto in quanto era gia' presente
                                packetToSend.outcome = CONTACT_ALREADY_EXISTS;

                                // Per logging
                                status = FAILURE;
                                sprintf(additionalMsg, "Contact not added, was duplicate");
                            } else { 
                                
                                // Non è stato aggiunto per problemi riguardanti file (apertura/scrittura)
                                packetToSend.outcome = SERVER_ERROR;
                                
                                // Per logging
                                status = FAILURE;
                                sprintf(additionalMsg, "Could not add new contact");
                            }

                        } else { // Autorizzazione fallita

                            // Mancata autorizzazione
                            packetToSend.outcome = CREDENTIALS_EXPIRED;

                            // Per logging
                            status = FAILURE;
                            sprintf(additionalMsg, "User failed to authenticate, wrong credentials");
                        }

                        // Facciamo log sul file
                        sprintf(requestMsg, "Requested to add new contact: [%s, %s, %s]", packetReceived.name, packetReceived.surname, packetReceived.phoneNumber);
                        formatMessage(&toBeLogged, operationAuthor, requestMsg, status, additionalMsg);
                        logF(toBeLogged);
                    
                        // Inviamo la risposta al client
                        memset(socketBuffer, '\0', PACKET_LENGTH);
                        buildMessage(socketBuffer, packetToSend);
                        if(write(clientFd, socketBuffer, PACKET_LENGTH) != PACKET_LENGTH) {
                            logMessage toBeLogged;
                            close(clientFd);
                            formatMessage(&toBeLogged, operationAuthor, "Connection terminated", FAILURE, "Error during client response, closing socket");
                            logF(toBeLogged);
                            exit(EXIT_FAILURE);
                        }
                        break;

                    /* 
                     * Il client ha richiesto un'operazione di rimozione di un contatto dalla rubrica
                     * Invia le proprie credenziali, per verificare l'autorizzazione, e il contatto
                     * da rimuovere
                     * Inviamo al client un pacchetto contenente l'esito
                     */
                    case DEL:

                        // Inizializziamo il pacchetto da spedire
                        buildEmptyPacket(&packetToSend);
                        packetToSend.operation = packetReceived.operation;
                        Contact toRemove;

                        // Controlliamo se l'utente è autorizzato
                        if(checkCredentials(packetReceived.username, packetReceived.password)) {

                            // Inizializziamo una struct con le informazioni del contatto da rimuovere
                            createEmptyContact(&toRemove);
                            strncpy(toRemove.name, packetReceived.name, strlen(packetReceived.name));
                            strncpy(toRemove.surname, packetReceived.surname, strlen(packetReceived.surname));
                            strncpy(toRemove.phoneNumber, packetReceived.phoneNumber, strlen(packetReceived.phoneNumber));

                            // Tentiamo la rimozione
                            int removed = removeContact(toRemove);
                            if(removed == 1) {

                                // Il contatto è stato rimosso con successo
                                packetToSend.outcome = OPERATION_SUCCESS;

                                // Per logging
                                status = SUCCESS;
                                sprintf(additionalMsg, "Contact removed successfully");
                            } else if (removed == 2) {
                                
                                // Il contatto non è stato rimosso perchè non presente
                                packetToSend.outcome = CONTACT_ALREADY_MODIFIED;
                                
                                // Per logging
                                status = FAILURE;
                                sprintf(additionalMsg, "Could not remove contact, wasn't present");
                            } else {
                                
                                // Non è stato rimosso per problemi riguardo il file rubrica
                                packetToSend.outcome = SERVER_ERROR;
                                
                                // Per logging
                                status = FAILURE;
                                sprintf(additionalMsg, "Could not remove contact");
                            }

                        } else { // Autorizzazione fallita

                            packetToSend.outcome = CREDENTIALS_EXPIRED;
                                
                            // Per logging
                            status = FAILURE;
                            sprintf(additionalMsg, "User failed to authenticate, wrong credentials");
                        }

                        // Facciamo log sul file
                        sprintf(requestMsg, "Requested to remove contact [%s, %s, %s]", packetReceived.name, packetReceived.surname, packetReceived.phoneNumber);
                        formatMessage(&toBeLogged, operationAuthor, requestMsg, status, additionalMsg);
                        logF(toBeLogged);

                        // Inviamo la risposta al client
                        memset(socketBuffer, '\0', PACKET_LENGTH);
                        buildMessage(socketBuffer, packetToSend);
                        if(write(clientFd, socketBuffer, PACKET_LENGTH) != PACKET_LENGTH) {
                            logMessage toBeLogged;
                            close(clientFd);
                            formatMessage(&toBeLogged, operationAuthor, "Connection terminated", FAILURE, "Error during client response, closing socket");
                            logF(toBeLogged);
                            exit(EXIT_FAILURE);
                        }
                        break;

                    /* 
                     * Il client ha richiesto un'operazione di modifica di un contatto, inviando
                     *  Le proprie credenziali, per verificarne la validita'
                     *  Il contatto da modificare
                     *  Un contatto che lo sostituira' nella rubrica
                     */
                    case MODIFY:

                        // Inizializziamo un pacchetto
                        buildEmptyPacket(&packetToSend);
                        packetToSend.operation = packetReceived.operation;
                        Contact toModify, modified;

                        // Controlliamo se l'utente è autorizzato
                        if(checkCredentials(packetReceived.username, packetReceived.password)) {

                            // Inizializziamo due struct, una con il contatto vecchio, da modificare, e una con il contatto nuovo
                            createEmptyContact(&toModify);
                            strncpy(toModify.name, packetReceived.name, strlen(packetReceived.name));
                            strncpy(toModify.surname, packetReceived.surname, strlen(packetReceived.surname));
                            strncpy(toModify.phoneNumber, packetReceived.phoneNumber, strlen(packetReceived.phoneNumber));

                            createEmptyContact(&modified);
                            strncpy(modified.name, packetReceived.newName, strlen(packetReceived.newName));
                            strncpy(modified.surname, packetReceived.newSurname, strlen(packetReceived.newSurname));
                            strncpy(modified.phoneNumber, packetReceived.newPhoneNumber, strlen(packetReceived.newPhoneNumber));

                            // Tentiamo la modifica
                            int modifiedRes = modifyContact(toModify, modified);
                            if(modifiedRes == 1) {

                                // Il contatto è stato modificato con successo
                                packetToSend.outcome = OPERATION_SUCCESS;

                                // Per logging
                                status = SUCCESS;
                                sprintf(additionalMsg, "Contact modified successfully");
                            } else if(modifiedRes == 2) {

                                // Il contatto non è stato modificato in quanto non era presente
                                packetToSend.outcome = CONTACT_ALREADY_MODIFIED;

                                // Per logging
                                status = FAILURE;
                                sprintf(additionalMsg, "Could not modify contact, wasn't present");
                            } else {

                                // Il contatto non è stato modificato per errore dovuto al file rubrica
                                packetToSend.outcome = SERVER_ERROR;

                                // Per logging
                                status = FAILURE;
                                sprintf(additionalMsg, "Could not modify contact");
                            }

                        } else { // Autorizzazione fallita

                            packetToSend.outcome = CREDENTIALS_EXPIRED;

                            // Per logging
                            status = FAILURE;
                            sprintf(additionalMsg, "User failed to authenticate, wrong credentials");
                        }

                        // Facciamo log sul file
                        sprintf(requestMsg, "Requested to modify contact [%s, %s, %s]", packetReceived.name, packetReceived.surname, packetReceived.phoneNumber);
                        sprintf(requestMsg + strlen(requestMsg), " with new info [%s, %s, %s]", modified.name, modified.surname, modified.phoneNumber);
                        formatMessage(&toBeLogged, operationAuthor, requestMsg, status, additionalMsg);
                        logF(toBeLogged);

                        // Inviamo la risposta al client
                        memset(socketBuffer, '\0', PACKET_LENGTH);
                        buildMessage(socketBuffer, packetToSend);
                        if(write(clientFd, socketBuffer, PACKET_LENGTH) != PACKET_LENGTH) {
                            logMessage toBeLogged;
                            close(clientFd);
                            formatMessage(&toBeLogged, operationAuthor, "Connection terminated", FAILURE, "Error during client response, closing socket");
                            logF(toBeLogged);
                            exit(EXIT_FAILURE);
                        }
                        break;

                    /*
                     * Il client ha richiesto di interrompere la connessione
                     * con il server
                     */
                    case INT:

                        // Al prossimo controllo del while usciamo oltre
                        connected = 0;
                        break;

                    /*
                     * Molto probabilmente è stato inviato un pacchetto non valido
                     * e quindi notifichiamo il tutto al client
                     */  
                    default:

                        // Facciamo log sul file
                        sprintf(requestMsg, "Received invalid packet");
                        status = FAILURE;
                        sprintf(additionalMsg, "No operation done");
                        formatMessage(&toBeLogged, operationAuthor, requestMsg, status, additionalMsg);
                        logF(toBeLogged);

                        // Inviamo la risposta al client
                        buildEmptyPacket(&packetToSend);
                        packetToSend.operation = INVALID_PACKET;
                        packetToSend.outcome = INVALID_PACKET;
                        memset(socketBuffer, '\0', PACKET_LENGTH);
                        buildMessage(socketBuffer, packetToSend);
                        if(write(clientFd, socketBuffer, PACKET_LENGTH) != PACKET_LENGTH) {
                            logMessage toBeLogged;
                            close(clientFd);
                            formatMessage(&toBeLogged, operationAuthor, "Connection terminated", FAILURE, "Error during client response, closing socket");
                            logF(toBeLogged);
                            exit(EXIT_FAILURE);
                        }
                }
            }

            // Prepariamo un pacchetto indicando la chiusura della connessione
            buildEmptyPacket(&packetToSend);
            packetToSend.operation = 'x';
            packetToSend.outcome = OPERATION_SUCCESS;

            // Facciamo log su file
            formatMessage(&toBeLogged, operationAuthor, "Socket closed", SUCCESS, "Session terminated correctly");
            logF(toBeLogged);

            // Inviamo il pacchetto al client
            memset(socketBuffer, '\0', PACKET_LENGTH);
            buildMessage(socketBuffer, packetToSend);
            if(write(clientFd, socketBuffer, PACKET_LENGTH) != PACKET_LENGTH) {
                logMessage toBeLogged;
                close(clientFd);
                formatMessage(&toBeLogged, operationAuthor, "Connection terminated", FAILURE, "Error during client response, closing socket");
                logF(toBeLogged);
                exit(EXIT_FAILURE);
            }
            
            // Chiudiamo la socket e terminiamo l'esecuzione
            close(clientFd);
            exit(EXIT_SUCCESS);

        } else { 
            // Processo padre, continua a stare in ascolto di richieste
            close(clientFd); // Chiude il descrittore di file
        }
    }
    return 0;
}