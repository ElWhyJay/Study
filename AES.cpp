#include <iostream>
#include <fstream>
#include <stdint.h>
#include <algorithm>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <stdlib.h> 
#pragma warning(disable:4996)
using namespace std;
//x^8+x^6+x^5+x^3+1 101101001

/*functions*/
void keyexpansion();
void SubstituteBytes();
void ShiftRows();
void MixColumns();
void AddRoundKey(uint8_t*);
void InverseSubstituteBytes();
void InverseShiftRows();
void InverseMixColumns();
void sboxgenerate();
void inverseSboxgenerate();
uint8_t Roundfunction(uint8_t);
int msb(int);
uint8_t binarytohex(uint8_t *);
uint8_t *hextobinary(uint8_t);
uint8_t ExtendedEuclideanAlgo(uint8_t); // compute inverse of 00 ~ FF about  irr(101101001;GF(2^8))
uint8_t gmul(uint8_t a, uint8_t b);
/*global Variables*/
//uint8_t key[11][16] = {0x00 ,0x11 ,0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb ,0xcc ,0xdd ,0xee ,0xff} ;
//uint8_t state[4][4] ={ {0x00,0x40,0x80,0xc0},{0x10,0x50,0x90,0xd0},{0x20,0x60,0xa0,0xe0},{0x30,0x70,0xb0,0xf0} }; //sample
uint8_t key[11][16];
uint8_t sconstan[8] = {1,0,1,0,1,0,0,0};
uint8_t inversesconstan[8] = { 1,1,0,1,0,1,1,0 }; // inverse of 0x15 is 0x6B;;
uint8_t irrpoly = 0x169;
uint8_t sbox[16][16];
uint8_t inversesbox[16][16];
uint8_t galoitable[255];
uint8_t word[44][4];
uint8_t state[4][4];
uint8_t RC[10];
int main()
{
	FILE *plain,*keys; 
	uint8_t temp,temp2;
	int m=0, n = 0;
	if ((plain = fopen("plain.bin", "r")) == NULL) return -1;
	if ((keys = fopen("key.bin", "r")) == NULL) return -2; // open binary files
	printf("Plain: ");
	while ((temp = fgetc(plain)) != EOF)  //read plaintext
	{
		if (m == 4 && n == 3) break;
		if (m > 3) { m = 0; n++; }
		state[m][n] = temp;
		printf("%x ",temp);
		m++;
	}
	m = 0;
	printf("\nKey: ");
	while ((temp2 = fgetc(keys)) != EOF) // read key
	{
		if (m == 16) break;
		key[0][m] = temp2;
		printf("%x ", temp2);
		m++;
	}
	m = 0;
	RC[0] = 1; 
	printf("\nRC: %x ",(int)RC[0]);
	for (int i = 1; i < 10; i++)
	{
		RC[i] = gmul(RC[i - 1], 2);
		m = (int)RC[i];
		printf("%x ", m);
	}
	printf("\n\n\n-------------------------KEY EXPANSION-----------------------------\n");
	sboxgenerate(); //sbox 생성;
	keyexpansion(); //key expansion
	for (int i = 0; i < 11; i++)
	{
		printf("ROUND%d: ", i);
		for (int j = 0; j < 16; j++)
		{
			printf("%x ", (int)key[i][j]);
		}
		printf("\n");
	}
	printf("\nROUND0\nAR: ");
	AddRoundKey(key[0]);
	for (int i = 1; i < 10; i++) //Round 1 to 9
	{
		
		printf("\n\nRound %d\nSB: ",i);
		SubstituteBytes();
		printf("\nSR: ");
		ShiftRows();
		printf("\nMC: ");
		MixColumns();
		printf("\nAR: ");
		AddRoundKey(key[i]);
	}
	//Round 10 without shiftrows.
	printf("\n\nRound Last 10\nSB: ");
	SubstituteBytes();
	printf("\nSR: ");
	ShiftRows();
	printf("\nAR: ");
	AddRoundKey(key[10]);
	printf("\n\nCipher : ");
	FILE *cipher; 
	uint8_t t2[16];
	cipher = fopen("cipher.bin", "wb"); //cipher.bin 파일에 입력
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
		{
			t2[4 * i + j] = state[j][i];
			printf("%x ", (int)state[j][i]);
		}
	fwrite(t2, sizeof(uint8_t), 16, cipher);
	
	
	
	printf("\n\n\n----------------DECRYPTION--------------------------\n\n");  //Decryption
	inverseSboxgenerate(); // inverse sbox 생성
	printf("Round 0\nAR : ");
	AddRoundKey(key[10]);
	for (int i = 9; i > 0; i--)
	{
		printf("\n\nRound %d\nInverSR:  ", 10 - i); // 1~9 Round
		InverseShiftRows();
		printf("\nInverSB: ");
		InverseSubstituteBytes();
		printf("\nInverAR: ");
		AddRoundKey(key[i]);
		printf("\nInverMC: ");
		InverseMixColumns();
	}
	printf("\n\nRound 10\nInverSR: ");
	InverseShiftRows();
	printf("\nInverSB: ");
	InverseSubstituteBytes();
	printf("\nInverAR: ");
	AddRoundKey(key[0]);

	printf("\n\nPlain : ");
	FILE *decrypt; //decryptfile에 입력 
	decrypt = fopen("decrypt.bin", "wb");
	uint8_t t1[16];
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
		{
			t1[4*i+j] = state[j][i];
			printf("%x ", (int)state[j][i]);

		}
	fwrite(t1, sizeof(uint8_t), 16, decrypt);
	fclose(cipher);
	fclose(decrypt);
	fclose(plain);
	fclose(keys);
}//main 끝


