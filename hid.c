#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define strtou64(x, y) (uint64_t)strtoull(x, NULL, y)
#define parity(n) __builtin_parity((uint64_t)n)
#define strcp(x, y, z) memcpy(x, y, z); x[z] = '\0';

static inline void d2b(uint64_t x, char *b, uint16_t c)
{
  register uint8_t i;
  for (i = c; i--;)
    *(b++) = (x >> i) & 0x1 ? '1' : '0';
  *(b++) = '\0';
}

static inline uint8_t format(char *s)
{
  if (toupper(s[1]) == 'X' && s[0] == '0') return 1;
  if (toupper(s[1]) == 'B' && s[0] == '0') return 2;
  if (s[1] == ':') {
    if (toupper(s[0]) == 'P') return 3;
    if (toupper(s[0]) == 'B') return 4;
  }
  return 0;
}

int main(int argc, char *argv[])
{
  register uint64_t dec = 0, fac = 0, num = 0, hex = 0, pm = 0;
  register uint8_t msb, lsb, msb_ok, lsb_ok, err = 0;;
  char *p, *s;
  char prox[32];
  char hbin[32];
  char pbin[32];
  char tline[150] = "────────────────────────────────────────";
  char mline[150] = "───────────┬────────────────────────────";
  char bline[150] = "───────────┴────────────────────────────";
  char errmsg[1024] = "";
  char buffer[255];
  int skip, ret = 0;
  if (argc < 2) return (-1);
  if (argc > 2) {
    fac = strtou64(argv[1], 10);
    num = strtou64(argv[2], 10);
    if (fac > 0xff || num > 0xffff) {
      snprintf(buffer, sizeof(buffer) - 1,
        " err: %03lu,%05lu [255,65535]\n", fac, num);
      strcat(errmsg, buffer);
      err++;
    }
  } else if ((s = strchr(argv[1], ','))) {
    p = argv[1];
    while (isblank(*p)) p++;
    *s = 0; s++;
    while (isblank(*s)) s++;
    fac = strtou64(p, 10);
    num = strtou64(s, 10);
    if (fac > 0xff || num > 0xffff) {
      snprintf(buffer, sizeof(buffer) - 1,
        " err: %03lu,%05lu [255,65535]\n", fac, num);
      strcat(errmsg, buffer);
      err++;
    }
  } else {
    switch((ret = format(argv[1])))
    {
      case 1:
        hex = strtou64(argv[1], 16);
        d2b(hex, hbin, 26);
        break;
      case 2:
        skip = strlen(argv[1]) - 26;
        if (skip < 2) skip = 2;
        strcp(hbin, &argv[1][skip], 26);
        hex = strtou64(hbin, 2);
        break;
      case 3:
        strcp(prox, &argv[1][2], 10);
        pm = strtou64(prox, 16);
        hex = ((pm & 0x7ffffff) ^ (1 << 26));
        d2b(pm, pbin, 27);
        d2b(hex, hbin, 26);
        break;
      case 4:
        skip = strlen(argv[1]) - 27;
        if (skip < 2) skip = 2;
        strcp(pbin, &argv[1][skip], 27);
        if (pbin[0] != '1' || strlen(pbin) != 27) {
          snprintf(buffer, sizeof(buffer) - 1,
            " Proxmark binary error: %s\n", pbin);
          strcat(errmsg, buffer);
          err++;
        }
        pm = strtou64(pbin, 2);
	hex = ((pm & 0x7ffffff) ^ (1 << 26));
        snprintf(prox, 11, "20%08lx", pm)  < 0 ? abort() : (void)0;
        d2b(hex, hbin, 26);
        break;
      default:
        dec = strtou64(argv[1], 10);
        fac = (dec >> 16) & 0xff;
        num =  dec & 0xffff;
    }
  }
  if (ret) {
    msb_ok = parity((hex >> 13) & 0x1fff) ^ 1;
    lsb_ok = parity( hex        & 0x1fff);
    if (!lsb_ok || !msb_ok) {
      snprintf(buffer, sizeof(buffer) - 1,
        " Parity check failed: %07lX %d %d\n",
        hex, msb_ok, lsb_ok);
      strcat(errmsg, buffer);
      err++;
    }
    fac = (hex >> 17) & 0xff;
    num = (hex >>  1) & 0xffff;
  } else {
    hex = ((fac & 0xff)  << 17) + ((num & 0xffff) << 1);
    msb = parity((hex >> 13) & 0xfff);
    lsb = parity((hex >>  1) & 0xfff) ^ 1;
    hex += (msb << 25) + lsb;
    d2b(hex, hbin, 26);
  }
  if (!pm) {
    pm = hex | (1 << 26);
    d2b(pm, pbin, 27);
    snprintf(prox, 11, "20%08lx", pm)  < 0 ? abort() : (void)0;;
  }
  dec = dec ? dec : (fac << 16) + num;
  if (err)
    fprintf(stdout, "\n%s\n", errmsg);
  fprintf(stdout, " %s\n", tline);
  fprintf(stdout, " %010lu %03lu,%05lu\n", dec, fac, num);
  fprintf(stdout, " %s\n", mline);
  fprintf(stdout, "%-12s%s %27.7lX\n", " Hex", "│", hex);
  fprintf(stdout, "%-12s%s %27s\n", " Proxmark", "│", prox);
  fprintf(stdout, "%-12s%s %27s\n", " Bin", "│", hbin);
  fprintf(stdout, "%-12s%s %27s\n", " Prox.Bin", "│", pbin);
  fprintf(stdout, " %s\n", bline);
  return 0;
}
