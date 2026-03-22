# HTTP/1.1 RFC7230 読解　勉強中

https://triple-underscore.github.io/RFC7230-ja.html

## parse

受信者は，受信したプロトコル要素を構文解析するときは、次のいずれも満たすような，どの値も構文解析できなければならない：
When a received protocol element is parsed, the recipient MUST be able to parse any value of＼

- 受信者の役割に適用可能である，
- 対応する ABNF 規則にて定義される文法に合致する，
- その要素に見合う長さである

要素の長さは RFC で定義されていないが、
Web サーバ実装者は 拒否する前提・最大値の設定 が必要。


一方危険なもの：

- Content-Length と実データが食い違う
- Host ヘッダーに NUL(\0) が入っている
- Transfer-Encoding の矛盾

これは安全性のため厳格に処理しなければならない。

## 送信

## state code

- 505
	- http/1.xに対応しているとき　2.x やABNFに対応しないとき
	- HTTP/1.0  → OK（互換あり）
	- HTTP/1.1  → OK
	- HTTP/2.0  → 505 Version Not Supported
	- HTTP/0.9  → 505 Version Not Supported
	- HTTP/1.99 → 1.1 より高いが主番号 1 なので互換扱いで OK
	- HTTP/3.0 → 505 Version Not Supported
	- 壊れた文字列 → 400 Bad Request か 505（解釈不能なら505）


# 改行について

| 種類             | 読むとき（受信）                              | 書くとき（送信）                      |
| -------------- | ------------------------------------- | ----------------------------- |
| **HTTP リクエスト** | **LF と CRLF の両方を改行として認識してよい** | **必ず CRLF（\r\n）で送信しなければならない** |
| **HTTP レスポンス** | 同じく **LF / CRLF どちらも認識してよい**          | **必ず CRLF（\r\n）で送信しなければならない** |
