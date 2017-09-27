#include <vector>
#include <nnu.h>
#include <smhasher/MurmurHash3.h>

class Bloomfilter: public nnu::ClassWrap<Bloomfilter>
{
public:
    static const char * const CLASS_NAME;

    static NAN_METHOD(ctor)
    {
        uint64_t numBits = info[0]->NumberValue();
        uint8_t numHash = info[1]->Uint32Value();
        
        Bloomfilter *bloom = new Bloomfilter(numBits, numHash);
        bloom->Wrap(info.This());
    }

    static void setupMember(v8::Local<v8::FunctionTemplate>& tpl)
    {
        Nan::SetPrototypeMethod(tpl, "put", wrapFunction<&Bloomfilter::put>);
        Nan::SetPrototypeMethod(tpl, "mightContain", wrapFunction<&Bloomfilter::mightContain>);
    }

private:
    Bloomfilter(uint64_t numBits, uint8_t numHash)
    : _mem(new char[sizeof(uint64_t) + sizeof(uint8_t) + numBits / 8 + 1]),
      _numBits(*(uint64_t*)(_mem)),
      _numHash(*(uint8_t*)(_mem + sizeof(uint64_t))),
      _bitvec(_mem + sizeof(uint64_t) + sizeof(uint8_t))
    {
        _numBits = numBits;
        _numHash = numHash;
        memset(_bitvec, 0, numBits / 8 + 1);
    }

    NAN_METHOD(put)
    {
        uint64_t hash[2] = { 0LL, 0LL };
        valueToHash(info[0], hash);

        for (uint64_t i = 1; i <= _numHash; i++) {
            uint64_t combinedHash = hash[0] + (i * hash[1]);
            setbit(combinedHash % _numBits);
        }
    }

    NAN_METHOD(mightContain)
    {
        uint64_t hash[2] = { 0LL, 0LL };
        valueToHash(info[0], hash);

        bool contains = true;
        for (uint64_t i = 1; i <= _numHash; i++) {
            uint64_t combinedHash = hash[0] + (i * hash[1]);

            contains &= getbit(combinedHash % _numBits);
            if (!contains) break;
        }

        info.GetReturnValue().Set(contains);
    }

    void valueToHash(v8::Local<v8::Value> val, uint64_t *out) {
        if (node::Buffer::HasInstance(val)) {
            const char *data = node::Buffer::Data(val);
            size_t len = node::Buffer::Length(val);
            MurmurHash3_x64_128(data, len, 0, out);
        } else {
            Nan::Utf8String str(val);
            MurmurHash3_x64_128(*str, str.length(), 0, out);
        }
    }

    bool getbit (uint64_t bit)
    {
        uint64_t byte = bit >> 3;
        bit = bit & 0x7;

        return (_bitvec[byte] >> bit) & 1;
    }

    void setbit (uint64_t bit )
    {
        uint64_t byte = bit >> 3;
        bit = bit & 0x7;

        _bitvec[byte] |= (1 << bit);
    }

private:
    char *_mem;
    uint64_t &_numBits;
    uint8_t &_numHash;
    char * const _bitvec;
};

const char * const Bloomfilter::CLASS_NAME = "Bloomfilter";

NAN_MODULE_INIT(initAll)
{
    Bloomfilter::setup(target);
}

NODE_MODULE(bloomfilter, initAll);
