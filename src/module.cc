#include <vector>
#include <nnu.h>
#include <smhasher/MurmurHash3.h>

class Bloomfilter: public nnu::ClassWrap<Bloomfilter>
{
public:
    static const char * const CLASS_NAME;

    static NAN_METHOD(ctor)
    {
        if (node::Buffer::HasInstance(info[0])) {
            const uint8_t *data = (uint8_t *)node::Buffer::Data(info[0]);
            size_t len = node::Buffer::Length(info[0]);

            assert(len >= 8);
            uint8_t numHash = data[0];
            uint64_t numBits = 0;
            for (int i = 0; i < 7; i++) {
                numBits |= uint64_t(data[7 - i]) << i * 8;
            }

            Bloomfilter *bloom = new Bloomfilter(numBits, numHash);
            if (len > 8) {
                memcpy(bloom->_bitvec, data + 8, len - 8);
            }

            bloom->Wrap(info.This());
        } else {
            uint64_t numBits = info[0]->NumberValue();
            uint8_t numHash = info[1]->Uint32Value();
            
            Bloomfilter *bloom = new Bloomfilter(numBits, numHash);
            bloom->Wrap(info.This());
        }
    }

    static void setupMember(v8::Local<v8::FunctionTemplate>& tpl)
    {
        Nan::SetPrototypeMethod(tpl, "getNumBits", wrapFunction<&Bloomfilter::getNumBits>);
        Nan::SetPrototypeMethod(tpl, "getNumHash", wrapFunction<&Bloomfilter::getNumHash>);

        Nan::SetPrototypeMethod(tpl, "getBuffer", wrapFunction<&Bloomfilter::getBuffer>);
        Nan::SetPrototypeMethod(tpl, "setBuffer", wrapFunction<&Bloomfilter::setBuffer>);

        Nan::SetPrototypeMethod(tpl, "put", wrapFunction<&Bloomfilter::put>);
        Nan::SetPrototypeMethod(tpl, "mightContain", wrapFunction<&Bloomfilter::mightContain>);
    }

private:
    Bloomfilter(uint64_t numBits, uint8_t numHash)
    : _mem(new char[8 + numBits / 8 + 1]),
      _bitvec(_mem + 8),
      _numHash(numHash),
      _numBits(numBits)
    {
        assert(numBits < 0x0100000000000000);

        _mem[0] = char(numHash);
        for (int i = 0; i < 7; i++) {
            _mem[7 - i] = char((numBits >> i * 8) & 0xFF);
        }

        memset(_bitvec, 0, numBits / 8 + 1);
    }

    NAN_METHOD(getNumBits)
    {
        info.GetReturnValue().Set(double(_numBits));
    }

    NAN_METHOD(getNumHash)
    {
        info.GetReturnValue().Set(_numHash);
    }

    NAN_METHOD(getBuffer)
    {
        uint64_t offset = info[0]->NumberValue();
        uint64_t size = info[1]->NumberValue();

        // JS will check the boundry.
        info.GetReturnValue().Set(
            Nan::CopyBuffer(_mem + offset, size).ToLocalChecked()
        );
    }

    NAN_METHOD(setBuffer)
    {
        const char *data = node::Buffer::Data(info[0]);
        size_t len = node::Buffer::Length(info[0]);

        uint64_t offset = info[1]->NumberValue();
        // JS will check the boundry.
        memcpy(_bitvec + offset, data, len);
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
    char *const _bitvec;
    uint8_t _numHash;
    uint64_t _numBits;
};

const char * const Bloomfilter::CLASS_NAME = "Bloomfilter";

NAN_MODULE_INIT(initAll)
{
    Bloomfilter::setup(target);
}

NODE_MODULE(bloomfilter, initAll);
