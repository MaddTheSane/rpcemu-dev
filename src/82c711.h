#ifndef __82C711__
#define __82C711__

extern void reset82c711();
extern void callbackfdc(void);
extern uint8_t read82c711(uint32_t addr);
extern uint8_t readfdcdma(uint32_t addr);
extern void writefdcdma(uint32_t addr, uint8_t val);
extern void write82c711(uint32_t addr, uint32_t val);
extern void loadadf(char *fn, int drive);
extern void saveadf(char *fn, int drive);

#endif //__82C711__
