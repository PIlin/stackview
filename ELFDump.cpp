/*
ELFDump.cpp - Dump ELF file using ELFIO library.
Copyright (C) 2001 Serge Lamikhov-Center <to_serge@users.sourceforge.net>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <cstdio>
#include <cstring>
#include <string>

#include "ELFIO.h"
#include "stackanalyse.h"

using namespace std;

void
PrintHeader( const IELFI* pReader )
{
    printf( "ELF Header\n" );
    printf( "  Class:      %s (%d)\n",
            ( ELFCLASS32 == pReader->GetClass() ) ? "CLASS32" : "Unknown",
            (int)pReader->GetClass() );
    if ( ELFDATA2LSB == pReader->GetEncoding() ) {
        printf( "  Encoding:   Little endian\n" );
    }
    else if ( ELFDATA2MSB == pReader->GetEncoding() ) {
        printf( "  Encoding:   Big endian\n" );
    }
    else {
        printf( "  Encoding:   Unknown\n" );
    }
    printf( "  ELFVersion: %s (%d)\n",
            ( EV_CURRENT == pReader->GetELFVersion() ) ? "Current" : "Unknown",
            (int)pReader->GetELFVersion() );
    printf( "  Type:       0x%04X\n",   pReader->GetType() );
    printf( "  Machine:    0x%04X\n",   pReader->GetMachine() );
    printf( "  Version:    0x%08X\n",   pReader->GetVersion() );
    printf( "  Entry:      0x%08X\n",   pReader->GetEntry() );
    printf( "  Flags:      0x%08X\n\n", pReader->GetFlags() );
}


string SectionTypes( Elf32_Word type )
{
    string sRet = "UNKNOWN";
    switch ( type ) {
    case SHT_NULL :
        sRet = "NULL";
        break;
    case SHT_PROGBITS :
        sRet = "PROGBITS";
        break;
    case SHT_SYMTAB :
        sRet = "SYMTAB";
        break;
    case SHT_STRTAB :
        sRet = "STRTAB";
        break;
    case SHT_RELA :
        sRet = "RELA";
        break;
    case SHT_HASH :
        sRet = "HASH";
        break;
    case SHT_DYNAMIC :
        sRet = "DYNAMIC";
        break;
    case SHT_NOTE :
        sRet = "NOTE";
        break;
    case SHT_NOBITS :
        sRet = "NOBITS";
        break;
    case SHT_REL :
        sRet = "REL";
        break;
    case SHT_SHLIB :
        sRet = "SHLIB";
        break;
    case SHT_DYNSYM :
        sRet = "DYNSYM";
        break;
    }
    
    return sRet;
}


string SectionFlags( Elf32_Word flags )
{
    string sRet = "";
    if ( flags & SHF_WRITE ) {
        sRet += "W";
    }
    if ( flags & SHF_ALLOC ) {
        sRet += "A";
    }
    if ( flags & SHF_EXECINSTR ) {
        sRet += "X";
    }

    return sRet;
}

string
SegmentTypes( Elf32_Word type )
{
    string sRet = "UNKNOWN";
    switch ( type ) {
    case PT_NULL:
        sRet = "NULL";
        break;
    case PT_LOAD:
        sRet = "PT_LOAD";
        break;
    case PT_DYNAMIC:        
        sRet = "PT_DYNAMIC";
        break;
    case PT_INTERP:
        sRet = "PT_INTERP";
        break;
    case PT_NOTE:
        sRet = "PT_NOTE";
        break;
    case PT_SHLIB:
        sRet = "PT_SHLIB";
        break;
    case PT_PHDR:
        sRet = "PT_PHDR";
        break;
    }
    
    return sRet;
}


void
PrintSegment( int i, const IELFISegment* pSeg )
{
    printf( "  [%2x] %-10.10s %08x %08x %08x %08x %08x %08x\n",
            i,
            SegmentTypes( pSeg->GetType() ).c_str(),
            pSeg->GetVirtualAddress(),
            pSeg->GetPhysicalAddress(),
            pSeg->GetFileSize(),
            pSeg->GetMemSize(),
            pSeg->GetFlags(),
            pSeg->GetAlign() );

    return;
}


void
PrintSymbol( std::string& name, Elf32_Addr value,
             Elf32_Word size,
             unsigned char bind, unsigned char type,
             Elf32_Half section )
{
    printf( "%-46.46s %08x %08x %04x %04x %04x\n",
            name.c_str(),
            value,
            size,
            (int)bind,
            (int)type,
            section );
}

int main( int argc, char** argv )
{
	printf( "\nAVR StackViewer v0.12 by Benedikt\n\n\n" );

    if ( argc != 2 )
	{	printf( "Usage: avrStackView <file_name>\n" );
		return 1;
    }

    // Open ELF reader
    IELFI* pReader;
    if ( ERR_ELFIO_NO_ERROR != ELFIO::GetInstance()->CreateELFI( &pReader ) ) {
        printf( "Can't create ELF reader\n" );
        return 2;
    }

    if ( ERR_ELFIO_NO_ERROR != pReader->Load( argv[1] ) )
	{   printf( "Can't open input file \"%s\"\n", argv[1] );
        return 3;
    }

	if (pReader->GetMachine()!=0x53)
	{   printf( "No AVR ELF file!\n");
        return 4;
    }

    // Print ELF file header
    //PrintHeader( pReader );

	int txtsect=2;
    int nSecNo = pReader->GetSectionsNum();
    int i;
    for ( i = 0; i < nSecNo; ++i )
	{	const IELFISection* pSec = pReader->GetSection( i );
		if ((!pSec->GetName().compare(".text"))||(!pSec->GetName().compare(".TEXT"))||(!pSec->GetName().compare(".Text")))
			txtsect=i;
		if ((pSec->GetAddress()>=0x0800000)&&(pSec->GetAddress()<=0x0810000))
		{	globalram+=pSec->GetSize();
		}
        pSec->Release();
    }

	for ( i = 0; i < nSecNo; ++i )
	{	const IELFISection* pSec = pReader->GetSection( i );
        if ( SHT_SYMTAB == pSec->GetType() || SHT_DYNSYM == pSec->GetType() )
		{   IELFISymbolTable* pSymTbl = 0;
            pReader->CreateSectionReader( IELFI::ELFI_SYMBOL, pSec, (void**)&pSymTbl );
            std::string   name;
            Elf32_Addr    value;
            Elf32_Word    size;
            unsigned char bind;
            unsigned char type;
            Elf32_Half    section;
            int nSymNo = pSymTbl->GetSymbolNum();
            if ( 0 < nSymNo )
			{	for ( int i = 0; i < nSymNo; ++i )
				{	pSymTbl->GetSymbol( i, name, value, size, bind, type, section );
					if (section==txtsect)
					{	function[funccnt].adress=value/2;
						function[funccnt].size=size/2;
						strcpy(function[funccnt].name,name.c_str());
						funccnt++;
					}
                }
            }
            pSymTbl->Release();
        }
        pSec->Release();
    }

	unsigned long size;

	const IELFISection* pSect = pReader->GetSection(txtsect);

	string s;
	s=pSect->GetName();
	size=pSect->GetSize();
    memcpy(program,pSect->GetData(),size);
	pSect->Release();

	stackanalyse(size/2);
    pReader->Release();

    return 0;
}

