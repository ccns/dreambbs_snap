/* mime.c */
void clearange(int from,int to);
int Pop3();
void remove_perm(void);
int check_personal_note(int newflag,char *userid);
int post_gem(XO *xo);
int post_tag(XO *xo);
int post_title(XO *xo);
int post_edit(XO *xo);
int mail_stat(int mode);
int m_total_size(void);
void move_post(HDR *hdr,char *board,int by_bm);
void check_new(BRD *brd);
void fwshm_init(void);
void fwshm_load(void);
int XoNewBoard(void);
int Mime(void);
/* acct.c */
int find_same_email(char *mail,int mode);
void deny_log_email(char *mail,time_t deny);
void bitmsg(char *msg,char *str,int level);
int check_yzuemail(char *email);
int add_deny(ACCT *u, int adm, int cross); /* statue.000713 */
void x_file(int mode, char *xlist[], char *flist[]); /* statue.000713 */
void logitfile(char *file, char *key, char *msg);
void su_setup(ACCT *u);
int del_email(EMAIL *email);
int mail_to_all(void);
int mail_to_bm(void);
int Chatmenu(void);
void keeplog(char *fnlog, char *board, char *title, int mode);
void acct_save(ACCT *acct);
int XoSongMain(void);
void post_mail(void);
int check_admin(char *name);
int BanMail(void);
int Admin(void);
int add_ban_mail(void);
int same_mail(char *mail);
int ban_addr(char *addr);
int mainbbs(void);
void lowcase(char *x);
int m_bmset();
void bm_setup(ACCT *u, int adm);
int bbssetenv(char *env, char *val);
int do_exec(char *com, char *wd);
int x_bbsnet(void);
int acct_load(ACCT *acct, char *userid);
int acct_userno(char *userid);
int acct_get(char *msg, ACCT *acct);
/* int m_xfile(void); */
/* int m_xhlp(void); */ /* statue.000703 增加所有 hlp statue.000706 改成 .so*/
/* pcbug.990620: 懶得login...:p */
void count_init();
void count_update();
void count_load();
usint bitset(usint pbits, int count, int maxon, char *msg, char *perms[]);
void acct_show(ACCT *u, int adm);
void acct_setup(ACCT *u, int adm);
int u_info(void);
int m_user(void);
int u_addr(void);
int u_setup(void);
int ue_setup(void);
int u_lock(void);
int u_xfile(void);
int m_newbrd(void);
void brd_edit(int bno);
#ifdef HAVE_REGISTER_FORM
int u_register();
int m_register();
#endif
/* bbsd.c */
#if defined(LINUX)
#ifndef REDHAT7
int getloadavg(float loadavg[], int nelem);
#endif
#endif
void blog(char *mode, char *msg);
void u_exit(char *mode);
void abort_bbs(void);
/* int main(int argc, char *argv[]); */
/* board.c */
void brh_get(time_t bstamp, int bhno);
int Favorite(void);
time_t brd_visit[MAXBOARD];
int Student(void);
int Information(void);
int brh_unread(time_t chrono);
void brh_visit(int mode);
void brh_add(time_t prev, time_t chrono, time_t next);
int bstamp2bno(time_t stamp);
void brh_save(void);
void XoPost(int bno);
int Select(void);
int Class(void);
int Profess(void);
void board_main(void);
int Boards(void);
/* cache.c */
UTMP * pid_find(int pid);
void ushm_init(void);
void utmp_mode(int mode);
int utmp_new(UTMP *up);
void utmp_free(void);
UTMP *utmp_find(int userno);
int utmp_count(int userno, int show);
void bshm_init(void);
int brd_bno(char *bname);
void fshm_init(void);
int film_out(int tag, int row);
void classtable_main(void);
void classtable_free(void);
int observeshm_find(int userno);
void observeshm_init();
void observeshm_load();
#if 0
/* chat.c */
int t_chat(void);
#endif
/* edit.c */
void ve_string(uschar *str);
char *tbf_ask(void);
FILE *tbf_open(void);
void ve_backup(void);
void ve_recover(void);
void ve_header(FILE *fp);
int ve_subject(int row, char *topic, char *dft);
int vedit(char *fpath, int ve_op);
/* gem.c */
int url_fpath(char *fpath, char *folder, HDR *hdr);
void brd2gem(BRD *brd, HDR *gem);
int gem_gather(XO *xo);
void XoGem(char *folder, char *title, int level);
void gem_main(void);
/* mail.c */
int mail_send(char *rcpt,char *title);
int mbox_check(void);
void ll_new(void);
void ll_add(char *name);
int ll_del(char *name);
int ll_has(char *name);
void ll_out(int row, int column, char *msg);
int m_internet(void);
int bsmtp(char *fpath, char *title, char *rcpt, int method);
usint m_quota(void);
int m_query(char *userid);
void m_biff(int userno);
int hdr_reply(int row, HDR *hdr);
int mail_external(char *addr);
void mail_reply(HDR *hdr);
void my_send(char *rcpt);
int m_send(void);
int mail_sysop(void);
int mail_list(void);
int tag_char(int chrono);
void hdr_outs(HDR *hdr, int cc);
void mbox_main(void);
/* menu.c */
int x_ueequery(void);
int x_untrust(void);
int x_spam(void);
int x_today(void);
int x_yesterday(void);
int x_day(void);
int x_week(void);
int x_month(void);
int pad_view(void);
void vs_head(char *title, char *mid);
void movie(void);
void menu(void);
/* more.c */
char *mgets(int fd);
void *mread(int fd, int len);
/*int more(char *fpath, char *footer);*/
int more(char *fpath, char *promptend);
/* post.c */
int seek_log(char *title,int);
int checksum_find(char *fpath,int check,int);
int cmpchrono(HDR *hdr);
void cancel_post(HDR *hdr);
int getsubject(int row, int reply);
int post_cross(XO *xo);
/* talk.c */
void bmw_save(); /* statue.000713 */
char *bmode(UTMP *up, int simple);
int is_pal(int userno);
void pal_cache(void);
void pal_sync(char *fpath);
void aloha_sync(void);
void banmsg_cache(void);
void banmsg_sync(char *fpath);
int t_pal(void);
int t_banmsg(void);
int bm_belong(char *board);
int XoBM(XO *xo);
void my_query(char *userid, int paling);
void bmw_reply(void);
int pal_list(int reciper);
int t_loginNotify(void);
void loginNotify(void);
int t_recall(void);
void bmw_rqst(void);
int t_pager(void);
int t_message(void);
int t_cloak(XO *xo);
int t_query(void);
void talk_rqst(void);
void talk_main(void);
/* visio.c */
void prints();
void bell(void);
void ansi_move(int y,int x);
void move(int y, int x);
void getyx(int *y, int *x);
void refresh(void);
void clear(void);
void clrtohol(void); /* visor.000710 */
void clrtoeol(void);
void clrtobot(void);
void outc(int ch);
void outs(uschar *str);
void outx(uschar *str);
void outz(uschar *msg);
void scroll(void);
void rscroll(void);
void cursor_save(void);
void cursor_restore(void);
void save_foot(screenline *slp);
void restore_foot(screenline *slp);
void vs_save(screenline *slp);
void vs_restore(screenline *slp);
int vmsg(char *msg);
void zmsg(char *msg);
void vs_bar(char *title);
void add_io(int fd, int timeout);
int igetch(void);
BRD *ask_board(char *board, int perm, char *msg);
int vget(int line, int col, uschar *prompt, uschar *data, int max, int echo);
int vans(char *prompt);
int vkey(void);
#if 0
/* vote.c */
int vote_result(XO *xo);
int XoVote(XO *xo);
#endif
/* xover.c */
void inline xo_fpath(char *fpach, char *dir, HDR *hdr); /* statue.000713 */
XO *xo_new(char *path);
XO *xo_get(char *path);
void xo_load(XO *xo, int recsiz);
int hdr_prune(char *folder, int nhead, int ntail,int post);
int xo_delete(XO *xo);
int Tagger(time_t chrono, int recno, int op);
void EnumTagHdr(HDR *hdr, char *dir, int locus);
int AskTag(char *msg);
int xo_uquery(XO *xo);
int xo_usetup(XO *xo);
int xo_getch(XO *xo, int ch);
void xover(int cmd);
void every_Z(void);
void every_U(void);
void every_B(void);
/* favorite.c */
void favorite_main(void); 
/* socket.c */
int Get_Socket(char *site,int *sock);
int POP3_Check(char *site,char *account,char *passwd);
int Ext_POP3_Check(char *site,char *account,char *passwd);
int NumPad(void);
/* mailgem.c */

/* popupmenu.c */
int popupmenu(MENU pmenu[],XO *xo,int x,int y);
int pmsg(char *msg);


