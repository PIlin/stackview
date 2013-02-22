#include <cstdio>
#include <cstring>
#include <string>


#ifdef WIN32
# include <windows.h>
#endif

#include "stackanalyse.h"

FUNC function[4096];
unsigned short funccnt;
unsigned short program[256*1024];
unsigned short level,totstack,maxstack,maxlevel,globalram, warnings, errors, intnest,nestmaxstack, nestaddstack, pcstack;
unsigned long flashsize=128;
bool isint,nested;
FILE *fp;

int getfuncname(unsigned long adress);
void searchprog(unsigned long adress, unsigned short size);
unsigned long searchstart(void);
void analyseint(void);

//#define DEBUG 0

void stackanalyse(unsigned long size)
{	unsigned long startadress;

	fp=fopen("calltree.txt","w");
#ifdef DEBUG
	for (int i=0; i<funccnt; i++)
	{	fprintf(fp,"%-32s %6d %6d\n", function[i].name, function[i].adress, function[i].size);	
	}
	fprintf(fp,"\n\n");
#endif

	while (flashsize<size)
		flashsize<<=1;

	if (flashsize<65536)
		pcstack=2;
	else
		pcstack=3;

	printf("flash usage (words):             %5d\n",size);
	printf("global RAM usage (bytes):        %5d\n",globalram);
	printf("\n");

	level=0;
	maxstack=0;
	maxlevel=0;
	startadress=searchstart();
	if (startadress)
	{	int pos,j;
		fprintf(fp,"Functions:\n\n");
		pos=getfuncname(startadress);
		if (pos)
		{	fprintf(fp,"function: %-40s", function[pos].name);
			for (j=0; j<40-2*level; j++)
				fprintf(fp," ");
			fprintf(fp,"size: %d\n",function[pos].size);
			isint=false;
			searchprog(function[pos].adress,function[pos].size);
			if (totstack)
			{	printf("Stack calculation error!\n");
				fprintf(fp,"Stack calculation error!\n");
				errors++;
				totstack=0;
			}
		}
		else
		{
#ifdef DEBUG
			printf("no function!\n");
#endif
			errors++;
			fprintf(fp,"unknown function: %6d\n", startadress);
		}
		unsigned short functotstack,funcmaxstack,funcmaxlevel,intmaxstack,inttotstack,intmaxlevel;
		functotstack=totstack;
		funcmaxstack=maxstack;
		funcmaxlevel=maxlevel;
		level=0;
		maxstack=0;
		maxlevel=0;
		totstack=0;
		
		fprintf(fp,"\n\nInterrupts:\n\n");
		analyseint();
		inttotstack=totstack;
		intmaxstack=maxstack;
		intmaxlevel=maxlevel;

		totstack=inttotstack+functotstack;
		maxstack=intmaxstack+funcmaxstack+nestaddstack;
		maxlevel=intmaxlevel+funcmaxlevel;

		printf ("\nAnalysis complete: %d errors, %d warnings\n\n",errors, warnings);
		printf ("maximum func. subroutine levels:   %3d\n",funcmaxlevel);
		printf ("maximum func. stack (bytes):     %5d\n",funcmaxstack);
		printf ("maximum int. subroutine levels:    %3d\n",intmaxlevel);
		printf ("maximum int. stack (bytes):      %5d\n",intmaxstack);
		printf ("nested interrupts:                 %3d\n",intnest);
		printf ("maximum nest. int. stack (bytes):%5d\n",nestaddstack);
		printf ("\n");
		printf ("maximum total subroutine levels:   %3d\n",maxlevel);
		printf ("maximum total stack (bytes):     %5d\n",maxstack);
		printf ("maximum total RAM usage (bytes): %5d\n",maxstack+globalram);

		fprintf (fp,"\n\n\n");
		fprintf (fp,"Analysis complete: %d errors, %d warnings\n\n",errors, warnings);
		fprintf (fp,"flash usage (words):             %5d\n",size);
		fprintf (fp,"global RAM usage (bytes):        %5d\n",globalram);
		fprintf (fp,"\n");
		fprintf (fp,"maximum func. subroutine levels:   %3d\n",funcmaxlevel);
		fprintf (fp,"maximum func. stack (bytes):     %5d\n",funcmaxstack);
		fprintf (fp,"maximum int. subroutine levels:    %3d\n",intmaxlevel);
		fprintf (fp,"maximum int. stack (bytes):      %5d\n",intmaxstack);
		fprintf (fp,"nested interrupts:                 %3d\n",intnest);
		fprintf (fp,"maximum nest. int. stack (bytes):%5d\n",nestaddstack);
		fprintf (fp,"\n");
		fprintf (fp,"maximum total stack (bytes):     %5d\n",maxstack);
		fprintf (fp,"maximum total subroutine levels:   %3d\n",maxlevel);
		fprintf (fp,"maximum total RAM usage (bytes): %5d\n",maxstack+globalram);
	}
	else
	{	printf("Error: unable to find starting adress!\n");
		fprintf(fp,"Error: unable to find starting adress!\n");
	}
	fclose(fp);
}

