= Harvey OS 環境構築

この章では, Harvey OS で遊ぶための環境作りを解説します. 実際にHarvey OSを起動できるところまでを確認します.
主に筆者環境であるMac OS X / Homebrew 環境について重点的に述べています.
基本的には Harvey OS 自体の README.md を見ればなんとかなります. gcc のソースビルド等したい場合は本章をご参照ください.

== 必要なパッケージの導入

OS X向けには bison, qemu, go を事前に入れておく必要があります.
なお Xcode の Command Line Tools を導入済みの場合, bison は Homebrew でインストールせずとも大丈夫です.

//cmd{
host% brew install bison
host% brew instlal qemu
host% brew go
//}

== OSX 向けクロスコンパイル環境の構築 (パッケージシステム利用)

Harvey OS のビルドには @<b>{x86_64-elf-gcc} が必要です. Homebrew 標準のリポジトリには導入されていないため, 下記の通り sevki/gcc_cross_comilers を tap しインストールします.

//cmd{
host% brew tap sevki/gcc_cross_compilers
host% brew install sevki/gcc_cross_compilers/x86_64-elf-gcc
//}


== OSX 向けクロスコンパイル環境の構築 (ソースビルド)

上記 tap を用いない場合, ソースから binutils と gcc をビルドする必要があります. 本稿ではそれぞれ以下のバージョンによる環境を構築します. 

 * binutils-2.25
 * gcc-4.93

クロスコンパイル環境はここでは @<i>{$HOME/cross} 以下に構築します. 環境変数として以下を設定しておきます.

//cmd{
host% export PREFIX=$HOME/cross
host% export TARGET=x86_64-elf
host% export PATH=$HOME/cross/bin:$PATH
host% export TOOLPREFIX=x86_64-elf-
//}

=== binutils の準備

