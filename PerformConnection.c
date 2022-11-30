#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>


#define BUF 1024

int performConnection(int filedescriptor, char* gameID){

	size_t size;
	char *buffer = (char*) malloc (sizeof(char) * BUF) ;
	char *gameVersionmsg = "VERSION 2.1\n";
	

	//Empfange erste Nachricht vom Server
	size = recv(filedescriptor, buffer, BUF-1, 0);
	
	if(size > 0) buffer[size] = '\0';
	//TODO: Formatierte Ausgabe der ersten Servernachricht GameServer Version
	

	if(buffer[0] == '-'){
		//TODO: Fehlerbehandlung 

	}

		//Version des Clients abschicken
		strcpy(buffer, gameVersionmsg); 
		send(filedescriptor, buffer, strlen(buffer), 0);	


		//Empfange zweite Nachricht vom Server
		size = recv(filedescriptor, buffer, BUF-1, 0);
		if(size > 0) buffer[size] = '\0';
		//TODO: Formatierte Ausgabe der zweiten Servernachricht GAME ID

	if(buffer[0] == '-'){
		//TODO: Fehlerbehandlung falsche CLIENT Version

	}
			
		strcpy(buffer, "ID ");
		strcat(buffer, gameID); 
		//Version des Clients abschicken
		send(filedescriptor, buffer, strlen(buffer), 0);	

		//Empfange dritte Nachricht vom Server
		size = recv(filedescriptor, buffer, BUF-1, 0);
		if(size > 0) buffer [size] = '\0';
		//TODO: Formatierte Ausgabe der dritten Servernachricht Gamekind-Name
		if (buffer[0] == '-'){
			//TODO: Fehlerbehandlung Game ID Fehler
		}
		if (strcmp(buffer,"+ PLAYING NMMorris\n") != 0){
			//TODO: Fehlerbehandlung Server erwartet falsches Spiel
		}

		//Empfange vierte Nachricht vom Server
	size = recv(filedescriptor, buffer, BUF-1, 0);
		if(size > 0) buffer [size] = '\0';
		//TODO: Formatierte Ausgabe der vierten Servernachricht Game-Name
		if (buffer[0] == '-'){
			//TODO: Fehlerbehandlung (?)
		}

		//Sende Mitspielernummer an Server 
		strcpy(buffer, "PLAYER\n");
		send(filedescriptor, buffer, strlen(buffer), 0);


		//Empfange fünfte Nachricht vom Server Spielerinfo
	size = recv(filedescriptor, buffer, BUF-1, 0);
		if(size > 0) buffer [size] = '\0';
		//TODO: Formatierte Ausgabe der fünften Servernachricht Spielerinfoanzeige
		if (buffer[0] == '-'){
			//TODO: Fehlerbehandlung Mitspielernummer
		}

		//Empfange sechste Nachricht vom Server Mitspieleranzahl
	size = recv(filedescriptor, buffer, BUF-1, 0);
		if(size > 0) buffer [size] = '\0';
		//TODO: Formatierte Ausgabe der fünften Servernachricht Mitspieleranzahl
		if (buffer[0] == '-'){
			//TODO: Fehlerbehandlung Spielerinfoanzeige
		}

		//Empfange siebte Nachricht vom Server Mitspieler BEREIT
	size = recv(filedescriptor, buffer, BUF-1, 0);
		if(size > 0) buffer [size] = '\0';
		//TODO: Formatierte Ausgabe der fünften Servernachricht Spielerinfoanzeige
		if (buffer[0] == '-'){
			//TODO: Fehlerbehandlung Mitspieleranzahl
		}

		//Empfange achte Nachricht vom Server ENDPLAYERS
	size = recv(filedescriptor, buffer, BUF-1, 0);
		if(size > 0) buffer [size] = '\0';
		//TODO: Formatierte Ausgabe der achten Servernachricht Spielerinfoanzeige
		if (buffer[0] == '-'){
			//TODO: Fehlerbehandlung Mitspielerinfo
		}
}





