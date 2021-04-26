//
// Created by barzen on 11.06.19.
//

#ifndef CLIONSCRATCHPAD_RANSCODER_H
#define CLIONSCRATCHPAD_RANSCODER_H

#include "rans64_custom.hpp"
#include <vector>

/**
 * @brief A rANS coder with a compression rate of an arithmetic coder, and the performance similar to Huffman coding.
 *
 * @details This class serves as a wrapper around the rANS coder developed by Fabian 'ryg' Giesen.
 *
 * The basic principle of this coder is that it takes as input symbols (i.e. numbers), along with a probability
 * distribution describing how often each symbol appears in a given text. The coder in the encoding step then outputs a
 * bunch of bytes which fully describe the original text. In the decoding step, you feed the coder the bytes the encoder
 * gave out, along with the probability distribution, and then the decoder will give the original text back.
 *
 * The space required by the encoded text should be less than the one required by the unencoded text. This is done by
 * essentially reserving more bits inside a word for symbols which appear more often. In an extreme example, if you know
 * that you only have symbol 'A' but never any other symbol, you only need to encode the number of symbols you have in
 * total. Consequently, when 'A' appears with 80% probability, and the rest of the symbols with the remaining 20%, then
 * less space is required to encode a large number of As.
 *
 * A common use case would be to encode, say, the green values of the pixels inside a picture. The "text" as above
 * would be the whole picture, and the "symbol" would be the green value of a pixel. The allowed values for a symbol
 * would range from 0 to 255, consequently you need a probability distribution for the values from 0 to 255. The hope
 * would be that a picture which is uniformly green (say a forest or a distinctly not-green car inside a garage) would
 * then be encoded concisely.
 *
 * Note that you may give a different probability distribution for each pixel.
 */
class rANSCoder {

private:

    uint64_t state;
    uint32_t PROB_BITS = 14;
    uint32_t PROB_SCALE = 1 << PROB_BITS;
    uint32_t FLOATSHIFT = 1 << 11;
    uint32_t MIN_PROBABILITY = 1;
    std::vector<uint32_t> vec;
    bool flushed = false;

    void convert_pdf(std::vector<float> orpdf, std::vector<uint32_t>& npdf, std::vector<uint32_t>& cdf);

public:

    /**
     * @brief Initializes the rANSCoder with default values.
     *
     * @details The default values are floatshift with 2 to the power of 11, and prob_bits with 14.
     *
     * This class has to works with exact probabilities, since encoding and decoding is an exact process. So internally
     * probabilities are represented using unsigned integers. Essentially you partition the unsigned integers into
     * buckets. So, for example, if you start with a probability distribution of (0.25, 0.25, 0.5) and you use the
     * integers from 0 to 31 to describe the probability distribution, then the first bucket would go from 0 to 7, the
     * second from 8 to 16, and the last one from 17 to 31.
     *
     * This class takes floating points as input for probability distributions, so there is a mechanism to convert
     * from floating to integer point. The value floatshift lets you specify how many decimal places of a floating
     * point number are used to determine the corresponding probability bucket. Prob_bits describes the number of bits
     * used for the buckets, i.e. a value of 10 means that the buckets partition the integer values 0 to 1023.
     *
     * @attention Call either init_ec> or init_dc after initialization in order to start encoding or decoding respectively.
     */
    rANSCoder();

