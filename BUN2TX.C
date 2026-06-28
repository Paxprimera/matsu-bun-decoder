/*  BUN2TX: New-Matu to Text		by muransky		*/
/*								*/
/*　でも，showさんの mtot.c をもとにしたことは一目瞭然でしょう．*/
/*								*/
/*　BUN2TXは，新松文書ファイルをMS-DOSテキストに変換します．	*/
/*　松86はファイル形式が根本的に異なるらしいので，従来通りMTOT	*/
/* を使ってください．						*/
/*　対象になるのは新松文書中の文字だけです．中央ぞろえ，右寄せ，*/
/* 罫線，左右端等は対象になりません．                           */
/*　文書枠やメモ行／制御行の中のテキストは生かしますが，それら	*/
/* の最後に改行がないと，本文とつながって見苦しくなります．	*/
/*　新松文書中の改行はCR/LF(0x0d/0x0a)，改頁はFF(0x0c)，タブ・デ*/
/* シマルタブはHT(0x08)，倍角，添え字は通常の文字に変換します． */
/*								*/
/*  ほらね．コメントまでMTOT.Cとほとんど同じでしょ．でも，	*/
/*								*/
/*	使用法: bun2tx 文書ファイル名 [テキストファイル名]	*/
/*								*/
/*　ちょっと違いますね．					*/
/*　文書ファイル名として複数の文書ファイルを指定しても無駄です．*/
/* 第２パラメータはテキスト名として解釈されます．んであとのもの	*/
/* は無視されます．フィルタにもなっていません．			*/
/*　ただ，第２パラメータを省略することができます．その場合，標準*/
/* 出力に吐き出します．						*/
/*　複数の文書をマージしたい向きは，あとでCATをかけるとか，リダ	*/
/* イレクトでアペンドするとか，自分で努力して下さい．		*/
/*								*/
/*　現在のバージョンは 0.80286 です．				*/
/*　バージョン１とするには，オプションの解釈をきちんとするとか，*/
/* .BUNを指定しなくてもよいようにするとか，文書の標題を出力する	*/
/* オプションを付けるとか，完成度を高める必要があると思います．	*/
/*　標題の出力なんてのは，文書ファイルの 7 バイトめからの40バイ	*/
/* トをそのまんま出せばよいのだから，実に簡単ですが，オプション */
/* との兼ね合いで今回は見送りました．				*/
/*　そのうち，BUN2JXWにも取組みたいとは思いますが，それよりも	*/
/* JXW2BUNのほうが意義があるので，難しいところです．とりあえず，*/
/* アトリビュートやプリンタ用書式をテキストで出力するというのも */
/* おもしろいのではないでしょうか．				*/
/*　まあ，それも遠い未来の話です．今を楽しく生きましょう．:-)	*/
/*								*/
/*　それにしても，mtot.cのコピーライト表\示には参りました．	*/
/*　僕もbitのtsinkyの記事に触発されて，				*/
/*	copywrong/all rights are reversed. 			*/
/* 程度は，やったことがありましたが，あそこまでやられると後がや	*/
/* りづらい！　							*/
/*　それはともかくとして．このbun2txは，mtotがＰＤＳである場合	*/
/* に限り，ＰＤＳです．といいますか，もうmtotの改変版を作って	*/
/* しまった以上，いまさらＰＤＳでないと言われても困りますが．	*/
/*　少なくとも僕は権利を主張しませんので，何かお返しがしたいと	*/
/* いう律義な方は，もとのプログラムを書かれたshowさんになんか	*/
/* 贈り物をしてください．やっぱりお酒かな．			*/
/*								*/
/*　ここまでは，readme.docですので，実際にコンパイルする時は	*/
/* 以下のきりとり線から下だけにするほうがよいと思われます．	*/
/* ------------------------きりとり線-------------------------- */

/* BUN2TX: New-Matu to Text	Ver 0.80286	by muransky	*/
/*		revision of MTOT.C by show de atterukana, eigo.	*/

#include        <stdio.h>

#define low     mchar[0]        /* 文字の下位バイト */
#define high    mchar[1]        /* 文字の上位バイト */

void    exit(int);
void    mmc(FILE *, FILE *);
void    putsj(short, FILE *);
short   tosj(unsigned char, unsigned char);

