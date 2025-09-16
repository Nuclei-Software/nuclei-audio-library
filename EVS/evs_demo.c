#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "data/memfop.h"
#include "data/input.h"
#include "data/enc_192.h"
#include "data/dec_raw.h"

#include "evalsoc.h"
#include "nmsis_bench.h"
#include "prot_fx.h"
#include "stat_enc_fx.h"
#include "typedefs.h"

#define INPUT_LEN (sizeof(input))
#define ENC_192_LEN (sizeof(enc_192))
#define DEC_RAW_LEN (sizeof(dec_raw))

/*------------------------------------------------------------------------------------------*
 * Global variables
 *------------------------------------------------------------------------------------------*/
long frame;                 /* Counter of frames */

BENCH_DECLARE_VAR();

uint8_t enc_result[ENC_192_LEN] = {0};
uint8_t dec_result[DEC_RAW_LEN] = {0};

int verify_result(const uint8_t *ref, const uint8_t *res, int len,
                  int threshold) {
    for (int i = 0; i < len; i++) {
        if (abs(ref[i] - res[i]) > threshold) {
            printf(
                "Result mismatch at byte %d!, expected 0x%02x, got 0x%02x\r\n",
                i, ref[i], res[i]);
            return EXIT_FAILURE;
        }
    }
    printf("Result matches!\r\n");
    return EXIT_SUCCESS;
}

