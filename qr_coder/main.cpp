/* 
 * File:   main.cpp
 * Author: yashren
 *
 * Created on 23 July 2016, 3:22 PM
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char * bit_stream;
int bit_counter = 0;

const short int version_capacity_byte_mode[40] = {
    17,32,53,78,106,134,154,192,230,271,321,367,425,458,520,586,644,718,792,858,929,1003,1091,1171,1273,1367,1465,1528,1628,1732,1840,1952,2068,2188,2303,2431,2563,2699,2809,2953
};

const short int version_error_corr_words[40] = {
    19,34,55,80,108,136,156,194,232,274,324,370,428,461,523,589,647,721,795,861,932,1006,1094,1174,1276,1370,1468,1531,1631,1735,1843,1955,2071,2191,2306,2434,2566,2702,2812,2956
};

void printstream(void){
    printf("%s\r\n",bit_stream);
}

void append2bitstream(char data, char num_elements, char start_bit){
    // append num_elements from lsb of data to bit_stream
    int i = 0;
     
    for (i = 0; i < num_elements; i++){
        if (data & start_bit)
            bit_stream[bit_counter++] = '1';
        else
            bit_stream[bit_counter++] = '0';
        
        data <<= 1; // shift data up once
    }
    printf("bit counter = %d\r\n",bit_counter);
    printstream();
}

void generate_qrcode(char * string){
    int length_bytes = strlen(string);
    
    // determine smallest version to use
    int version = 0;
    int i = 0;
    for (i = 0; i < 40; i++){
        if (version_capacity_byte_mode[i] > length_bytes){
            version = i + 1;
            bit_stream = (char*) calloc (version_capacity_byte_mode[i], sizeof(char));
            break;
        }
    }
    if (version == 0){
        printf("Message too large\r\n");
        exit(-1);
    }
    // mode set by default to byte
    char mode = 0b0100;
    append2bitstream(mode,4,0x08);
    
    // if version is between 1-9 use 8 bits else use 16
    char byte_mode = 8;
    if (version > 9)
        byte_mode = 16;
    
    // append character counter
    if (byte_mode == 8){
        append2bitstream((char)length_bytes,8,0x80);    
    }
    else{
        append2bitstream((char)length_bytes,8,0x80);
        append2bitstream((char)(length_bytes >> 8),8,0x80);
    }
    
    // append text
    for (i = 0; i < length_bytes; i++){
        append2bitstream(string[i],8,0x80);
    }
    
    // check if divisible by 8, if not pad
    char rem = bit_counter % 8;
    for (i = 0; i < rem; i++){
        append2bitstream(0x00,1,0x01);
    }
    
    // pad bytes for correction level
    int bytes2pad = (version_error_corr_words[version-1]*8 - bit_counter)/8;
    bool toggle = true;
    for (i = 0; i < bytes2pad; i++){
        if (toggle){
            append2bitstream(236,8,0x80);    
            toggle = !toggle;
        }
        else{
            append2bitstream(17,8,0x80);
            toggle = !toggle;
        }
    }
        
        
    
    // determine required number of bits for qr code
    printf("\r\n%d\r\n",version_error_corr_words[version-1]*8);    
    
    printf("%d, %d\r\n", version, length_bytes);   
    //printf("%s\r\nlength: %d\r\n",string,length);   
}

int main(int argc, char** argv) {
    char * string = "HELLO WORLD";//"Hello world I am testing the version number";
    
    // Only doing the byte implementation of encoding
    // Only low correction level
    generate_qrcode(string);

    return 0;
}

