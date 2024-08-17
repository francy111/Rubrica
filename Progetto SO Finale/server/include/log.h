#define SUCCESS 1
#define FAILURE 0
#define IGNORED -1

/**
 * Struttura utilizzata per il logging dei messaggi
 * riunisce informazioni utili in un'unica struct
 * 
 * Campi:
 *  dateTime - Data e ora a cui avviene l'operazione
 *  client - Chi ha richiesto l'operazione
 *  operation - Descrive brevemente l'operazione eseguita
 *  success - Indica successo (SUCCESS) o fallimento (FAILURE) dell'operazione, puo' essere ignorato l'esito (IGNORED)
 *  additionalMsg - Puo' essere specificata per indicare informazioni aggiuntive
 */
typedef struct {
    char *dateTime;
    char *client;
    char *operation;
    short int success; 
    char *additionalMsg;
} logMessage;

/**
 * Formatta il messaggio msg con le informazioni fornite
 * La data viene impostata automaticamente durante l'esecuzione
 * 
 * La funzione permette di preparare in anticipo i dati, anche
 * separatamente, e di creare il messaggio per il logging
 * insieme in un'unica linea
 */
void formatMessage(logMessage *msg, char *_client, char *_operation, short int _success, char *_additionalMsg);

/**
 * Scrive le informazioni contenute nel messaggio msg
 * nel file "log.txt"
 */
void logF(logMessage msg);