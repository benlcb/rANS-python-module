#include "rans64_custom.hpp"
#include <vector> 
#include <iostream>
#include <rANSCoder.h>

#define ALPH_SIZE 3
#define BUFSIZE 200000
#define FLOATSHIFT 1024

int main__test(){
    std::vector<float> pf = {0.25, 0.25, 0.25, 0.25};
    std::vector<unsigned int> p;
    p.push_back(1);
    p.push_back(2);
    p.push_back(3);

    rANSCoder mycoder;
    mycoder.init_ec();

    for (int j = 0; j < p.size(); ++j) {
        mycoder.encode_sym(p[j], pf);
    }

    std::vector<uint32_t> data = mycoder.get_buffer();
    std::cout << "size: " << data.size() << std::endl;
    for (int j = 0; j < data.size(); ++j) {
        std::cout << "data[" << j << "] = " << data[j] << std::endl;
    }

}


int main(){

    // set frequencies
    std::vector<float> pf(256, 0);
    pf.emplace(pf.begin(), 1);

    std::vector<unsigned int> p(10000);
    std::cout << "max_size: " << p.max_size() << "\n";
    for (unsigned int i = 0; i<p.size(); i++) {
        unsigned int rn = 3;//rand() % pf.size();
        p[i] = rn;
    }

    rANSCoder mycoder;
    mycoder.init_ec();

    for (int j = 0; j < p.size(); ++j) {
        mycoder.encode_sym(p[j], pf);
    }

    //std::cout << "CODER STATE BEFORE FLUSH: " << std::endl;
    //std::cout << mycoder.state << std::endl;
    //std::cout << mycoder.vec.back() << mycoder.vec.end()[-2] << std::endl;

    std::vector<uint32_t> data = mycoder.get_buffer();
    std::cout << "size: " << data.size() << std::endl;

    rANSCoder mydec;
    mydec.init_dc(data);


    //std::cout << "DECODER STATE AFTER INIT: " << std::endl;
    //std::cout << mydec.state << std::endl;
    //std::cout << mydec.vec.back() << mydec.vec.end()[-2] << std::endl;

    std::vector<unsigned int> res;

    for (int k = 0; k < p.size(); ++k) {
        res.insert(res.begin(), mydec.decode_sym(pf));
    }

    if (p == res) {
        std::cout << "Test passed" << std::endl;
    } else {
        std::cout << "Test failed" << std::endl;

        for (int i = 0; i < p.size(); ++i) {
            std::cout << " " << res[i] << " " << p[i];
        }
    }
}


int main_buffer(){

    // set frequencies
    std::vector<float> pf = {0.25, 0.000001, 0.000002, 0.25};

    std::vector<unsigned int> p(10000);
    std::cout << "max_size: " << p.max_size() << "\n";
    for (unsigned int i = 0; i<p.size(); i++) {
        unsigned int rn = rand() % pf.size();
        p[i] = rn;
    }

    rANSCoder mycoder;
    mycoder.init_ec();

    for (int j = 0; j < p.size(); ++j) {
        mycoder.encode_sym(p[j], pf);
    }

    size_t size;
    uint32_t* addr;
    mycoder.get_buffer(&addr, size);
    std::cout << "size: " << size << std::endl;

    rANSCoder mydec;
    mydec.init_dc(addr, size);

    std::vector<unsigned int> res;

    for (int k = 0; k < p.size(); ++k) {
        res.insert(res.begin(), mydec.decode_sym(pf));
    }

    if (p == res) {
        std::cout << "Test passed" << std::endl;
    } else {
        std::cout << "Test failed" << std::endl;

        for (int i = 0; i < p.size(); ++i) {
            std::cout << " " << res[i] << " " << p[i];
        }
    }
}

int main_basic(){

    std::vector<unsigned int> p(100000);
    std::cout << "max_size: " << p.max_size() << "\n";
    for (unsigned int i = 0; i<p.size(); i++) {
        unsigned int rn = rand() % ALPH_SIZE;
        p[i] = rn;
    }

    // set frequencies
    std::vector<float> pf = {0.333, 0.333, 0.333};

    rANSCoder mycoder;
    mycoder.init_ec();

    for (int j = 0; j < p.size(); ++j) {
        mycoder.encode_sym(p[j], pf);
    }

    std::vector<unsigned int> res;

    for (int k = 0; k < p.size(); ++k) {
        res.insert(res.begin(), mycoder.decode_sym(pf));
    }

    if (p == res) {
        std::cout << "Test passed" << std::endl;
    } else {
        std::cout << "Test failed" << std::endl;
    }
}

