= Harvey OS を拡張してみる

この章では, 前章でセットアップした環境で実際にHarvey OSの拡張や開発を行ってみます. 具体的にはユーザランドプログラムの追加やデバドラの追加を行ってみます.また, この拡張に当たって必要となるビルドコンフィグについて解説します.

== Harvey OS のビルド概要

Plan 9のビルドは mkfile なる, いわゆる"Makefileのようなもの"でなされています. Harvey OSはPlan9をベースにしていますが, このmkfileを残したままそこにオーバラップする形で独自のビルドシステムを追加しています.

=== ビルドの対象物と順序

前章の「ビルド」にて用いたBUILDスクリプトは以下の単位および順序でHarvey OSのコンポーネントをビルドします.

 1. utils: ビルドに用いる各種プログラム(go)のビルド
 2. libs: ユーザランド向け各種ライブラリのビルドおよび"/$ARCH/lib"への配置
 3. klibs: カーネル側のユーティリティや, システムコールテーブルの生成
 4. cmds: ユーザランドプログラムのビルドおよび"/$ARCH/bin"への配置
 5. kcmds: rcやbind, mountといったカーネルと密接に関係している一部プログラムのビルド
 6. kernel: Harvey OS カーネル本体のビルド (併せて指定されたユーザランドプログラムを含むラムディスクをカーネルイメージに付与)
 7. regress: リグレッションテスト用の各種ユーティリティのビルド

=== ビルドコンフィグ

Harvey OS では JSON形式のファイルを用いてビルドの定義をしています. これはベースとなっている, オリジナルのPlan 9 from Bell Labsのビルドがmkfileにより制御されているのとは対照的です. 例としてipconfigコマンドのビルドコンフィグは@<list>{config_example_ipconfig}に示されるJSONファイルとして定義されています.

//list[config_example_ipconfig][ipconfigコマンドのビルドコンフィグ(sys/src/cmd/ip/ipconfig/ipconfig.json)]{
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
	子となるビルドコンフィグの配列
	Makefileのinclude相当であり, 後述する Pre を走らせる前にここで指定されたビルドコンフィグのビルドを実行
: Pre
	ビルド前に実行させたいコマンド列の配列
: Post
	ビルド後に実行させたいコマンド列の配列
: Cflags
	ビルド時にgccに引き渡すCFLAGSを指定
: Include
	ビルド時に参照するビルドコンフィグ
	対象のCflags等を参照できるため, 主に後述するSourceFilesCmdでビルドする
	個々のプログラムや, 
: Libs
	ビルド時にリンクするライブラリの配列
: Env
	ビルド時に使用する環境変数
: Program
	当該ビルドコンフィグでビルド・出力されるプログラム(最終成果物)の名前
	gcc -o に与える名前として利用される
: SourceFilesCmd
	個別のcファイルを個々のプログラムとしてビルドする場合に利用
	ただし個々のビルド時には当該JSONファイルのCflags等を共通で用いる
	たとえば abc.c, def.c を並べた場合, abcとdefという2つのプログラムとしてビルドする
: SourceFiles
	上記 "Program" に指定されたプログラム生成に用いるソースファイル(.c, .S)の配列
: Install
	ビルド完了後に生成した成果物を配置する場所
: Library
	生成するライブラリの名前@<fn>{librarynote}
	ビルドコンフィグがライブラリを成果物としていた場合に利用
: Kernel
	カーネルのビルドコンフィグにのみ存在(次節参照)

//footnote[librarynote][Plan9に動的ライブラリはないので常に".a"]

=== カーネル用ビルドコンフィグ

前節で述べた通り, カーネルのビルドコンフィグには特殊なディレクティブが存在します.
今のところ触る必要のあるものは k8cpu.json (sys/src/9/k10/k8cpu.json) のみです.
Plan 9のカーネルビルド時には, ビルドコンフィグ(jsonではなくPCCPUF等のテキストファイル)
に基づいてCソースファイルを動的に生成するロジックが存在します.
この生成部分を Harvey OS で取り扱うためにカーネルビルド時には前節の"Kernel"なる
ディレクティブを特別に用いる必要があります.

: Config
	当該カーネルでサポートするデバイス(機能)の定義
