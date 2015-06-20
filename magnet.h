extern char magnetPolarity;
extern unsigned char magnetbitbuffermaxindex;
extern unsigned char magnetbitbuffer;
extern unsigned char lrcbyte;


void writeEMagBit(char bit);
void calculateLRC(void);
void writeBitToBuffer(unsigned char);
void writeCharToBuffer(char);
void writeLRC(void);
void writeOutEMagBitBuffer(void);
void resetMagnetBitBuffer(void);