    /**
     * @brief Initializes the rANSCoder with custom values.
     *
     * @details
     *
     *This class has to works with exact probabilities, since encoding and decoding is an exact process. So internally
     * probabilities are represented using unsigned integers. Essentially you partition the unsigned integers into
     * buckets. So, for example, if you start with a probability distribution of (0.25, 0.25, 0.5) and you use the
     * integers from 0 to 31 to describe the probability distribution, then the first bucket would go from 0 to 7, the
     * second from 8 to 16, and the last one from 17 to 31.
     *
     * This class takes floating points as input for probability distributions, so there is a mechanism to convert
     * from floating to integer point. The value floatshift lets you specify how many decimal places of a floating
     * point number are used to determine the corresponding probability bucket. Prob_bits describes the number of bits
     * used for the buckets, i.e. a value of 10 means that the buckets partition the integer values 0 to 1023.*
     * @param[in] floatshift A power of 2. Describes how many decimal positions are used to calculate the buckets.
     * @param[in] prob_bits The number of bits used to describe probabilities.
     *
     * @attention Call either init_ec or init_dc after initialization in order to start encoding or decoding respectively.
     */
    rANSCoder(uint32_t floatshift, uint32_t prob_bits);

    /**
     * @brief Starts the rANSCoder as an encoder.
     *
     * @details
     *
     * After calling this function, you may use encode_sym to encode symbols. Once you are done encoding symbols,
     * can get the encoded text by calling get_buffer.
     *
     * @attention Do not call init_ec or init_dc after calling this method.
     */
    void init_ec();

    /**
     * @brief Starts the rANSCoder as a decoder.
     *
     * @details
     *
     * After calling this function, you may use decode_sym to decode symbols. You need to supply the the rANSCoder with
     * the encoded text.
     *
     * @param[in] dc_bs A pointer describing the start of an array containing the encoded text. You should have this from a previous call to get_buffer.
     * @param[in] size Size of the array. You should have this from a previous call to get_buffer.
     *
     * @attention Do not call init_ec or init_dc after calling this method.
     */
    void init_dc(uint32_t* dc_bs, size_t size);

    /**
     * @brief Starts the rANSCoder as a decoder.
     *
     * @details
     *
     * After calling this function, you may use decode_sym to decode symbols. You need to supply the the rANSCoder with
     * the encoded text.
     *
     * @param[in] data A vector of the encoded text. You should have this from a previous call to get_buffer.
     *
     * @attention Do not call init_ec or init_dc after calling this method.
     */
    void init_dc(std::vector<uint32_t> data);

    /**
     * @brief Encodes a symbol.
     *
     * @details
     *
     * This uses the given probability distribution to encode a symbol. Note that probability distributions are allowed
     * to vary during the encoding process. Be aware that for the encoding to work as well as possible (i.e. in order for
     * the output size to be small), the chance that a symbol is encoded should somewhat correspond to the probability
     * distribution you provide.
     *
     * Note you are allowed to have a probability of 0 on a symbol, and you can still encode that symbol. The coder
     * automatically assigns a very small probability to symbols you declare to have probability 0 in order to prevent
     * hard crashes. However, the size of the encoded text will explode if this happens too often.
     *
     * An example:
     *
     *     rANSCoder encoder();
     *     encoder.init_ec();
     *     vector<float> probs{0.25, 0.25, 0.5};
     *     vector<float> probs{0.5, 0.25, 0.25};
     *     // Encode the word 10 with prob distribution probs, then 200 with prob distribution probs2
     *     encoder.encode_sym(1, probs);
     *     encoder.encode_sym(0, probs);
     *     encoder.encode_sym(2, probs2);
     *     encoder.encode_sym(0, probs2);
     *     encoder.encode_sym(0, probs2);
     *
     *     auto encoded = encoder.get_buffer(); // Encoded text
     *
     * @param[in] sym Symbol to encode
     * @param[in] pdf Probability distribution to encode with.
     *
     * @attention You must call init_ec before calling this method
     */
    void encode_sym(unsigned int sym, std::vector<float> pdf);

