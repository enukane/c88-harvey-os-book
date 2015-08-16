= Harvey OS のビルド概要

Plan 9のビルドは mkfile なる, いわゆる Makefile のようなものでなされています. Harvey OSはPlan9をベースにしていますが, このmkfileを残したままそこにオーバラップする形で独自のビルドシステムを追加しています. ここでは Harvey OS を拡張するにあたり知っておくべきビルドシステムの概要について解説します.

=== ビルドの対象物と順序

前章の「ビルド」にて用いたBUILDスクリプトは以下の単位および順序でHarvey OSのコンポーネントをビルドします.

 1. utils: ビルドに用いる各種プログラム(build や data2c 等 @<i>{util/src/harvey/cmd} 以下の go 製プログラム)
 2. libs: ユーザランド向け各種ライブラリ, および @<i>{$ARCH/lib} への配置
 3. klibs: カーネル側のユーティリティ, システムコールテーブルの生成
 4. cmds: ユーザランドプログラム, および @<i>{$ARCH/bin} への配置
 5. kcmds: rcやbind, mountといったカーネルと密接に関係している一部プログラム
 6. kernel: Harvey OS カーネル本体, および指定されたユーザランドプログラムを含むラムイメージ
 7. regress: リグレッションテスト用の各種ユーティリティ

=== ビルドコンフィグ

Harvey OS では JSON形式のファイルを用いてビルドの定義をしています.
#@# これはベースとなっている, オリジナルのPlan 9 from Bell Labsのビルドがmkfileにより制御されているのとは対照的です.
例としてipconfigコマンドのビルドコンフィグは@<list>{config_example_ipconfig}に示されるJSONファイルとして記述されています.

//list[config_example_ipconfig][ipconfigコマンドのビルドコンフィグ(@<i>{sys/src/cmd/ip/ipconfig/ipconfig.json})]{
{
        "Include": [
                "../../cmd.json"
        ],
        "Install": "/$ARCH/bin/ip",
        "Name": "ipconfig",
        "Program": "ipconfig",
        "SourceFiles": [
                "ipv6.c",
                "main.c",
                "ppp.c"
        ]
}
//}

JSONのハッシュのキーであるディレクティブ@<fn>{directive}には以下の種類があります.

//footnote[directive][公式の表現ではありません.本稿では勝手にこう呼称します]

: Name
	識別用の名前
: Projects
	子となるビルドコンフィグの配列. Makefileのinclude相当であり, 後述する Pre を走らせる前にここで指定されたビルドコンフィグの処理を実施
: Pre
	ビルド前に実行させたいコマンド列の配列
: Post
	ビルド後に実行させたいコマンド列の配列
: Cflags
	ビルド時にgccに引き渡すCFLAGSを指定
: Include
	ビルド時に参照するビルドコンフィグの配列. 対象のコンフィグ中の Cflags や Libs 等を参照できるため, 共通パラメータを一つのコンフィグファイルに押し込め個々のコンフィグから参照といった形態を取るために用いられる
: Libs
	ビルド時にリンクするライブラリの配列
: Env
	ビルド時に使用する環境変数
: Program
	当該ビルドコンフィグでビルド・出力されるプログラム(最終成果物)の名前 (gcc -o に与える名前)
: SourceFilesCmd
	個別のcファイルを個々のプログラムとしてビルドする場合に利用. たとえば abc.c, def.c を並べた場合, abcとdefという2つのプログラムがビルドされる. 個々のビルド時には, 当該JSONファイルのCflags等を共通で用いられる. Plan 9では @<i>{sys/src/cmd} 等に単独のCファイルで1つのプログラムになるようなソースが大量に並べられるディレクトリがいくつかあり, こういったターゲットをビルドする際に個々にビルドコンフィグを書かずにまとめてコンパイルするために当該ディレクティブを用いる
: SourceFiles
	上記 Program に指定されたプログラム生成に用いるソースファイル(.c, .S)の配列
: Install
	ビルド完了後に生成した成果物を配置する場所