int encode() {
    MemoryFile *f_stream = NULL;                          /* output bitstream file */
    Indice_fx ind_list[MAX_NUM_INDICES];                  /* list of indices */
    Encoder_State_fx *st_fx;                              /*Encoder state struct*/
    Word16           enc_delay;
    MemoryFile *f_input;                                  /* input signal file */
    MemoryFile *f_rate;                                   /* bitrate switching profile file */
    MemoryFile *f_bwidth;                                 /* bandwidth switching profile file */
    MemoryFile *f_rf = NULL;                              /* Channel aware configuration file */
    Word16 input_frame;
    Word16 tmps;
    Word16 n_samples;
    Word16 data[L_FRAME48k];                              /* Input buffer */
    Word32 bwidth_profile_cnt = 0;                        /* counter of frames for bandwidth switching profile file */
    Word16 quietMode = 0;
    Word16 noDelayCmp = 0;
    Word16 Opt_RF_ON_loc, rf_fec_offset_loc;

    UWord8 pFrame[(MAX_BITS_PER_FRAME + 7) >> 3];
    Word16 pFrame_size = 0;

    // init frame count
    frame = 0;

    /* start WMOPS counting */
    BASOP_init
    BENCH_INIT();

    /*Inits*/
    f_bwidth = f_rate = NULL;

    /*------------------------------------------------------------------------------------------*
     * Allocate memory for static variables
     * Processing of command-line parameters
     * Encoder initialization
     *------------------------------------------------------------------------------------------*/

    if ( (st_fx = (Encoder_State_fx *) calloc( 1, sizeof(Encoder_State_fx) ) ) == NULL )
    {
        fprintf(stderr, "Can not allocate memory for encoder state structure\n");
        exit(-1);
    }

    int argc = 0;
    while (enc_argv[argc] != NULL) {
        argc++;
    }
    io_ini_enc_fx( argc, enc_argv, input, INPUT_LEN, &f_input,
                   enc_result, ENC_192_LEN, &f_stream, &f_rate, &f_bwidth,
                   &f_rf,
                   &quietMode, &noDelayCmp, st_fx);

    /*input_frame = (short)(st->input_Fs / 50);*/
    st_fx->input_frame_fx = extract_l(Mult_32_16(st_fx->input_Fs_fx , 0x0290));
    input_frame = st_fx->input_frame_fx;

    Opt_RF_ON_loc = st_fx->Opt_RF_ON;
    rf_fec_offset_loc = st_fx->rf_fec_offset;

    st_fx->ind_list_fx = ind_list;
    init_encoder_fx( st_fx );

    /*------------------------------------------------------------------------------------------*
     * Compensate for encoder delay (bitstream aligned with input signal)
     * Compensate for the rest of codec delay (local synthesis aligned with decoded signal and original signal)
     *------------------------------------------------------------------------------------------*/

    enc_delay = NS2SA(st_fx->input_Fs_fx, get_delay_fx(ENC, st_fx->input_Fs_fx));

    if ( noDelayCmp == 0 )
    {
        /* read samples and throw them away */
        if( (tmps = (Word16)memfread(data, sizeof(short), enc_delay, f_input)) != enc_delay )
        {
        }

    }

    /*------------------------------------------------------------------------------------------*
     * Loop for every frame of input data
     * - Read the input data
     * - Select the best operating mode
     * - Run the encoder
     * - Write the parameters into output bitstream file
     *------------------------------------------------------------------------------------------*/
    BASOP_end_noprint;
    BASOP_init;

#if (WMOPS)
    Init_WMOPS_counter();
    Reset_WMOPS_counter();
    setFrameRate(48000, 960);
#endif


    if (quietMode == 0)
    {
        fprintf( stdout, "\n------ Running the encoder ------\r\n" );
        fprintf( stdout, "Frames processed:       " );
    }
    else
    {
        fprintf( stdout, "\n-- Start the encoder (quiet mode) --\r\n" );
    }


    /*Encode-a-frame loop start*/
    while( (n_samples = (short)memfread(data, sizeof(short), input_frame, f_input)) > 0 )
    {
#if (WMOPS)
        Reset_WMOPS_counter();
#endif
        SUB_WMOPS_INIT("enc");

        IF(f_rf != NULL)
        {
            read_next_rfparam_fx(
                &st_fx->rf_fec_offset, &st_fx->rf_fec_indicator, f_rf);
            rf_fec_offset_loc = st_fx->rf_fec_offset;
        }

        IF(f_rate != NULL)
        {
            /* read next bitrate from profile file (only if invoked on the cmd line) */
            read_next_brate_fx( &st_fx->total_brate_fx, st_fx->last_total_brate_fx,
                                f_rate, st_fx->input_Fs_fx, &st_fx->Opt_AMR_WB_fx, &st_fx->Opt_SC_VBR_fx, &st_fx->codec_mode );
        }

        IF (f_bwidth != NULL)
        {
            /* read next bandwidth from profile file (only if invoked on the cmd line) */
            read_next_bwidth_fx( &st_fx->max_bwidth_fx, f_bwidth, &bwidth_profile_cnt, st_fx->input_Fs_fx );
        }

        IF( ( st_fx->Opt_RF_ON && ( L_sub( st_fx->total_brate_fx, ACELP_13k20 ) != 0 ||  L_sub( st_fx->input_Fs_fx, 8000 ) == 0 || st_fx->max_bwidth_fx == NB ) )
            || st_fx->rf_fec_offset == 0 )
        {
            IF( L_sub( st_fx->total_brate_fx, ACELP_13k20) == 0 )
            {
                st_fx->codec_mode = MODE1;
                reset_rf_indices(st_fx);
            }
            st_fx->Opt_RF_ON = 0;
            st_fx->rf_fec_offset = 0;

        }

        IF( Opt_RF_ON_loc && rf_fec_offset_loc != 0 && L_sub( st_fx->total_brate_fx, ACELP_13k20 ) == 0 && L_sub( st_fx->input_Fs_fx, 8000 ) != 0 && st_fx->max_bwidth_fx != NB )
        {
            st_fx->codec_mode = MODE2;
            IF(st_fx->Opt_RF_ON == 0)
            {
                reset_rf_indices(st_fx);
            }
            st_fx->Opt_RF_ON = 1;
            st_fx->rf_fec_offset = rf_fec_offset_loc;
        }

        /* in case of 8kHz sampling rate or when in "max_band NB" mode, limit the total bitrate to 24.40 kbps */
        IF ( (L_sub( st_fx->input_Fs_fx, 8000 ) == 0 || (st_fx->max_bwidth_fx == NB)) && L_sub( st_fx->total_brate_fx, ACELP_24k40 ) > 0 )
        {
            st_fx->total_brate_fx = ACELP_24k40;
            st_fx->codec_mode = MODE2;
        }


        /* run the main encoding routine */
        BENCH_START(evs_encode);

        IF ( st_fx->Opt_AMR_WB_fx )
        {
            SUB_WMOPS_INIT("amr_wb_enc");
            amr_wb_enc_fx( st_fx, data, n_samples);
            END_SUB_WMOPS;
        }
        ELSE
        {
            SUB_WMOPS_INIT("evs_enc");
            /* EVS encoder*/
            evs_enc_fx( st_fx, data, n_samples);
            END_SUB_WMOPS;
        }
        /* pack indices into serialized payload format */
        if( st_fx->bitstreamformat == MIME )
        {
            indices_to_serial(st_fx, pFrame, &pFrame_size);
        }

        /* write indices into bitstream file */
        write_indices_fx( st_fx, f_stream, pFrame, pFrame_size );

        END_SUB_WMOPS;
        BENCH_SAMPLE(evs_encode);
        /* update WMPOS counting (end of frame) */


        fflush(stderr);

        frame++;
        if (quietMode == 0)
        {
            fprintf( stdout, "%-8ld\b\b\b\b\b\b\b\b", frame );
        }

#if (WMOPS)
        fwc();
#endif
    }
    /* ----- Encode-a-frame loop end ----- */


    if (quietMode == 0)
    {
        fprintf( stdout, "\r\n" );
        fprintf(stderr, "Encoding finished\r\n");
    }
    else
    {
        fprintf(stderr, "Encoding of %ld frames finished\r\n", frame);
    }

    double avg_cycle = BENCH_GET_SUMCYC() * 1.0 / BENCH_GET_LPCNT();
    double mcps = avg_cycle * 50 / 1000000;
    printf("CSV, evs_encode, %.02f\r\n", mcps);

#if (WMOPS)
    fwc();
    printf("\nEncoder complexity\n");
    WMOPS_output(0);
    printf("\n");
#endif

    /* Close Encoder, Close files and free ressources */
    BASOP_init


    IF(st_fx != NULL)
    {
        /* common delete function */
        destroy_encoder_fx( st_fx );
        free(st_fx);
    }

    BASOP_end_noprint


    IF(f_input)
    memfclose(&f_input);
    IF(f_stream)
    memfclose(&f_stream);
    IF(f_rate)
    memfclose(&f_rate);
    IF(f_bwidth)
    memfclose(&f_bwidth);

    return EXIT_SUCCESS;
}

