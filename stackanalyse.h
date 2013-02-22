void stackanalyse(unsigned long size);

typedef struct {
	unsigned long adress;
	unsigned long size;
	char name[256];

} FUNC;

extern FUNC function[];
extern unsigned short funccnt,globalram;
extern unsigned short program[];