: Ramfiles
	ラムディスク(ramfs)に含めるユーザランドプログラムの名前とバイナリの位置のハッシュ



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
同様に Harvey OS でも k8cpu.json からsys/src/9/k10/k8cpu.c なるソースファイルが生成され, これまでに述べた各種ディレクティブで指定したソース・ドライバ等がCソースファイル上の記述としてカーネルに含められるようになります.

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
                - ./sys/src/klib.json
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
                - /sys/src/cmd/kernel.json
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

おおよその分類として "複数形".json (例: cmds.json ) と "単数形".json (例: cmd.json) が存在しており, 前者が基幹となって当該ディレクトリ以下の木構造を定義し後者で共通のCflagsやビルドオプション等を定義するという構造になっています.


== ユーザランドアプリの追加

起動して /bin 等を漁ってみると分かるかと思いますが, 素の状態のHarvey OSはほとんど何も載っかっていないためここにいろいろ追加していく必要があります. ここでは主にユーザランドプログラムの追加について解説します.

=== 既存コマンドの追加: ping

一度でも BUILD スクリプトを走らせた後には /amd64/bin の下にコンパイル済みのユーザランドプログラムが大量に生成されています. これらを

//list[add_ping][pingをラムディスクに追加する]{
diff --git a/sys/src/9/k10/k8cpu.json b/sys/src/9/k10/k8cpu.json
index 2c70efe..ff2a01b 100644
--- a/sys/src/9/k10/k8cpu.json
+++ b/sys/src/9/k10/k8cpu.json
@@ -75,6 +75,7 @@
                        "listen1": "/amd64/bin/aux/listen1",
                        "ls": "/amd64/bin/ls",
                        "mount": "/amd64/bin/mount",
+                       "ping": "/amd64/bin/ip/ping",
                        "rc": "/amd64/bin/rc",
                        "rcmain": "/rc/lib/rcmain",
                        "srv": "/amd64/bin/srv",
//}

一度 ./BUILD all を走らせた後であれば既にping自体のバイナリは生成されているため, カーネルの再ビルド(ラムディスク部分の再生成)だけで今回はOKです.

//cmd{
./BUILD kernel
//}

起動後, 適当にIPアドレスを設定してやると以下の様にpingコマンドが追加され実行できているのが確認できます.

//cmd{
harvey@cpu% ipconfig -g 192.168.0.1 ether /net/ether0 192.168.0.130 255.255.255.0
harvey@cpu% ping 192.168.0.130
sending 32 64 byte messages 1000 ms apart to icmp!192.168.0.130!1
0: rtt 3246 µs, avg rtt 3246 µs, ttl = 255
1: rtt 2475 µs, avg rtt 2860 µs, ttl = 255
2: rtt 518 µs, avg rtt 2079 µs, ttl = 255
3: rtt 614 µs, avg rtt 1713 µs, ttl = 255
4: rtt 568 µs, avg rtt 1484 µs, ttl = 255
//}

=== 新規コマンドの追加: bffileserver

前項で述べたpingは, すでにHarvey OS のビルドターゲットとなっていたプログラムであり, これを新規にラムディスクに加えて触れるようにしてみたというものでした.
ここでは json であるビルドコンフィグを書いてみるサンプルとして bffileserver@<fn>{bffileserver_note}を追加してみます.

//footnote[bffileserver_note][https://github.com/enukane/bffileserver]

==== bffileserverの準備

まず, bffileserverのソースを入手してきます. 当該コードは Plan 9 ベースでは動作確認が取れているので直接 Harvey OS のソースにねじ込みます. bffileserverはいくつかのソースファイルから成るので, cmds.jsonに混ぜずに個別にビルドコンフィグを作成する方針とします.

//cmd{
% cd sys/src/cmd
% git clone https://github.com/enukane/bffileserver.git
//}


次に当該ディレクトリにjsonのビルドコンフィグ (sys/src/cmd/bffileserver/bffileserver.json) を新規追加します.

//list[bffileserver_json][bffileserver用JSONビルドコンフィグ(sys/src/cmd/bffileserver/bffileserver.json]{
{
        "Include": [
                "../cmd.json"
        ],
        "Name": "bffileserver",
        "Post": [
                "mv bffileserver $HARVEY/amd64/bin/"
        ],
        "Program": "bffileserver",
        "SourceFiles": [
                "bf.c",
                "main.c"
        ]
}
//}


==== 既存ビルドコンフィグの変更

まず cmds.json の Projects ディレクティブに前述の bffileserver.json を追加し, ビルド対象に含める必要があります.