: Library
	生成するライブラリの名前@<fn>{librarynote}. ビルドコンフィグがライブラリを成果物としていた場合に利用.
: Kernel
	カーネルのビルドコンフィグにのみ存在(次節参照)

//footnote[librarynote][Plan 9 には動的ライブラリはないので常に".a"]

=== カーネル用ビルドコンフィグ

前節で述べた通り, カーネルのビルドコンフィグには一部に特殊なディレクティブが存在します.
カーネルビルドコンフィグとして, 現状の Harvey OS で触る必要のあるものは k8cpu.json (@<i>{sys/src/9/k10/k8cpu.json}) のみです.
この JSON ファイルの内容は @<list>{k8cpu_json} の様な構成になっています.

//list[k8cpu_json][@<i>{sys/src/9/k10/k8cpu.json} の内容(抜粋)]{
{
	"Env": [
		"CONF=k8cpu"
	],
	"Include": [
		....
	],
	"Kernel": {
		"Config": {
			"Code": [
				"int cpuserver = 1;",
				"uint32_t kerndate = 1;"
			],
			"Dev": [
				"root",
				....
			],
			"Ip": [
				"tcp",
				....
			],
			"Link": [
				"ether8169",
				....
			],
			"Sd": [
				"sdiahci"
			],
			"Systab": "/sys/src/libc/9syscall/sys.h",
			"Uart": [
				....
			]
		},
		"Ramfiles": {
			"bind": "/amd64/bin/bind",
			....
		}
	},
	"Name": "k8cpu",
	"Program": "9k8cpu",
	"SourceFiles": [
		"cga.c",
		....
	]
}
//}

おおよそユーザランドのそれと同じですが, Kernel ディレクティブが追加で用いられています.

Plan 9のカーネルビルド時には, ビルドコンフィグ(jsonではなくテキストファイル@<fn>{pccpu})に基づいて, 必要なコンポーネントを連結するためのCソースファイルを動的に生成するロジックが存在します.
Kernelディレクティブは, Harvey OS にてこの生成を制御するために用いられます.
オリジナルの Plan 9 では pccpuf として記述されたコンフィグから pccpuf.c が生成されますが, Harvey OS では k8cpu.json からその内容に従って k8cpu.c が生成されます.

//footnote[pccpu][Plan 9だと @<i>{sys/src/9/pc/pccpuf} や @<i>{pccpu} といったファイル]



Kernelディレクティブは以下の2つのディレクティブを子として持ちます.

: Config
	当該カーネルでサポートするデバイス(機能)の定義
: Ramfiles
	ラムイメージ(ramfs)に含めるユーザランドプログラムの名前とバイナリの位置の組 (ハッシュ)


Configディレクティブはさらに以下の要素から構成されます.

: Code
	コンパイル時に生成されるCソースファイルに追記するコード行の配列(変数の初期化や定義に用いる)
: Dev
	サポートするデバイスドライバの配列
: Ip
	サポートするL3, L4のプロトコルスタックの配列
: Link
	サポートするリンク(イーサドライバやusbバスドライバ)の配列
: Sd
	サポートするストレージドライバの配列
: Systab
	システムコール番号の定義ファイルパス
: Uart
	サポートするuartの一覧

オリジナルのPlan 9では, たとえばpccpufなるビルドコンフィグからpccpuf.cを生成し各種ドライバのテーブルを定義します.
同様に Harvey OS でも k8cpu.json からsys/src/9/k10/k8cpu.c なるソースファイルが生成され, これまでに述べた各種ディレクティブで指定したソース・ドライバ等がCソースファイル上の記述としてカーネルに参照され, 初期化ルーチン等が呼び出しされるようになります.


Ramfilesディレクティブは, ラムイメージに含めるユーザランドプログラムの一覧を定義するためのディレクティブです.
第二章で紹介した QRUN スクリプト( Harvey OS 起動スクリプト)の中身を見ると, どこにもユーザランドのファイルシステムやらイメージやらを指定しているところがないことに気付きます.

