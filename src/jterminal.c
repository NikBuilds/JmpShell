#include "jmpShell.h"

/** defines **/

// Macro das k (ein char) bitwise-UND mit 00011111 in binär verknüpft.
// Setzt also die obersten 3 bits von einem char auf 0, das spiegelt was ctrl im terminal macht:
//	- Es entfernt die Bits 5 und 6 von der Taste, die Sie in Kombination mit Strg drücken, und sendet diese. (Die Bitnummerierung beginnt üblicherweise bei 0.)
//	  Der ASCII-Zeichensatz scheint absichtlich so konzipiert zu sein. 
//	  (Er ist auch ähnlich konzipiert, sodass Sie Bit 5 setzen und löschen können, um zwischen Klein- und Großbuchstaben zu wechseln.)
#define CTRL_KEY(k) ((k) & 0x1f)

/** functions **/

void die(const char *s) {
	// Cleart den Screen und positinoiert die Cursor oben Links
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	perror(s);
	exit(1);
}

void disableRawMode() {

	// Setzt Terminal wieder auf orig_termios (So wie vorher) -> Um raw mode beim beenden des Programms zu verlassen
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
		die("tcsetattr");
}

struct termios enableRawMode() 
{
    struct termios orig_termios; 
	// Schreibt akteullen Zustand (Unverändert) des terminals in orig_termios -> Einstellungen von der Standardeingabe (STDIN_FILENO) in orig_termios
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");

	// atexit() wird automatisch beim beenden des Programms aufgerufen
	//atexit(disableRawMode);		
	
	// Unveränderter Terminalzustand in neues termios Struct speichern
	struct termios raw = orig_termios;
	
	// Ctrl-S, Ctrl-Q und Ctrl-M deaktivieren
	// IXON für Ctrl-S und Q kommt von <termios.h> und ist ein input flag
	//		- Standardmäßig werden Strg-S und Strg-Q für die Software-Flusskontrolle verwendet
	//		- Strg-S stoppt die Datenübertragung an das Terminal, bis Sie Strg-Q drücken.
	// ICRNL (für Ctrl-M) auch input flag CR steht für „Carriage Return“ und NL für „New Line“.
	// Jetzt kann Strg-S als 19-Byte und Strg-Q als 17-Byte gelesen werden.
	// Und Ctrl-M als 13 und Enter als 13
	//
	// BRKINT wenn aktiviert, sendet bei einer break condition ein SIGINT was das Programm beendet (Wie Ctrl-C)
	// INPCK ermöglicht die Paritätsprüfung, was für moderne Terminalemulatoren nicht zu gelten scheint.
	// ISTRIP bewirkt, dass das 8. Bit jedes Eingabebytes entfernt wird, d. h. es wird auf 0 gesetzt.
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP |  IXON);

	
	// OPOST kommt aus <termios.h>. = steht für output flag POST für "post-prcessing of output"
	// Deaktiviert Ausgabeverarbeitungsfunktionen da sonst \r\n atuomatisch aktiviert ist und man so nicht selbst entscheiden kann, wann eine \r oder ein \n passieren soll
	raw.c_oflag &= ~(OPOST);
	
	// CS8 ist kein Flag, sondern eine Bitmaske mit mehreren Bits, die wir im Gegensatz zu allen Flags, die wir deaktivieren, mit dem bitweisen ODER-Operator (|) setzen. Es setzt die Zeichengröße (CS) auf 8 Bit pro Byte (Eig. default)
	raw.c_cflag |= (CS8);

	// Echo, canonical mode, Ctrl-C, Ctrl-Z, Ctrl-V und Ctrl-O (MacOS) deaktivieren
	// ECHO, ICANON (canonical mode), ISIG und IEXTEN kommen von <termios.h>, ist also kein input flag obwohl I davor.
	// ISIG:
	//		- Ctrl-C sendet SIGINT signal zum aktuelle Prozess was ihn beendet (Kann jetzt als 4 byte gelesen werden)
	//		- Ctrl-Z sendet SIGTSTP was ihn aussetzt (Kann jetzt als 26 byte gelesen werden)
	// IEXTEN:
	//		- Ctrl-V & Ctrl-O (Mac) durch V und O wartet Terminal bis neuer Input. (V jetzt als 22 byte und O als 15 byte)
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
	
	// VMIN und VTIME stammen aus <termios.h>. 
	// Sie sind Indizes im Feld c_cc, das für „Steuerzeichen / Control characters“ steht, ein Byte-Array, das verschiedene Terminaleinstellungen steuert.
	//		- VMIN: Legt die Mindestanzahl an Bytes fest bevor read() zurückgibt. Auf 0 bedeutet das read() zurückgibt sobald eine zu lesende Eingabe vorliegt.
	//		- VTIME: legt die maximale Wartezeit fest, bevor read(= zurückgibt. Angabe in Zehntelsekunde also 100 Millisekunden.
	// Bei einem Timeout (also wenn nichts eingegeben wird) von read() wird 0 zurückgegeben. Das ist sinnvoll, da der übliche Rückgabewert die Anzahl der gelesenen Bytes ist.
	// Wenn keine Eingabe gemacht wird, wird read() zurückgeben ohne die Variable c zu setzen. Diese behält den Wert 0 bei, sodass Nullen ausgegeben werden.
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	
    // Neuer Zustand auf das Terminal anwenden & error handling
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