//list[diff_bffileserver_cmds][cmds.jsonへの変更]{
diff --git a/sys/src/cmd/cmds.json b/sys/src/cmd/cmds.json
index f584c68..5c4a035 100644
--- a/sys/src/cmd/cmds.json
+++ b/sys/src/cmd/cmds.json
@@ -10,6 +10,7 @@
                "astro/astro.json",
                "auth/auth.json",
                "aux/aux.json",
+               "bffileserver/bffileserver.json",
                "bzip2/bzip2.json",
                "cb/cb.json",
                "cdfs/cdfs.json",
//}


次に, ラムディスクに含めるために k8cpu.json に対して bffileserver を追加します. bffileserver.json の Post ディレクティブに記載したようにbffileserverのバイナリは /amd64/bin/に配置されているため, このパスを指定します.

//list[diff_][k8cpu.jsonへの変更]{
diff --git a/sys/src/9/k10/k8cpu.json b/sys/src/9/k10/k8cpu.json
index 2c70efe..be26cd0 100644
--- a/sys/src/9/k10/k8cpu.json
+++ b/sys/src/9/k10/k8cpu.json
@@ -66,6 +66,7 @@
                        ]
                },
                "Ramfiles": {
+                       "bffileserver": "/amd64/bin/bffileserver",
                        "bind": "/amd64/bin/bind",
                        "boot": "boot.fs",
                        "cat": "/amd64/bin/cat",
//}

==== ビルド

ビルドに当たって, 今回はcmds.jsonに変更が加わっているためまずはユーザランドプログラムの再ビルドを行ってからさらにカーネルのビルドと順に行う必要があります.

//cmd{
1. ユーザランド(含む bffileserver)のビルド
% ./BUILD cmd

2. カーネルのビルド(今回は主にラムディスクの再構成を目的として)
% ./BUILD kernel
//}

==== 動作確認

実際に組み込まれたはずの bffileserver の動作確認をしてみます. Harvey OS の起動後以下の様に brainfuck の実行ができれば正しく組込が完了しています.

//cmd{
1. bffileserverの起動
harvey@cpu% bffileserver

2. /srv/bfができていることの確認
harvey@cpu% ls /srv/  
/srv//bf
/srv//bf

3. ファイルサーバのマウント
harvey@cpu% mount /srv/bf /mnt

4. ファイルが見えていることの確認
harvey@cpu% ls /mnt
/mnt/cmd
/mnt/data

5. brainfuckのコードを流し込む
harvey@cpu% echo '+++++++++[>++++++++>+++++++++++>+++++<<<-]>.>++.+++++++..+++.>-.------
------.<++++++++.--------.+++.------.--------.>+.' > /mnt/cmd

6. 結果が正しく返ってくることの確認(改行コードはないことに注意)
harvey@cpu% cat /mnt/data
Hello, world!harvey@cpu% 
//}


== カーネルランドの拡張

#@# === devbrainfuckを追加してみる
#@# # 微妙なのでけずるか？

=== 新規ドライバの追加: ethervirtio

#@# 前置き: さすがに前節のdevbrainfuckは実用性がなさ過ぎるので

ここではより実用の可能性があるドライバとして ethervirtio (virtio-net) ドライバを Harvey OS に追加してみます.

==== ethervirtioドライバの準備

今回用いるethervirtioのソースは, 9legacy@<fn>{9legacy} にて patch として配布されている Nick Owens 氏作のものを用います. 以下のURLのdiffから ethervirtio.c を切り出します. 当該ドライバは仮想環境, 特にQEMU/KVM上でしか利用しないものなので sys/src/9/k10 に配置します.

//footnote[9legacy][@<href>{http://www.9legacy.org/}]

//emlist{
http://www.9legacy.org/9legacy/patch/pc-ethervirtio.diff
//}

切り出した ethervirtio.c の先頭には以下の行を追加します.

//list[ethervirtio_add][ethervirtio.cに追加する型定義]{
typedef unsigned char uchar;
typedef uint8_t  u8int;
typedef uint16_t u16int;
typedef uint64_t u64int;
typedef uint32_t u32int;
typedef uint32_t ulong;
//}

これを追記しないとビルドエラーになります. オリジナルのPlan 9では上記の様に uchar や ulong といったgccにはない型がいくつか存在しており, ethernetvirtio がこれらを利用してしまっているためです. 上記は最低限 ethervirtio.c をビルドする際に必要な型のみを記載しています.

==== 既存ビルドコンフィグの変更

まず, Plan 9自体のカーネルコンフィグである k8cpu に以下の記述を追加します.
//list[ethervirtio_k8cpu_p9_add][ethervirtio: k8cpuへの変更]{
diff --git a/sys/src/9/k10/k8cpu b/sys/src/9/k10/k8cpu
index 9f74a4f..dc2eb54 100644
--- a/sys/src/9/k10/k8cpu
+++ b/sys/src/9/k10/k8cpu
@@ -57,6 +57,7 @@ link +dev
 ##     etherbcm        pci ethermii
        ether8139       pci ethermii
        ethermedium
+       ethervirtio     pci
        loopbackmedium
 #      usbuhci
 #      usbohci
//}


今回はカーネル側のみのため, k8cpu.json に ethervirtio をビルド対象に含めるよう Devディレクティブと SourceFiles ディレクティブにそれぞれ必要な項目を追記しています.

//list[ethervirtio_k8cpu_add][ethervirtio: k8cpu.jsonへの変更]{
diff --git a/sys/src/9/k10/k8cpu.json b/sys/src/9/k10/k8cpu.json
index 2c70efe..ff0b75f 100644
--- a/sys/src/9/k10/k8cpu.json
+++ b/sys/src/9/k10/k8cpu.json
@@ -53,6 +53,7 @@
                                "etherigbe",
                                "ether8139",
                                "ethermedium",
+                               "ethervirtio",
                                "loopbackmedium",
                                "netdevmedium"
                        ],
