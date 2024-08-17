#include "utility.h"

// Dimensioni settori del pacchetto
#define OPERATION_LENGTH 1
#define OUTCOME_LENGTH 1
#define AUTH_PARAM_LENGTH 20
#define CONTACT_PARAM_LENGTH 10
#define PACKET_LENGTH 113
// Indirizzo di ogni settore del pacchetto
#define OPERATION_INDEX 0
#define OUTCOME_INDEX (OPERATION_INDEX + OPERATION_LENGTH) // 0+1 = 1
#define USERNAME_INDEX (OUTCOME_INDEX + OUTCOME_LENGTH) // 1+1 = 2
#define PASSWORD_INDEX (USERNAME_INDEX + AUTH_PARAM_LENGTH) // 2+20 = 22
#define MATCHINDEX_INDEX (PASSWORD_INDEX + AUTH_PARAM_LENGTH) // 22+20 = 42
#define NAME_INDEX (MATCHINDEX_INDEX + CONTACT_PARAM_LENGTH) // 42+10 = 52
#define SURNAME_INDEX (NAME_INDEX + CONTACT_PARAM_LENGTH) // 52+10 = 62
#define PHONE_NUMBER_INDEX (SURNAME_INDEX + CONTACT_PARAM_LENGTH) // 62+10 = 72
#define NEW_NAME_INDEX (PHONE_NUMBER_INDEX + CONTACT_PARAM_LENGTH) // 72+10 = 82
#define NEW_SURNAME_INDEX (NEW_NAME_INDEX + CONTACT_PARAM_LENGTH) // 82+10 = 92
#define NEW_PHONE_NUMBER_INDEX (NEW_SURNAME_INDEX + CONTACT_PARAM_LENGTH) // 92+10 = 102

// Costanti delle operazioni
#define READ 'r'
#define AUTH 'a'
#define ADD '+'
#define DEL '-'
#define MODIFY 'm'
#define INT 'x'
#define INVALID_PACKET 'e'
// Errori server
#define SERVER_ERROR '0'
#define OPERATION_SUCCESS '1'
#define READ_CONTACT_MISSING '2'
#define CREDENTIALS_EXPIRED '3'
#define CONTACT_ALREADY_MODIFIED '4'
#define CONTACT_ALREADY_EXISTS '5'

/**
 * Rappresenta la struttura dei messaggi di comunicazione tra client e server
 * 
 * Campi:
 *  operation - Operazione da eseguire / eseguita
 *  outcome - Esito dell'operazione
 *  username - Nome utente (da usare per le operazioni che richiedono autenticazione)
 *  password - Password (da usare per le credenziali che richiedono autenticazione)
 *  matchIndex - 
 *  name - Nome del contatto
 *  surname - Cognome del contatto
 *  phoneNumber - Numero di telefono del contatto
 *  newName - Nuovo nome del contatto (da usare per la modifica)
 *  newSurname - Nuovo cognome del contatto (da usare per la modifica)
 *  newPhoneNumber - Nuovo numero di telefono del contatto (da usare per la modifica)
 */
typedef struct{
    char operation;
    char outcome;
    char username[AUTH_PARAM_LENGTH + 1]; // +1 per il carattere di fine stringa
    char password[AUTH_PARAM_LENGTH + 1];
    unsigned int matchIndex;
    char name[CONTACT_PARAM_LENGTH + 1];
    char surname[CONTACT_PARAM_LENGTH + 1];
    char phoneNumber[CONTACT_PARAM_LENGTH + 1];
    char newName[CONTACT_PARAM_LENGTH + 1];
    char newSurname[CONTACT_PARAM_LENGTH + 1];
    char newPhoneNumber[CONTACT_PARAM_LENGTH + 1];
} serverPacket;

/**
 * Invia alla socket clientFD una richiesta di lettura di un contatto
 * dal server che corrisponda ai criteri specificati in toRead, in 
 * particolar modo, tra i vari contatti che possano combaciare con i
 * parametri richiesti, e' possibile specificare quale voler leggere
 * tramite matchIndex, il risultato della lettura viene salvato in serverRead
 * 
 * Restituisce l'esito dell'operazione
 */
int readContact(int clientFD, Contact *toRead,int matchIndex, Contact *serverRead);
/**
 * Invia alla socket clientFD una richiesta di autenticazione
 * utilizzando le credenziali username e password
 * 
 * Restituisce l'esito dell'operazione
 */
int authenticate(int clientFD, char *username, char *password);
/**
 * Invia alla socket clientFD una richiesta di aggiunta del contatto
 * doAdd nella rubrica del server
 * Vengono passate le credenziali per autenticarsi
 * 
 * Restituisce l'esito dell'operazione
 */
int addContact(int clientFD, char *username, char *password, Contact *toAdd);
/**
 * Invia alla socket clientFD una richiesta di cancellazione del contatto
 * toDelete, inviando anche le credenziali username e password per autenticarsi
 * 
 * Restituisce l'esito dell'operazione
 */
int deleteContact(int clientFD, char *username, char *password, Contact *toDelete);
/**
 * Invia alla socket clientFD una richiesta di modifica del contatto
 * toModify con il contatto modifiedContact
 * Invia le credenziali per autenticarsi
 * 
 * Restituisce l'esito dell'operazione
 */
int modifyContact(int clientFD, char *username, char *password, Contact *toModify, Contact *modifiedContact);



/**
 * Stampa il buffer (message) cercando
 * di formattarlo in un pacchetto
 * 
 * N.B. Utile per debugging
 */
void printMessage(char *message);



/**
 * Inizializza un pacchetto 'svuotandolo'
 * Dopo la funzione il pacchetto packet ha
 * ogni campo impostato a valori nulli
 * (per le stringhe ogni carattere sara' \0)
 */
void buildEmptyPacket(serverPacket *packet);
/**
 * Inserisce il pacchetto (packet) in un buffer (message) 
 * 
 * Da utilizzare per l'invio di pacchetti tra client e server
 * 
 * Le posizioni delle informazioni all'interno di message
 * si trovano all'inizio del file connection.h, definite come costanti
 */
void buildMessage(char *message, serverPacket packet);
/**
 * Formatta in un pacchetto (packet) le informazioni contenute nel buffer (message)
 * 
 * Da utilizzare per la ricezione di pacchetti
 * 
 * Le posizioni delle informazioni all'interno di message
 * si trovano all'inizio del file connection.h, definite come costanti
 */
void parseMessage(char *message, serverPacket *packet);
/**
 * Procedura di chiusura della socket (con descriptor socketFD)
 * Comunica al server l'intenzione di chiudere
 * e attende la conferma da parte del server
 */
int closeConnection(int socketFd);