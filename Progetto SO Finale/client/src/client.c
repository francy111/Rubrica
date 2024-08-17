#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>
#include <termios.h>
#include <arpa/inet.h>
#include "./../include/connection.h"

#define DEFAULT_PORT 50000 // Porta default
#define MAX_CONNECTION_ATTEMPTS 5 // Numero massimo di tentativi di connessione
#define INITIAL_DELAY 2 // Tempo di attesa iniziale dopo un tentativo di connessione
#define MAX_DELAY 30 // Tempo di attesa massimo dopo un tentativo di connessione

#define MAX_AUTH_ATTEMPTS 3 // Numero massimo di tentativi di autenticazione falliti
#define AUTH_COOLDOWN_TIME 60 // Tempo di attesa dopo un che l'utente supera il massimo di tentativi di autenticazione falliti

/*
 * clientFd - FD della socket usata per comunicare con il server, ottenuta tramite accept
 * 
 * Dichiarata come variabile globale in quanto la gestione della socket 
 * avviene anche nella gestione del segnale e quindi è necessario
 * poter accedere ai file descriptor fuori dal main
 */
int clientFD;

/*
 * Gestiamo il CTRL-C chiudendo correttamente la socket, richiamando
 * l'apposita procedura di chiusura della connessione
 */
void ctrlc_handler(int sig) {
    closeConnection(clientFD);
    exit(EXIT_SUCCESS);
}

/*
 * Gestiamo il segnale SIGPIPE
 * Vogliamo gestire la chiusura improvvisa della socket del server
 * Chiudendo correttamente la socket
 */
