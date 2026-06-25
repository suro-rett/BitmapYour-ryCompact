#pragma once
#include <windows.h>
#include <vector>
#include <memory>

//BitmapYC ver1.0

using namespace Gdiplus;

#define NOTMATCH			0b00000000	//何も一致せず

//別の色同士なら複合する
#define BLUEMATCH			0b10000000	//B値が一致
#define GREENMATCH			0b01000000	//G値が一致
#define REDMATCH			0b00100000	//R値が一致
#define ALPHAMATCH			0b00010000	//A値が一致

#define BLUE255				0b00001000	//B値が全て255
#define GREEN255			0b00000100	//G値が全て255
#define RED255				0b00000010	//R値が全て255
#define ALPHA255			0b00000001	//A値が全て255

#define BLUE0				0b10001000	//B値が全て0
#define GREEN0				0b01000100	//G値が全て0
#define RED0				0b00100010	//R値が全て0
#define ALPHA0				0b00010001	//A値が全て0

enum class COLLER{
	BLUE = 0,
	GREEN = 1,
	RED   = 2,
	ALPHA = 3
};


class BitmapYC
{
public:
	BitmapYC(IN Bitmap* bitmap) {
		Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());

        BitmapData data;

        bitmap->LockBits(&rect,ImageLockModeRead,PixelFormat32bppARGB,&data);

		Width = data.Width;
		Height = data.Height;
		Stride = data.Stride;

		BYTE* ptr = (BYTE*)data.Scan0;

		UINT height = 0;
		UINT width = 0;