一方, コンパイル時に自動生成される @<i>{sys/src/9/k10/k8cpu.c} を見ると, ファイルの先頭から "ramfs_${プログラム名}_code" という名前の配列がいくつも連なっているのが見えるかと思います.

//list[kernel_ramfs][@<i>{sys/src/9/k10/k8cpu.c}の内容(抜粋)]{
....
static unsigned char ramfs_cat_code[] = {
0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 
0x02, 0x00, 0x3e, 0x00, 0x01, 0x00, 0x00, 0x00, 0xad, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00,
0x00, 
....
};
int ramfs_cat_len = 29416;

static unsigned char ramfs_ipconfig_code[] = {
0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 
0x02, 0x00, 0x3e, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x5e, 0x41, 0x00, 0x00, 0x00, 0x00,
0x00, 
....

void
links(void)
{
addbootfile("cat", ramfs_cat_code, ramfs_cat_len);
addbootfile("ipconfig", ramfs_ipconfig_code, ramfs_ipconfig_len);
....
//}


コンパイルされ @<i>{$ARCH/bin} に配置されたバイナリは, さらに C の配列(十六進数の数値列)に変換されこのファイル中に, ある意味カーネルの一部として埋め込まれます. カーネルは起動時, このlinks関数が呼ばれた際に addbootfile関数によりルートファイルシステム上にこれらのユーザプログラムのバイナリデータを展開, ユーザの目に見えるようお膳立てします. これにより, さも指定されたユーザランドプログラムが含まれたファイルシステムが付与されているかのように動作することとなります.

=== BUILD スクリプトから参照されるビルドコンフィグの構造

BUILD スクリプトはそれぞれ上記ビルド対象物の基幹となるビルドコンフィグを参照しており, そこを足がかりに必要なコンポーネントをビルドしています. ビルドコンフィグ同士は Projects と Include ディレクティブにより包含されます. この木構造は以下の様になっています.

//list[build_tree_libs][ライブラリビルドコンフィグの木構造]{
- sys/src/libs.json
    - Projects
        - sys/src/lib*/*.json
            - Include
                - sys/src/lib/lib.json
//}

//list[build_tree_klibs][カーネルライブラリビルドコンフィグの木構造]{
- sys/src/klibs.json
    - Projects:
        - sys/src/libc/klibc.json
            - Projects:
                - sys/src/libc/9syscall/9syscall.json
            - Include:
                - sys/src/klib.json
        - sys/src/libip/klibip.json
            - Include:
                - sys/src/klib.json
        - sys/src/libdraw/klibdraw.json
            - Include:
                - sys/src/klib.json
//}

//list[build_tree_cmds][ユーザランドプログラムビルドコンフィグの木構造]{
- sys/src/cmd/cmds.json
    - Projects
        - sys/src/cmd/*/*.json
            - Include
                - sys/src/cmd/cmd.json
    - Include
        - sys/src/cmd/cmd.json
//}

//list[build_tree_kcmds][特殊ユーザランドプログラムビルドコンフィグの木構造]{
- sys/src/cmd/kcmds.json
    - Projects:
        - sys/src/cmd/rc/rc.json
            - Include:
                - sys/src/cmd/kernel.json
        - sys/src/cmd/ip/ipconfig/kipconfig.json
            - Include:
                - sys/src/cmd/kernel.json
    - Include:
        - sys/src/cmd/kernel.json
//}

//list[build_tree_kernel][カーネルビルドコンフィグの木構造]{
- sys/src/9/k10/k8cpu.json
    - Projects
        - sys/src/libc/9syscall/9syscall.json
    - Include:
        - sys/src/9/k10/core.json
            - Projects:
                - sys/src/9/k10/clean.json
                - sys/src/9/k10/inith.json
        - sys/src/9/386/386.json
        - sys/src/9/ip/ip.json
        - sys/src/9/port/port.json
//}

おおよその分類として @<b>{複数形}.json (例: cmds.json ) と @<b>{単数形}.json (例: cmd.json) が存在しており, 前者が基幹となって当該ディレクトリ以下の木構造を定義し後者で共通のCflagsやビルドオプション等を定義するという構造になっています.


