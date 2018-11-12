/*-------------------------------------------------------*/
/* config.h	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : site-configurable settings		 	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/

#ifndef	_CONFIG_H_
#define	_CONFIG_H_

#undef	TREAT
#undef	TRANUFO

/* ----------------------------------------------------- */
/* 定義 BBS 站名位址					 */
/* ------------------------------------------------------*/

#define BOARDNAME       "元智大學 風之塔"		/* 中文站名 */
#define NICKNAME	"風之塔"			/* 中文簡稱 */
#define MYHOSTNAME	"bbs.yzu.edu.tw"		/* 網路位址 */
#define SYSOPNICK	"風之塔塔主"			/* SYSOP 暱稱 */
#define BBSHOME		"/home/bbs"			/* BBS 的家 */
#define BBSUID		9999
#define BBSGID		99
#define TAG_VALID       "[WindTopBBS]To "                 /* 身分認證函token */
#define SYSOPNAME	"visor"				/* 特級站務 */
#define	HIDEDN_SRC	"bbs.yzu.edu.tw"		/* 隱藏來源位置 */
#define BBSNAME		"[WindTopBBS Ver.20031002]"	/* 英文站名 */
#define	BBSIP		"140.138.2.235"			/* bbs 的 ip */

/* ----------------------------------------------------- */
/* 隨 BBS 站規模成長而擴增				 */
/* ----------------------------------------------------- */

#ifndef	MAXBOARD
#define MAXBOARD	(1024)		/* 最大開板個數 */
#endif

#ifndef	MAXACTIVE
#define MAXACTIVE	(2500)		/* 最多同時上站人數 (ex.1400)*/
					/* 若要超過 1024 請修改 kernel */
					/* options         SHMMAXPGS=? */ 
#endif

#define MAXFIREWALL	(300)		/* 全域擋信列表上限 */

#define MAXOFILEWALL	(500)		/* 看版擋信列表上限 */

#define PAL_MAX		(512)		/* 好友名單上限 */

#define PAL_ALMR	(450)		/* 警告 */

#define	BANMSG_MAX	(32)		/* 拒收訊息名單上限 */

/* ----------------------------------------------------- */
/* guest的名字                             by Jerics     */
/* ------------------------------------------------------*/

#define GUESTNAME	(1)                               /* guest名稱的數量 */
#define GUEST_NAMES	GUEST_1
#define GUEST_1         NICKNAME"訪客"

#define CHATROOMNAME	"風言風語"

/* ----------------------------------------------------- */
/* 將元智大學的 mail server 預先 define                  */
/* ----------------------------------------------------- */

#define YZUMAIL84	"alumni.yzu.edu.tw"
#define	YZUMAIL85	"alumni.yzu.edu.tw"
#define	YZUMAIL86	"alumni.yzu.edu.tw"
#define YZUMAIL87	"alumni.yzu.edu.tw"
#define YZUMAIL88	"bach.yzu.edu.tw"
#define	YZUMAIL89	"chopin.yzu.edu.tw"
#define	YZUMAIL90	"haydn.yzu.edu.tw"
#define YZUMAIL91	"mozart.yzu.edu.tw"
#define	DEFAULTSERVER	"mail.yzu.edu.tw"

#if 0
#define	YZUMAIL		{"84",YZUMAIL84,\
			 "85",YZUMAIL85,\
			 "86",YZUMAIL86,\
			 "87",YZUMAIL87,\
			 "88",YZUMAIL88,\
			 "89",YZUMAIL89,\
			 "90",YZUMAIL90,\
			 "91",YZUMAIL91,\
			 NULL,YZUMAIL84}
#endif
#define YZUMAIL         {NULL,"pop3.yzu.edu.tw"}

/* ----------------------------------------------------- */
/* 組態規劃						 */
/* ------------------------------------------------------*/

#define	HAVE_WEBBBS		/* only for WindTop */