int main_float() {

    static const uint32_t prob_bits = 14;
    static const uint32_t prob_scale = 1 << prob_bits;
    uint32_t cum_freqs[ALPH_SIZE+1];
    uint32_t freqs[ALPH_SIZE];


    // Generate Input data
    std::vector<unsigned int> p(1000000);
    std::cout << "max_size: " << p.max_size() << "\n";
    for (unsigned int i = 0; i<p.size(); i++) {
        unsigned int rn = rand() % ALPH_SIZE;
        p[i] = rn;
    }

    // set frequencies
    std::vector<float> pf = {0.333, 0.333, 0.333};

    for (int i = 0; i<ALPH_SIZE; i++) {
        freqs[i] = pf[i]*FLOATSHIFT;
    }

    cum_freqs[0] = 0;
    for(int i = 0; i<ALPH_SIZE; i++) {
        cum_freqs[i+1] = cum_freqs[i] + freqs[i];
    }

    std::cout << "cum_freqs: ";
    uint32_t cur_total = cum_freqs[ALPH_SIZE];
    for (int i = 1; i <= ALPH_SIZE; i++) {
        cum_freqs[i] = ((uint64_t)prob_scale * cum_freqs[i])/cur_total;
        std::cout << cum_freqs[i] <<  " ";
    }
    std::cout << std::endl;

    for (int i = 0; i<ALPH_SIZE; i++) {
        freqs[i] = cum_freqs[i + 1] - cum_freqs[i];
    }


    // Encoding
    uint32_t* out_buf = new uint32_t[BUFSIZE];
    uint32_t* out_end = out_buf + BUFSIZE;
    uint32_t* out_cur = out_end;
    Rans64State state;
    std::vector<unsigned int> res;

    Rans64EncInit(&state);

    for (unsigned int i = 0; i<p.size(); i++) {
        //printf("encode val: %u & i: %u \n", p[i], i);
        Rans64EncPut(&state, &out_cur, cum_freqs[p[i]], freqs[p[i]], prob_bits);
    }

    /* Flush & Load Buffer */
    Rans64EncFlush(&state, &out_cur);
    Rans64DecInit(&state, &out_cur);
    /***********************/

    for (unsigned int i = 0; i<p.size(); i++) {
        uint32_t f = Rans64DecGet(&state, prob_bits);
        //printf("decoded freq: %d \n", f);

        int j = 0;
        while (f > cum_freqs[j] & f >= cum_freqs[j+1]) {
            j++;
        }
        //printf("decoded val: %d \n", j);
        res.insert(res.begin(), j);

        Rans64DecAdvance(&state, &out_cur, cum_freqs[j], freqs[j], prob_bits);
    }

    if (p == res) {
        std::cout << "Test passed" << std::endl;
    } else {
        std::cout << "Test failed" << std::endl;
    }

    return 1;
}

int main_int() {

    static const uint32_t prob_bits = 14;
    static const uint32_t prob_scale = 1 << prob_bits;
    uint32_t cum_freqs[ALPH_SIZE+1];
    uint32_t freqs[ALPH_SIZE];


    // Generate Input data and calculate frequencies
    std::vector<unsigned int> p(100000);
    std::cout << "max_size: " << p.max_size() << "\n";
    for (unsigned int i = 0; i<p.size(); i++) {
        unsigned int rn = rand() % ALPH_SIZE;
        p[i] = rn;
    }

    for (int i = 0; i<ALPH_SIZE; i++) {
        freqs[i] = 0;
    }
    for (int i = 0; i<p.size(); i++ ) {
        freqs[p[i]] += 1;
    }

    cum_freqs[0] = 0;
    for(int i = 0; i<ALPH_SIZE; i++) {
        cum_freqs[i+1] = cum_freqs[i] + freqs[i];
    }

    std::cout << "cum_freqs: ";
    uint32_t cur_total = cum_freqs[ALPH_SIZE];
    for (int i = 1; i <= ALPH_SIZE; i++) {
        cum_freqs[i] = ((uint64_t)prob_scale * cum_freqs[i])/cur_total;
        std::cout << cum_freqs[i] <<  " ";
    }
    std::cout << std::endl;

    for (int i = 0; i<ALPH_SIZE; i++) {
        freqs[i] = cum_freqs[i + 1] - cum_freqs[i];
    }


    // Encoding
    uint32_t* out_buf = new uint32_t[BUFSIZE];
    uint32_t* out_end = out_buf + BUFSIZE;
    uint32_t* out_cur = out_end;
    Rans64State state;
    std::vector<unsigned int> res;

    Rans64EncInit(&state);

    for (unsigned int i = 0; i<p.size(); i++) {
        //printf("encode val: %u & i: %u \n", p[i], i);
        Rans64EncPut(&state, &out_cur, cum_freqs[p[i]], freqs[p[i]], prob_bits);
    }

    /* Flush & Load Buffer */
    Rans64EncFlush(&state, &out_cur);
    Rans64DecInit(&state, &out_cur);
    /***********************/

    for (unsigned int i = 0; i<p.size(); i++) {
        uint32_t f = Rans64DecGet(&state, prob_bits);
        //printf("decoded freq: %d \n", f);

        int j = 0;
        while (f > cum_freqs[j] & f >= cum_freqs[j+1]) {
            j++;
        }
        //printf("decoded val: %d \n", j);
        res.insert(res.begin(), j);

        Rans64DecAdvance(&state, &out_cur, cum_freqs[j], freqs[j], prob_bits);
    }

    if (p == res) {
        std::cout << "Test passed" << std::endl;
    } else {
        std::cout << "Test failed" << std::endl;
    }

	return 1;
}
	