unsigned long searchstart(void)					// search main call
{/*	signed long newadress, adress;
	if ((program[0]&0xF000)==0xC000)			// rjmp
	{	newadress=(program[0]&0x0FFF);
		if (newadress&2048)
			newadress=newadress-4096;
		newadress++;
		if (newadress<0)
			newadress+=flashsize;
		else if (newadress>=flashsize)
			newadress-=flashsize;
	}
	else if ((program[0]&0xFE0E)==0x940C)		// jmp
	{	newadress=(program[0]&0x01F0)*8192;
		newadress|=(program[0]&0x0001)*65536;
		newadress|=program[1];
	}
	else
	{	printf("Invalid Reset Vector!\n");
		errors++;
		return 0;
	}
	adress=newadress;
#ifdef DEBUG
	printf ("reset vector: 0x%04X\n",adress);
#endif
	for (;adress<flashsize;adress++)
	{	if ((program[adress]&0xF000)==0xD000)	// rcall
		{
#if (DEBUG>0)
			printf ("rcall: 0x%04X\n",program[adress]);
#endif
			newadress=(program[adress]&0x0FFF);
			if (newadress&2048)
				newadress=newadress-4096;
			newadress+=adress+1;
			if (newadress<0)
				newadress+=flashsize;
			else if (newadress>=flashsize)
				newadress-=flashsize;
			return newadress;
		}
		else if ((program[adress]&0xFE0E)==0x940E)	// call
		{	newadress=(program[adress]&0x01F0)*8192;
			newadress|=(program[adress]&0x0001)*65536;
			newadress|=program[adress+1];
			return newadress;
		}
	}*/

	int i;
	for (i=0; i<funccnt; i++)
	{	if (!(strcmp(function[i].name,"main")))
			return function[i].adress;
	}
	return 0;
}

unsigned long searchfirstfunc(void)
{	int i;
	unsigned long minadress=0xFFFFFFFF;
	for (i=0; i<funccnt; i++)
	{	if ((function[i].adress<minadress)&&(function[i].adress))
			minadress=function[i].adress;
	}
	return minadress;
}

void analyseint(void)
{	int j, pos;
	signed long newadress;
	unsigned long adress, endvect;
	endvect=searchfirstfunc();
#ifdef DEBUG
	printf ("Int vector size: %d\n",endvect);
#endif
	isint=true;
	for (adress=0;adress<endvect;adress++)			// test interrupt table
	{	nested=false;
		if ((program[adress]&0xF000)==0xC000)		// rjmp
		{	newadress=(program[adress]&0x0FFF);
			if (newadress&2048)
				newadress=newadress-4096;
			newadress+=adress+1;
			if (newadress<0)
				newadress+=flashsize;
			else if (newadress>=flashsize)
				newadress-=flashsize;
#ifdef DEBUG
			printf ("Int vector: %d\n",newadress);
#endif
			if (adress)
			{	pos=getfuncname(newadress);
				if (pos)
				{	for (j=0; j<2*level; j++)
						fprintf(fp," ");
					fprintf(fp,"function: %-80s", function[pos].name);
					fprintf(fp,"size: %d\n",function[pos].size);
					searchprog(function[pos].adress,function[pos].size);
				}
				else
				{
#ifdef DEBUG
					printf("no function!\n");
#endif
					errors++;
					fprintf(fp,"unknown function: %6d 0x%04X\n", newadress,program[adress]);
				}
			}
			if (totstack)
			{	printf("Stack calculation error!\n");
				fprintf(fp,"Stack calculation error!\n");
				errors++;
				totstack=0;
			}
		}
		else if ((program[adress]&0xFE0E)==0x940C)	// jmp
		{	newadress=(program[adress]&0x01F0)*8192;
			newadress|=(program[adress]&0x0001)*65536;
			newadress|=program[adress+1];
			adress++;
#ifdef DEBUG
			printf ("Int vector: %d\n",newadress);
#endif
			if (adress)
			{	pos=getfuncname(newadress);
				if (pos)
				{	for (j=0; j<2*level; j++)
						fprintf(fp," ");
					fprintf(fp,"function: %-80s", function[pos].name);
					fprintf(fp,"size: %d\n",function[pos].size);
					searchprog(function[pos].adress,function[pos].size);
				}
				else
				{
#ifdef DEBUG
					printf("no function!\n");
#endif
					errors++;
					fprintf(fp,"unknown function: %6d 0x%04X\n", newadress,program[adress]);
				}
			}
			if (totstack)
			{	printf("Stack calculation error!\n");
				fprintf(fp,"Stack calculation error!\n");
				errors++;
				totstack=0;
			}
		}
		else
		{	printf("Invalid vector jump!\n");
			fprintf(fp, "Invalid vector jump (0x%04X, 0x%04X)!\n",adress,program[adress]);
			errors++;
		}
		if (nestmaxstack)
		{	nestaddstack+=nestmaxstack;
			nestmaxstack=0;
		}
	}
}

