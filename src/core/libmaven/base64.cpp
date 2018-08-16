#include "base64.h"
#include "mzUtils.h"

using namespace std;

namespace base64 {

    char encode(unsigned char u) {
        //Merged to 776
        if(u < 26) return 'A'+u;
        if(u < 52) return 'a'+(u-26);
        if(u < 62) return '0'+(u-52);
        if(u == 62) return '+';
        return '/';
    }

    unsigned char decode(char c){
        //Merged to 776
        if(c >= 'A' && c <= 'Z') return(c - 'A');
        if(c >= 'a' && c <= 'z') return(c - 'a' + 26);
        if(c >= '0' && c <= '9') return(c - '0' + 52);
        if(c == '+') return 62;
        return 63;
    }

    static const std::string base64_chars = 
                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                 "abcdefghijklmnopqrstuvwxyz"
                 "0123456789+/=";

    bool is_base64(unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/') || (c == '='));
    }

    string decodeString(string const& encoded_string) {
        int in_len = encoded_string.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        string ret;

        while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
            char_array_4[i++] = encoded_string[in_]; in_++;
            if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = ( char_array_4[0] << 2       ) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) +   char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
            }
        }

        if (i) {
            for (j = 0; j < i; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

            for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
        }
        return ret;
    }

    vector<float> convertDecodedDataBackToFloat(char *dest, int float_size,
            bool neworkorder, int size) {
        //Merged to 776
#if (LITTLE_ENDIAN == 1)
        neworkorder = !neworkorder;
#endif

        // we will cast everything as a float may be this is not wise, but have not
        // found a need for double
        // precission yet
        vector<float> decodedArray(size);

        if (float_size == 8) {
            if (neworkorder == false) {
                for (int i = 0; i < size; i++)
                    decodedArray[i] = (float)((double *)dest)[i];
            } else {
                uint64_t *u = (uint64_t *)dest;
                double data = 0;
                for (int i = 0; i < size; i++) {
                    uint64_t t = swapbytes64(u[i]);
                    memcpy(&data, &t, 8);
                    decodedArray[i] = (float)data;
                }
            }
        } else if (float_size == 4) {
            if (neworkorder == false) {
                for (int i = 0; i < size; i++)
                    decodedArray[i] = ((float *)dest)[i];
            } else {
                uint32_t *u = (uint32_t *)dest;
                float data = 0;
                for (int i = 0; i < size; i++) {
                    uint32_t t = swapbytes(u[i]);
                    memcpy(&data, &t, 4);
                    decodedArray[i] = data;
                }
            }
        }

        free(dest);
        return decodedArray;
    }

    vector<float> decode_base64(const string& src, int float_size, bool neworkorder, bool decompress) {
        //Merged to 776
        if (decompress)
        {
            vector<float> out;
            out.clear();
            if (src == "") return out;

            string decompressed;

            QByteArray qt_byte_array = QByteArray::fromRawData(src.c_str(), (int) src.size());
            QByteArray bazip = QByteArray::fromBase64(qt_byte_array);
            QByteArray czip;
            czip.resize(4);
            czip[0] = (bazip.size() & 0xff000000) >> 24;
            czip[1] = (bazip.size() & 0x00ff0000) >> 16;
            czip[2] = (bazip.size() & 0x0000ff00) >> 8;
            czip[3] = (bazip.size() & 0x000000ff);
            czip += bazip;
            QByteArray base64_uncompressed = qUncompress(czip);

            if (base64_uncompressed.isEmpty())
            {
                throw(std::runtime_error("Decompression error"));
            }
            decompressed.resize(base64_uncompressed.size());

            std::copy(base64_uncompressed.begin(), base64_uncompressed.end(), decompressed.begin());

            char *byte_buffer = reinterpret_cast<char *>(&decompressed[0]);
            size_t buffer_size = decompressed.size();

            const float * float_buffer = reinterpret_cast<const float *>(byte_buffer);
            if (buffer_size % float_size != 0)
            {
                throw(std::runtime_error("Bad buffer count"));
            }
            
            size_t float_count = buffer_size / float_size;

            // change endianness if necessary
#if (LITTLE_ENDIAN == 1)
            neworkorder = !neworkorder;
#endif
            if (neworkorder)
            {
                if (float_size == 4) // 32 bit
                {
                    uint32_t * p = reinterpret_cast<uint32_t *>(byte_buffer);
                    std::transform(p, p + float_count, p, swapbytes);
                }
                else // 64 bit
                {
                    uint64_t * p = reinterpret_cast<uint64_t *>(byte_buffer);
                    std::transform(p, p + float_count, p, swapbytes64);
                }
            }

            // copy values
            out.assign(float_buffer, float_buffer + float_count);
            return out;
        }

        int size = 1 + (src.length() * 3 / 4 - 4) / float_size;

        string decoded = decodeString(src);
        char *dest = (char *) malloc(sizeof(char) * decoded.size());
        decoded.copy(dest, decoded.size());

        vector<float> decodedArray =
            convertDecodedDataBackToFloat(dest, float_size, neworkorder, size);

        return decodedArray;

    }

    void decompressString(const string &str, char **dest, int &size, int float_size) {
        //Merged to 776
        string uncompressed = mzUtils::uncompressString(str);

        free(*dest);
        *dest = (char *) uncompressed.c_str();
        size = 1 + (uncompressed.size() * 3 / 4 - 4) / float_size;
    }

    unsigned char *convertFromFloatToCharacter(float *srcF,
            const vector<float> &farray) {
        //Merged to 776
        int i;
        int sizeF = farray.size();

        // copy vector to C array
        for (i = 0; i < sizeF; i++) {
            srcF[i] = farray[i];
        }

        unsigned char *src = (unsigned char *)srcF;
        return src;
    }

    unsigned char *encodeString(unsigned char *src, int size) {
        //Merge to 776
        unsigned char *out =
            (unsigned char *)calloc(sizeof(char), size * 4 / 3 + 4);
        unsigned char *p = out;
        int i;
        for (i = 0; i < size; i += 3) {

            unsigned char b1=0, b2=0, b3=0, b4=0, b5=0, b6=0, b7=0;

            b1 = src[i];

            if(i+1<size)
                b2 = src[i+1];

            if(i+2<size)
                b3 = src[i+2];

            b4 = b1>>2;
            b5 = ((b1&0x3)<<4)|(b2>>4);
            b6 = ((b2&0xf)<<2)|(b3>>6);
            b7 = b3&0x3f;

            *p++= encode(b4);
            *p++= encode(b5);

            if(i+1<size) {
                *p++= encode(b6);
            } else {
                *p++= '=';
            }

            if(i+2<size) {
                *p++= encode(b7);
            } else {
                *p++= '=';
            }

        }
        return out;
    }

    unsigned char *encode_base64(const vector<float>& farray) {
        //Merged to 776

        //copy vector t C array
        int sizeF = farray.size();
        int size = sizeF * 4;

        float* srcF= (float*) calloc(sizeof(float), sizeF);
        unsigned char *src = convertFromFloatToCharacter(srcF, farray);

        unsigned char* out = encodeString(src, size);

        free(srcF);
        return out;
    }

} //namespace