unsigned char mchar[2], buf[256];

void
main(argc, argv)
        int      argc;
        char    **argv;
{
        FILE    *in_fp, *out_fp;
	int	not_std;

        if (argc == 1) 
		help();

	not_std = 1;
        if (argc == 2)	{
		if(*argv[1] == '-')
			help();	/* オプションと見ればヘルプ出すんだから
				　ひでえ話だ． */
		else
			out_fp = stdout;
			not_std = 0;
		}
	else	{
        	if ((out_fp = fopen(argv[2], "w")) == NULL) {
                printf("あの，テキスト（出力）ファイル%sが",argv[2]);
                printf("オープンできないんですが．\n");
                exit(1);
		}
        }

	if ((in_fp = fopen(*(++argv), "rb")) == NULL) {
		printf("あの，文書（入力）ファィル %s が",*argv);
		printf("オープンできないんですが．\n");
                exit(1);
                }
        mmc(in_fp, out_fp);

}

help()	{
	printf("BUN2TX: 新松文書コンバータ  VER. 0.80286\n");
	printf("    新松文書を標準テキストに変換します．\n");
	printf("    使用方法: bun2tx 文書ファイル名");
	printf(" [テキストファイル名]\n");
	printf("    テキストファイル名省略の場合，標準出力へ出力します．\n");
	exit(1);
	}


void
mmc(in_fp, out_fp)
        FILE    *in_fp, *out_fp;
{
        int     c, blks, skip;
	int	road;
	long	orange;

	orange = 266;
	road = 0;
	fseek(in_fp,orange,road);

	skip = 0;
	for (;;) {
                if ((blks = fread(mchar, 2, 1, in_fp)) != 1) {
                        printf("あの，文書形式が異常なんですが．\n");
                        exit(1);
                }
		if(high == 0x84) break;	/* 文書終了 */
		if(skip) {
			if(high == 0xfe) skip=0;
		}
		else {
			switch	(high)	{
				case	0xfe:
					skip = 1;
					break;
			/* よくわかりませんが，high=0xfeが１個出現したら
			　次のhigh=0xfeまでをスキップした方がよさそうなので
			　スキップします．そうしないと画像ファイルの名前なん
			　てのがテキストに変な形で混入すると思われます */
				case	0:	/* 半角 */
				case	1:	/* 上付 */
				case	2:	/* 下付 */
                        	case    0xc5:	/* 0x00 - 0x1F */
					fputc(low, out_fp);
					break;
        	                case    0x80:	/* 改行 */
                	                fputc('\n', out_fp);
                        	        break;
                  		case    0x81:	/* 改ページ */
                        	case    0x87:	/* 奇数改ページ */
                        	case    0x85:	/* 改段 */
                        	case    0x86:	/* 条件付改段 */
				/* 新松の出力するテキストファイルでは
				　きっちり条件を解釈しているようですが
				　ここでは無条件に改ページとします */
                                	fputc('\f', out_fp);
                                	break;
				case    0x82:	/* タブ */
				case    0x83:	/* デシマルタブ */
                	                fputc('\t', out_fp);
                        	        break;
				default:
					if((low  > 0x20 && low  < 0x80) &&
				   	(high > 0x20 && high < 0x80))
						putsj(tosj(low,high), out_fp);
			}
                }
        }
}

void
putsj(sj, out_fp)       /* シフトＪＩＳ文字の出力 */
        short   sj;
        FILE    *out_fp;
{
        int     sjh, sjl;

        sjh = (int)(sj >> 8);
        sjl = (int)(sj & 0x00ff);
        fputc(sjh, out_fp);
        fputc(sjl, out_fp);
}

short
tosj(l, h)              /* ＪＩＳからシフトＪＩＳへの変換 */
        unsigned char   l,h;
{
        unsigned char   t;

        l += 0x1f;
        t = h;
        h /= 2;
        if (t % 2 == 0) {
                h--;
                l += 0x5e;
        }
        if (l >= 0x7f) l++;
        h += (h <= (unsigned char) 0x2e)
                ? (unsigned char) 0x71: (unsigned char) 0xb1;
        return((short)(h * 256 + l));
}

/* プログラムはここまで */
