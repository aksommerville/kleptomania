#include "kleptomania.h"

/* Validate a 9-char high score. Wipe if fishy.
 */
 
static void hiscore_sanitize(char *v) {
  if (
    (v[0]<'0')||(v[0]>'9')||
    (v[1]<'0')||(v[1]>'9')||
    (v[2]!=':')||
    (v[3]<'0')||(v[3]>'9')||
    (v[4]<'0')||(v[4]>'9')||
    (v[5]!='.')||
    (v[6]<'0')||(v[6]>'9')||
    (v[7]<'0')||(v[7]>'9')||
    (v[8]<'0')||(v[8]>'9')
  ) {
    memset(v,0,9);
  }
}

/* Load.
 */
 
void hiscore_load() {
  egg_store_get(g.hiscore_any,sizeof(g.hiscore_any),"hiscore_any",11);
  egg_store_get(g.hiscore_100,sizeof(g.hiscore_100),"hiscore_100",11);
  hiscore_sanitize(g.hiscore_any);
  hiscore_sanitize(g.hiscore_100);
}

/* Check globals against hiscore.
 */
 
int hiscore_check() {
  int ms=(int)(g.playtime*1000.0);
  if (ms<=0) return 0; // Fishy. Get out.
  int sec=ms/1000; ms%=1000;
  int min=sec/60; sec%=60;
  if (min>99) {
    min=sec=99;
    ms=999;
  }
  char t[9]={
    '0'+min/10,
    '0'+min%10,
    ':',
    '0'+sec/10,
    '0'+sec%10,
    '.',
    '0'+ms/100,
    '0'+(ms/10)%10,
    '0'+ms%10,
  };
  const char *k;
  char *v;
  if (g.rung_bell) {
    k="hiscore_100";
    v=g.hiscore_100;
  } else {
    k="hiscore_any";
    v=g.hiscore_any;
  }
  if (v[0]&&(memcmp(t,v,9)>=1)) {
    // Not a new hiscore for the ideal key.
    // But if our key is 100%, check again against any%.
    if (!g.rung_bell) return 0;
    k="hiscore_any";
    v=g.hiscore_any;
    if (v[0]&&(memcmp(t,v,9)>=1)) return 0;
  }
  memcpy(v,t,9);
  egg_store_set(k,11,v,9);
  return 1;
}
