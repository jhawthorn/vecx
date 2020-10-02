#include<stdio.h>
int main()
{
  

/* user defined read and write functions */
printf("**********************************************");
extern unsigned char (*e6809_read8) (unsigned address);
extern void (*e6809_write8) (unsigned address, unsigned char data);

void e6809_reset (void);
unsigned e6809_sstep (unsigned irq_i, unsigned irq_f);

#endif