@@ -90,6 +92,7 @@
                "devusb.c",
                "ether8139.c",
                "ether82563.c",
+               "ethervirtio.c",
                "k8cpu.c",
                "sdata.c",
                "usbehcipc.c",
//}

==== ビルド

今回はカーネルのみに関わる変更のため, 以下の通りkernelのビルドのみ実施します.

//cmd{
./BUILD kernel
//}


==== 起動スクリプトの新設

これまで起動スクリプトとして用いてきた QRUN は rtl8139 を用いているため, これをvirtioに切り替えます. QRUN_virtio として以下の様なスクリプトをもう一つ util/QRNU_virtio に追加します.

//list[QRUN_virtio][util/QRUN_virtio: virtio-netを有効にした起動スクリプト]{
sudo qemu-system-x86_64 -s -cpu Opteron_G1 -smp 1 -m 512  \
-serial stdio \
--machine pc \
-net nic,macaddr=00:11:22:33:44:55,model=virtio \
-net user,hostfwd=tcp::5555-:1522 \
-kernel 9k8cpu.32bit $*
//}


==== 動作確認

前項で述べた QRUN_virtio で起動してみます.

//cmd{
% (cd sys/src/9/k10 && sh ../../../../util/QRUN_virtio)
//}


まず起動時に以下の行が出力されることを確認しましょう.

//emlist{
#l0: virtio: 1000Mbps port 0xc000 irq 11 tu 1514: 001122334455
//}


次に正しくEtherデバイスとしてマウントできていることを確認していきます.

//cmd{
1. /net/ether0以下にイーサネットデバイスが生えている
harvey@cpu% ls /net/ether0
/net/ether0/0
/net/ether0/1
/net/ether0/2
/net/ether0/addr
/net/ether0/clone
/net/ether0/ifstats
/net/ether0/mtu
/net/ether0/stats

2. 起動スクリプトで指定されたMACアドレス(00:11:22:33:44:55)が割当たっている
harvey@cpu% cat /net/ether0/addr
001122334455harvey@cpu% 

3. virtio-netデバイスとして取得できる各種情報が得られる
harvey@cpu% cat /net/ether0/ifstats
devfeat 79BF8064
drvfeat 70020
devstatus 7
netstatus 1
vq0 0xfffffffff0e9b180 size 256 avail->idx 130 used->idx 2 lastused 2 nintr 2 nnote 1
vq1 0xfffffffff0e9b208 size 256 avail->idx 3 used->idx 3 lastused 2 nintr 3 nnote 3
vq2 0xfffffffff0e9b290 size 64 avail->idx 0 used->idx 0 lastused 0 nintr 0 nnote 0
//}

上記の通りvirtqueueが3本(Rx, Tx, Control)生えており各キュー長やインデックスが取得できることが確認できました.