binutils-2.25.tar.gz 自体は本家 (http://ftp.gnu.org/gnu/binutils/) から拾ってきます.

//cmd{

host% tar zxvf binutils-2.25.tar.gz

host% mkdir build-binutils
host% cd build-binutils

host% ../binutils-2.25/configure --target=$TARGET --prefix="$PREFIX" \
--with-sysroot --disable-nls --disable-werror

host% make

host% make install
//}

=== gcc の準備

gcc-4.9.3.tar.gz についてはMirrorのどこかから tar.gz ファイルを拾ってきます.
今回は http://ftp.tsukuba.wide.ad.jp/software/gcc/releases/gcc-4.9.3/ から取得しました.

//cmd{

host% tar zxvf gcc-4.9.3.tar.gz

host% mkdir build-gcc
host% cd build-gcc

host% ../gcc-4.9.3/configure --target=$TARGET --prefix="$PREFIX" --disable-nls \
--enable-languages=c,c++ --without-headers

host% make all-gcc
host% make all-target-libgcc
host% make install-gcc
host% make install-target-libgcc

host% ls ~/cross/bin/x86_64-elf-gcc
//}

== Harvey OS
=== インストール

Harvey OS は github 上と gerrithub 上のそれぞれにリポジトリを持っています. 基本的には gerrithub 側を推奨しているようです.

//emlist{
https://github.com/Harvey-OS/harvey.git
https://review.gerrithub.io/Harvey-OS/harve
//}

=== ビルド

ビルド時にはいくつか環境変数をセットしておく必要があります. 特に OS X でクロスコンパイラ利用時は TOOLPREFIX と ARCH の指定が必要です.

//cmd{
環境変数の設定(必須)
host% export HARVEY=$(pwd)
host% export TOOLPREFIX=x86_64-elf-
host% export ARCH=amd64
//}

ビルドにはソースディレクトリ直下にある BUILD スクリプトを用います. MacBookAir (Mid2012, Core i5 1.8GHz)において, cleanall && all のクリーンビルドで3分程度かかりました.

//cmd{
全体をビルドする場合
host% ./BUILD all
または
host% ./BUILD cleanall && ./BUILD all

カーネルのみビルドする場合
host% ./BUILD kernel
または
host% ./BUILD cleankernel && ./BUILD kernel

ユーザランドのみビルドする場合
host% ./BUILD cmd
または
host% ./BUILD cleancmd && ./BUILD cmd
//}

その他細かいビルド対象については BUILD スクリプトのヘルプ(引数無し実行)に記載されています. もう少し細かいビルドの流れについては第三章「Harvey OSのビルド概要」にて取り扱います.


=== 実行と終了

ビルドした Harvey OS は以下のコマンドにて起動可能です.
//cmd{
host% (cd sys/src/9/k10 && sh ../../../../util/QRUN)
//}
終了するには、上記コマンドを実行したコンソールにて Ctrl + C (SIGINT) を入力します.


QRUNの中身は@<list>{QRUN}にあるとおり, qemu自体の実行と各種引数(パラメータ)の設定になっています.
なおデフォルトのメモリ量として 2GB (2048) が指定されていますが, 本稿では512MB (-m 512) に変更しています.

//list[QRUN][QRUNの内容(抜粋)]{
sudo qemu-system-x86_64 -s -cpu Opteron_G1 -smp 1 -m 512  \
-serial stdio \
--machine pc \
-net nic,model=rtl8139 \
-net user,hostfwd=tcp::5555-:1522 \
-kernel 9k8cpu.32bit $*
//}

起動が正常に進むと@<img>{boot_seq0}のようにQEMUの画面にてプロンプトが上がってきます.

//image[boot_seq0][QEMUの画面]

一方、元の起動コマンドを実行した端末でも下記に示されるように,起動時の各種メッセージが表示されコンソールを触ることができます.Harvey OSでは主にこちらでの操作がメインになるでしょう.というのも, この段階では Harvey OS 自体に終了用のコマンドを組み込んでおらずCtrl+Cに頼り切りになること, またrio等のウィンドウシステムを上げていない限りはこちらで操作の方が利便性が高いためです.

//cmd{
host% (cd sys/src/9/k10 && sh ../../../../util/QRUN)

Harvey
mmuinit: vmstart 0xfffffffff0000000 vmunused 0xfffffffff037c000 vmunmapped 0xfffffffff040
0000 vmend 0xfffffffff4000000
sys->pd 0x108003 0x108023
cpu0: mmu l3 pte 0xfffffffff0106ff8 = 107023
cpu0: mmu l2 pte 0xfffffffff0107ff8 = 108023
cpu0: mmu l1 pte 0xfffffffff0108c00 = e3
cpu0: mmu l1 pte 0xfffffffff0108c00 = e3
asm: addr 0x0000000004000000 end 0x000000001ffe0000 type 1 size 469630976
cm 0: addr 0x4000000 npage 114656
480 223 0
npage 114656 upage 114656 kpage 16384
base 0xfffffffff037f000 ptr 0xfffffffff037f000 nunits 3965185
physalloc color=0 base=0x4000000 size=0x1bfe0000
apic0: hz 1000100000 max 10001000 min 100010 div 2
sipiptr 0xfffffffff0003000 sipipa 0x3000
bus dev type vid  did intl memory
0   0/0 06 00 00 8086 1237   0  
0   1/0 06 01 00 8086 7000   0  
0   1/1 01 01 80 8086 7010   0  4:0000c101 16 
0   1/3 06 80 00 8086 7113   9  
0   2/0 03 00 00 1234 1111   0  0:fd000008 16777216 2:febd0000 4096 
0   3/0 02 00 00 10ec 8139  11  0:0000c001 256 1:febd1000 256 
#l0: rtl8139: 100Mbps port 0xc000 irq 11 tu 1514: 525400123456
bootcore: all cores done
CPU Freq. 2294MHz
schedinit...
Hello, I am Harvey :-)
listen started
harvey@cpu%
//}