		for (height = 0; height < Height; height += 2) {
			for (width = 0; width < (UINT)Stride; width += 8) {
				byte.push_back(NOTMATCH);

				UINT HSW[4] = {height * Stride + width,				//左上pxのバイト位置
							   height     * Stride + width + 4 ,	//右上pxのバイト位置
							   (height + 1) * Stride + width ,		//左下pxのバイト位置
							   (height + 1) * Stride + width + 4 };	//右下pxのバイト位置


				if (ptr[HSW[0] + 3] == 0 && ptr[HSW[1] + 3] == 0
					&& ptr[HSW[2] + 3] == 0 && ptr[HSW[3] + 3] == 0) {
					byte.back() = ALPHA0;
				}
				else {
					if (ptr[HSW[0]] == ptr[HSW[1]] &&
						ptr[HSW[1]] == ptr[HSW[2]] &&
						ptr[HSW[2]] == ptr[HSW[3]]) {
						if (ptr[HSW[0]] == 255) {
							byte.back() += BLUE255;
						}
						else if (ptr[HSW[0]] == 0) {
							byte.back() += BLUE0;
						}
						else {
							byte.back() += BLUEMATCH;
						}
					}


					if (ptr[HSW[0] + 1] == ptr[HSW[1] + 1] &&
						ptr[HSW[1] + 1] == ptr[HSW[2] + 1] &&
						ptr[HSW[2] + 1] == ptr[HSW[3] + 1]) {
						if (ptr[HSW[0] + 1] == 255) {
							byte.back() += GREEN255;
						}
						else if (ptr[HSW[0] + 1] == 0) {
							byte.back() += GREEN0;
						}
						else {
							byte.back() += GREENMATCH;
						}
					}


					if (ptr[HSW[0] + 2] == ptr[HSW[1] + 2] &&
						ptr[HSW[1] + 2] == ptr[HSW[2] + 2] &&
						ptr[HSW[2] + 2] == ptr[HSW[3] + 2]) {
						if (ptr[HSW[0] + 2] == 255) {
							byte.back() += RED255;
						}
						else if (ptr[HSW[0] + 2] == 0) {
							byte.back() += RED0;
						}
						else {
							byte.back() += REDMATCH;
						}
					}


					if (ptr[HSW[0] + 3] == ptr[HSW[1] + 3] &&
						ptr[HSW[1] + 3] == ptr[HSW[2] + 3] &&
						ptr[HSW[2] + 3] == ptr[HSW[3] + 3]) {
						if (ptr[HSW[0] + 3] == 255) {
							byte.back() += ALPHA255;
						}
						else{
							byte.back() += ALPHAMATCH;
						}

					}
				}


				auto header = byte.back();

				if (header == ALPHA0) {		//ピクセルが透明の場合
					//バイト情報無し
				}
				else {
					if ((header & BLUE0) == BLUE0) {
						//バイト情報無し
					}
					else if ((header & BLUE255)) {
						//バイト情報無し
					}
					else if (header & BLUEMATCH) {	//BLUEMATCH
						byte.push_back(ptr[HSW[1]]);
					}
					else {
						SetData(ptr, HSW, 0);
					}

					if ((header & GREEN0) == GREEN0) {
						//バイト情報無し
					}
					else if ((header & GREEN255)) {
						//バイト情報無し
					}
					else if (header & GREENMATCH) {	//GREENMATCH
						byte.push_back(ptr[HSW[0] + 1]);
					}
					else {
						SetData(ptr, HSW, 1);
					}

					if ((header & RED0) == RED0) {
						//バイト情報無し
					}
					else if ((header & RED255)) {
						//バイト情報無し
					}
					else if (header & REDMATCH) {	//REDMATCH
						byte.push_back(ptr[HSW[1] + 2]);
					}
					else {
						SetData(ptr, HSW, 2);
					}

					if ((header & ALPHA0) == ALPHA0) {
						//バイト情報無し
					}
					else if ((header & ALPHA255)) {
						//バイト情報無し
					}
					else if (header & ALPHAMATCH) {	//ALPHAMATCH
						byte.push_back(ptr[HSW[1] + 3]);
					}
					else {
						SetData( ptr, HSW, 3);
					}
				}
			}
		}
		bitmap->UnlockBits(&data);
	}

	//圧縮したデータを復元して返す
	std::unique_ptr<Bitmap> getYC() {
		auto bitmap = std::make_unique<Bitmap>(Width,Height,PixelFormat32bppARGB);
		Rect rect(0, 0, Width,Height);

		BitmapData data;

		bitmap->LockBits(&rect, ImageLockModeWrite, PixelFormat32bppARGB, &data);

		BYTE* ptr = (BYTE*)data.Scan0;

		int next[3] = { 0,0 ,0};		//次のbyteの位置、次のBitmapポインタの位置、現在の行

		
		
		while(byte.size() > next[0]){
			if ((byte[next[0]] & ALPHA0) == ALPHA0) {
				BYTE Clear[4] = { 0,0,0,0 };
				SetPXColor(ptr, next[1]		+ Stride *  next[2]		, Clear);
				SetPXColor(ptr, next[1]		+ Stride * (next[2] + 1), Clear);
				SetPXColor(ptr, next[1] + 4 + Stride *  next[2]		, Clear);
				SetPXColor(ptr, next[1] + 4 + Stride * (next[2] + 1), Clear);

				next[0] ++;
			}
			else {
				BYTE pxCollar[4][4] = { 0 };//1番目引数は左上右上左下右下とどのピクセルかを
											//2番目引数は各PXのBGRAの色情報
				int nextByte = next[0] + 1;
				BYTE header = byte[next[0]];	//header座標

				if ((header & BLUE0) == BLUE0) {		//BLUE0
					SetColor(pxCollar, (int)COLLER::BLUE, 0);	//headerがBLUE0なら全てのpxのblueに0を代入
				}
				else if ((header & BLUE255)) {		//BLUE255
					SetColor(pxCollar, (int)COLLER::BLUE, 255);	//headerがBLUE255なら全てのpxのblueに255を代入
				}
				else if (header & BLUEMATCH) {		//blueMATCH
					SetColor(pxCollar, (int)COLLER::BLUE, nextByte, true);	//headerの後ろ1つ目のデータを全てのpxのblueに代入
					nextByte++;
				}
				else {
					SetColor(pxCollar, (int)COLLER::BLUE, nextByte, false);	//ばらばらなため、header後ろ4つのデータを各pxに代入で返す
					nextByte += 4;
				}

				if ((header & GREEN0) == GREEN0) {	//GREEN0
					SetColor(pxCollar, (int)COLLER::GREEN, 0);
				}
				else if ((header & GREEN255)) {		//GREEN255
					SetColor(pxCollar, (int)COLLER::GREEN, 255);
				}
				else if (header & GREENMATCH) {		//GREENMATCH
					SetColor(pxCollar, (int)COLLER::GREEN, nextByte, true);
					nextByte++;
				}
				else {
					SetColor(pxCollar, (int)COLLER::GREEN, nextByte, false);
					nextByte += 4;
				}

				if ((header & RED0) == RED0) {		//RED0
					SetColor(pxCollar, (int)COLLER::RED, 0);
				}
				else if ((header & RED255) ) {		//RED255
					SetColor(pxCollar, (int)COLLER::RED, 255);
				}
				else if (header & REDMATCH) {		//REDMATCH
					SetColor(pxCollar, (int)COLLER::RED, nextByte, true);
					nextByte++;
				}
				else {
					SetColor(pxCollar, (int)COLLER::RED, nextByte, false);
					nextByte += 4;
				}

				if ((header & ALPHA0) == ALPHA0) {	//ALPHA0←これらの処理の前にif文でALPHA0とそれ以外に分けているため基本通らない
					SetColor(pxCollar, (int)COLLER::ALPHA, 0);
				}
				else if ((header & ALPHA255)) {		//ALPHA255
					SetColor(pxCollar, (int)COLLER::ALPHA, 255);
				}
				else if (header & ALPHAMATCH) {		//ALPHAMATCH
					SetColor(pxCollar, (int)COLLER::ALPHA, nextByte, true);
					nextByte++;
				}
				else {
					SetColor(pxCollar, (int)COLLER::ALPHA, nextByte, false);
					nextByte += 4;
				}

				SetPXColor(ptr, next[1]		+ Stride *  next[2],      pxCollar[0]);//空ビットマップの指定した位置にBGRAをセット　左上
				SetPXColor(ptr, next[1] + 4 + Stride *  next[2],	  pxCollar[1]);//右上
				SetPXColor(ptr, next[1]		+ Stride * (next[2] + 1), pxCollar[2]);//左下
				SetPXColor(ptr, next[1] + 4 + Stride * (next[2] + 1), pxCollar[3]);//右下

				next[0] = nextByte;
			}

			next[1] += 8;
			if (next[1] >= (int)Width * 4) {
				next[1] = 0;
				next[2] += 2;
			}
		}
		bitmap->UnlockBits(&data);

		return bitmap;
	}


private:
	std::vector<BYTE> byte = {};

	UINT Width;
	UINT Height;
	INT Stride;

	void SetData(BYTE* bitmapPtr, UINT* bitmapLocation, int BGRAnum)	//BGRAnum←Bなら0、Gなら1と0～4の数値
	{
		for (int i = 0; i < 4; i++) {
			byte.push_back(bitmapPtr[bitmapLocation[i] + BGRAnum]);
		}
	}

	void SetPXColor(BYTE bitmapPtr[4], int nextPtr, BYTE color[4]) {
		bitmapPtr[nextPtr    ] = color[0];
		bitmapPtr[nextPtr + 1] = color[1];
		bitmapPtr[nextPtr + 2] = color[2];
		bitmapPtr[nextPtr + 3] = color[3];
	}

	void SetColor(BYTE pxCollar[4][4], int BGRA, int collar) {
		for (int i = 0; i < 4; i++) {
			pxCollar[i][BGRA] = collar;
		}
	}

	void SetColor(BYTE pxCollar[4][4], int BGRA, int nextByte, bool match) {
		for (int i = 0; i < 4; i++) {
			pxCollar[i][BGRA] = byte[nextByte];
			if (!match) {
				nextByte++;
			}
		}
	}
};

