#include "qrencoder.h"

#define PATTERN_RESERVE 2
#define TIMING_RESERVE 3
#define DARK_RESERVE 4
#define FORMAT_RESERVE 10

char * bit_stream;
int bit_counter = 0;

char polynomial_coefficients[256] = {0};
char data_stream[256] = {0};
int data_stream_bytes = 0;
char error_correction_codes[256] = {0};

char ** QR_module;

const short int version_capacity_byte_mode[40] = {
    17,32,53,78,106,134,154,192,230,271,321,367,425,458,520,586,644,718,792,858,929,1003,1091,1171,1273,1367,1465,1528,1628,1732,1840,1952,2068,2188,2303,2431,2563,2699,2809,2953
};

const short int version_error_corr_words[40] = {
    19,34,55,80,108,136,156,194,232,274,324,370,428,461,523,589,647,721,795,861,932,1006,1094,1174,1276,1370,1468,1531,1631,1735,1843,1955,2071,2191,2306,2434,2566,2702,2812,2956
};

const short int correction_codewords[40] = {
	7, 10, 15, 20, 26,  36,  40,  48,  60,  72,  80,  96, 104, 120, 132, 144, 168, 180, 196, 224, 224, 252, 270, 300,  312,  336,  360,  390,  420,  450,  480,  510,  540,  570,  570,  600,  630,  660,  720,  750
};

const short int correction_blocks[40] = {
	1, 1, 1, 1, 1, 2, 2, 2, 2, 4,  4,  4,  4,  4,  6,  6,  6,  6,  7,  8,  8,  9,  9, 10, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25
};

void ReedSolomonGenerator(int degree);
uint8_t multiply(uint8_t x, uint8_t y);
void printstream(void);
void append2bitstream(char data, char num_elements, char start_bit);
int convertBitStream2Bytes(void);
void getRemainder(int data_size, int degree);
void drawPatterns(int modules, int version);

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
//    printf("bit counter = %d\r\n",bit_counter);
//    printstream();
}

void generate_qrcode(char * string){
    int length_bytes = strlen(string);
    
    printf("String to encode: \"%s\"\r\n", string);
    
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
    
    printstream();
    printf("Data bits: %d\r\n", bit_counter);
    
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
    
    int num_bytes = convertBitStream2Bytes();
    // now data_stream has the data in byte format...next append ECC
        
    // get error correction block length
    int block_len = correction_codewords[version - 1]/correction_blocks[version - 1];
    ReedSolomonGenerator(block_len);
    
    getRemainder(num_bytes, block_len);
    
    // append ECC
    memcpy(&data_stream[num_bytes], &error_correction_codes[0], block_len);
    
    for (i = 0; i < num_bytes + block_len; i++)
        printf("0x%.2x ", (uint8_t)data_stream[i]);
    
    data_stream_bytes = num_bytes + block_len;
    printf("\r\nTotal: %d bytes, %d bits\r\n", data_stream_bytes, (num_bytes + block_len)*8);

    // compute number of modules
    int modules = (version-1)*4+21;
    printf("QR code size: %d x %d\r\n", modules, modules);
    
    // next module placement
    QR_module = (char **) calloc(modules, sizeof(char *));
    for (i = 0; i < modules; i++)
        QR_module[i] = (char *) calloc(modules, sizeof(char));
    
    drawPatterns(modules, version);
}

