c88-harvey-os-book
==================

## なんだこれは
C88, 2015夏コミックマーケットで頒布した「Harvey OS 読本」の文章データになります.

印刷に使った完成原稿(pdf)は含まれていません. また一部のリソースは意図的に劣化ないし排除しています.
完成原稿はReVIEWのソースからコンパイルして生成してください.
なお印刷した完成原稿は OS X Yosemite 10.10.4, TexShop 3.52, review 1.60の環境でビルドしました.

## ビルドに必要なもの

- Ruby (1.9.3以上)
- [Re:VIEW](https://github.com/kmuto/review)

## ReVIEWへのパッチ

デフォルトのRe:VIEWで生成したpdfでは, 目次の文字色が赤くなってしまうため,
本書を生成するにあたっては以下の様なパッチを適用しました。

- Change TOC color from red to black (${REVIEWPATH}/lib/review/review.tex.erb)
```diff
@@ -41,6 +41,7 @@
 \usepackage[dvipdfm,bookmarks=true,bookmarksnumbered=true,colorlinks=true,%
             pdftitle={<%= values["booktitle"] %>},%
             pdfauthor={<%= values["aut"] %>}]{hyperref}
+\hypersetup{ linkcolor=black }

 \newenvironment{reviewimage}{%
   \begin{figure}[H]
```

## ビルド方法
このソースのあるディレクトリでmakeコマンドを叩けば```c88book.pdf```ができあがるはず！

## 内容物
ReVIEWで必要となる画像データやsty以外の内容物について

(TBD)
