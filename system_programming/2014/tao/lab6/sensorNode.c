#include<stdio.h>
#include<stdlib.h>

struct bits{

            unsigned char b0:1 b1:1 b2:1 b3:1 b4:1 b5:1 b6:1 b7:1 b8:1 b9:1 b10:1
                          b11:1 b12:1 b13:1 b14:1 b15:1 b16:1 b17:1 b18:1
                          b19:1 b20:1 b21:1 b22:1 b23:1 b24:1 b25:1 b26:1
                          b27:1 b28:1 b29:1 b30:1 b31:1
}value;

union myByte{
            unsigned char byte[4];
            unsigned value; 
};
union muByte SN;
SN.value=0x11223344;