    /**
     * @brief Decodes a symbol.
     *
     * @details
     *
     * This decodes a symbol which was previously encoded. Given the same probability distribution as was used to encode
     * a symbol, this will return the same symbol. Note that if you give a different probability distributions, you will
     * likely receive a different symbol. Note that an error in a single symbol will propagate to all future symbols
     * which are decoded.
     *
     * Note that the order in which symbols are decoded is reversed from the order they are encoded in. The
     * encoding/decoding process essentially works like a stack. So, for example, if you encoded the symbols
     * 1 0 2 0 0 you will get the symbols 0 0 2 0 1 back. If you encoded exactly as in the documentation for
     * encode_sym, you may decode the output the following way:
     *
     *      ransCoder decoder();
     *      decoder.init_dc(encoded); // Encoded is the vec containing the encoded data.
     *      vector<float> probs{0.25, 0.25, 0.5};
     *      vector<float> probs2{0.5, 0.25, 0.25};
     *
     *      cout << decoder.decode_sym(probs2); // 0
     *      cout << decoder.decode_sym(probs2); // 0
     *      cout << decoder.decode_sym(probs2); // 2
     *      cout << decoder.decode_sym(probs);  // 0
     *      cout << decoder.decode_sym(probs);  // 1
     *
     *
     * Notice in order to decode you must know the probability distributions used to encode, and the size of the
     * original text.
     *
     * @param[in] pdf Probability distribution to decode with - must correspond exactly to the distribution used to encode.
     * @return A decoded symbol.
     *
     * @attention You must call init_dc before calling this method
     */
    uint32_t decode_sym(std::vector<float> pdf);

    /**
     * @brief Returns an array containing the previously encoded data.
     *
     * @details
     *
     * This method should be called after the encoding is done. This will give you the encoded data, that you may
     * save or do other things with. If the probability distributions somewhat corresponded to the actual probabilities
     * that the symbols occurred, then the size of the encoded text should be smaller than the size of the unencoded
     * text (unless the probability distributions used are close to the uniform distribution).
     *
     * Note that you probably want to use other get_buffer() method, which returns a vec.
     *
     * Example usage:
     *
     *     rANSCoder encoder();
     *     encoder.init_ec();
     *     vector<float> probs{0.25, 0.25, 0.5};
     *     vector<float> probs{0.5, 0.25, 0.25};
     *     // Encode the word 10 with prob distribution probs, then 200 with prob distribution probs2
     *     encoder.encode_sym(1, probs);
     *     encoder.encode_sym(0, probs);
     *     encoder.encode_sym(2, probs2);
     *     encoder.encode_sym(0, probs2);
     *     encoder.encode_sym(0, probs2);
     *
     *     size_t size;
     *     uint32_t** data;
     *     encoder.get_buffer(data, size); // Encoded text
     *
     * @param[out] addr A pointer to the start of the array containing the encoded data.
     * @param[out] size The size of the array.
     *
     * @attention You must call init_ec before calling this method.
     */
    void get_buffer(uint32_t** addr, size_t& size);

    /**
     * @brief Returns an array containing the previously encoded data.
     *
     * @details
     *
     * This method should be called after the encoding is done. This will give you the encoded data, that you may
     * save or do other things with. If the probability distributions somewhat corresponded to the actual probabilities
     * that the symbols occurred, then the size of the encoded text should be smaller than the size of the unencoded
     * text (unless the probability distributions used are close to the uniform distribution).
     *
     * Example usage:
     *
     *     rANSCoder encoder();
     *     encoder.init_ec();
     *     vector<float> probs{0.25, 0.25, 0.5};
     *     vector<float> probs{0.5, 0.25, 0.25};
     *     // Encode the word 10 with prob distribution probs, then 200 with prob distribution probs2
     *     encoder.encode_sym(1, probs);
     *     encoder.encode_sym(0, probs);
     *     encoder.encode_sym(2, probs2);
     *     encoder.encode_sym(0, probs2);
     *     encoder.encode_sym(0, probs2);
     *
     *     auto encoded = encoder.get_buffer(); // Encoded text
     *
     * @return The encoded data as a vector.
     *
     * @attention You must call init_ec before calling this method.
     */
    std::vector<uint32_t> get_buffer();

};


#endif //CLIONSCRATCHPAD_RANSCODER_H