void sigpipe_handler(int sig) {
    close(clientFD);
    printf("Connessione chiusa");
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {

	/* Inizializzazione strutture dati e collegamento al server */
    struct sockaddr_in serverAddress;
    struct sockaddr* serverAddressPtr;
    serverAddressPtr = (struct sockaddr*) &serverAddress;
    serverAddress.sin_family = AF_INET;
    int port;

    /* 
     * Diamo l'opportunita' al client di specificare indirizzo ip e numero di porta 
     * del server a cui vuole provare a collegarsi, supponendo che il file eseguibile si chiami "client"
     * 
     * Esempi di modalita' di avvio:
     *   ./client - In automatico cerca di collegarsi un server Locale sulla porta 50000
     *   ./client localhost 50000 o ./client 127.0.0.1 50000 - Stesso effetto della riga soprastante
     *   ./client 54.23.132.12 54434 - Cerca di collegarsi ad un server all'IP 54.23.132.12 al numero di porta 54434
     */
    if(argc == 3) { // Se sono sufficienti i parametri

        /*
         * Trasformiamo la stringa contente l'indirizzo IP in
         * un tipo utilizzabile e impostabile nella struttuta indirizzo socket
         * 
         * La funzione che trasforma la stringa in un indirizzo valido è inet_pton
         * Questa pero' lavora solo con indirizzi ipv4, quindi non funzionerebbe sulla stringa 'localhost'
         * Controlliamo manualmente se è stato scritto localhost
         */
        if(strncmp(argv[1], "localhost", strlen(argv[1])) == 0) { 
            serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        } else { // Controlliamo se è un indirizzo IP valido (prova a impostarlo e restituisce l'esito)
            if (inet_pton(AF_INET, argv[1], &serverAddress.sin_addr) <= 0) { 
                exit(EXIT_FAILURE); // Se l'IP non è valido chiudiamo subito, perchè non riusciremmo ad aprire la socket
            }
        }
        port = atoi(argv[2]); 
        // Se la porta non è valida chiude
        if(port <= 0) {
            exit(EXIT_FAILURE);
        }
    } else { // Non sono sufficienti, in automatico impostiamo localhost e porta default
        port = DEFAULT_PORT;
        serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }

    // Impostiamo numero di porta nella struct per l'indirizzo della socket
    serverAddress.sin_port = htons(port);

    // Proviamo ad aprire la socket
    clientFD = socket(AF_INET, SOCK_STREAM, 0);
    if(clientFD < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    int result;
    // Proviamo a connetterci al server
    result = connect(clientFD, serverAddressPtr, sizeof(serverAddress));
    
    /*
     * Gestione dei SIGPIPE
     * Vogliamo limitare le interruzioni della connessione al server il piu' possibile
     * perlomeno controllando la scrittura su socket chiusa
     * 
     * Gestiamo SIGPIPE
     */
    signal(SIGPIPE, sigpipe_handler);

    /*
     * Non proviamo a connetterci al server una volta soltanto, in quanto potrebbe non funzionare subito al primo tentativo
     * Per questo proviamo piu' tentativi di connessione, aumentando ogni volta l'intervallo tra un tentativo e l'altro
     */
    int delay = INITIAL_DELAY, attempt = 1;;
    while(result == -1 && attempt < MAX_CONNECTION_ATTEMPTS) { // Finchè non siamo connessi e abbiamo tentativi a disposizione
        
        // Aumenta il tempo che intercorre per il prossimo tentativo
        int delay = INITIAL_DELAY * ((attempt-1) * (attempt-1) + 1); // backoff esponenziale

        // Non superiamo comunque un tempo massimo di intervallo di attesa
        if(delay > MAX_DELAY)
            delay = MAX_DELAY;

        // Decrementiamo un secondo alla volta per comunicare in modo più preciso i secondi mancanti all'utente
        while (delay >0){
            printf(CLEAR);
            printf(RED "Connessione non riuscita\n" RESET_COLOR);
            printf("Nuovo tentativo di connessione tra " BMAGENTA "%d" RESET_COLOR " secondi, tentativo: [" BCYAN "%d" RESET_COLOR "]\n", delay, attempt);
            sleep(1);
            delay--;
        }
        
        result = connect(clientFD, serverAddressPtr, sizeof(serverAddress));
        attempt++;
    }

    /*
     * Gestione del segnale CTRL-C
     * Vogliamo limitare le interruzioni della connessione al server il piu' possibile
     * perlomeno gestiamo diversamente quelle manuali 
     * 
     * Geestendo CTRL-C in modo da chiamare la procedura di chiusura
     * (invia al server un pacchetto notificandolo) e chiudendo la socket
     * 
     * Impostiamo questo handler solo dopo la connessione, fino a prima
     * il CTRL-C ha l'effetto standard
     */
    signal(SIGINT, ctrlc_handler);

    short int connected = 0;
    if(result == -1) {
        // Tutti tentativi falliti
        printf(CLEAR);
        printf(RED "\nImpossibile stabilire una connessione con il server\n" RESET_COLOR);
    
    } else {
        // Connessi
        printf(CLEAR);
        connected = 1;
        printf(GREEN "\nConnessione con il server stabilita con successo\n" RESET_COLOR);
        sleep(1);
        printf(CLEAR);
    }
    

    // Credenziali per l'autenticazione
    int authenticated = 0, authentication_allowed = 1, authAttempts = 0;;
    time_t authentication_time;
    char username[AUTH_PARAM_LENGTH], password[AUTH_PARAM_LENGTH];

    // Finchè siamo connessi possiamo fare operazioni sul server
    
    while(connected) {
        char operation;
        // Menu
        printTitle("       GESTIONE RUBRICA       ");
        printf("[" BCYAN "1" RESET_COLOR "] Leggere rubrica\n");
        printf(authenticated ? "["BCYAN "2" RESET_COLOR "] Modificare rubrica\n" : "[" BCYAN "2" RESET_COLOR "] Autenticati\n");
        printf("[" BRED "x" RESET_COLOR "] Terminare sessione\n\n");
        printf("Selezionare un'opzione: " CYAN); // Per rendere il colore del testo digitato dall'utente ciano
        operation = getSingleChar();
        printf(RESET_COLOR); // Dopo l'acquisizione dall'utente avviene il reset del colore
        printf(CLEAR);
        switch(operation) {
            case '1': // Lettura dalla rubrica
                // La prima lettura viene effettuata subito senza stampare il menu
                // Acquisizione dei parametri per la ricerca
                Contact toRead;
                Contact *toReadPtr = &toRead;
                int matchIndex = 1;
                getOptionalContact(toReadPtr, "       ACQUISIZIONE DATI LETTURA       ");

                Contact serverContact;
                // Richiesta di lettura al server
                int outcome = readContact(clientFD,toReadPtr,matchIndex,&serverContact);

                int reading = 1, matchedContact = 1;
                // Controlliamo il risultato dell'operazione sul server
                if(outcome == 1) { // Lettura avvenuta con successo
                    // Mostriamo il contatto trovato, quello che è stato letto
                    printCommunication("Individuata corrispondenza con successo",GREEN);
                    printContactIndex(serverContact, matchIndex);
                
                } else if(outcome == 2) { // Non sono state trovate corrispondenze
                    matchedContact = 0;
                    printCommunication( "Non sono state individuate corrispondenze con i dati inseriti", YELLOW);
                
                } else {
                    reading = 0;
                    matchedContact = 0;
                    printCommunication("Il server ha riscontrato un errore, operazione annullata", RED);
                }
                    
                /* 
                * Diamo opportunita' all'utente di cercare un contatto modificando i parametri di ricerca
                * oppure trovare il prossimo contatto che corrisponde ai parametri indicati precedentemente
                * infine, puo' uscire da questo sottomenu per cambiare tipo di operazione
                */
                char readOption;
                while(reading) {
                    printTitle("       MENU DI LETTURA       ");
                    printf("[" BCYAN "1" RESET_COLOR "] Inserire dei dati da cercare differenti\n");
                    if(matchedContact)
                        printf("[" BCYAN "2" RESET_COLOR "] Leggere corrispondenza successiva con gli stessi dati\n");
                    printf("[" BRED "x" RESET_COLOR "] Terminare la lettura\n\n");
                    printf("Selezionare un'opzione: " CYAN);
                    readOption = getSingleChar();
                    printf(RESET_COLOR);
                    printf(CLEAR);
                    
                    switch (readOption) {
                        
                        case '1': // Modificare i criteri di ricerca
                            matchIndex = 1;
                            
                            // Acquisisce i dati diversi per cercare il record
                            getOptionalContact(toReadPtr, "       ACQUISIZIONE DATI LETTURA       ");
                            
                            // Richiesta al server
                            outcome = readContact(clientFD,&toRead,matchIndex,&serverContact);
                            
                            // Lettura avvenuta con successo
                            if(outcome == 1) {
                                printCommunication("Individuata corrispondenza con successo",GREEN);
                                printContactIndex(serverContact, matchIndex);
                                matchedContact = 1;
                            
                            // Nessun contatto trovato
                            } else if(outcome == 2) {
                                printCommunication( "Non sono state individuate corrispondenze con i dati inseriti", YELLOW);
                                matchedContact = 0;
                            
                            // Errore lato server
                            } else {
                                printCommunication("Il server ha riscontrato un errore, operazione annullata", RED);
                                reading = 0;
                            }
                            break;

                        case '2': // Lettura del prossimo contatto usando gli stessi criteri di ricerca
                            // Questa opzione è valida solo se era stato trovato un contatto precedentemente
                            if(matchedContact) {

                                // Richiede al server il record successivo
                                matchIndex++;

                                // Richiesta al server
                                outcome = readContact(clientFD,&toRead,matchIndex,&serverContact);
                                
                                // Lettura avvenuta con successo
                                if(outcome == 1) {
                                    printCommunication("Individuata corrispondenza con successo",GREEN);
                                    printContactIndex(serverContact, matchIndex);
                                
                                // Nessun contatto trovato
                                }else if(outcome == 2) {
                                    printCommunication( "Non sono state individuate ulteriori corrispondenze con i dati inseriti", YELLOW);
                                    matchedContact = 0;
                                
                                // Errore lato server
                                } else {
                                    printCommunication("Il server ha riscontrato un errore, operazione annullata", RED);
                                    reading = 0;
                                }

                            } else { // Se non era stato trovato un contatto precedentemente
                                printCommunication("Operazione selezionata non valida", RED);
                            }
                            break;

                        // Interrompiamo la lettura (usciamo anche dal sottomenu)
                        case 'x':
                        case 'X':
                            reading = 0;
                            break;

                        default: // Qualsiasi carattere che non sia valido
                            printCommunication("Operazione selezionata non valida", RED);
                            if(matchedContact) // Stampa della corrispondenza letta solo se era stata trovata
                                printContactIndex(serverContact, matchIndex);
                            break;
                    }
                }
                break;
                
            case '2':

                // Il client può effettuare la modifica solo se autenticato
                if(authenticated) {
                    /*
                     * Raggruppiamo le varie operazioni di modifica della rubrica
                     * Tutte queste operazioni necessitano di autorizzazione per essere scelte
                     */
                    char modifyOperation, modifying = 1;
                    while(modifying) {
                        printTitle("       MENU DI MODIFICA       ");
                        printf("[" BCYAN "1" RESET_COLOR "] Aggiungere nuovo contatto\n");
                        printf("[" BCYAN "2" RESET_COLOR "] Cancellare contatto dalla rubrica\n");
                        printf("[" BCYAN "3" RESET_COLOR "] Modificare contatto sulla rubrica\n");
                        printf("[" BRED "x" RESET_COLOR "] Terminare la modifica\n\n");
                        printf("Selezionare un'opzione: " CYAN);
                        modifyOperation = getSingleChar();
                        printf(RESET_COLOR);
                        printf(CLEAR);
                        
                        int matchIndex;
                        Contact serverContact;
                        switch (modifyOperation) {

                            case '1': // Aggiunta di un nuovo contatto
                                // Acquisizione delle informazioni del contatto da aggiungere
                                Contact toAdd;
                                Contact *toAddPtr = &toAdd;
                                getNotEmptyContact(toAddPtr,"       ACQUISIZIONE DATI AGGIUNTA       ");
                                int adding = 1;
                                while(adding) {
                                    char addOption;
                                    printf("Contatto digitato:\n");
                                    printContact(toAdd);

                                    // Chiediamo conferma per l'aggiunta
                                    printTitle("       MENU DI AGGIUNTA      ");
                                    printf("[" BCYAN "1" RESET_COLOR "] Procedere con l'aggiunta del contatto\n");
                                    printf("[" BRED "x" RESET_COLOR "] Annullare aggiunta del nuovo contatto\n\n");
                                    printf("Selezionare un'opzione: " CYAN);
                                    addOption = getSingleChar();
                                    printf(RESET_COLOR);
                                    printf(CLEAR);
                                    switch (addOption) {

                                        case '1': // L'utente ha confermato l'aggiunta
                                            // Richiesta di aggiunta al server
                                            int outcome= addContact(clientFD, username, password, toAddPtr);
                                            if(outcome == 1) // Il contatto è stato aggiunto
                                                printCommunication("Contatto aggiunto con successo", GREEN);
                                            
                                            else if(outcome == 3) { // L'utente ha utilizzato credenziali che non sono piu' valide
                                                printCommunication("Le credenziali digitate precedentemente non sono più valide", RED);
                                                authenticated = 0;
                                                modifying = 0;
                                            
                                            } else if(outcome == 5) { // è stato digitato un contatto che era gia' presente
                                                printCommunication("Impossibile aggiungere il nuovo contatto, è già presente un contatto identico", RED);
                                                authenticated = 0;
                                            }
                                            else // Errore lato server
                                                printCommunication("Il server ha riscontrato un errore, operazione annullata", RED);
                                            adding = 0;
                                            break;
                                        
                                        // L'utente ha annullato l'aggiunta
                                        case 'x':
                                        case 'X':
                                            adding = 0;
                                            break;
                                        default: 
                                            printCommunication("Operazione selezionata non valida", RED);
                                            break;
                                    }
                                }
                                break;

                            /*
                             * Cancellazione di un contatto
                             * Eseguiamo una o piu' richieste di read per cercare il contatto
                             * che ci interessa cancellare, poi successivamente richiediamo una
                             * delete passando le informazioni del contatto cancellato
                             */
                            case '2':
                                // Acquisiamo le informazioni del contatto da rimuovere
                                matchIndex = 1;
                                Contact toDelete;
                                Contact *toDeletePtr = &toDelete;
                                getOptionalContact(toDeletePtr,"       ACQUISIZIONE DATI CANCELLAZIONE       ");
                                
                                // Richiesta di cancellazione al server
                                outcome = readContact(clientFD, toDeletePtr, matchIndex, &serverContact);
                                
                                int matchedContact = 1, deleting = 1;
                                if(outcome == 1) { // Lettura avvenuta con successo
                                    // Mostriamo il contatto trovato, quello che verra' cancellato
                                    printCommunication("Individuata corrispondenza con successo",GREEN);
                                    printContactIndex(serverContact, matchIndex);
                                
                                } else if(outcome == 2) { // Non sono state trovate corrispondenze
                                    matchedContact = 0;
                                    printCommunication( "Non sono state individuate corrispondenze con i dati inseriti", YELLOW);
                                
                                } else { // Errore lato server
                                    deleting = 0;
                                    matchedContact = 0;
                                    printCommunication("Il server ha riscontrato un errore, operazione annullata", RED);
                                }
                                
                                while(deleting) {
                                    char deleteOption;
                                    // Diamo l'opportunita' di scegliere se procedere con l'eliminazione o cercare un contatto differente
                                    printTitle("       MENU DI CANCELLAZIONE       ");
                                    printf("[" BCYAN "1" RESET_COLOR "] Inserire dei dati di ricerca differenti\n");
                                    if(matchedContact) {
                                        printf("[" BCYAN "2" RESET_COLOR "] Leggere contatto successivo con gli stessi dati\n");
                                        printf("[" BCYAN "3" RESET_COLOR "] Procedere con l'eliminazione\n");
                                    }
                                    printf("[" BRED "x" RESET_COLOR "] Terminare l'eliminazione\n\n");
                                    printf("Selezionare un'opzione: " CYAN);
                                    deleteOption = getSingleChar();
                                    printf(RESET_COLOR);
                                    printf(CLEAR);
                                    switch (deleteOption) {
                                    
                                        case '1': // Cercare un contatto differente, modificando i criteri di ricerca
                                            // Acquisisce i dati diversi per cercare il record
                                            matchIndex = 1;
                                            getOptionalContact(toDeletePtr,"       ACQUISIZIONE DATI CANCELLAZIONE       ");

                                            // Richiesta al server di lettura
                                            outcome = readContact(clientFD,&toDelete,matchIndex,&serverContact);
                                            if(outcome == 1) { // Contatto trovato
                                                matchedContact = 1;
                                                printCommunication("Individuata corrispondenza con successo",GREEN);
                                                printContactIndex(serverContact, matchIndex);
                                            
                                            } else if (outcome == 2) { // Nessun match
                                                matchedContact = 0;
                                                printCommunication( "Non sono state individuate corrispondenze con i dati inseriti", YELLOW);
                                            
                                            } else { // Errore lato server
                                                printCommunication("Il server ha riscontrato un errore, operazione annullata", RED);
                                                deleting = 0;
                                            }
                                            break;

                                        case '2': // Stessi criteri di ricerca per il contatto successivo
                                            // Questa opzione è valida solo se era stato trovato un contatto precedentemente
                                            if(matchedContact) {
                                                matchIndex++;
                                                // Richiesta al server
                                                outcome = readContact(clientFD,&toDelete,matchIndex,&serverContact);
                                                if(outcome == 1) { // Contatto trovato
                                                    matchedContact = 1;
                                                    printCommunication("Individuata corrispondenza con successo",GREEN);
                                                    printContactIndex(serverContact, matchIndex);
                                                
                                                }else if(outcome == 2) { // Nessun match
                                                    matchedContact = 0;
                                                    printCommunication( "Non sono state individuate corrispondenze con i dati inseriti", YELLOW);
                                                }
                                                 else { // Errore lato server
                                                    deleting = 0;
                                                    printCommunication("Il server ha riscontrato un errore, operazione annullata", RED);
                                                }

                                            } else { // Se non era stato trovato un contatto precedentemente
                                                printCommunication("Operazione selezionata non valida", RED);
                                            }
                                            break;

                                        case '3': // Procediamo con la cancellazione
                                            // Questa opzione è valida solo se era stato trovato un contatto precedentemente
                                            if(matchedContact) {
                                                char deleteChoice;

                                                // Mostriamo il contatto da eliminare
                                                printContactIndex(serverContact, matchIndex);

                                                // Chiediamo una conferma
                                                printTitle("       CONFERMA ELIMINAZIONE       ");
                                                printf("Vuoi procedere? Il contatto verrà " UNDERLINE "eliminato permanentemente" RESET_COLOR " dalla rubrica:\n\n");
                                                printf("[" BGREEN "Y" RESET_COLOR "] si\n");
                                                printf("[" RED "any other key" RESET_COLOR "] no\n\n");
                                                printf("Selezionare un'opzione: " CYAN);
                                                deleteChoice = getSingleChar();
                                                printf(RESET_COLOR);
                                                printf(CLEAR);
                                                outcome = 0;

                                                // L'utente ha confermato la cancellazione
                                                if(deleteChoice == 'y' || deleteChoice == 'Y') {

                                                    // Inviamo al server una richiesta di cancellazione
                                                    outcome = deleteContact(clientFD, username, password, &serverContact);
                                                    if(outcome == 1) { // Utente rimosso
                                                        printCommunication("Contatto rimosso con successo", GREEN);
                                                    
                                                    } else if(outcome == 3) { // Autenticazione fallita
                                                        printCommunication("Le credenziali digitate precedentemente non sono più valide", RED);
                                                        authenticated = 0;
                                                        modifying = 0;
                                                   
                                                    } else if(outcome == 4) { // Il contatto è stato gia' cancellato 
                                                        printCommunication("Eliminazione fallita, il contatto è già stato eliminato", RED);
                                                    }
                                                     else { // Errore lato server
                                                        printCommunication("Il server ha riscontrato un errore, operazione annullata", RED);
                                                    }
                                                    deleting = 0;

                                                } else { // Annullata eliminazione
                                                    printCommunication("Eliminazione contatto annullata", RESET_COLOR);
                                                    printContactIndex(serverContact, matchIndex);
                                                }

                                            } else { // Se non era stato trovato un contatto precedentemente
                                                printCommunication("Operazione selezionata non valida", RED);
                                            }
                                            break;
                                        
                                        // L'utente ha annullato la cancellazione
                                        case 'x':
                                        case 'X':
                                            deleting = 0;
                                            break;

                                        default: // Qualsiasi carattere che non sia valido
                                            printCommunication("Operazione selezionata non valida", RED);
                                            if(matchedContact) // Stampa della corrispondenza letta solo se era stata trovata
                                                printContactIndex(serverContact, matchIndex);
                                            break;
                                    }
                                }
                                break;

                            /*
                             * Modifica di un contatto
                             * Eseguiamo una o piu' richieste di read per cercare il contatto
                             * che ci interessa modificare, poi successivamente chiediamo le
                             * informazioni del contatto nuovo da inserire e richiediamo una
                             * modify passando le informazioni del contatto cancellato e nuovo
                             */
                            case '3':
                                // Acquisiamo il contatto da modificare
                                matchIndex = 1;
                                Contact toModify;
                                Contact *toModifyPtr = &toModify;
                                getOptionalContact(toModifyPtr,"       ACQUISIZIONE DATI DI RICERCA PER MODIFICA       ");

                                // Richiesta di lettura al server
                                outcome = readContact(clientFD, &toModify, matchIndex, &serverContact);
                                
                                int modifyingContact = 1;
                                matchedContact = 1;
                                if(outcome == 1) { // Lettura avvenuta con successo
                                    // Mostriamo il contatto trovato, quello che verra' cancellato
                                    printCommunication("Individuata corrispondenza con successo",GREEN);
                                    printContactIndex(serverContact, matchIndex);
                                
                                } else if(outcome == 2) { // Non sono state trovate corrispondenze
                                    matchedContact = 0;
                                    printCommunication( "Non sono state individuate corrispondenze con i dati inseriti", YELLOW);
                                
                                } else { // Errore lato server
                                    modifyingContact = 0;
                                    matchedContact = 0;
                                    printCommunication("Il server ha riscontrato un errore, operazione annullata", RED);
                                }
                                    
                                // Diamo l'opportunita' di scegliere se procedere con la modifica o cercare un contatto differente
                                while(modifyingContact) {
                                    char modifyOption;
                                    printTitle("       MENU DI MODIFICA       ");
                                    printf("[" BCYAN "1" RESET_COLOR "] Inserire dei dati di ricerca differenti\n");
                                    if(matchedContact) {
                                        printf("[" BCYAN "2" RESET_COLOR "] Leggere contatto successivo con gli stessi dati\n");
                                        printf("[" BCYAN "3" RESET_COLOR "] Procedere con la modifica\n");
                                    }
                                    printf("[" BRED "x" RESET_COLOR "] Terminare la modifica\n\n");
                                    printf("Selezionare un'opzione: " CYAN);
                                    modifyOption = getSingleChar();
                                    printf(RESET_COLOR);
                                    printf(CLEAR);
                                    switch (modifyOption) {
                                        
                                        case '1': // Modifichiamo i criteri di ricerca per il contatto
                                            // Acquisisce i dati diversi per cercare il record
                                            matchIndex = 1;
                                            getOptionalContact(toModifyPtr, "       ACQUISIZIONE DATI DI RICERCA PER MODIFICA       ");
                                            // Richiesta di lettura al server
                                            outcome = readContact(clientFD, &toModify, matchIndex, &serverContact);
                                            
                                            if(outcome == 1) { // Contatto trovato
                                                matchedContact = 1;
                                                printCommunication("Individuata corrispondenza con successo",GREEN);
                                                printContactIndex(serverContact, matchIndex);
                                            
                                            } else if (outcome == 2) { // Contatto non trovato
                                                matchedContact = 0;
                                                printCommunication( "Non sono state individuate corrispondenze con i dati inseriti", YELLOW);
                                            
                                            } else { // Errore del server
                                                printCommunication("Il server ha riscontrato un errore, operazione annullata", RED);
                                                modifyingContact = 0;
                                            }
                                            break;
                                        
                                        case '2': // Stessi criteri di ricerca, ma il contatto successivo
                                            // Questa opzione è valida solo se era stato trovato un contatto precedentemente
                                            if(matchedContact) {
                                                matchIndex++;
                                                // Richiesta al server
                                                outcome = readContact(clientFD,&toModify,matchIndex,&serverContact);
                                                if(outcome == 1) { // Contatto trovato
                                                    printCommunication("Individuata corrispondenza con successo",GREEN);
                                                    printContactIndex(serverContact, matchIndex);
                                                } else if(outcome == 2) { // Contatto non trovato
                                                    matchedContact = 0;
                                                    printCommunication( "Non sono state individuate corrispondenze con i dati inseriti", YELLOW);
                                                } else { // Errore del server
                                                    modifyingContact = 0;
                                                    printCommunication("Il server ha riscontrato un errore, operazione annullata", RED);
                                                }
                                            
                                            } else { // Se non era stato trovato un contatto precedentemente
                                                printCommunication("Operazione selezionata non valida", RED);
                                            }
                                            break;

                                        case '3': // Procediamo con la modifica
                                            // Questa opzione è valida solo se era stato trovato un contatto precedentemente
                                            if(matchedContact) {
                                                // Acquisiamo le informazioni nuove da aggiungere al posto di quelle vecchie
                                                char modifyChoice;
                                                Contact modified;
                                                Contact *modifiedPtr = &modified;
                                                getNotEmptyContact(modifiedPtr,"       ACQUISIZIONE DATI CONTATTO MODIFICATI       ");
                                                printContactIndex(serverContact, matchIndex);
                                                printf("Contatto modificato:\n");
                                                printContact(modified);

                                                // Chiediamo la conferma per poter procedere con la modifica
                                                printTitle("       CONFERMA MODIFICA       ");
                                                printf("Vuoi procedere? La modifica del contatto è " UNDERLINE "irreversibile\n\n" RESET_COLOR);
                                                printf("[" BGREEN "Y" RESET_COLOR "] si\n");
                                                printf("[" RED "any other key" RESET_COLOR "] no\n\n");
                                                printf("Selezionare un'opzione: " CYAN);
                                                modifyChoice = getSingleChar();
                                                printf(RESET_COLOR);
                                                printf(CLEAR);

                                                outcome = 0;
                                                // L'utente ha confermato la modifica
                                                if(modifyChoice == 'y' || modifyChoice == 'Y') {
                                                    // Invio richiesta di modifica al server
                                                    outcome = modifyContact(clientFD, username, password, &serverContact, modifiedPtr);
                                                    
                                                    if(outcome == 1) // Contatto modificato
                                                        printCommunication("Contatto modificato con successo", GREEN);
                                                    
                                                    else if(outcome == 3) { // Autenticazione fallita
                                                        printCommunication("Le credenziali digitate precedentemente non sono più valide", RED);
                                                        authenticated = 0;
                                                        modifying = 0;
                                                    
                                                    } else if(outcome == 4) { // Contatto non piu' presente in rubrica
                                                        printCommunication("Modifica fallita, il contatto da modificare non è più in rubrica", RED);
                                                    
                                                    } else // Errore lato server
                                                        printCommunication("Il server ha riscontrato un errore, operazione annullata", RED);
                                                    modifyingContact = 0;
                                                
                                                } else { // L'utente ha annullato la modifica
                                                    printCommunication("Modifica contatto annullata", RESET_COLOR);
                                                    printContactIndex(serverContact, matchIndex);
                                                }

                                            } else { // Se non era stato trovato un contatto precedentemente
                                                printCommunication("Operazione selezionata non valida", RED);
                                            }
                                            break;

                                        // Termina la modifica del contatto
                                        case 'x':
                                        case 'X':
                                            modifyingContact = 0;
                                            break;

                                        default: // Qualsiasi carattere che non sia valido
                                            printCommunication("Operazione selezionata non valida", RED);
                                            if(matchedContact) // Stampa della corrispondenza letta solo se era stata trovata
                                                printContactIndex(serverContact, matchIndex);
                                            break;
                                    }
                                }
                                break;

                            // Termina il menu di modifica
                            case 'x':
                            case 'X':
                                modifying = 0;
                                break;
                            
                            default: // Qualsiasi carattere che non sia valido
                                printCommunication("Operazione selezionata non valida", RED);
                                break;
                        }
                    }
                    
                } else { // L'utente non è autenticato
          	        
                    /*
                     * Diamo un massimo numero di tentativi per l'autenticazione, dopo i quali l'autenticazione
                     * sara' disabilitata per un certo intervallo di tempo, l'utente potra' comunque eseguire
                     * l'operazione di lettura
                     */
                    if(!authentication_allowed) { // Se c'è un cooldown, controlliamo se è scaduto e se lo è permettiamo di nuovo di autenticarsi altrimenti indichiamo il tempo mancante
                        time_t diff = difftime(time(NULL), authentication_time);
                        if( diff > AUTH_COOLDOWN_TIME) {
                            authentication_allowed = 1;
                            authAttempts = MAX_AUTH_ATTEMPTS - 1; // Se il tempo da aspettare è passato permettiamo un'altro tentativo
                        } else{
                            printf("Devi attendere " BMAGENTA "%ld" RESET_COLOR " secondi prima di poter ritentare l'autenticazione\n\n", AUTH_COOLDOWN_TIME - diff);
                        }
                    }
                    
                    // Permessa solo se non c'è un cooldown sull'autenticazione
                    if(authentication_allowed) {
                        int authenticating = 1;
                        while(authenticating) {
                            
                            // Acquisizione delle credenziali dall'utente
                            printTitle("       AUTENTICAZIONE       ");
                            printf("Tentativi rimanenti: " BMAGENTA "%d\n\n" RESET_COLOR, MAX_AUTH_ATTEMPTS - authAttempts);
                            printf("Username: " CYAN);
                            getOptionalInput(username, AUTH_PARAM_LENGTH + 1);
                            printf(RESET_COLOR "Password: ");
                            disableEcho();
                            getOptionalInput(password, AUTH_PARAM_LENGTH + 1);
                            enableEcho();
                            printf(CLEAR);
                            authAttempts++;

                            // Inizializzato a 0 in modo che se le credenziali non sono valide è come se l'autenticazione fosse fallita
                            int outcome = 0;
                            // Chiamata al server per l'autenticazione, solo se le credenziali sono valide (non vuote, lunghe meno di 20 caratteri e senza virgole)
                            if(isUsernameValidAndNotEmpty(username) && isPasswordValid(password))
                                outcome = authenticate(clientFD, username, password);
                            
                            // Controlliamo la risposta del server
                            if(outcome) { // Autenticazione avvenuta con successo
                                authenticated = 1;
                                authenticating = 0;
                                authAttempts = 0; // azzeriamo i tentativi effettuati
                                printCommunication("Autenticato con successo",GREEN);
                           
                            } else { // Autenticazione fallita
                                // Sottomenu per chiedere all'utente se vuole ritentare l'autenticazione
                                if(authAttempts < MAX_AUTH_ATTEMPTS) { // Controlliamo se l'utente ha superato i tentativi massimi di autenticazione
                                    printCommunication("Credenziali non valide",RED);
                                    char authenticateOption;
                                    printTitle("       AUTENTICAZIONE       ");
                                    printf("[" BCYAN "1" RESET_COLOR "] Ritenta l'autenticazione\n");
                                    printf("[" RED "any other key" RESET_COLOR "] Torna al menu principale\n\n");
                                    printf("Selezionare un'opzione: " CYAN);
                                    authenticateOption = getSingleChar();
                                    printf(RESET_COLOR);
                                    if(authenticateOption != '1')
                                        authenticating = 0;
                                    printf(CLEAR);
                                
                                } else { // Se supera i tentativi l'autenticazione viene bloccata temporaneamente
                                    printf("Troppi tentativi falliti, autenticazione disabilitata per " BMAGENTA "%d" RESET_COLOR " secondi\n\n",AUTH_COOLDOWN_TIME);
                                    authentication_time = time(NULL); // Salviamo il tempo iniziale per calcolare in futuro quanto tempo è trascorso
                                    authentication_allowed = 0;
                                    authenticating = 0;
                                }
                            }
                        }
                    }
                }
                break;
            
            // Usciamo dal menu principale e terminiamo la sessione chiudendo la connessione con il server
            case 'x':
            case 'X':
                connected = closeConnection(clientFD);
                break;

            default: // Qualsiasi carattere che non sia valido
                printCommunication("Operazione selezionata non valida", RED);
                break;
        }
    }
	return 0;
}
