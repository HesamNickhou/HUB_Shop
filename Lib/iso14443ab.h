#ifndef ISO14443AB_H
#define ISO14443AB_H

void MFRC522_Init(void);
unsigned char ISO14443_SingleTagSelect(unsigned char *snr);
#define NO_ERROR  0
#define HAS_ERROR 1

#endif
