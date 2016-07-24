/* 
 * File:   qrencoder.h
 * Author: yashren
 *
 * Created on 24 July 2016, 10:35 AM
 */

#ifndef QRENCODER_H
#define	QRENCODER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

void printstream(void);
void append2bitstream(char data, char num_elements, char start_bit);
void generate_qrcode(char * string);


#ifdef	__cplusplus
}
#endif

#endif	/* QRENCODER_H */

