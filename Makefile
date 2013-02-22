all:
	g++ -Wall -g \
	ELFDump.cpp stackanalyse.cpp \
	-I"ELFIO-1.0.3/install_root/include/" \
	-L"ELFIO-1.0.3/install_root/lib/" -lELFIO \
	-o elfdump
