size = recv(filedescriptor, buffer, BUF-1, 0);
	if(size >0) buffer[size] = '\0';