#undef	HAVE_PIP_FIGHT		/* 小雞對戰 */ /* 尚未完工 很多 bug */
#define	HAVE_PIP_FIGHT1

#define	EMAIL_JUSTIFY		/* 發出 InterNet Email 身份認證信函 */

#define	HAVE_BRDTITLE_CHANGE	/* 版主修改版名 */

#define	BLACK_SU		/* 系統 debug 用 */

#define	HAVE_OBSERVE_LIST	/* 系統觀察名單 */

#undef	NEWUSER_LIMIT		/* 新手上路的三天限制 */

#undef	HAVE_REPORT		/* 系統追蹤報告 */

#define	HAVE_RECOMMEND		/* 推薦文章 */
#ifdef	HAVE_RECOMMEND
#endif

#define	HAVE_MMAP		/* 採用 mmap(): memory mapped I/O */
				/* 在 SunOS、FreeBSD 上可以加速 30 倍 */
				/* Linux 的 mmap() 有問題，請暫時不要用 */

#ifdef	HAVE_MMAP
#include <sys/mman.h>
#ifdef MAP_FILE		/* 44BSD defines this & requires it to mmap files */
#  define BBS_MAP	(MAP_SHARED | MAP_FILE)
#else
#  define BBS_MAP	(MAP_SHARED)
#endif
#endif

#define	HAVE_INPUT_TOOLS	/* 符號輸入工具 */

#define	HAVE_BANMSG		/* 拒收訊息功能列表 */

#define	HAVE_DETECT_CROSSPOST	/* cross-post 自動偵測 */

#define	HAVE_DETECT_VIOLAWATE	/* 違法紀錄 */

#define	HAVE_RESIST_WATER	/* 防灌水功能 */   

#define	HAVE_CLASSTABLE		/* 個人功課表 */

#ifdef	HAVE_CLASSTABLE
#define	HAVE_CLASSTABLEALERT	/* 課表時刻通知 */
#endif

#undef	HAVE_CHANGE_SKIN	/* 切換 skin *//* 尚未完成不要開啟 */

#define	HAVE_SEM		/* 使用 Semaphores */

#define	HAVE_MULTI_CROSSPOST	/* 群組轉貼 */

#define	HAVE_BM_CHECK		/* 版主確認 */

#define	HAVE_RAND_INCOME	/* 亂數進站畫面 */

#define	HAVE_MAILGEM		/* 使用信件精華區 */

#define	HAVE_FAVORITE		/* 使用我的最愛 */

#define	HAVE_PROFESS		/* 使用專業討論區 */

#define	HAVE_USER_MODIFY	/* 使用者修改文章 */

#define	HAVE_INFO		/* 使用校方公告區 */

#define	HAVE_SHOWNUMMSG		/* 顯示水球個數 */

#define	HAVE_STUDENT		/* 使用學生公告區 */

#undef	HAVE_MAXRESERVE		/* 站長保留上站位置 *//* 尚有問題  請勿開啟 */

#define	HAVE_MAILUNDELETE	/* 復原刪除信箱中的信件 */

#define HAVE_WRITE		/* 白名單 */

#define	HAVE_SONG		/* 提供點歌系統 */

#undef	HAVE_SONG_TO_CAMERA	/* 是否點歌到動態看版 */

#define	HAVE_MAIL_FIX		/* 修正 mail title 的功能 *//* ps:小心不要亂開 */

#define	HAVE_MODERATED_BOARD	/* 提供秘密版 */

#ifdef	HAVE_MODERATED_BOARD
#define HAVE_WATER_LIST		/* 提供水桶功能 */

#ifdef	HAVE_WATER_LIST
#define	HAVE_SYSOP_WATERLIST	/* SYSOP 水桶名單 */
#endif
#endif	

#undef	HAVE_GAME_QKMJ		/* 提供 QKMJ 遊戲 */

#define	HAVE_BOARD_PAL		/* 提供板友功能 */