// read() ließt genau 1 char in c ein -> angegeben in argument 2.
// Das 3. Argument gibt an, wieviele byts eingelesen werden dürfen.
// read() gibt die Anzahl an bytes zurück, die eingelesen wurden /0 wenn es das ende von einer Datei erreicht (EOF) / -1 bei Fehler.
char shellReadKey() 
{
	int nread;
	char c; 
	while ((nread = read(STDIN_FILENO, &c, 1)) !=1 ) {
		if (nread == -1 && errno != EAGAIN) die("read");
	}
	return c; 
}

void shellProcessKeypress()
{
    char *line = NULL;
    char *tmp = NULL;
    size_t size = 0, index = 0;
    bool fin = false;

    while(!fin) 
    {  
        char c = shellReadKey();
        if  (c == '\r' || c == '\n') {
            printf("ENTER\n");
            fin = true;
        }

        if(c == '\x1b')  { // 27 -> Escapesequenz
            char seq[2];

            read(STDIN_FILENO, &seq[0], 1);
            read(STDIN_FILENO, &seq[1], 1);

            if (seq[0] == '[') {
  	            switch (seq[1]) {
                    case 'A':
                        printf("UP\n");
                        break;
                    case 'B':
                        printf("DOWN\n");
                        break;
                    case 'D':
                        printf("LEFT\n");
                        break;
                    case 'C':
                        printf("RIGHT\n");
                        break;
                    }
                    break; // End while
                }   
            } else { // esle Escapesequenz
                printf("Normal char\n");
                if(size <= index) {
                    size += 1;
                    tmp = realloc(line, size); 

                    if(!tmp) {
                        free(line);
                        line = NULL;
                        fin = true;
                    }
                line = tmp;
            }
            /* Store Chars into string. */
            line[index++] = c;
        }
    }
}


// -----------------------------------------------------------------------------------------------------


char* read_line() 
{
    char *line = NULL;
    char *tmp = NULL;
    size_t size = 0, index = 0;
    int ch = EOF;

    while (ch) {
        ch = getc(stdin);                       // Reads one char from standard input stdin 
        if(ch == EOF || ch == '\n') ch = 0;     // 0 = false so loop stops

        if(size <= index) {
            size += 1;
            tmp = realloc(line, size); 

            if(!tmp) {
                free(line);
                line = NULL;
                break;
            }
            line = tmp;
        }

        /* Store Chars into string. */
        line[index++] = ch;
    }
    return line;
}

// Hier realloc 2d array
Command split_command(char *line) 
{
    Command cmd_struct;
    cmd_struct.command_line = NULL;
    cmd_struct.count = 0;
    char *saveptr;          // Pos for token
    
    char *token = strtok_r(line, " ", &saveptr); // Splits string into tokens unsing " " as delimiter

    while (token != NULL) { // Loops trough the tokens
        char **temp = realloc(cmd_struct.command_line, 
                             (cmd_struct.count + 2) * sizeof(char*)); // +2 because need space for NULL

        if(temp == NULL) {
            perror("Realloc failed");
            break;
        }

        cmd_struct.command_line = temp;

        // Speicher für das Wort selbst reservieren
        cmd_struct.command_line[cmd_struct.count] = malloc(strlen(token) + 1); // + 1 for Nullbyte '\0'
        if (cmd_struct.command_line[cmd_struct.count] == NULL) {
            perror("malloc failed");
            break;
        }

        // Copy token into command_line
        strcpy(cmd_struct.command_line[cmd_struct.count], token);
        cmd_struct.count++;
        cmd_struct.command_line[cmd_struct.count] = NULL; // execvp() needs a NULL terminated array to know when the arguments end

        token = strtok_r(NULL, " ", &saveptr);
    }

    return cmd_struct;
}

void free_command(Command *cmd) 
{
    for (int i = 0; i < cmd->count; i++) {
        free(cmd->command_line[i]);
    }
    free(cmd->command_line);
}