void drawPatterns(int modules, int version){
    int i,j;
    
    for (i = 0; i < 7; i++){
        // draw finder pattern top-left
        QR_module[0][i] = PATTERN_RESERVE;
        QR_module[6][i] = PATTERN_RESERVE;        // top/bottom row
        QR_module[i][0] = PATTERN_RESERVE;
        QR_module[i][6] = PATTERN_RESERVE;        // left/right column
    
        // draw finder pattern top-right
        QR_module[0][modules - 7 + i] = PATTERN_RESERVE;
        QR_module[6][modules - 7 + i] = PATTERN_RESERVE;          // top/bottom row
        QR_module[i][modules - 7] = PATTERN_RESERVE;
        QR_module[i][modules - 1] = PATTERN_RESERVE;              // left/right column        
    
        // draw finder pattern bottom-left
        QR_module[modules - 7][i] = PATTERN_RESERVE;
        QR_module[modules - 1][i] = PATTERN_RESERVE;              // top/bottom row
        QR_module[modules - 7 + i][0] = PATTERN_RESERVE;
        QR_module[modules - 7 + i][6] = PATTERN_RESERVE;          // left/right column        
    }
    
    // draw insides of pattern
    for (i = 0; i < 3; i++){
        for (j = 0; j < 3; j++){
            QR_module[2 + i][2 + j] = PATTERN_RESERVE;
            QR_module[2 + i][modules -7 + 2 + j] = PATTERN_RESERVE;   
            QR_module[modules -7 + 2 + i][2 + j] = PATTERN_RESERVE;   
        }
    }
    
    // TODO: Alignments
    
    // Add timing patterns
    for (i = 7; i < modules - 7; i++){
        if (!(i % 2)){
            QR_module[6][i] = TIMING_RESERVE;
            QR_module[i][6] = TIMING_RESERVE;
        }
    }
    
    // Add dark module
    QR_module[4*version+9][8] = DARK_RESERVE;
    
    
    // Add format reserve areas
    for (i = 0; i < 8; i++){
        QR_module[8][modules - 8 + i] = FORMAT_RESERVE;
    }
    for (i = 0; i < 6; i++){
        QR_module[8][i] = FORMAT_RESERVE;
    }    
    for (i = 0; i < 6; i++){
        QR_module[i][8] = FORMAT_RESERVE;
    }        
    QR_module[7][8] = QR_module[8][7] = QR_module[8][8] = FORMAT_RESERVE;
    for (i = 0; i < 7; i++){
        QR_module[modules - 7 + i][8] = FORMAT_RESERVE;
    }
    
// TODO: version 7 and larger must have format area    
//    for (i = 0; i < 6; i++){
//        QR_module[modules - 9][i] = FORMAT_RESERVE;
//        QR_module[modules - 10][i] = FORMAT_RESERVE;
//        QR_module[modules - 11][i] = FORMAT_RESERVE;
//    }    
//    
    /*
    int row, col, col2, x, y;
    char data = 0;
    bool upwards = true;
    j = 0;      // control bit position
    // add data
    int byte_stream_index = 0;
    for (col = modules - 1; col >= 0; col -= 2){
        for (row = 0; row < modules; row++){
            for (col2 = 0; col2 < 2; col2++){
                x = col - col2;
                if (upwards){                
                    y = modules - row - 1;
                    // moving upwards and you hit the top then switch down
                    if (y == 0)
                        upwards = false;
                }
                else{
                    y = row;
                    // moving downwards and you hit the bottom then switch down
                    if (y == modules)
                        upwards = true;
                }
                
                if (data_stream[byte_stream_index] & (0x80 >> j++))
                    data = 1;
                else
                    data = 0;
                if (j == 8){
                    j = 0;
                    byte_stream_index++;
                }
                QR_module[y][x] = data;
            }
        }
    }
     */ 
    
//    for (i = 0; i < data_stream_bytes; i++){
//        for (j = 0; j < 8; j++){
//            if (data_stream[i] & (0x80 >> j))
//                data = 1;
//            else
//                data = 0;
//            
//            // placement
//            QR_module[row][col] = data; 
//            // check if next column is free
//            col--;
//            if (QR_module[row][col] == R)
//                
//        }
//    }
//    
//    for (r = modules - 1; r >= 0; r--){
//        
//    }
    
    // print matrix
    for (i = 0; i < modules; i++){
        for (j = 0; j < modules; j++){
            if ((QR_module[i][j] > 1) && (QR_module[i][j] < 10)){
                printf("*");
            }
            else if (QR_module[i][j] == FORMAT_RESERVE){
                printf("R");
            }
            else
                printf(" ");
        }
        printf("\r\n");
    }    
}

// ported from c++ https://github.com/nayuki/QR-Code-generator/tree/master/cpp/QrCode.cpp (line 567))
void ReedSolomonGenerator(int degree){
        
    polynomial_coefficients[degree - 1] = 1;
    	
    // coefficient of highest power dropped
    // rest stored in descending power i.e. polynomial_coefficient[0] corresponds to degree - 1
    int root = 1;
    int i,j;
    for (i = 0; i < degree; i++) {
    	// Multiply the current product by (x - r^i)
    	for (j = 0; j < degree; j++) {
            polynomial_coefficients[j] = multiply(polynomial_coefficients[j], (uint8_t)root);
            if (j + 1 < degree)
    		polynomial_coefficients[j] ^= polynomial_coefficients[j + 1];
    	}
    	root = (root << 1) ^ ((root >> 7) * 0x11D);  // Multiply by 0x02 mod GF(2^8/0x11D)
    }
    
//    for (i = 0; i < degree; i++)
//        printf("0x%x ",(unsigned char)polynomial_coefficients[i]);
//    printf("\r\n");
}

// ported from c++ https://github.com/nayuki/QR-Code-generator/tree/master/cpp/QrCode.cpp (line 606))
uint8_t multiply(uint8_t x, uint8_t y) {
    // Russian peasant multiplication
    int z = 0, i = 0;
    for (i = 7; i >= 0; i--) {
    	z = (z << 1) ^ ((z >> 7) * 0x11D);
	z ^= ((y >> i) & 1) * x;
    }

    if (z >> 8 != 0)
        exit(-1);
       
    return (uint8_t)(z);
}

int convertBitStream2Bytes(void){
    // only handle 256 bytes for now
    // TODO: expand make dynamic
    if (bit_counter > 256*8)
        exit(-1);
    
    int i = 0, j = 0, k = 0x80;
    // go through each byte
    for (i = 0; i < bit_counter; i++){      
        if (bit_stream[i] == '1'){
            data_stream[j] |= k;
        }            
        k >>= 1;        // shift modifying mask down
        
        if (k == 0){
            k = 0x80;   // bit to start modifying
            j++;
        }
    }
    
    return j;
}

void getRemainder(int data_size, int degree) {
    // Compute the remainder by performing polynomial division
    
//    data_stream[0] = 0x12;
//    data_stream[1] = 0x34;
//    data_stream[2] = 0x56;
//    
//    polynomial_coefficients[0] = 0x0f;
//    polynomial_coefficients[1] = 0x36;
//    polynomial_coefficients[2] = 0x78;
//    polynomial_coefficients[3] = 0x40;
//
//    data_size = 3;
//    
//    degree = 4;
    
    int i = 0, j = 0;
    for (i = 0; i < data_size; i++) {
        uint8_t factor = data_stream[i] ^ error_correction_codes[0];
        
        // shift error correction codes forward
        for (j = 0; j < degree - 1; j++){
            error_correction_codes[j] = error_correction_codes[j + 1];
        }
        error_correction_codes[degree - 1] = 0;

	for (j = 0; j < degree; j++)
            error_correction_codes[j] ^= multiply(polynomial_coefficients[j], factor);
        
    }  
    
//    for (j = 0; j < degree; j++)
//        printf("0x%x ",(unsigned char)error_correction_codes[j]);
//    printf("\r\n");              
}