#define HAVE_ANONYMOUS		/* 提供 anonymous 板 */

#define	HAVE_ORIGIN		/* 顯示 author 來自何處 */

#define HAVE_MIME_TRANSFER      /* 提供 MIME 解碼 */

#define HAVE_SIGNED_MAIL	/* Thor.990409: 外送信件加簽名 */

#ifdef HAVE_SIGNED_MAIL
#define PRIVATE_KEY_PERIOD	0	/* Thor.990409: 不換key, auto gen */
#endif

#define SHOW_USER_IN_TEXT       /* 在文件中自動顯示 User 的名字 */

#ifndef	SHOW_USER_IN_TEXT
#define outx	outs
#endif

#define HAVE_BOARD_SEE		/* PERM_ALLBOARD 可以看到全部看板 */

#define	HAVE_PERSON_DATA	/* 提供生日等其他資料 */

#define	HAVE_TERMINATOR		/* 終結文章大法 --- 拂花落楓斬 */

#define HAVE_XYPOST		/* 文章串接模式 */

#define	LOGIN_NOTIFY            /* 系統協尋網友 */

#define	AUTHOR_EXTRACTION	/* 尋找同一作者文章 */

#define EVERY_Z			/* ctrl-z Everywhere (除了寫文章) */

#define EVERY_BIFF		/* Thor.980805: 郵差到處來按鈴 */

#define BMW_TIME		/* Thor.980911: 熱訊時間記錄 */

#define	HAVE_COUNT_BOARD	/* visor.20030816: 看板資訊統計 */

#undef	MODE_STAT               /* 觀察及統計 user 的生態, 以做為經營方針 */

#define	HAVE_DECLARE		/* 使某些title更明顯 */

#define	HAVE_CHK_MAILSIZE	/* visor.20031111: 信箱超過大小，上站鎖信箱 */

#define	HAVE_ALOHA              /* lkchu.981201: 好友上站通知 */

#define	HAVE_DECORATE           /* 標記和上色 */

#define MULTI_MAIL		/* 群組寄信功能 */ /* Thor.981016: 為防幸運信 */

#define	HAVE_REGISTER_FORM	/* 註冊單認證 */

#ifdef	HAVE_REGISTER_FORM
#undef	HAVE_SIMPLE_RFORM	/* 簡化註冊單 */
#endif

#define	JUSTIFY_PERIODICAL	/* 定期身分認證 */

#define	LOG_ADMIN		/* Thor.990405: lkchu patch: 紀錄更改權限 */

#define LOG_BMW                 /* lkchu.981201: 熱訊記錄處理 */

#define	LOG_TALK		/* lkchu.981201: 聊天記錄處理 */

#define	LOG_CHAT		/* 聊天室紀錄 */

#define	LOG_BRD_USIES		/* lkchu.981201: 記錄看板閱讀狀況可供統計 */

#define HAVE_ETC_HOSTS          /* lkchu.981201: 參照 etc/hosts 的 database */

#define HAVE_CHANGE_FROM        /* lkchu.981201: ^F 暫時更改故鄉 */

#define COLOR_HEADER		/* lkchu.981201: 變換彩色標頭 */

#undef	EMAIL_PAGE		/* lkchu.990429: 電子郵件傳呼 */

#define	FRIEND_FIRST		/* lkchu.990512: 好友置前 */

#undef	HAVE_SMTP_SERVER	/* bmtad是收信程式, 不是寄信程式
				寄信程式是在bbsd中, mail.c的bsmtp
				要relay的話 statue.00725 */
#ifdef	HAVE_SMTP_SERVER
#define	SMTP_SERVER		{"mail.yzu.edu.tw",NULL}
#endif

#undef	HAVE_FORCE_BOARD	/* 強迫 user login 時候讀取某公告看板: */	
#ifdef	HAVE_FORCE_BOARD
#define	FORCE_BOARD		BRD_ANNOUNCE /* statue.000725 */
#endif

