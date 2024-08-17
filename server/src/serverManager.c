#include "../include/utility.h"
#include "../include/log.h"
#include "../include/connection.h"
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <openssl/sha.h>

/*
 * Il processo figlio, che comunica con il client è immune a CTRL-C
 * Per il padre invece, questo era l'unico modo per potersi interrompere
 * 
 * Gestiamo il CTRL-C incaricando un figlio a eseguire questo programma,
 * fornendo un menu per eseguire operazioni aggiuntive che possono essere 
 * utili a lato server, come aggiunta e rimozione di un utente, lo spegnimento
 * inviando al padre un segnale di chiusura della socket, o la semplice
 * uscita dal menu, perchè abbiamo finito di utilizzarlo o anche semplicemente
 * perchè aperto per sbaglio
 */
int main(int argc, char **argv) {
    char operation, conferma;
    char username[AUTH_STRINGS_LENGTH + 1], password[AUTH_STRINGS_LENGTH + 1];
    int done = 0;
    logMessage toBeLogged;
    char opMsg[100], additional[100];
    memset(additional, '\0', 100);
    int status;

    // Queste informazioni vengono passate dal padre al momento dell'avvio del manager
    int parentPid = atoi(argv[1]);
    char *operationAuthor = argv[2];

    while(!done) {
        memset(opMsg, '\0', 100);
        memset(username, '\0', AUTH_STRINGS_LENGTH + 1);
        memset(password, '\0', AUTH_STRINGS_LENGTH + 1);

        // Ogni volta stampiamo un menu pulito, togliendo cio' che c'era prima
        printf(CLEAR);
        printf("%s", additional); // Eventuale messaggio extra deciso durante l'iterazione precedente
        printf(RESET_COLOR "Server utilities - Scegli operazione da eseguire:\n\n");
        printf("[" BCYAN "+" RESET_COLOR "] Aggiungi utente\n");
        printf("[" BCYAN "-" RESET_COLOR "] Rimuovi utente\n");
        printf("[" BYELLOW "x" RESET_COLOR "] Esci dal menu\n");
        printf("[" BRED "S" RESET_COLOR "] Termina server\n\n");

        memset(additional, '\0', 100);

        // Scelta operazione
        printf("Selezionare un'opzione: " MAGENTA);
        operation = getSingleChar();
        printf(RESET_COLOR);
        printf(CLEAR);
        
        switch(operation) {
        
            // Aggiunta di un utente
            case '+':

                // Chiediamo nome utente e password da aggiungere
                printf("Digitare le credenziali dell'utente da aggiungere:\n\n");
                printf("  Username: " MAGENTA);
                getOptionalInput(username, AUTH_PARAM_LENGTH + 2); // Acquisisco una stringa di 21 + '\0' così posso verificare se è più lunga di 20
                printf(RESET_COLOR);

                // Se l'username non è valido viene riacquisito comunicando l'errore e ristampando il menu
                while(!isUsernameValidAndNotEmpty(username)) {
                    printf(CLEAR);
                    if(strlen(username) > AUTH_PARAM_LENGTH) 
                        printf(RED "Nome utente digitato non valido, digitare massimo 20 caratteri\n\n" RESET_COLOR);
                    else if(strlen(username) == 0) 
                        printf(RED "Nome utente digitato non valido, non può essere lasciato vuoto\n\n" RESET_COLOR);
                    else
                        printf(RED "Nome utente digitato non valido, utilizzare solo lettere e numeri\n\n" RESET_COLOR);
                    printf("Digitare le credenziali dell'utente da aggiungere:\n\n");
                    printf("  Username: " MAGENTA);
                    getOptionalInput(username, AUTH_PARAM_LENGTH + 2);
                    printf(RESET_COLOR);
                }

                // Mostriamo l'username scritto dall'utente, e chiediamo la password
                printf(CLEAR);
                printf("Digitare le credenziali dell'utente da aggiungere:\n\n");
                printf("  Username: " MAGENTA "%s\n" RESET_COLOR, username);
                printf("  Password: " );
                disableEcho();
                getOptionalInput(password, AUTH_PARAM_LENGTH + 2); // Acquisisco una stringa di 21 + '\0' così posso verificare se è più lunga di 20
                enableEcho();

                // Se la password non è valida viene riacquisita comunicando l'errore e ristampando il menu
                while(!isPasswordValid(password)) {
                    printf(CLEAR);
                    if(strlen(password) > AUTH_PARAM_LENGTH) 
                        printf(RED "Password digitata non valida, digitare massimo 20 caratteri\n\n" RESET_COLOR);
                    else 
                        printf(RED "Password digitata non valida, utilizzare solo lettere e numeri\n\n" RESET_COLOR);
                    printf("Digitare le credenziali dell'utente da aggiungere:\n\n");
                    printf("  Username: " MAGENTA "%s\n" RESET_COLOR, username);
                    printf("  Password: ");
                    disableEcho();
                    getOptionalInput(password, AUTH_PARAM_LENGTH + 2);
                    enableEcho();
                }

                // Mostriamo username e password (nascosta) inserite
                printf(CLEAR);
                printf("Aggiunta dell'utente:\n\n");
                printf("  Username: " MAGENTA "%s\n" RESET_COLOR, username);
                char hiddenPassword[21];
                int i;
                for(i = 0; i < strlen(password); i++)
                    hiddenPassword[i] = '*';
                hiddenPassword[i] = '\0';
                printf("  Password: "MAGENTA "%s\n" RESET_COLOR, hiddenPassword);

                // Chiediamo una conferma
                printf("\n\nVuoi procedere?\n\n[" BGREEN "Y" RESET_COLOR "] Conferma\n[" RED "any other key" RESET_COLOR "] Annulla\n\n");
                printf("Selezionare un'opzione: " MAGENTA);
                conferma = getSingleChar();
                printf(RESET_COLOR);
                printf(CLEAR);

                // Procediamo solo se ha confermato
                if(conferma == 'Y' || conferma == 'y') {

                    // Proviamo a eseguire l'operazione e controlliamo l'esito dell'operazione
                    if(addUser(username, password)) {
                        status = SUCCESS;
                        sprintf(additional, GREEN "Utente aggiunto correttamente" RESET_COLOR "\n\n");
                    } else {
                        status = FAILURE;
                        sprintf(additional, RED "Utente non aggiunto, era gia' presente" RESET_COLOR "\n\n");
                    }

                    // Facciamo log dell'operazione
                    sprintf(opMsg, "+user [%s]", username);
                    formatMessage(&toBeLogged, operationAuthor, opMsg, status, NULL);
                    logF(toBeLogged);
                } else {
                    sprintf(additional, "Operazione annullata\n\n");
                }

                break;
            
            // Rimozione di un utente
            case '-':

                // Chiediamo il nome utente da rimuovere
                printf("Digitare il nome utente dell'utente da eliminare:\n\n");
                printf("  Username: " MAGENTA);
                getOptionalInput(username, AUTH_PARAM_LENGTH + 2); // Acquisisco una stringa di 21 + '\0' così posso verificare se è più lunga di 20
                printf(RESET_COLOR);

                // Se l'username non è valido viene riacquisito comunicando l'errore e ristampando il menu
                while(!isUsernameValidAndNotEmpty(username)) {
                    printf(CLEAR);
                    if(strlen(username) > AUTH_PARAM_LENGTH) 
                        printf(RED "Nome utente digitato non valido, digitare massimo 20 caratteri\n\n" RESET_COLOR);
                    else if(strlen(username) == 0) 
                        printf(RED "Nome utente digitato non valido, non può essere lasciato vuoto\n\n" RESET_COLOR);
                    else
                        printf(RED "Nome utente digitato non valido, utilizzare solo lettere e numeri\n\n" RESET_COLOR);
                    printf("Digitare il nome utente dell'utente da eliminare:\n\n");
                    printf("  Username: " MAGENTA);
                    getOptionalInput(username, AUTH_PARAM_LENGTH + 2);
                    printf(RESET_COLOR);
                }

                // Mostriamo il nome utente inserito
                printf(CLEAR);
                printf("Eliminazione dell'utente:\n\n");
                printf("  Username: " MAGENTA "%s" RESET_COLOR, username);
                
                // Chiediamo una conferma
                printf("\n\nVuoi procedere?\n[" GREEN "Y/y" RESET_COLOR "] Conferma\n[" RED "Altro carattere" RESET_COLOR "] Annulla\n" MAGENTA);
                conferma = getSingleChar();
                printf(RESET_COLOR);

                // Procediamo solo se ha confermato
                if(conferma == 'Y' || conferma == 'y') {

                    // Proviamo a eseguire l'operazione e controlliamo l'esito dell'operazione
                    if(removeUser(username)) {
                        status = SUCCESS;
                        sprintf(additional, GREEN "Utente rimosso correttamente" RESET_COLOR "\n\n");
                    } else {
                        status = FAILURE;
                        sprintf(additional, RED "Utente non rimosso, non era presente" RESET_COLOR "\n\n");
                    }

                    // Facciamo log dell'operazione
                    sprintf(opMsg, "-user [%s]", username);
                    formatMessage(&toBeLogged, operationAuthor, opMsg, status, NULL);
                    logF(toBeLogged);                    
                } else {
                    sprintf(additional,"Operazione annullata\n\n");
                }
                break;

            // Uscita dal menu
            case 'x':
            case 'X':

                // Alla prossima iterazione usciremo dal ciclo
                done = 1;
                
                // Inviamo al server un segnale, notificando la chiusura del manager
                kill(parentPid, SIGUSR1);
                break;

            // Terminazione del server
            case 's':
            case 'S':

                // Chiediamo una conferma
                printf("\nVuoi procedere con la terminazione del server?\nI gestori delle richieste delle sessioni già aperte " UNDERLINE "non" RESET_COLOR " termineranno:\n\n");
                printf("[" GREEN "Y" RESET_COLOR "] Conferma\n[" RED "any other key" RESET_COLOR "] Annulla\n\n");
                printf("Selezionare un'opzione: " MAGENTA);
                conferma = getSingleChar();
                printf(RESET_COLOR);

                // Procediamo solo se ha confermato
                if(conferma == 'Y' || conferma == 'y') {
                    printf(CLEAR);

                    // Inviamo al server un segnale, per la chiusura
                    kill(parentPid, SIGUSR2);
                    done = 1;
                }

                break;

            default:
                sprintf(additional, RED "Operazione non valida, inserisci un'operazione corretta" RESET_COLOR "\n\n");
        }
    }

    // Siamo usciti dal ciclo, interrompiamo
    exit(EXIT_SUCCESS);
}