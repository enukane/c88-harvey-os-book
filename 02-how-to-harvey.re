= Harvey OS 環境構築
#@# NOT YET

この章では, Harvey OS で遊ぶための環境作りを解説します. また実際にHarvey OSを起動できるところまでを確認します.

== 想定環境

#@# 想定環境の提示
#@# 一番めんどくさい環境として
#@# clangには対応してないので、クロスのgccがひつよう

== クロスコンパイル環境のインストール

Harvey OS のビルドには "x86_64-elf-gcc"が必要です. 公式のドキュメントにもあるとおりそれぞれLinux, Mac OS X では以下の様にこのクロスコンパイラの準備が可能です.
#@# x86_64-elf-gccが必要
#@# binutils-2.25とgcc-4.9.3を用意する

#@# ~/crossに用意

=== binutilsのインストール

=== gcc-4.9.3

== Harvey OS
=== インストール

=== ビルド

ビルドにはソースディレクトリ直下にある BUILD スクリプトを用います. MacBookAir (Mid2012, Core i5 1.8GHz)において, cleanall && all のクリーンビルドで3分程度かかりました.

//cmd{
1. 環境変数の設定(必須)
% export TOOLPREFIX=x86_64-elf-
% export HARVEY=$(pwd)
% export ARCH=amd64

2.a 全体をビルドする場合
% ./BUILD all
または
% ./BUILD cleanall && ./BUILD all

2.b カーネルのみビルドする場合
% ./BUILD kernel
または
% ./BUILD cleankernel && ./BUILD kernel

2.c ユーザランドのみビルドする場合
% ./BUILD cmd
または
% ./BUILD cleancmd && ./BUILD cmd
//}

その他細かいビルド対象については BUILD スクリプトのヘルプ(引数無し実行)に記載されています. もう少し細かいビルドの流れについては第三章「Harvey OSのビルド概要」にて取り扱います.


=== 実行と終了

ビルドした Harvey OS は以下のコマンドにて起動可能です.
//cmd{
% (cd sys/src/9/k10 && sh ../../../../util/QRUN)
//}
終了するには、上記コマンドを実行したコンソール内で Ctrl + C (SIGINT) を入力します.


QRUNの中身は@<list>{QRUN}にあるとおり, qemu自体の実行と各種引数(パラメータ)の設定になっています. デフォルトのメモリ量として 2GB (2048) を指定していますが本稿では512MB (-m 512) に変更しています.

//list[QRUN][QRUNの内容(抜粋)]{
sudo qemu-system-x86_64 -s -cpu Opteron_G1 -smp 1 -m 2048  \
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
% (cd sys/src/9/k10 && sh ../../../../util/QRUN)

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


