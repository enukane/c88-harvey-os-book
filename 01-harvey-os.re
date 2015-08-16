= Harvey OS とは

この章では, さっくりと Harvey OS の概要について説明します.

== Harvey OS の概要

Harvey OS は Plan 9 を GCC や clang でビルドできるようにすることを目指しているプロジェクトです. 
これまでの Plan 9 では Plan 9 C Compiler とよばれる 8c や 9c の名前を持つコンパイラを Plan 9 上で用いてプログラムやカーネルなどのビルドを行ってきました. これを他の環境そして GCC でも可能にすることを目指しています.
Harvey OS の中心開発メンバには, 9legacy@<fn>{9legacy} メンテナの David du Colombier 氏や Akaros@<fn>{akaros} 開発メンバの一人である Ron Minnich氏等が加わっています.

//footnote[9legacy][http://www.9legacy.org]
//footnote[akaros][http://akaros.cs.berkeley.edu/]

//image[harvey-os-logo][Harvey OS のロゴ?][scale=2]

Harvey OS の開発自体は 2014年春頃から始まっていたようです. 2015年2月あたりから徐々に盛り上がり, 夏には USENIX ATC の BOF で現在のステータスについて発表が行われました@<fn>{usenix_bof}.

//footnote[usenix_bof][http://harvey-os.org/docs/Harvey-Usenix-2015-ATC-BOF-slides.pdf]

githubにもコードを公開@<fn>{github}していますが, 開発自体は gerrithub@<fn>{gerrithub}にて行っており, こちらを参照することがプロジェクト的には推奨されています. 本家 Plan 9 のリポジトリが見つけやすい位置になかったり, 9legacy があくまでWebで公開しているパッチ集だったり, plan9front の載っていた Google Code のサービス縮退に伴い独自 Mercurial リポジトリサーバに移動してしまったのに比べるとだいぶオープンにアクセスし易い位置にコードが存在します@<fn>{case_insensive}.
CI等への取り組みも盛んで, Travis CI を用いた自動ビルドテストはもちろん, Coverity を用いての静的解析もコミットやpushに紐付いて走るようになっています.

//footnote[github][https://github.com/Harvey-OS/harvey]
//footnote[gerrithub][https://review.gerrithub.io/#/admin/projects/Harvey-OS/harvey]
//footnote[case_insensive][9frontのそれも悪くはないのですが, case insensiveなOS Xだとcloneに失敗するので, それがないHarvey OS が大変にやりやすい...]


Plan 9 C Compiler から GCC への移行に加えてビルドシステムの刷新も行われています.
従来の Plan 9 では Mk とよばれる GNU make 相当のプログラムと Makefile 相当の mkfile とによってビルドが定義されていました.
Harvey OS ではこの部分に相当するシステムとして, 新たに build なる go で書かれたプログラムと JSON 形式のビルドコンフィグによる仕組みを導入しています.

== 関連技術: LP49との比較

同じ様に GCC でのビルドを目指していた Plan 9 派生プロジェクトとしては LP49@<fn>{lp49} があります.
Harvey OS との比較を考える場合, まず大きな違いはそれぞれの OS の構成の違いです. LP49 は Plan 9 を L4 マイクロカーネルの上のサーバとして実現するものです. 一方で Harvey OS は実体としてはオリジナルの Plan 9 同様古典的なモノリシックカーネルです.

//footnote[lp49][http://research.nii.ac.jp/H2O/LP49/]

次に大きな違いは「GCC でビルドできるようにする」にあたって用いた手段の違いです.
あるいはより詳細には, よく問題として上げられていた Plan 9 C Compiler がサポートしている無名フィールドや無名パラメータをどう GCC に持ってくるかの違いです.
LP49 ではこれにあたり力業で書き換えるという手法を取りました. 一方のHarvey OS では GCC-4.6 (2011年リリース)にて導入された plan9-extensions オプションに依存することにより書き換えを基本的には回避しています. LP49 の登場は GCC-4.6 の登場よりだいぶ前(2006年頃?), Harvey OS はだいぶ後(2014年頃)という差があるためこのような手法の差異が存在します.

またビルドシステムについても違いが存在します. LP49 でも mkfile は使いません. その代わりに GNU Make を用い新たに記述した Makefile をソースツリーのそこかしこに配置しています. Harvey OS は前述の通り JSON 形式の独自ビルドコンフィグと独自プログラムを用いてビルドを行っています.

== 本書での表記

本書で用いる各種フォーマント, 表記についてここにまとめます.

==== ファイルパス

特に明記しない限り, ファイルパスは Harvey OS を展開したソースディレクトリからの相対パスになります. またファイルパスには斜体を用います (例: @<i>{sys/src/9/k10} )

==== ファイルの内容

ファイルやテキストデータ, ソースコードの中身を例示する際には以下のようなリストを用い, 文中からの参照にあたって "@<list>{sample_file}" といった表記を用います.

//list[sample_file][テキストの例示]{
ここからはじまって

.... (中略を示す場合はドット4つを用いる)

ここまでが内容だった
//}


==== コマンドの実行結果

本書ではコマンドの実行結果の例示に以下のフォーマットを用います

//cmd{
黒背景に白背景はコマンドの実行とその結果の出力を示します
% echo hello
hello

ホスト上でのコマンド実行では以下のプロンプトを用います
host% 

以下のプロンプトが出ている場合は Harvey OS 上でのコマンド実行を示しています
harvey@cpu%
//}