#define	HAVE_BBSNET		/* 提供 BBSNET */

#define HAVE_COLOR_VMSG		/* 彩色的VMSG */


/* ----------------------------------------------------- */
/* 其他系統上限參數					 */
/* ----------------------------------------------------- */

#ifdef  HAVE_PIP_FIGHT1
#define	PIP_MAX		(16)
#endif

#define LOGINATTEMPTS	(3)		/* 最大進站失誤次數 */

#define LOGINASNEW	(1)		/* 採用上站申請帳號制度 */

#define	MAX_REGIST	(3)		/* 最大註冊數 */

#define	MAX_RESERVE	(16)		/* 站務保留位置 */

#define MAX_MEMORANDUM	(200)		/* 最大備忘錄個數 */

#define MAX_CONTACT	(200)		/* 最大聯絡名單個數 */

#define	BMW_MAX		(128)		/* 熱訊上限 */

#define	BMW_PER_USER	(9)		/* 每位使用者熱訊上限 */

#define	BMW_EXPIRE	(60)		/* 熱訊處理時間 */

#define BMW_MAX_SIZE	(100)		/* 熱訊上限 (K) */

#define	MAX_BBSMAIL	(10000)		/* bbsmail 收信上限 (封) */

#define	MAX_VALIDMAIL	(500)		/* 認證 user 收信上限 (封) */

#define	MAX_NOVALIDMAIL	(100)		/* 未認證 user 收信上限 (封) */

#define MAIL_SIZE_NO	(100)		/* 未認證 user 收信上限 (K) */

#define MAIL_SIZE	(1000)		/* 最大信箱上限 (K) */

#define	MAX_LIST	(6)		/* 群組名單最大數 */

#define MAX_MAIL_SIZE	(10240)		/* 總信箱容量 */

                       /* Thor.981011: 以上for bbs主程式用, 超過則提示 */
                       /* 記得同步修改 etc/mail.over */
                       /* Thor.981014: bquota 用來清過期檔案/信件, 時間如下 */
                       /* 記得同步修改 etc/justified 通知函以及 etc/approved */

#define MARK_DUE        (180)		/* 標記保存之信件 (天) */
#define MAIL_DUE        (60)		/* 一般信件 (天) */
#define FILE_DUE        (30)		/* 其他檔案 (天) */

#define	MAXGUEST	(1024)		/* 最多有幾個 guest？ */
					/* statue change 128 -> 1024 for test*/

#define	MAX_ALOHA	(64)		/* 上站通知引入數 */

#define MAXSIGLINES	(6)		/* 簽名檔引入最大行數 */

#define MAXQUERYLINES	(19)		/* 顯示 Query/Plan 訊息最大行數 */

#define	MAX_CHOICES	(32)		/* 投票時最多有 32 種選擇 */

#define MAX_BOOKS	(16)		/* 最大的名冊數 */

#define	TAG_MAX		(256)		/* TagList 標籤數目之上限 */

#define NBRD_MAX	(30)		/* 最大連署數 */

#define NBRD_DAYS	(14)		/* 連署天數 */

#define NBRD_MAX_CANCEL	(30)		/* 最大廢版主人數 */

#define	MAXPAGES	(256)		/* more.c 中文章頁數上限 (lines/22),預設:128 */

#define MAXLASTCMD	(8)		/* line input buffer */

#define	MOVIE_MAX	(180)		/* 動畫張數 */

#define	MOVIE_SIZE	(108*1024)	/* 動畫 cache size */

#ifdef	HAVE_RECOMMEND
#define	MIN_RECOMMEND	(1)		/* 最小推薦文章顯示數字 */
#endif

#ifdef	HAVE_DETECT_CROSSPOST
#define MAX_CHECKSUM	(6)		/* crosspost checksum 行數 */

#define MAX_CROSS_POST	(3)		/* cross post 最大數量 */
#endif

