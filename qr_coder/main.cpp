/* 
 * File:   main.cpp
 * Author: yashren
 *
 * Created on 23 July 2016, 3:22 PM
 */

#include "qrencoder.h"

int main(int argc, char** argv) {
    char * string = "HELLO WORLD";//"Hello world I am testing the version number";
    
    // Only doing the byte implementation of encoding
    // Only low correction level
    generate_qrcode(string);

    return 0;
}

