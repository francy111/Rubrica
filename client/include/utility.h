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

#define CONTACT_STRINGS_LENGTH 11
#define AUTH_STRING_LENGTH 21

typedef struct {
    char name[CONTACT_STRINGS_LENGTH];
    char surname[CONTACT_STRINGS_LENGTH];
    char phoneNumber[CONTACT_STRINGS_LENGTH];
} Contact;

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
long getOptionalInput(char *string,size_t size);
/**
 * Acquisisce un singolo carattere dallo standard input (stdin)
 * e lo restituisce
 * 
 * Restituisce il carattere letto se era stato inserito un singolo carattere
 * altrimenti restituisce EOF
 */
char getSingleChar(void);
/**
 * Procedura che apre un menu per l'acquisizione del contatto
 * fa in modo che il contatto sia interrito corretto
 * 
 * title - Specifica il titolo del menu
 * 
 * Il risultato viene salvato in contact
 */
void getNotEmptyContact(Contact *contact, char* title);
/**
 * Procedura che apre un menu per l'acquisizione del contatto
 * fa in modo che il contatto sia interrito corretto
 * 
 * Variante di getNotEmptyContact che permette
 * di lasciare vuoti i campi
 * 
 * title - Specifica il titolo del menu
 * 
 * Il risultato viene salvato in contact
 */
void getOptionalContact(Contact *contact, char* title);



/**
 * Controlla se il nome name non sia piu' lungo della lunghezza massima
 */
int isNameValid(char *name);
/**
 * Controlla se il cognome surname non sia piu' lungo della lunghezza massima
 */
int isSurnameValid(char *surname);
/**
 * Controlla se il numero di telefono e' valido
 * questo e' il caso in due situazioni:
 *  - Il numero di telefono e' composto da 10 caratteri
 *  - Il numero di telefono e' lasciato vuoto (0 caratteri)
 */
int isPhoneNumberValid(char *phoneNumber);
/**
 * Controlla che il nome non sia piu' lungo della lunghezza massima e contenga almeno un carattere
 */
int isNameValidAndNotEmpty(char *name);
/**
 * Controlla che il cognome non sia piu' lungo della lunghezza massima e contenga almeno un carattere
 */
int isSurnameValidAndNotEmpty(char *surname);
/**
 * Controlla che il numero di telefono e' valido questo e' il caso solamente se e' composto da 10 caratteri
 */
int isPhoneNumberValidAndNotEmpty(char *phoneNumber);
/**
 * Controlla se l'username fornito e' valido o meno
 * E' valido solo se non contiene virgole ed e' compreso
 * tra 1 e la lunghezza massima
 */
int isUsernameValidAndNotEmpty(char *username);
/**
 * Controlla se la password fornita e' valida o meno
 * E' valida solo se non contiene virgole ed e' minore
 * o uguale alla lunghezza massima
 */
int isPasswordValid(char *password);
/**
 * Controlla se il carattere (c) e' una cifra decimale (da '0' a '9')
 */
int isDigit(char c);
/**
 * Controlla se il carattere (c) e' una lettera (da 'a/A' a 'z/Z')
 */
int isLetter(char c);


/**
 * Stampa il messaggio message in maniera formattata
 * aggiungendo delle linee (===) sia sopra che sotto
 */
void printTitle(const char *message);
/**
 * Stampa un messaggio seguito da 3 newline e gli applica il colore color
 */
void printCommunication(const char* message, const char* color);
/**
 * Stampa un contatto in maniera formattata, indicando
 * la corrispondenza (matchIndex) fornita
 */
void printContactIndex(Contact contact, int matchIndex);
/**
 * Stampa un contatto in maniera formattata
 */
void printContact(Contact contact);



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
