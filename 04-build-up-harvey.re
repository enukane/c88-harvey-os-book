= Harvey OS の拡張

この章では, セットアップした Harvey OS を実用的なモノあるいは遊べるモノにしていくために, 必要なコンポーネントを追加していく方法について解説します.  具体的にはユーザランドプログラムの追加やデバイスドライバの追加を行ってみます.

== ユーザランドアプリの追加

起動して /bin 等を漁ってみると分かるかと思いますが, 素の状態のHarvey OSはほとんど何も載っかっていないためここにいろいろ追加していく必要があります. ここでは主にユーザランドプログラムの追加について解説します.

=== 既存コマンドの追加: ping

一度でも BUILD スクリプトを走らせた後には @<i>{amd64/bin} の下にコンパイル済みのユーザランドプログラムが大量に生成されています.
これらは Plan 9 上で動作するコマンドとしてビルドされており, @<list>{add_ping}にあるようにカーネルコンフィグの Ramfiles に追記することで, カーネルイメージに含めることができます.

//list[add_ping][pingをramfsに追加する (@<i>{sys/src/9/k10/k8cpu.json})]{
....
                "Ramfiles": {
                        "bind": "/amd64/bin/bind",
                        "boot": "boot.fs",
                        "cat": "/amd64/bin/cat",
                        "date": "/amd64/bin/date",
                        "echo": "/amd64/bin/echo",
                        "ipconfig": "/amd64/bin/ip/ipconfig",
                        "listen1": "/amd64/bin/aux/listen1",
                        "ls": "/amd64/bin/ls",
                        "mount": "/amd64/bin/mount",
+                       "ping": "/amd64/bin/ip/ping",
                        "rc": "/amd64/bin/rc",
....
//}

前述の通り ./BUILD all を走らせた後であれば ping 自体のバイナリ(@<i>{amd64/bin/ip/ping})は生成されているため, カーネルの再ビルド(ramfs部分の再生成)だけで今回はOKです.

//cmd{
./BUILD kernel
//}

起動後, 適当にIPアドレスを設定してやると以下の様にpingコマンドが追加され実行できているのが確認できます@<fn>{ping_test}.
//footnote[ping_test][今回はまだ QEMU に tap の設定を入れていないため外に出られません. なので自分自身にpingを当てています]

//cmd{
harvey@cpu% ipconfig -g 192.168.0.1 ether /net/ether0 192.168.0.130 255.255.255.0

harvey@cpu% ping 192.168.0.130
sending 32 64 byte messages 1000 ms apart to icmp!192.168.0.130!1
0: rtt 3246 µs, avg rtt 3246 µs, ttl = 255
1: rtt 2475 µs, avg rtt 2860 µs, ttl = 255
2: rtt 518 µs, avg rtt 2079 µs, ttl = 255
3: rtt 614 µs, avg rtt 1713 µs, ttl = 255
4: rtt 568 µs, avg rtt 1484 µs, ttl = 255
....
//}

=== 新規コマンドの追加: bffileserver

前項で述べたpingはすでにHarvey OS のビルドターゲットとなっていたプログラムであり, これを新規にramfsに加えて触れるようにしてみた, というものでした.
この節では, 自分で何らかプログラムを作って or 外部から拾ってきて組み込む場合に必要となる手順について bffileserver@<fn>{bffileserver_note}の追加を例に解説します.

//footnote[bffileserver_note][https://github.com/enukane/bffileserver :Plan 9っぽくファイルサーバにしたbrainf*ckインタプリタ]


==== bffileserverの準備

まず, bffileserverのソースを入手してきます. 当該コードは Plan 9 ベースでは動作確認が取れているので直接 Harvey OS のソースにねじ込みます. bffileserverはいくつかのソースファイルから成るので, cmds.jsonに混ぜずに個別にビルドコンフィグを作成する方針とします.

//cmd{
% cd sys/src/cmd
% git clone https://github.com/enukane/bffileserver.git
//}


次に当該ディレクトリにjsonのビルドコンフィグ (@<i>{sys/src/cmd/bffileserver/bffileserver.json}) を新規追加します.
前章で述べた JSON 形式 & ディレクティブのルールに従って, 名前やターゲットファイルなどを @<list>{bffileserver_json}のように記述します.

//list[bffileserver_json][bffileserver用JSONビルドコンフィグ (@<i>{sys/src/cmd/bffileserver/bffileserver.json}) の追加]{
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

//list[diff_bffileserver_cmds][@<i>{sys/src/cmd/cmds.json} への変更]{
....
        "Projects": [
                "9660srv/9660srv.json",
                "9nfs/9nfs.json",
                "astro/astro.json",
                "auth/auth.json",
                "aux/aux.json",
+               "bffileserver/bffileserver.json",
                "bzip2/bzip2.json",
                "cb/cb.json",
                "cdfs/cdfs.json",
....
//}


次に, ramfsに含めるために k8cpu.json に対して bffileserver を追加します. bffileserver.json の Post ディレクティブに記載したように bffileserver のバイナリは @<i>{amd64/bin/} に配置されるため, このパスを指定します.

//list[diff_bffileserver_k8cpu][@<i>{sys/src/9/k10/k8cpu.json} への変更]{
....
                "Ramfiles": {
+                       "bffileserver": "/amd64/bin/bffileserver",
                        "bind": "/amd64/bin/bind",
                        "boot": "boot.fs",
                        "cat": "/amd64/bin/cat",
                        "date": "/amd64/bin/date",
....
//}

なお, pingがそうであるように Post ディレクティブの中で @<i>{amd64/bin/bffileserver/bffileserver} といったディレクトリを作りその中にバイナリを置くことも可能です. この場合は, Ramfiles の記述を変える必要があります.

==== ビルド

今回は cmds.json に変更が加わっているため, まずはユーザランドプログラムの再ビルドを行ってからさらにカーネルのビルドを順に行う必要があります.

//cmd{
1. ユーザランド(含む bffileserver)のビルド
% ./BUILD cmd

2. カーネルのビルド(今回は主にramfsの再構成を目的として)
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
harvey@cpu% echo '+++++++++[>++++++++>+++++++++++>+++++<<<-]>.>++.+++++++..+++.>-.----
--------.<++++++++.--------.+++.------.--------.>+.' > /mnt/cmd

6. 結果が正しく返ってくることの確認(改行コードはないことに注意)
harvey@cpu% cat /mnt/data
Hello, world!harvey@cpu% 
//}


== カーネルランドの拡張

=== 新規ドライバの追加: ethervirtio

#@# 前置き: さすがに前節のdevbrainfuckは実用性がなさ過ぎるので

ここでは, 実用の可能性があるドライバとして ethervirtio (virtio-net) ドライバを Harvey OS に追加してみます.

==== ethervirtioドライバの準備

今回用いるethervirtioのソースは, 9legacy@<fn>{9legacy} にて patch として配布されている Nick Owens 氏作のものを用います. 以下のURLのdiffから ethervirtio.c を切り出します. 当該ドライバは仮想環境, 特にQEMU/KVM上でしか利用しないものなので @<i>{sys/src/9/k10/ethervirtio.c} に配置します.

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
//list[ethervirtio_k8cpu_p9_add][ethervirtio: @<i>{sys/src/9/k10/k8cpu} への変更]{
 ##     etherbcm        pci ethermii
        ether8139       pci ethermii
        ethermedium
+       ethervirtio     pci
        loopbackmedium
 #      usbuhci
 #      usbohci
//}


今回はカーネル側のみのため, k8cpu.json に ethervirtio をビルド対象に含めるよう Devディレクティブと SourceFiles ディレクティブにそれぞれ必要な項目を追記しています.

//list[ethervirtio_k8cpu_add][ethervirtio: @<i>{sys/src/9/k10/k8cpu.json} への変更]{
....
                        "Link": [
                                "ether8169",
                                "ether82557",
                                "ether82563",
                                "etherigbe",
                                "etherigbe",
                                "ether8139",
                                "ethermedium",
+                               "ethervirtio",
                                "loopbackmedium",
                                "netdevmedium"
                        ],
....
        "SourceFiles": [
                "devusb.c",
                "ether8139.c",
                "ether82563.c",
+               "ethervirtio.c",
                "k8cpu.c",
                "sdata.c",
                "usbehcipc.c",
....
//}

==== ビルド

今回はカーネルのみに関わる変更のため, 以下の通りkernelのビルドのみ実施します.

//cmd{
./BUILD kernel
//}


==== 起動スクリプトの新設

これまで起動スクリプトとして用いてきた QRUN は rtl8139 を用いているため, これをvirtioに切り替えます. QRUN_virtio として以下の様なスクリプトをもう一つ @<i>{util/QRNU_virtio} に追加します.

//list[QRUN_virtio][virtio-netを有効にした起動スクリプト (@<i>{util/QRUN_virtio}) の追加]{
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