int decode() {
    Decoder_State_fx  *st_fx;                             /* decoder state structure     */
    Word16            zero_pad, dec_delay,output_frame;
    MemoryFile              *f_stream;                          /* input bitstream file        */
    MemoryFile              *f_synth;                           /* output synthesis file       */
    UWord16           bit_stream[MAX_BITS_PER_FRAME+16];
    Word16            output[L_FRAME48k];                 /* buffer for output synthesis */
#ifdef SUPPORT_JBM_TRACEFILE
    char              *jbmTraceFileName = NULL;           /* VOIP tracefile name         */
#endif
    Word16            quietMode = 0;
    Word16            noDelayCmp = 0;
    char              *jbmFECoffsetFileName = NULL;       /* FEC offset file name */

    // init frame count
    frame = 0;

    BASOP_init
    BENCH_RESET(evs_decode);

    /*------------------------------------------------------------------------------------------*
     * Allocation of memory for static variables
     *   - I/O initializations
     *   - Decoder variables initialization
     *   - Find frame length
     *------------------------------------------------------------------------------------------*/

    if ( (st_fx = (Decoder_State_fx *) calloc(1, sizeof(Decoder_State_fx) ) ) == NULL )
    {
    }

    /*------------------------------------------------------------------------------------------*
     * I/O initializations
     * Decoder variables initialization
     *------------------------------------------------------------------------------------------*/

    st_fx->bit_stream_fx = bit_stream;

    int argc = 0;
    while (dec_argv[argc] != NULL) {
        argc++;
    }
    io_ini_dec_fx( argc, dec_argv, enc_192, ENC_192_LEN, &f_stream,
                   dec_result, DEC_RAW_LEN, &f_synth,
                   &quietMode,
                   &noDelayCmp,
                   st_fx,
#ifdef SUPPORT_JBM_TRACEFILE
                   &jbmTraceFileName,
#endif
                   &jbmFECoffsetFileName
                 );

    /*output_frame = (short)(st_fx->output_Fs / 50);*/
    st_fx->output_frame_fx = extract_l(Mult_32_16(st_fx->output_Fs_fx , 0x0290));

    srand((unsigned int)time(0));

    reset_indices_dec_fx(st_fx);

    IF(st_fx->Opt_VOIP_fx)
    {
#ifdef SUPPORT_JBM_TRACEFILE
        IF( decodeVoip(st_fx, f_stream, f_synth, jbmTraceFileName, jbmFECoffsetFileName, quietMode ) != 0 )
#else
        IF( decodeVoip(st_fx, f_stream, f_synth, jbmFECoffsetFileName, quietMode ) != 0 )
#endif
        {
            free( st_fx );
            memfclose( &f_synth );
            memfclose( &f_stream );
            return -1;
        }
    }
    ELSE
    {
        /*------------------------------------------------------------------------------------------*
         * Regular EVS decoder with ITU-T G.192 bitstream
         *------------------------------------------------------------------------------------------*/

        init_decoder_fx(st_fx);

        /* output frame length */
        output_frame = st_fx->output_frame_fx;

        if( noDelayCmp == 0)
        {
            /* calculate the compensation (decoded signal aligned with original signal) */
            /* the number of first output samples will be reduced by this amount */
            dec_delay = NS2SA_fx2(st_fx->output_Fs_fx, get_delay_fx(DEC, st_fx->output_Fs_fx));
        }
        else
        {
            dec_delay = 0;
        }

        zero_pad = dec_delay;

        /*------------------------------------------------------------------------------------------*
         * Loop for every packet (frame) of bitstream data
         * - Read the bitstream packet
         * - Run the decoder
         * - Write the synthesized signal into output file
         *------------------------------------------------------------------------------------------*/
        if (quietMode == 0)
        {
            fprintf( stdout, "\n------ Running the decoder ------\r\n" );
            fprintf( stdout, "Frames processed:       " );
        }
        else {
            fprintf( stdout, "\n-- Start the decoder (quiet mode) --\r\n" );
        }
        BASOP_end_noprint;
        BASOP_init;
#if (WMOPS)
        Init_WMOPS_counter();
        Reset_WMOPS_counter();
        setFrameRate(48000, 960);
#endif

        /*----- loop: decode-a-frame -----*/
        WHILE( st_fx->bitstreamformat==G192 ? read_indices_fx( st_fx, f_stream, 0 ) : read_indices_mime( st_fx, f_stream, 0) )
        {
#if (WMOPS)
            fwc();
            Reset_WMOPS_counter();
#endif


            SUB_WMOPS_INIT("evs_dec");

            /* run the main encoding routine */
            BENCH_START(evs_decode);
            IF(sub(st_fx->codec_mode, MODE1) == 0)
            {
                IF ( st_fx->Opt_AMR_WB_fx )
                {
                    amr_wb_dec_fx( output,st_fx);
                }
                ELSE
                {
                    evs_dec_fx( st_fx, output, FRAMEMODE_NORMAL);
                }
            }
            ELSE
            {
                IF(st_fx->bfi_fx == 0)
                {
                    evs_dec_fx( st_fx, output, FRAMEMODE_NORMAL);
                }
                ELSE /* conceal */
                {
                    evs_dec_fx( st_fx, output, FRAMEMODE_MISSING);
                }
            }

            END_SUB_WMOPS;
            BENCH_SAMPLE(evs_decode);

            /* increase the counter of initialization frames */

            if( sub(st_fx->ini_frame_fx,MAX_FRAME_COUNTER) < 0 )
            {
                st_fx->ini_frame_fx = add(st_fx->ini_frame_fx,1);
            }

            /* write the synthesized signal into output file */
            /* do final delay compensation */
            IF ( dec_delay == 0 )
            {
                memfwrite( output, sizeof(Word16), output_frame, f_synth );
            }
            ELSE
            {
                IF ( sub(dec_delay , output_frame) <= 0 )
                {
                    memfwrite( output +dec_delay, sizeof(Word16), sub(output_frame , dec_delay), f_synth );
                    dec_delay = 0;
                    move16();
                }
                ELSE
                {
                    dec_delay = sub(dec_delay, output_frame);
                }
            }


            frame++;
            if (quietMode == 0)
            {
                fprintf( stdout, "%-8ld\b\b\b\b\b\b\b\b", frame);
            }
        }

        /*----- decode-a-frame-loop end -----*/

        fflush( stderr );
        if (quietMode == 0)
        {
            fprintf( stdout, "\r\n" );
            printf("Decoding finished\r\n");
        }
        else
        {
            printf("Decoding of %ld frames finished\r\n", frame);
        }
        fprintf( stdout, "\r\n" );
        fflush(stdout);

        double avg_cycle = BENCH_GET_SUMCYC() * 1.0 / BENCH_GET_LPCNT();
        double mcps = avg_cycle * 50 / 1000000;
        printf("CSV, evs_decode, %.02f\r\n", mcps);

        fflush(stdout);
        fflush(stderr);

        /* end of WMOPS counting */
#if (WMOPS)
        fwc();
        printf("\nDecoder complexity\n");
        WMOPS_output(0);
        printf("\n");
#endif

        /* add zeros at the end to have equal length of synthesized signals */
        set16_fx( output, 0, zero_pad );
        memfwrite( output, sizeof(Word16), zero_pad, f_synth );
        BASOP_init
        destroy_decoder( st_fx );
        BASOP_end_noprint
    }

    /* free memory etc. */
    free( st_fx );
    memfclose( &f_synth );
    memfclose( &f_stream );

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    // enable prefetch
    unsigned long iinfo_base = CpuIRegionBase + IREGION_IINFO_OFS;
    uint32_t *pfl1dctrl1 = (uint32_t *) (iinfo_base + 0x100);
    uint32_t *pfl1dctrl4 = (uint32_t *) (iinfo_base + 0x124);
    uint32_t old = *pfl1dctrl1;
    *pfl1dctrl1 = (old & ~0x000000FF) | 0x0000003F;
    *pfl1dctrl4 = 1;

    printf("Start Encoding...\r\n");
    if (encode() != EXIT_SUCCESS) {
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }
    if (verify_result(enc_192, enc_result, ENC_192_LEN, 0) != EXIT_SUCCESS) {
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }

    printf("Start Decoding...\r\n");
    if (decode() != EXIT_SUCCESS) {
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }
    if (verify_result(dec_raw, dec_result, DEC_RAW_LEN, 0) != EXIT_SUCCESS) {
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }
    printf("PASS\r\n");
    return EXIT_SUCCESS;
}