void keyexpansion()
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			word[i][j] = key[0][4 * i + j];
		}
	}
	for (int i = 4; i < 44; i++)
	{
		uint8_t temp[4];
		if (i % 4 == 0)
		{
			for (int j = 0; j < 4; j++)
			{
				
				if (j == 0)
					word[i][0] = word[i - 4][0] ^ sbox[word[i - 1][1] / 16][word[i - 1][1] % 16] ^ RC[(i/4)-1];
				else
				{
					word[i][j] = word[i - 4][j] ^ sbox[word[i - 1][(j + 1) % 4] / 16][word[i - 1][(j + 1) % 4] % 16];
				}
				key[i / 4][(i%4)*4+j]=word[i][j];
			}

		}
		else
		{
			for (int j = 0; j < 4; j++)
			{
				word[i][j] = word[i - 1][j] ^ word[i - 4][j];
				key[i / 4][(i % 4) * 4 + j] = word[i][j];
			}
		}
	}
	
}

void sboxgenerate()//sbox inverse-sbox generate function
{
	uint8_t input, raw = 0, column = 0; int num;
	uint8_t output[8];
	uint8_t b[8];
	uint8_t m = 0, n = 0;
	for (int k = 0; k < 256; k++) //00 ~ FF까지
	{
		num = 0;
		input = ExtendedEuclideanAlgo(k); // multiply matrix by inverse of given number
		column = k % 16; raw = k / 16; // sbox raw,column
		for (int j = 0; j<8; j++)
		{
			b[j] = input % 2; // inverse num to binary
			input = input / 2;
		}
		int t;
		for (int i = 0; i < 8; i++)
		{
			output[i] = b[i] ^ b[(i + 4) % 8] ^ b[(i + 5) % 8] ^ b[(i + 6) % 8] ^ b[(i + 7) % 8] ^ sconstan[i]; // matrix multiple and add changed constant
			t = (int)output[i];
			num += t * pow(2, i); //sbox
		}
		sbox[raw][column] = num;
	}
}
void inverseSboxgenerate()
{
	uint8_t input, raw = 0, column = 0; int num;
	uint8_t output[8];
	uint8_t s[8];
	uint8_t b[8];
	uint8_t m = 0, n = 0;
	for (int k = 0; k < 256; k++) //00 ~ FF까지
	{
		num = 0;
		input = k; // multiply matrix by inverse of given number
		column = k % 16; raw = k / 16; // sbox raw,column
		for (int j = 0; j<8; j++)
		{
			b[j] = input % 2; // inverse num to binary
			input = input / 2;
		}
		for (int j = 0; j < 8; j++) 
		{
			b[j] ^= sconstan[j];
		}
		int t;
		for (int i = 0; i < 8; i++)
		{
			output[i] =b[(i+2)%8] ^ b[(i + 5) % 8] ^  b[(i + 7) % 8]; // inverse of affine transform
			/* inverse matrix of s box matrix
			
			*/
			t = (int)output[i];
			num += t * pow(2, i); 
		}
		num = (int)ExtendedEuclideanAlgo(num); // multiplitive inverse
		inversesbox[raw][column] = num;
	}
}
void SubstituteBytes()
{
	for(int i=0;i<4;i++)
		for (int j = 0; j < 4; j++)
		{
			state[j][i] = sbox[state[j][i]/16][state[j][i] % 16]; // read s box
		}

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			printf("%x ", (int)state[j][i]);
}
void InverseSubstituteBytes()
{
	for (int i = 0; i<4; i++)
		for (int j = 0; j < 4; j++)
		{
			state[j][i] = inversesbox[state[j][i] / 16][state[j][i] % 16]; // read s box
		}

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			printf("%x ", (int)state[j][i]);
}
void MixColumns()
{
	uint8_t tmp1, tmp2, tmp3, tmp4;
	for (int i = 0; i < 4; i++)
	{
		tmp1 = state[0][i];
		tmp2 = state[1][i];
		tmp3= state[2][i];
		tmp4= state[3][i];
		state[0][i] =gmul(2,tmp1)^gmul(3,tmp2)^tmp3^tmp4 ;
		state[1][i] = tmp1^gmul(2,tmp2)^gmul(3,tmp3)^tmp4;
		state[2][i] = tmp1^tmp2^gmul(2,tmp3)^gmul(3,tmp4);
		state[3][i] = gmul(3,tmp1)^tmp2^tmp3^gmul(2,tmp4);
	}
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			printf("%x ", (int)state[j][i]);
}
void InverseMixColumns()
{
	uint8_t tmp1, tmp2, tmp3, tmp4;
	for (int i = 0; i < 4; i++)
	{
		tmp1 = state[0][i];
		tmp2 = state[1][i];
		tmp3 = state[2][i];
		tmp4 = state[3][i];
		state[0][i] = gmul(14, tmp1) ^ gmul(11, tmp2) ^gmul(13, tmp3)^gmul(9,tmp4);
		state[1][i] = gmul(9, tmp1) ^ gmul(14, tmp2) ^ gmul(11, tmp3) ^ gmul(13, tmp4);
		state[2][i] = gmul(13, tmp1) ^ gmul(9, tmp2) ^ gmul(14, tmp3) ^ gmul(11, tmp4);
		state[3][i] = gmul(11, tmp1) ^ gmul(13, tmp2) ^ gmul(9, tmp3) ^ gmul(14, tmp4);
	}
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			printf("%x ", (int)state[j][i]);
}
void ShiftRows()
{
	uint8_t tmp[4];
	for (int i = 1; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			tmp[j] = state[i][j];
		for(int k=0;k<4;k++)
			state[i][k] = tmp[(k + i)%4];
	}
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			printf("%x ", (int)state[j][i]);
}
void InverseShiftRows()
{
	uint8_t tmp[4];
	for (int i = 1; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			tmp[j] = state[i][j];
		for (int k = 0; k < 4; k++)
			state[i][k] = tmp[(k +4-i) % 4];
	}
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			printf("%x ", (int)state[j][i]);
}
void AddRoundKey(uint8_t *k)
{
	for(int i =0;i<4;i++)
		for (int j = 0; j < 4; j++)
		{
			state[j][i] ^= k[4 * i + j];
			printf("%x ", (int)state[j][i]);
		}
}
//uint8_t binarytohex(int*arr)
//{
//	int k = sizeof(arr);
//	uint8_t num=0;
//	for (int i =0;i<k;i++)
//	{
//		if(arr[i])	num+=pow(2, i);
//	}
//	return num;
//}
//uint8_t *hextobinary(int num)
//{
//	uint8_t arr[8];
//	uint8_t tmp;
//	for(int i =0;num=0;i++)
//	{
//		arr[i] = num % 2;
//		num = num / 2;
//	}
//	return *arr;
//}

