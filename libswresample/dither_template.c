
#if defined(TEMPLATE_DITHER_DBL)
#    define RENAME(N) N ## _double
#    define DELEM  double
#    define CLIP(v)

#elif defined(TEMPLATE_DITHER_FLT)
#    define RENAME(N) N ## _float
#    define DELEM  float
#    define CLIP(v)

#elif defined(TEMPLATE_DITHER_S32)
#    define RENAME(N) N ## _int32
#    define DELEM  int32_t
#    define CLIP(v) v = FFMAX(FFMIN(v, INT32_MAX), INT32_MIN)

#elif defined(TEMPLATE_DITHER_S16)
#    define RENAME(N) N ## _int16
#    define DELEM  int16_t
#    define CLIP(v) v = FFMAX(FFMIN(v, INT16_MAX), INT16_MIN)

#else
ERROR
#endif

void RENAME(swri_noise_shaping)(SwrContext *s, AudioData *dsts, const AudioData *srcs, const AudioData *noises, int count){
    int i, j, pos, ch;
    int taps  = s->dither.ns_taps;
    float S   = s->dither.ns_scale;
    float S_1 = s->dither.ns_scale_1;

    for (ch=0; ch<srcs->ch_count; ch++) {
        const float *noise = ((const float *)noises->ch[ch]) + s->dither.noise_pos;
        const DELEM *src = (const DELEM*)srcs->ch[ch];
        DELEM *dst = (DELEM*)dsts->ch[ch];
        float *ns_errors = s->dither.ns_errors[ch];
        pos  = s->dither.ns_pos;
        for (i=0; i<count; i++) {
            double d1, d = src[i]*S_1;
            for(j=0; j<taps; j++)
                d -= s->dither.ns_coeffs[j] * ns_errors[pos + j];
            pos = pos ? pos - 1 : taps - 1;
            d1 = rint(d + noise[i]);
            ns_errors[pos + taps] = ns_errors[pos] = d1 - d;
            d1 *= S;
            CLIP(d1);
            dst[i] = d1;
        }
    }

    s->dither.ns_pos = pos;
}

#undef RENAME
#undef DELEM
#undef CLIP