#ifdef	HAVE_OBSERVE_LIST	/* 系統觀察名單 */
#define	MAXOBSERVELIST	(32)
#define	BRD_OBSERVE		"ObserveList"
#endif

#define MOVIE_LINES	(20)		/* 動畫最多有 10 列 */

#define	CHECK_PERIOD	(86400 * 14)	/* 整理信箱/好友名單的週期 */

#define	VALID_PERIOD	(86400 * 180)	/* 身分認證有效期 */
                     /* Thor.981014: 記得同步修改 etc/re-reg */

#define CHECK_BM_TIME	(86400 * 14)	/* 版主確認時間 */

#define IDLE_TIMEOUT	(30)		/* 發呆過久自動簽退 */

#ifdef	HAVE_RESIST_WATER
#define	CHECK_QUOT	(3)		/* 文章灌水最小行數 */

#define CHECK_QUOT_MAX	(3)		/* 文章灌水連續篇數 */	
#endif

#define	BANMAIL_EXPIRE	(30)		/* 擋信列表更新 (天) */

#define	BLK_SIZ		4096		/* disk I/O block size */

#define CNA_MAX         20              /* lkchu.981201: 即時新聞上限 */

#define	BBSNETMAX	10		/* BBSNET 最大連線數量 */

/* ----------------------------------------------------- */
/* chat.c & xchatd.c 中採用的 port 及 protocol		 */
/* ------------------------------------------------------*/

#define	BBTP_PORT	4884

#define CHAT_PORT	3838
#define CHAT_SECURE			/* 安全的聊天室 */

#define EXIT_LOGOUT     0
#define EXIT_LOSTCONN   -1
#define EXIT_CLIERROR   -2
#define EXIT_TIMEDOUT   -3
#define EXIT_KICK       -4

#define CHAT_LOGIN_OK       "OK"
#define CHAT_LOGIN_EXISTS   "EX"
#define CHAT_LOGIN_INVALID  "IN"
#define CHAT_LOGIN_BOGUS    "BG"


/* ----------------------------------------------------- */
/* 系統參數						 */
/* ----------------------------------------------------- */

#define BRDSHM_KEY	(2997)
#define UTMPSHM_KEY	(1998)
#define FILMSHM_KEY	(2999)
#define FWSHM_KEY	(3999)
#define FWOSHM_KEY	(5000)
#define COUNT_KEY	(4000)

#ifdef	HAVE_OBSERVE_LIST	/* 系統觀察名單 */
#define	OBSERVE_KEY	(6000)
#endif

#define	SPEAK_MAX	(50)
#define	CONDITION_MAX	(100)
#define	PARTY_MAX	(150)



#define	BSEM_KEY	2000	/* semaphore key */
#define	BSEM_FLG	0600	/* semaphore mode */
#define BSEM_ENTER      -1	/* enter semaphore */
#define BSEM_LEAVE      1	/* leave semaphore */
#define BSEM_RESET	0	/* reset semaphore */


/* ----------------------------------------------------- */
/* 申請帳號時要求申請者真實資料				 */
/* ----------------------------------------------------- */

#define REALINFO

#ifdef	REALINFO
#undef	POST_REALNAMES		/* 貼文件時附上真實姓名 */
#undef	MAIL_REALNAMES		/* 寄站內信件時附上真實姓名 */
#undef	QUERY_REALNAMES		/* 被 Query 的 User 告知真實姓名 */
#endif

/* ----------------------------------------------------- */
/* 好友名單顏色色定					 */
/* ----------------------------------------------------- */

#define         COLOR_PAL       "\033[1;32m"
#define         COLOR_BAD       "\033[1;31m"
#define         COLOR_CLOAK     "\033[1;35m"
#define         COLOR_BOTH      "\033[1;37m"
#define         COLOR_OPAL      "\033[1;33m"
#define		COLOR_BOARDPAL	"\033[36m"

#endif				/* _CONFIG_H_ */