uint8_t ExtendedEuclideanAlgo(uint8_t a) // 주어진값의 inverse 를 반환
{
	int a3 = 361; // irr polynominal 0x169 or x^8+x^6+x^5+x^3+x+1
	int b3 = (int)a;
	int c1 = 0, a2 = 0, b2 = 0;
	int flag = 0;
	if (a == 1) return 1;
	while (b3 > 1)
	{
		int  q = 0;
		int tmp = b3;
		int q1;
		while ((a3>tmp) || (msb(a3) >= msb(tmp)))
		{
			q1 = 1;
			while (!(msb(b3) == msb(a3)))
			{
				b3 <<= 1;
				q1 *= 2;
			}
			q += q1;
			a3 ^= b3;
			b3 = tmp;
		}
		b3 = a3;
		a3 = tmp;
		if (!flag) b2 = 1;
		int n = msb(q);
		tmp = b2; int c2 = 0;
		while (n >= 1)
		{
			if (((1 << (n - 1)) & q) != 0)
			{
				c1 = (b2 << (n - 1));
				c2 ^= c1;
			}
			n--;
		}
		b2 = c2 ^ a2;
		a2 = tmp;
		flag++;
	}
	return (uint8_t)b2;
}
int msb(int x) //most set bit
{
	int a = x;
	int tmp = 0;
	while (1)
	{
		if (a == 0) break;
		a >>= 1;
		tmp++;
	}
	return tmp;
}
uint8_t galoidadd(int a, int b) // galois field 위에서의 덧셈은 xor
{
	return a ^ b;
}
uint8_t gmul(uint8_t a, uint8_t b) {
	uint8_t p = 0;
	uint8_t counter;
	uint8_t hi_bit_set;
	for (counter = 0; counter < 8; counter++) {
		if ((b & 1) == 1)
			p ^= a;
		hi_bit_set = (a & 0x80);
		a <<= 1;
		if (hi_bit_set == 0x80)
			a ^= 0x69;
		b >>= 1;
	}
	return p;
}