void searchprog(unsigned long adress, unsigned short size)	// searches a function for calls and stack commands
{	int i, j, pos;
	signed long newadress;
	unsigned short locstack;
	locstack=pcstack;
	totstack+=locstack;
	level++;
	if (level>maxlevel)
		maxlevel=level;
	for (i=adress; i<adress+size; i++)
	{	if (program[i]==0xD000)					// rcall 0 (handled as push)
		{	locstack+=2;
			totstack+=2;
			if (totstack>maxstack)
				maxstack=totstack;
		}		
		else if ((program[i]&0xF000)==0xD000)	// rcall
		{
#if (DEBUG>0)
			printf ("rcall: 0x%04X\n",program[i]);
#endif
			newadress=(program[i]&0x0FFF);
			if (newadress&2048)
			{	newadress=newadress-4096;
			}
			newadress+=i+1;
			if (newadress<0)
				newadress+=flashsize;
			else if (newadress>=flashsize)
				newadress-=flashsize;
			pos=getfuncname(newadress);
			if (pos)
			{	for (j=0; j<2*level; j++)
					fprintf(fp," ");
				fprintf(fp,"function: %-40s", function[pos].name);
				for (j=0; j<40-2*level; j++)
					fprintf(fp," ");
				fprintf(fp,"size: %d\n",function[pos].size);
				if (newadress==adress)
				{	level++;
					if (level>maxlevel)
						maxlevel=level;
					locstack*=2;
					totstack+=locstack;
					if (totstack>maxstack)
						maxstack=totstack;
#ifdef DEBUG
					printf("Warning: recursive function found!\n");
#endif
					fprintf(fp,"Warning: recursive function found!                                                        local stack: %5d, total stack: %5d\n",pcstack, totstack);
					for (j=0; j<2*level-2; j++)
						fprintf(fp," ");
					fprintf(fp,"end function");
					for (j=0; j<80-2*level; j++)
						fprintf(fp," ");
					fprintf(fp,"local stack: %5d, total stack: %5d\n",locstack, totstack);
					totstack-=locstack;
					locstack/=2;
					level--;
					warnings++;
				}
				else
					searchprog(function[pos].adress,function[pos].size);
			}
			else
			{
#ifdef DEBUG
				printf("no function!\n");
#endif
				errors++;
				fprintf(fp,"unknown function: %6d 0x%04X\n", newadress,program[i]);
			}
		}
		else if ((program[i]&0xFE0E)==0x940E)	// call
		{	newadress=(program[i]&0x01F0)*8192;
			newadress|=(program[i]&0x0001)*65536;
			newadress|=program[i+1];
			i++;
			pos=getfuncname(newadress);
			if (pos)
			{	for (j=0; j<2*level; j++)
					fprintf(fp," ");
				fprintf(fp,"function: %-40s", function[pos].name);
				for (j=0; j<40-2*level; j++)
					fprintf(fp," ");
				fprintf(fp,"size: %d\n",function[pos].size);
				if (newadress==adress)
				{	level++;
					if (level>maxlevel)
						maxlevel=level;
					locstack*=2;
					totstack+=locstack;
					if (totstack>maxstack)
						maxstack=totstack;
#ifdef DEBUG
					printf("Warning: recursive function found!\n");
#endif
					fprintf(fp,"Warning: recursive function found!                                                        local stack: %5d, total stack: %5d\n",pcstack, totstack);
					for (j=0; j<2*level-2; j++)
						fprintf(fp," ");
					fprintf(fp,"end function");
					for (j=0; j<80-2*level; j++)
						fprintf(fp," ");
					fprintf(fp,"local stack: %5d, total stack: %5d\n",locstack, totstack);
					totstack-=locstack;
					locstack/=2;
					level--;
					warnings++;
				}
				else
					searchprog(function[pos].adress,function[pos].size);
			}
			else
			{
#ifdef DEBUG
				printf("no function!\n");
#endif
				errors++;
				fprintf(fp,"unknown function: %6d 0x%04X\n", newadress,program[i]);
			}
		}
		else if (program[i]==0x9509)			// icall
		{	level++;
			if (level>maxlevel)
				maxlevel=level;
			totstack+=pcstack;
			if (totstack>maxstack)
				maxstack=totstack;
#ifdef DEBUG
			printf("Warning: indirect call found!\n");
#endif
			fprintf(fp,"Warning: indirect call (via function pointer) found!                                      local stack: %5d, total stack: %5d\n",pcstack, totstack);
			totstack-=pcstack;
			level--;
			warnings++;
		}
		else if ((program[i]&0xFE0F)==0x920F)	// push
		{	locstack++;
			totstack++;
			if (totstack>maxstack)
				maxstack=totstack;
		}
		else if (((program[i]&0xFE0F)==0xB60D)||((program[i]&0xFE0F)==0xB60E))	// in rxx, SPL/SPH
		{
#ifdef DEBUG
			printf ("in rxx, SPL/SPH\n");
#endif
			i++;
			if (((program[i]&0xFE0F)==0xB60D)||((program[i]&0xFE0F)==0xB60E))	// in rxx, SPL/SPH
				i++;
			if ((program[i]&0xFF00)==0x9700)			// sbiw rxx, k
			{	int k;
				k=(program[i]&0x00C0)/4;
				k|=program[i]&0x000F;
#ifdef DEBUG
				printf ("sbiw rxx, 0x%04X %d\n",program[i],k);
#endif
				locstack+=k;
				totstack+=k;
			}
			else if ((program[i]&0xF000)==0x5000)			// sbui rxx, k
			{	int k;
				k=(program[i]&0x0F00)/16;
				k|=program[i]&0x000F;
#ifdef DEBUG
				printf ("subi rxx, 0x%04X %d\n",program[i],k);
#endif
				locstack+=k;
				totstack+=k;
				
				if ((program[i+1]&0xF000)==0x4000)		// sbci rxx, k
				{	int k;
					i++;
					k=(program[i]&0x0F00)/16;
					k|=program[i]&0x000F;
					k*=256;
#ifdef DEBUG
					printf ("sbci rxx, 0x%04X %d\n",program[i],k);
#endif
					locstack+=k;
					totstack+=k;
				}		
			}
		}
		else if (program[i]==0x9478)					// sei
		{	if (isint)
			{	nested=true;
#ifdef DEBUG
				printf ("sei\n");
#endif
				intnest++;
			}
		}
		else if (program[i]==0x94F8)					// cli
		{	if (isint)
			{	if (nested)
				{	if (totstack>nestmaxstack)
						nestmaxstack=totstack;
				}		
				nested=false;
#ifdef DEBUG
				printf ("cli\n");
#endif
			}
		}
	}
	for (j=0; j<2*level-2; j++)
		fprintf(fp," ");
	fprintf(fp,"end function");
	for (j=0; j<80-2*level; j++)
		fprintf(fp," ");
	fprintf(fp,"local stack: %5d, total stack: %5d\n",locstack, totstack);
	if (totstack>maxstack)
		maxstack=totstack;
	if ((isint)&&(nested))
	{	if (totstack>nestmaxstack)
			nestmaxstack=totstack;
	}
	totstack-=locstack;
	level--;
}

int getfuncname(unsigned long adress)	// retrieve function name from address
{	int i;
	for (i=0; i<funccnt; i++)
	{	if (function[i].adress==adress)
			return i;
	}
	return 0;
}