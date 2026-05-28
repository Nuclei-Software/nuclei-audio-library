#include <string.h>
char *enc_argv[] = {
    "./bin/EVS_cod", 
    "-dtx", 
    "8", 
    "5900", 
    "48", 
    "audio/stv48n2_1s.raw", 
    "enc.192", 
    NULL};
char *dec_argv[] = {
    "./bin/EVS_dec", 
    "48", 
    "enc.192", 
    "dec.raw", 
    NULL};
