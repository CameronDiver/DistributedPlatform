#include <stdlib.h>
#include <string.h>
#include <sys.h>

#define LINESIZE (64*1024)

void shellRun(int fdIn, int fdOut, int fdErr);
void shellParseLine(int fdIn, int fdOut, int fdErr, char *line);
void shellCd(char *save);
void shellExport(char *save);
void shellPathReduce(char *path); // Remove redundant parts of a path such as repeated slashes or dir/../.
char *shellPathDisplay(const char *path); // Calls shellPathReduce and also replaces users home directory with ~ (if env. var. HOME is set).
char *shellPathAbsolute(const char *path); // Allocates new string with malloc (can be longer due to e.g. replacing ~ with full path). Calls shellPathReduce before returning.
void shellRunProgram(const char *path);

int main(int argc, char **argv) {
	// Parse any files passed as arguments.
	unsigned int i;
	for(i=1;i<argc;++i) {
		int fd=open(argv[i], O_RDWR, 0);
		if (fd==-1)
			continue;

		// TODO: this
//		shellRun(fd, 2, 3);
		shellRun(fd, fd, -1);

		close(fd);
	}

	// Now parse from stdin.
	// TODO: this
//	shellRun(0, 1, 2);

	return EXIT_SUCCESS;
}

void shellRun(int fdIn, int fdOut, int fdErr) {
	char line[LINESIZE];

	// Set current working directory environment variable.
	if (getcwd(line, LINESIZE)!=NULL)
		setenv("PWD", line, 1);

	// Input loop.
	while(1) {
		// Send prompt.
		if (fdOut!=-1) {
			// TODO: Check return from writes.

			char *str=shellPathDisplay(getcwd(line, LINESIZE));
			if (str!=NULL)
				write(fdOut, str, strlen(str));
			write(fdOut, " $ ", 3);
			free(str);
		}

		// Grab line.
		int count=read(fdIn, line, LINESIZE-1);
		if (count<=0)
			break;
		line[count]='\0';

		// Parse line.
		shellParseLine(fdIn, fdOut, fdErr, line);
	}
}

void shellParseLine(int fdIn, int fdOut, int fdErr, char *line) {
	// Strip comment.
	char *hash=strchr(line, '#'); // TODO: Allow spaces (either escaped or quoted).
	if (hash!=NULL)
		*hash='\0';

	// Trim newline.
	size_t lineLen=strlen(line);
	if (line[lineLen-1]=='\n')
		line[lineLen-1]='\0';

	// Empty line?
	// TODO: really a hack, should remove excess white space too?
	if (strlen(line)==0)
		return;

	// Find command.
	char *save=NULL;
	char *part=strtok_r(line, " ", &save); // TODO: Allow spaces (either escaped or quoted).
	if (part==NULL)
		return;
	if (!strcmp(part, "cd"))
		shellCd(save);
	else if (!strcmp(part, "export"))
		shellExport(save);
	else {
		// TODO: Check for part being a path.

		if (fdOut!=-1) {
			// TOOD: Check return of both writes.
			write(fdOut, line, strlen(line));
			write(fdOut, ": Command not found.\n", strlen(": Command not found.\n"));
		}
	}
}

void shellCd(char *save) {
	// Grab path.
	// TODO: Allow spaces (either escaped or quoted).
	char *gpath=strtok_r(NULL, " ", &save);

	// Resolve path to absolute
	char *path=shellPathAbsolute(gpath);
	if (path==NULL)
		return;

	// Call chdir.
	if (chdir(path)!=0) {
		free(path);
		return;
	}

	// Update current working directory environment variable.
	char line[LINESIZE];
	if (getcwd(line, LINESIZE)!=NULL)
		setenv("PWD", line, 1);

	// Free path.
	free(path);
}

void shellExport(char *save) {
	// Grab string.
	// TODO: Allow spaces (either escaped or quoted).
	char *string=strtok_r(NULL, " ", &save);

	char *equals=strchr(string, '=');
	if (equals==NULL)
		return;

	*equals='\0';
	setenv(string, equals+1, 1);
}

void shellPathReduce(char *path) {
	size_t len=strlen(path), oldLen;
	do {
		oldLen=len;

		// Replace multiple slashes with single ones.
		char *s, *t, *u;
		for(s=path;*s!='\0';++s) {
			for(t=s;*t=='/';++t) ;
			--t;
			if (t<s)
				continue;
			u=s;
			while(*t!='\0')
				*u++=*t++;
			*u='\0';
		}

		// Simplify dir/../.
		// TODO: this

		// Ends with a slash (and does not consist of only a slash).
		size_t pathLen=strlen(path);
		if (strcmp(path, "/") && path[pathLen-1]=='/')
			path[pathLen-1]='\0';
	} while((len=strlen(path))!=oldLen);
}

char *shellPathDisplay(const char *path) {
	// Find absolute (which also reduces).
	char *str=shellPathAbsolute(path);
	if (str==NULL)
		return NULL;

	// Now if within home directory, replace with ~.
	const char *home=getenv("HOME");
	if (home==NULL)
		return str;

	size_t homeLen=strlen(home);
	if (!strncmp(path, home, homeLen)) {
		size_t strLen=strlen(str);
		char *temp=realloc(str, strLen+2);
		if (temp==NULL) {
			free(str);
			return NULL;
		}
		memmove(str+2, str+homeLen, strLen-homeLen+1);
		str[0]='~';
		str[1]='/';
	}

	// Reduce again.
	shellPathReduce(str);

	return str;
}

char *shellPathAbsolute(const char *path) {
	if (path==NULL)
		return NULL;

	// Replace initial ~ with home directory if defined.
	const char *home=NULL;
	if (path[0]=='~') {
		home=getenv("HOME");
		if (home==NULL)
			return NULL;
		++path;
	} else if (!strncmp(path, "./", 2)) {
		//	TODO: this (use getcwd not PWD var.)
	}

	// Allocate memory.
	size_t homeLen=(home!=NULL ? strlen(home)+1 : 0); // +1 for extra /.
	size_t pathLen=strlen(path);
	char *str=malloc(homeLen+pathLen+1);
	if (str==NULL)
		return NULL;

	// Copy strings.
	memcpy(str, home, homeLen);
	if (home!=NULL) str[homeLen-1]='/';
	memcpy(str+homeLen, path, pathLen+1);

	// Reduce.
	shellPathReduce(str);

	return str;
}