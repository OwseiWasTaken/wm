#include "buffy.h"

#define	Magenta "\x1b[38;2;255;0;255m"
#define Yellow "\x1b[38;2;255;255;0m"
#define	White "\x1b[38;2;255;255;255m"
#define	HOLDMIN 30

enum {
	LClick,
	RClick,
	MClick,
	LDrag,
	RDrag,
	MDrag
};

int talks;
char* inme = Magenta"[MOUSE]"White;
char* outme = Magenta"[MOUSE"Yellow" IPC"Magenta"]"White;
// 1 = master
// 2 = mouse
char ID = 2;

void ASend(char *buff, uint8 bufflen) {
	printf("%s: write msg len: %d\n", outme, bufflen);
	write(talks, &bufflen, 1);
	printf("%s: write whoami: %ld\n", outme, write(talks, &ID, 1));
	printf("%s: write buff: %ld\n", outme, write(talks, buff, bufflen));
	// notify parrent
	printf("%s> sending signal\n", outme);
	kill(getppid(), SIGUSR1);
}

void SendClick(int y, int x) {
	char* msg = malloc(4+4+1); // 4 for y, 4 for x, 1 for info type
	msg[0] = LClick;
	msg[1+0  ] = y<<(8*0)>>(8*3);
	msg[1+1  ] = y<<(8*1)>>(8*3);
	msg[1+2  ] = y<<(8*2)>>(8*3);
	msg[1+3  ] = y<<(8*3)>>(8*3);
	msg[1+4+0] = x<<(8*0)>>(8*3);
	msg[1+4+1] = x<<(8*1)>>(8*3);
	msg[1+4+2] = x<<(8*2)>>(8*3);
	msg[1+4+3] = x<<(8*3)>>(8*3);
	ASend(msg, 9);
}

void SegV(int sig) {
	printf("segmentation fault in Mouse\n");
	kill(getppid(), SIGSEGV);
	CloseFb(GlobalJar);
}

// 8 = released
// 8+2 =
// 8+4 = middle
int main(){
	printf("%s: started\n", inme);
	signal(SIGSEGV, SegV);

	struct fbjar jar = InitBuffy();
	int mice = open("/dev/input/mice", O_RDONLY);
	assert(mice != -1);
	printf("hello, from %s@%s\n", inme, jar.tty);
	printf("%s: reading mouse from fd%d\n", inme, mice);

	talks = open("talks", O_RDWR);
	printf("%s> mouse pipe %d\n", outme, talks);
	//SendClick(15431,5);
	//return 0;

	//polar PointerM = MakePolar(12, 45);
	//color black = RGB(0,0,0);

	point loc = MakePoint(100,100);
	uint8 buff[3] = {0};
	int8 ry, rx;
	int hy = 0, hx = 0;
	uint8 left=0, right=0, middle=0;
	int bytes;
	uint8* draw = GetFbPos(jar, 0, 0);
	long int at = GetPixelPnt(jar, loc);
	char Rem_R = draw[at+0];
	char Rem_G = draw[at+1];
	char Rem_B = draw[at+2];

	while (1) {
		bytes = read(mice, buff, 3);
		if (!bytes) {
			continue;
		}
		// clear non button data
		buff[0] = buff[0]<<5;
		buff[0] = buff[0]>>5;
		if (buff[0] < (left|middle|right)) {
			printf("%s: released @ %d,%d\n", inme, loc.y, loc.x);
			if (abs(hy)+abs(hx) > HOLDMIN) {
				printf("%s: dragged %d,%d -> %d,%d d(%d,%d)\n",
					inme, hy, hx, loc.y, loc.x, loc.y-hy, loc.x-hx);
			}
		}
		left   = (buff[0]&1);
		right  = (buff[0]&2);
		middle = (buff[0]&4);
		if (middle) {
			printf("%d,%d m(%d,%d)\n", loc.y, loc.x, jar.rows, jar.cols);
		}
		if (loc.x < jar.cols) {
			rx = buff[1];
		} else {
			rx = -1;
		}
		if (loc.y < jar.rows) {
			ry = buff[2];
		} else {
			ry = -1;
		}
		if (!(buff[0])) {
			hx = 0;
			hy = 0;
		} else {
			hx+=rx;
			hy+=ry;
		}
		if (!(hx+hy)) {
			if (right) {
				printf("%s: pressed right button @ %d,%d\n", inme, loc.y, loc.x);
			}
			if (left) {
				printf("%s: pressed left button @ %d,%d\n", inme, loc.y, loc.x);
			}
			if (middle) {
				printf("%s: pressed middle button @ %d,%d\n", inme, loc.y, loc.x);
			}
		}
		//if (left) {
		//	Rem_R = 255;
		//	Rem_G = 255;
		//	Rem_B = 255;
		//}
		//swap bitmap
		draw[at+0] = Rem_B;
		draw[at+1] = Rem_G;
		draw[at+2] = Rem_R;

		loc.y-=ry;
		loc.x+=rx;
		at = GetPixelPnt(jar, loc);

		Rem_B = draw[at+0];
		Rem_G = draw[at+1];
		Rem_R = draw[at+2];
		draw[at+0] = 120;
		draw[at+1] = 120;
		draw[at+2] = 255;
		//swap bitmap
	}
}
