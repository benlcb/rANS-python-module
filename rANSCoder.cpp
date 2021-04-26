//
// Created by barzen on 11.06.19.
//

#include "rANSCoder.h"
#include <iostream>

rANSCoder::rANSCoder() {
    Rans64EncInit(&(this->state));
    flushed = true;
}

rANSCoder::rANSCoder(uint32_t floatshift, uint32_t prob_bits) {
    FLOATSHIFT = floatshift;
    PROB_BITS = prob_bits;
    PROB_SCALE = 1 << PROB_BITS;

    Rans64EncInit(&(this->state));
    flushed = true;
}

void rANSCoder::convert_pdf(std::vector<float> orpdf, std::vector<uint32_t>& npdf, std::vector<uint32_t>& cdf){

    for (int i = 0; i<orpdf.size(); i++) {
        npdf[i] = orpdf[i]*FLOATSHIFT;

        if (npdf[i] < MIN_PROBABILITY) {
            npdf[i] = MIN_PROBABILITY;
        }
    }

    cdf[0] = 0;
    for(int i = 0; i<orpdf.size(); i++) {
        cdf[i+1] = cdf[i] + npdf[i];
    }

    uint32_t cur_total = cdf[orpdf.size()];
    for (int i = 1; i <= orpdf.size(); i++) {
        // cdf has to be strictly increasing
        cdf[i] = ((uint64_t)PROB_SCALE * cdf[i])/cur_total;
    }

    for (int i = 0; i<orpdf.size(); i++) {
        npdf[i] = cdf[i + 1] - cdf[i];
    }

}

void rANSCoder::init_ec(){
    Rans64EncInit(&(this->state));
}

void rANSCoder::init_dc(uint32_t* dc_bs, size_t size) {
    if(!flushed) {
        std::cout << "ERROR: Trying to initialize decoder with unflushed buffer." << std::endl;
    }

    vec.assign(dc_bs, dc_bs+size);

    Rans64DecInit(&state, vec);
}
void rANSCoder::init_dc(std::vector<uint32_t> data) {
    if(!flushed) {
        std::cout << "ERROR: Trying to initialize decoder with unflushed buffer." << std::endl;
    }

    vec = data;

    Rans64DecInit(&state, vec);

}


void rANSCoder::encode_sym(unsigned int sym, std::vector<float> pdf) {

    std::vector<uint32_t> npdf(pdf.size());
    std::vector<uint32_t> cdf(pdf.size()+1);
    convert_pdf(pdf, npdf, cdf);

    Rans64EncPut(&state, vec, cdf[sym], npdf[sym], PROB_BITS);
    flushed = false;

}

uint32_t rANSCoder::decode_sym(std::vector<float> pdf) {

    uint32_t cum_prob = Rans64DecGet(&state, PROB_BITS);

    std::vector<uint32_t> npdf(pdf.size());
    std::vector<uint32_t> cdf(pdf.size()+1);
    convert_pdf(pdf, npdf, cdf);

    uint32_t sym = 0;
    while (cum_prob > cdf[sym] & cum_prob >= cdf[sym+1]) {
        sym++;
    }

    Rans64DecAdvance(&state, vec, cdf[sym], npdf[sym], PROB_BITS);

    return sym;
}

void rANSCoder::get_buffer(uint32_t** addr, size_t& size) {
    if (!flushed) Rans64EncFlush(&state, vec);
    *addr = vec.data();
    size = vec.size();

    Rans64EncInit(&(this->state));
}

std::vector<uint32_t> rANSCoder::get_buffer() {
    if (!flushed) Rans64EncFlush(&state, vec);
    Rans64EncInit(&(this->state));
    return vec;
}