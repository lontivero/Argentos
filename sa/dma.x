#include <asm/io.h>
#include <asm/dma.h>
#include <asm/system.h>
#include <phantom/mm.h>
#include <sys/types.h>

#define NR_DMA		0x08
#define IS16BIT(ch)	((ch)>3)

struct dma_st
  {
    unsigned char bit;
    unsigned char baseaddr;
    unsigned char wordcount;
    unsigned char request;
    unsigned char mask;
    unsigned char mode;
    unsigned char flipflop;
    unsigned char page;
  };

struct dma_st dma[NR_DMA] =
{
  {0x01, 0x00, 0x01, 0x09, 0x0A, 0x0B, 0x0C, 0x87},
  {0x02, 0x02, 0x03, 0x09, 0x0A, 0x0B, 0x0C, 0x83},
  {0x04, 0x04, 0x05, 0x09, 0x0A, 0x0B, 0x0C, 0x81},
  {0x08, 0x06, 0x07, 0x09, 0x0A, 0x0B, 0x0C, 0x82},
  {0x01, 0xC0, 0xC2, 0xD2, 0xD4, 0xD6, 0xD8, 0x8F},
  {0x02, 0xC4, 0xC6, 0xD2, 0xD4, 0xD6, 0xD8, 0x8B},
  {0x04, 0xC8, 0xCA, 0xD2, 0xD4, 0xD6, 0xD8, 0x89},
  {0x08, 0xCC, 0xCE, 0xD2, 0xD4, 0xD6, 0xD8, 0x8A}
};

struct dma_info_st
  {
    void *p_addr;
    void *v_addr;
    int buffer_size;
  };

struct dma_info_st dma_info[NR_DMA] =
{
  {NULL, NULL, 0},
};

void
dma_mask (char chan)
{
  outb (chan | 4, dma[chan].mask);
}

void
dma_umask (char chan)
{
  outb (chan, dma[chan].mask);
}

void
dma_stop (char chan)
{
  dma_mask (chan);
  outb (0x00, dma[chan].flipflop);
}

/* count should be always (and is) buffer_size else dma_getpos 
   would not work in some cases... */

void
dma_setuptransfer (unsigned char chan, int cmd, int count)
{
  ulg buff;

  if (chan >= NR_DMA)
    {
      printk ("dma.c: invalid channel (%d)\n", chan);
      return;
    }

  buff = (ulg) (dma_info[chan].p_addr);

#ifdef DMADEBUG
  printk ("dma_setuptransfer(): tranfer at physical 0x%x\n", buff);
#endif

  dma_mask (chan);
  outb (0x00, dma[chan].flipflop);	/* order reset */
  outb (cmd | chan, dma[chan].mode);

  outb (buff & 0xFF, dma[chan].baseaddr);
  outb (buff >> 8, dma[chan].baseaddr);
  outb (buff >> 16, dma[chan].page);

  count--;
  outb (count & 0xFF, dma[chan].wordcount);
  outb (count >> 8, dma[chan].wordcount);

  dma_umask (chan);
}

int
dma_getpos (unsigned char chan)
{
  int pos;

  if (chan >= NR_DMA)
    return (-1);

  outb (0x00, dma[chan].flipflop);	/* order reset */

  cli ();
  pos = inb (dma[chan].wordcount);
  pos |= inb (dma[chan].wordcount) << 8;
  sti ();

  if (IS16BIT (chan))
    pos <<= 1;			/* to bytes */
  else
    pos = dma_info[chan].buffer_size - pos;

  return (pos);
}

void *
dma_alloc (u8 chan, int buffer_size)
{
  void *v_addr;

  if (chan >= NR_DMA)
    return (NULL);

  if (dma_info[chan].buffer_size)
    {
      printk ("dma_alloc: channel %d already used\n");
      return (NULL);
    }

#ifdef DMADEBUG
  printf ("dma_alloc: calling kmalloc, buffer_size=%d\n", buffer_size);
#endif

  if (!(v_addr = kmalloc (buffer_size, PG_DMA)))
    return (NULL);

#ifdef DMADEBUG
  printf ("  v_addr=0x%p, calling virt2phys\n", v_addr);
#endif

  dma_info[chan].p_addr = (void *) virt2phys (v_addr);
  dma_info[chan].v_addr = v_addr;
  dma_info[chan].buffer_size = buffer_size;
#ifdef DMADEBUG
  printf ("  all done!\n");
#endif
  return v_addr;
}

int
dma_free (u8 chan)
{
  if (chan >= NR_DMA)
    return (-1);

  if (!dma_info[chan].buffer_size)
    return (-2);

  kfree (dma_info[chan].v_addr);
  dma_info[chan].buffer_size = 0;

  return (0);
}
