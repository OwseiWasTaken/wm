#include "buffy.h"

#define Blue "\x1b[38;2;0;0;255m"
#define Yellow "\x1b[38;2;255;255;0m"
#define White "\x1b[38;2;255;255;255m"

typedef struct {
	uint8 from;
	char* cont;
	uint8 len;
} msg;

char* inme = Blue"[MAIN]"White;
char* outme = Blue"[MAIN"Yellow" IPC"Blue"]"White;
int talks;
char ID = 1;
char *Devices[] = {
	NULL,
	"Master",
	"Mouse"
};

void Send(char *buff) {
	int bufflen = strlen(buff);
	write(talks, buff, bufflen+1);
	write(talks, &ID, 1);
}

char *UpdateBuff(char *obuff, char *nbuff, int olen) {
	for (int i = 0; i<olen; i++) {
		nbuff[i] = obuff[i];
	}
	return nbuff;
}

msg Receive() {
	char *sbufflen = malloc(1);
	read(talks, sbufflen, 1); // read from\0
	int bufflen = (uint8)*sbufflen;
	printf("%s: got len: %d\n", outme, bufflen);
	char *buff = malloc(bufflen);
	uint8 from[2];
	msg cmd;

	read(talks, from, 1); // read from\0
	printf("%s: got sender: %s\n", outme, Devices[from[0]]);

	read(talks, buff, bufflen);
	cmd.cont = buff;
	cmd.from = from[0];
	cmd.len = bufflen;
	return cmd;
}

void act(msg cmd) {
	printf("%s> reading msg {", outme);
	int i;
	for (i = 0; i<cmd.len-1; i++) {
		printf("%d, ", cmd.cont[i]);
	}
	printf("%d", cmd.cont[i]);
	printf("} from %s\n", Devices[cmd.from]);
}

void SigUser1(int sig) {
	printf("%s> recieved sigu1\n", outme);;
	act(Receive());
}

int main(void) {
	printf("%s: started\n", inme);

	signal(SIGUSR1, SigUser1);
	struct fbjar jar = InitBuffy();

	printf("hello, from %s@%s\n", inme, jar.tty);
	talks = open("talks", O_RDWR);
	printf("%s> reading pipe %d\n", outme, talks);

	pid_t mouse = fork();
	if (mouse < 0) {
		fprintf(stderr, "%s: can't open fork\n", inme);
		return 1;
	} else if (mouse == 0) {
		printf("%s: started Mouse proc\n", inme);
		printf("%s: Rememmap %s to Mouse exe\n\n", inme, inme);
		execl("./utils/mouse/out", "", (char*)NULL);
	} else {
		// wait for messages
		while (1) {
			//printf("%s: start sleep\n", inme);
			sleep(2);
			//printf("%s: sleep done\n", inme);
		}
	}
}
