// Dimensioni settori del pacchetto
#define PACKET_LENGTH 113

// Indirizzo di ogni settore del pacchetto
#define OPERATION_INDEX 0
#define OUTCOME_INDEX 1
#define USERNAME_INDEX 2
#define PASSWORD_INDEX 22
#define MATCHINDEX_INDEX 42
#define NAME_INDEX 52
#define SURNAME_INDEX 62
#define PHONE_NUMBER_INDEX 72
#define NEW_NAME_INDEX 82
#define NEW_SURNAME_INDEX 92
#define NEW_PHONE_NUMBER_INDEX 102

// Lungezze di ogni settore del pacchetto
#define OPERATION_LENGTH 1
#define OUTCOME_LENGHT 1
#define AUTH_PARAM_LENGTH 20
#define CONTACT_PARAM_LENGTH 10

// Costanti delle operazioni
#define READ 'r'
#define AUTH 'a'
#define ADD '+'
#define DEL '-'
#define MODIFY 'm'
#define INT 'x'

// Outcome delle operazioni
#define SERVER_ERROR '0'
#define OPERATION_SUCCESS '1'
#define READ_CONTACT_MISSING '2' 
#define CREDENTIALS_EXPIRED '3'
#define CONTACT_ALREADY_MODIFIED '4'
#define CONTACT_ALREADY_EXISTS '5'
#define INVALID_PACKET 'e'


/**
 * Rappresenta la struttura dei messaggi di comunicazione tra client e server
 * 
 * Campi:
 *  operation - Operazione da eseguire / eseguita
 *  outcome - Esito dell'operazione
 *  username - Nome utente (da usare per le operazioni che richiedono autenticazione)
 *  password - Password (da usare per le credenziali che richiedono autenticazione)
 *  matchIndex - Indice di corrispondenza del contatto nella rubrica
 *  name - Nome del contatto
 *  surname - Cognome del contatto
 *  phoneNumber - Numero di telefono del contatto
 *  newName - Nuono nome del contatto (da usare per la modifica)
 *  newSurname - Nuono cognome del contatto (da usare per la modifica)
 *  newPhoneNumber - Nuono numero di telefono del contatto (da usare per la modifica)
 */
typedef struct {
    char operation;
    char outcome;
    char username[AUTH_PARAM_LENGTH + 1];
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
 * Inizializza un pacchetto 'svuotandolo'
 * Dopo la funzione il pacchetto packet ha
 * ogni campo impostato a valori nulli
 * (per le stringhe ogni carattere sara' \0)
 */
void buildEmptyPacket(serverPacket *packet);

/**
 * Inserisce il pacchetto (packet) in un buffer (message) 
 * 
 * Da utilizzare per l'invio di pacchetti
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
 * Stampa il buffer (message) cercando
 * di formattarlo in un pacchetto
 * 
 * N.B. Utile per debugging
 */
void printMessage(char *message);