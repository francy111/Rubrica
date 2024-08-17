#include <stdlib.h>
#define CONTACT_STRINGS_LENGTH 10
#define AUTH_STRINGS_LENGTH 20
#define HASH_LENGTH 20
// Costanti per i colori nelle printf()
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define RED "\x1b[31m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define GREY "\x1b[90m"
#define RESET_COLOR "\x1b[0m"

#define CLEAR "\x1B[3J\x1B[H\x1B[2J"

#define UNDERLINE "\x1B[4m"
#define BOLD "\x1B[1m"
#define WHITEBG "\e[47m"

#define BWHITE "\e[1;37m"
#define BRED "\e[1;31m"
#define BGREEN "\e[1;32m"
#define BYELLOW "\e[1;33m"
#define BMAGENTA "\e[1;35m"
#define BCYAN "\e[1;36m"

/**
 * Rappresenta un contatto della rubrica
 * Campi:
 *  name
 *  surname
 *  phoneNumber
 * 
 * Permette una gestione di queste 3 informazioni
 * in modo compatto e piu' organizzata, specialmente
 * per quanto riguarda il passaggio di parametri
 */
typedef struct {
    char name[CONTACT_STRINGS_LENGTH + 1];
    char surname[CONTACT_STRINGS_LENGTH + 1];
    char phoneNumber[CONTACT_STRINGS_LENGTH + 1];
} Contact;

/**
 * Restituisce l'n-esimo contatto salvato nella rubrica
 * 
 * cntc - Struttura che conterra' le informazioni del contatto trovato
 * index - Indice nella rubrica, indica quale contatto vogliamo recuperare
 * 
 * Restituisce 1 se viene trovato il contatto, 0 altrimenti (es. indice piu' grande del numero di contatti)
 */
int getContact(Contact *cntc, int index);

/**
 * Aggiunge il contatto salvato in cntc nella rubrica
 * se non è gia presente, in quanto non si ammettono duplicati
 * 
 * cntc - Contatto da aggiungere alla rubrica
 * 
 * Restituisce
 *  0 - Contatto non aggiunto per errore su file
 *  1 - Contatto aggiunto correttamente
 *  2 - Contatto non aggiunto perchè gia' presente
 */
int addContact(Contact cntc);

/**
 * Rimuove il contatto salvato in cntc dalla rubrica
 * 
 * cntc - Contatto da rimuovere dalla rubrica
 * 
 * Restituisce
 *  0 - Contatto non rimosso per errore su file
 *  1 - Contatto rimosso correttamente
 *  2 - Contatto non rimosso perchè non presente
 */
int removeContact(Contact cntc);

/**
 * Modifica il contatto della rubrica salvato in old
 * aggiornando le sue informazioni con quelle salvate in new
 * 
 * old - Contatto da modificare
 * new - Nuove informazioni del contatto
 * 
 * Restituisce
 *  0 - Contatto non modificato per errore su file
 *  1 - Contatto modificato correttamente
 *  2 - Contatto non modificato perchè non presente
 */
int modifyContact(Contact old, Contact new);

/**
 * Controlla se le credenziali fornite corrispondono 
 * a quelle di un utente esistente e salvato sul server
 * 
 * Restituisce 1 se sono presenti (e quindi valide) o 0 in caso contrario
 */
int checkCredentials(char *username, char *password);

/**
 * Inizializza un contatto 'svuotandolo'
 * ottenendo cosi' un contatto con tutti i campi vuoti
 * 
 * cntc - Struttura contatto da inizializzare
 */
void createEmptyContact(Contact *cntc);

/**
 * Controlla se il contatto ha i campi vuoti
 * secondo i criteri sulle stringhe in C
 * 
 * Ovvero una stringa è "vuota" se ha lunghezza 0 
 * (ha come primo carattere il terminatore \0)
 * 
 * Restituisce 1 in tal caso, 0 altrimenti
 * 
 * N.B. Chiamare prima createEmptyContact e poi isContactEmpty risultera' sempre vero,
 * mentre se isContactEmpty restituisce vero, non significa per forza che il contenuto
 * di cntc sia quello che genererebbe createEmptyContact
 */
int isContactEmpty(Contact cntc);

/**
 * Controlla se i campi del contatto asked coincidono con quelli
 * del contatto found
 * 
 * asked - Primo dei contatti da controllare
 * found - Secondo dei contatti da controllare
 * 
 * Restituisce 1 se i parametri specificati di asked coincidono con quelli di found
 */
int matchesParameters(Contact asked, Contact found);

/**
 * Legge una linea dal file con descriptor fd e la salva nel buffer buf
 * 
 * fd - File descriptor da cui leggere
 * buf - Buffer in cui salvare la linea letta
 * 
 * Restituisce il numero di bytes letti, in caso
 * di errore sul fd restituisce -1
 */
int readLine(int fd, char *buf);

/**
 * Esegue l'hashing della stringa toHash e salva la 
 * rappresentazione a stringa del risultato nella stringa hash
 * 
 * toHash - Stringa della quale vogliamo l'hash
 * hash - Hash della stringa toHash 
 */
void hashFunction(char *toHash, char *hash);

/**
 * Aggiunge un utente con le credenziali fornite, se non 
 * era gia' presente, in quanto non ci possono essere 2
 * utenti con lo stesso username
 * 
 * Password puo' essere non specificata, questo
 * crea un utente senza password
 *
 * Restituisce 1 se è stato aggiunto, 0 altrimenti
 */
int addUser(char *username, char *password);

/**
 * Rimuove dal sistema l'utente che ha come username
 * quello indicato, se è presente
 * 
 * Restituisce 1 se è stato rimosso, 0 altrimenti
 */
int removeUser(char *username);

/**
 * Svuota il buffer di ingresso (stdin) da tutti
 * gli eventuali caratteri rimasti all'interno
 */
void cleanInputBuffer(void);

/**
 * Acquisisce una stringa di ingresso dallo standard input (stdin)
 * e lo salva in string, con una dimensione massima di size
 * 
 * Permette di inserire come input una stringa vuota (sia \n che spazi)
 * a differenza per esempio della scanf()
 * 
 * Restituisce il numero di caratteri letti
 */
long getOptionalInput(char* string, size_t size);

/**
 * Acquisisce un singolo carattere dallo standard input (stdin)
 * e lo restituisce
 * 
 * Restituisce il carattere letto se era stato inserito un singolo carattere
 * altrimenti restituisce EOF
 */
char getSingleChar(void);

/**
 * Disabilita l'ECHO nella shell, rendendo invisibile
 * la scrittura su tastiera
 */
void disableEcho(void);

/**
 * Abilita l'ECHO nella shell, rendendo visibile
 * la scrittura su tastiera
 */
void enableEcho(void);

/**
 * Controlla se l'username fornito è valido o meno
 * è valido solo se non contiene virgole ed è compreso
 * tra 1 e 20 caratteri
 */
int isUsernameValidAndNotEmpty(char *username);

/**
 * Controlla se la password fornita è valida o meno
 * è valida solo se non contiene virgole ed è minore
 * o uguale a 20 caratteri
 */
int isPasswordValid(char *password);

/**
 * Controlla se il carattere (c) è una cifra decimale (da '0' a '9')
 */
int isDigit(char c);

/**
 * Controlla se il carattere (c) è una lettera (da 'a/A' a 'z/Z')
 */
int isLetter(char c);
