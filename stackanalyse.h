#ifndef STACKANALYSE_H__
#define STACKANALYSE_H__

void stackanalyse(unsigned long size, char const* start_func);

typedef struct {
	unsigned long adress;
	unsigned long size;
	char name[256];

} FUNC;

extern FUNC function[];
extern unsigned short funccnt,globalram;
extern unsigned short program[];


#endif // STACKANALYSE_H__


