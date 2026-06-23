#include "main.h"
#if defined pos
unsigned char SHA1(unsigned char * str1, int len, unsigned char * res)  
{  
#define rotateleft(x,n) ((x<<n) | (x>>(32-n)))  
#define rotateright(x,n) ((x>>n) | (x<<(32-n)))
  
    unsigned long int h0,h1,h2,h3,h4,a,b,c,d,e,f,k,temp,i,j,cnt;  
    unsigned char  str[30+100];
	
    h0 = 0x67452301;  
    h1 = 0xEFCDAB89;  
    h2 = 0x98BADCFE;  
    h3 = 0x10325476;  
    h4 = 0xC3D2E1F0;  
   
    for(cnt=0;cnt<len;cnt++)
    	str[cnt] = str1[cnt];
   	

  	int current_length = len; 
    int original_length = current_length;  
    str[current_length] = 0x80;  
    str[current_length + 1] = '\0';  
   
    char ic = str[current_length];  
    current_length++;  
   
    int ib = current_length % 64;  
    if(ib<56)  
        ib = 56-ib;  
    else  
        ib = 120 - ib;  
   
    for( i=0;i < ib;i++)  {  
        str[current_length]=0x00;  
        current_length++;  
    }  
    str[current_length + 1]='\0';  
   
    for( i=0;i<6;i++)  
    {  
        str[current_length]=0x0;  
        current_length++;  
    }  
    str[current_length] = (original_length * 8) / 0x100 ;  
    current_length++;  
    str[current_length] = (original_length * 8) % 0x100;  
    current_length++;  
    str[current_length+i]='\0';  
   
    int number_of_chunks = current_length/64;  
    unsigned long int word[80];  
    for(i=0;i<number_of_chunks;i++)  
    {  
        for(int j=0;j<16;j++)  
        {  
            word[j] = str[i*64 + j*4 + 0] * 0x1000000 + str[i*64 + j*4 + 1] * 0x10000 + str[i*64 + j*4 + 2] * 0x100 + str[i*64 + j*4 + 3];  
        }  
        for(j=16;j<80;j++)  
        {  
            word[j] = rotateleft((word[j-3] ^ word[j-8] ^ word[j-14] ^ word[j-16]),1);  
        }  
   
        a = h0;  
        b = h1;  
        c = h2;  
        d = h3;  
        e = h4;  
   
        for(int m=0;m<80;m++)  
        {  
            if(m<=19)  
            {  
                f = (b & c) | ((~b) & d);  
                k = 0x5A827999;  
            }  
            else if(m<=39)  
            {  
                f = b ^ c ^ d;  
                k = 0x6ED9EBA1;  
            }  
            else if(m<=59)  
            {  
                f = (b & c) | (b & d) | (c & d);  
                k = 0x8F1BBCDC;  
            }  
            else  
            {  
                f = b ^ c ^ d;  
                k = 0xCA62C1D6;   
            }  
   
            temp = (rotateleft(a,5) + f + e + k + word[m]) & 0xFFFFFFFF;  
            e = d;  
            d = c;  
            c = rotateleft(b,30);  
            b = a;  
            a = temp;  
   
        }  
   
        h0 = h0 + a;  
        h1 = h1 + b;  
        h2 = h2 + c;  
        h3 = h3 + d;  
        h4 = h4 + e; 				    
		}  				
		res[0] = 	(h0 >> 24) & 0xff;
		res[1] = 	(h0 >> 16) & 0xff;
		res[2] = 	(h0 >> 8 ) & 0xff;
		res[3] = 	 h0 & 0xff ;
		
		res[4] = 	(h1 >> 24) & 0xff;
		res[5] = 	(h1 >> 16) & 0xff;
		res[6] = 	(h1 >> 8 ) & 0xff;
		res[7] = 	 h1 & 0xff ;	

		res[8]  = 	(h2 >> 24) & 0xff;
		res[9]  = 	(h2 >> 16) & 0xff;
		res[10] = 	(h2 >> 8 ) & 0xff;
		res[11] = 	 h2 & 0xff ;

		res[12]  = 	(h3 >> 24) & 0xff;
		res[13]  = 	(h3 >> 16) & 0xff;
		res[14]  = 	(h3 >> 8 ) & 0xff;
		res[15]  = 	 h3 & 0xff ;
		
		res[16]  = 	(h4 >> 24) & 0xff;
		res[17]  = 	(h4 >> 16) & 0xff;
		res[18]  = 	(h4 >> 8 ) & 0xff;
		res[19]  = 	 h4 & 0xff ;
}  
#endif