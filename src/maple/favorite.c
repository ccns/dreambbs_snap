/*-------------------------------------------------------*/
/* maple/favorite.c	( YZU WindTopBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : 建立 [我的最愛區] cache			 */
/* create : 2000/07/20				 	 */
/* update : 					 	 */
/*-------------------------------------------------------*/

#include "bbs.h"

#ifdef	HAVE_FAVORITE
extern BCACHE *bshm;


/* ----------------------------------------------------- */
/* build favorite image					 */
/* ----------------------------------------------------- */


static ClassHeader *chx[2];

static int
favorite_parse(key)
  char *key;
{
  char *str,fpath[128];
  ClassHeader *chp;
  HDR hdr;
  BRD *bhead, *btail;
  int i, count,check;
  FILE *fp,*fw;
  
  sprintf(fpath,"%s.new",key);
  /* build classes */
  bhead = bshm->bcache;
  btail = bhead + bshm->number;  
  if ((fw = fopen(fpath,"w")) == NULL)
    return CH_END;
  
  if (fp = fopen(key, "r"))
  {
    struct stat st;

    if (fstat(fileno(fp), &st) || (count = st.st_size / sizeof(HDR)) <= 0)
    {
      fclose(fp);
      return CH_END;
    }

    chx[1] = chp = (ClassHeader *) malloc(sizeof(ClassHeader) + count * sizeof(short));
    memset(chp->title, 0, CH_TTLEN);
    strcpy(chp->title, "Favorite");

    count = 0;
    check = 0;

    while (fread(&hdr, sizeof(hdr), 1, fp) == 1)
    {
      if (hdr.xmode & GEM_BOARD)
      {
	BRD *bp;

	i = -1;
	str = hdr.xname;
	bp = bhead;

	for (;;)
	{
	  i++;
	  if (!strcasecmp(str, bp->brdname))
	    break;
	  

	  if (++bp >= btail)
	  {
	    check = 1;
	    i = -1;
	    break;
	  }
	}

	if (i < 0)
	  continue;
      }
      else
      {
        check = 1;
        continue;
      }
        
      chp->chno[count++] = i;
      fwrite(&hdr, sizeof(HDR), 1, fw);
      
    }
    fclose(fp);
    fclose(fw);
    if(check)
    {
      logitfile(FN_FAVORITE_LOG,"< ERR >","< NULL BOARD >");
      unlink(key);
      rename(fpath,key);
    }
    else
      unlink(fpath);
    chp->count = count;
    
    if(count > 0)
      return 1;
  }
  else
  {
    unlink(fpath);
  }

  return CH_END;
}

static int
chno_cmp(i, j)
  short *i, *j;
{
  BRD *bhead;
  bhead = bshm->bcache;
  return strcasecmp(bhead[*i].brdname, bhead[*j].brdname);
}


static void
favorite_sort()
{
  ClassHeader *chp;
  int i, j, max;
  BRD *bp;
  BRD *bhead, *btail;

  max = bshm->number;
  bhead = bp = bshm->bcache;
  btail = bp + max;

  chp = (ClassHeader *) malloc(sizeof(ClassHeader) + max * sizeof(short));

  for (i = j = 0; i < max; i++, bp++)
  {
    if (bp->brdname)
    {
      chp->chno[j++] = i;
    }
  }

  chp->count = j;

  qsort(chp->chno, j, sizeof(short), chno_cmp);

  memset(chp->title, 0, CH_TTLEN);
  strcpy(chp->title, "Boards");
  chx[0] = chp;
} 


void
favorite_main()
{
  int chn,i;
  ClassHeader *chp;
  FILE *fp;
  short len, pos[3];
  char fpath[128];
        
  usr_fpath(fpath,cuser.userid,FN_FAVORITE);
  favorite_sort();
  chn = favorite_parse(fpath);
  
  usr_fpath(fpath,cuser.userid,FN_FAVORITE_IMG);
  unlink(fpath);
  if (chn < 1)  /* visor.990106: 尚沒有我的最愛 */
    return;

  len = sizeof(short) * 3; 
  for (i = 0; i <= 1 ; i++)
  {
    pos[i] = len;
    len += CH_TTLEN + chx[i]->count * sizeof(short);
  }
  pos[i] = len;
          
  if (fp = fopen(fpath, "w"))
  {
     fwrite(pos, sizeof(short), 3, fp);
     for (i = 0; i <= 1; i++)
     { 
       chp = chx[i];
       fwrite(chp->title, 1, CH_TTLEN, fp);
       fwrite(chp->chno, 1, chp->count * sizeof(short), fp);
       free(chp);
     }
     fclose(fp);
  }
  return;
}

#endif	/* HAVE_FAVORITE */
