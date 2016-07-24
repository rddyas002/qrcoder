/* 
 * File:   main.cpp
 * Author: yashren
 *
 * Created on 23 July 2016, 3:22 PM
 */

#include "qrencoder.h"

int main(int argc, char** argv) {
    char * string = "Hello World";
    
    generate_qrcode(string);

    return 0;
}

