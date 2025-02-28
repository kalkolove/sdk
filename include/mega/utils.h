﻿/**
 * @file mega/utils.h
 * @brief Mega SDK various utilities and helper classes
 *
 * (c) 2013-2014 by Mega Limited, Auckland, New Zealand
 *
 * This file is part of the MEGA SDK - Client Access Engine.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

#ifndef MEGA_UTILS_H
#define MEGA_UTILS_H 1

#include <type_traits>
#include <condition_variable>
#include <thread>
#include <mutex>

#include "types.h"
#undef SSIZE_MAX
#include "mega/mega_utf8proc.h"
#undef SSIZE_MAX

// Needed for Windows Phone (MSVS 2013 - C++ version 9.8)
#if defined(_WIN32) && _MSC_VER <= 1800 && __cplusplus < 201103L && !defined(_TIMESPEC_DEFINED) && ! __struct_timespec_defined
struct timespec
{
    long long	tv_sec; 	/* seconds */
    long        tv_nsec;	/* nanoseconds */
};
# define __struct_timespec_defined  1
#endif

namespace mega {
// convert 1...8 character ID to int64 integer (endian agnostic)
#define MAKENAMEID1(a) (nameid)(a)
#define MAKENAMEID2(a, b) (nameid)(((a) << 8) + (b))
#define MAKENAMEID3(a, b, c) (nameid)(((a) << 16) + ((b) << 8) + (c))
#define MAKENAMEID4(a, b, c, d) (nameid)(((a) << 24) + ((b) << 16) + ((c) << 8) + (d))
#define MAKENAMEID5(a, b, c, d, e) (nameid)((((uint64_t)a) << 32) + ((b) << 24) + ((c) << 16) + ((d) << 8) + (e))
#define MAKENAMEID6(a, b, c, d, e, f) (nameid)((((uint64_t)a) << 40) + (((uint64_t)b) << 32) + ((c) << 24) + ((d) << 16) + ((e) << 8) + (f))
#define MAKENAMEID7(a, b, c, d, e, f, g) (nameid)((((uint64_t)a) << 48) + (((uint64_t)b) << 40) + (((uint64_t)c) << 32) + ((d) << 24) + ((e) << 16) + ((f) << 8) + (g))
#define MAKENAMEID8(a, b, c, d, e, f, g, h) (nameid)((((uint64_t)a) << 56) + (((uint64_t)b) << 48) + (((uint64_t)c) << 40) + (((uint64_t)d) << 32) + ((e) << 24) + ((f) << 16) + ((g) << 8) + (h))

std::string toNodeHandle(handle nodeHandle);
std::string toNodeHandle(NodeHandle nodeHandle);
std::string toHandle(handle h);
std::pair<bool, TypeOfLink> toTypeOfLink (nodetype_t type);
#define LOG_NODEHANDLE(x) toNodeHandle(x)
#define LOG_HANDLE(x) toHandle(x)
class SimpleLogger;
class LocalPath;
SimpleLogger& operator<<(SimpleLogger&, NodeHandle h);
SimpleLogger& operator<<(SimpleLogger&, UploadHandle h);
SimpleLogger& operator<<(SimpleLogger&, NodeOrUploadHandle h);
SimpleLogger& operator<<(SimpleLogger& s, const LocalPath& lp);

typedef enum
{
    FORMAT_SCHEDULED_COPY = 0,  // 20221205123045
    FORMAT_ISO8601        = 1,  // 20221205T123045
} date_time_format_t;

std::string backupTypeToStr(BackupType type);

struct MEGA_API ChunkedHash
{
    static const int SEGSIZE = 131072;

    static m_off_t chunkfloor(m_off_t);
    static m_off_t chunkceil(m_off_t, m_off_t limit = -1);
};

/**
 * @brief Padded encryption using AES-128 in CBC mode.
 */
struct MEGA_API PaddedCBC
{
    /**
     * @brief Encrypts a string after padding it to block length.
     *
     * Note: With an IV, only use the first 8 bytes.
     *
     * @param data Data buffer to be encrypted. Encryption is done in-place,
     *     so cipher text will be in `data` afterwards as well.
     * @param key AES key for encryption.
     * @param iv Optional initialisation vector for encryption. Will use a
     *     zero IV if not given. If `iv` is a zero length string, a new IV
     *     for encryption will be generated and available through the reference.
     */
    static void encrypt(PrnGen &rng, string* data, SymmCipher* key, string* iv = NULL);

    /**
     * @brief Decrypts a string and strips the padding.
     *
     * Note: With an IV, only use the first 8 bytes.
     *
     * @param data Data buffer to be decrypted. Decryption is done in-place,
     *     so plain text will be in `data` afterwards as well.
     * @param key AES key for decryption.
     * @param iv Optional initialisation vector for encryption. Will use a
     *     zero IV if not given.
     * @return Void.
     */
    static bool decrypt(string* data, SymmCipher* key, string* iv = NULL);
};

class MEGA_API HashSignature
{
    Hash* hash;

public:
    // add data
    void add(const byte*, unsigned);

    // generate signature
    unsigned get(AsymmCipher*, byte*, unsigned);

    // verify signature
    bool checksignature(AsymmCipher*, const byte*, unsigned);

    HashSignature(Hash*);
    ~HashSignature();
};

/**
 * @brief Crypto functions related to payments
 */
class MEGA_API PayCrypter
{
    /**
     * @brief Length of the AES key
     */
    static const int ENC_KEY_BYTES = 16;

    /**
     * @brief Lenght of the key to generate the HMAC
     */
    static const int MAC_KEY_BYTES = 32;

    /**
     * @brief Length of the IV for AES-CBC
     */
    static const int IV_BYTES = 16;

    /**
     * @brief Buffer for the AES key and the HMAC key
     */
    byte keys[ENC_KEY_BYTES+MAC_KEY_BYTES];

    /**
     * @brief Pointer to the buffer with the AES key
     */
    byte *encKey;

    /**
     * @brief Pointer to the buffer with the HMAC key
     */
    byte *hmacKey;

    /**
     * @brief Buffer with the IV for AES-CBC
     */
    byte iv[IV_BYTES];

    /**
     * @brief Random blocks generator
     */
    PrnGen &rng;

public:

    /**
     * @brief Constructor. Initializes keys with random values.
     */
    PayCrypter(PrnGen &rng);

    /**
     * @brief Updates the crypto keys (mainly for testing)
     * @param newEncKey New AES key (must contain ENC_KEY_BYTES bytes)
     * @param newHmacKey New HMAC key (must contain MAC_KEY_BYTES bytes)
     * @param newIv New IV for AES-CBC (must contain IV_BYTES bytes)
     */
    void setKeys(const byte *newEncKey, const byte *newHmacKey, const byte *newIv);

    /**
     * @brief Encrypts the cleartext and returns the payload string.
     *
     * The clear text is encrypted with AES-CBC, then a HMAC-SHA256 is generated for (IV + ciphertext)
     * and finally returns (HMAC + IV + ciphertext)
     *
     * @param cleartext Clear text to generate the payload
     * @param result The function will fill this string with the generated payload
     * @return True if the funcion succeeds, otherwise false
     */
    bool encryptPayload(const string *cleartext, string *result);

    /**
     * @brief Encrypts the cleartext using RSA with random padding.
     *
     * A 2-byte header is inserted just before the clear text with the size in bytes.
     * The result is padded with random bytes. Then RSA is applied and the result is returned
     * in the third parameter, with a 2-byte header that contains the size of the result of RSA.
     *
     * @param cleartext Clear text to encrypt with RSA
     * @param pubkdata Public key in binary format (result of AsymmCipher::serializekey)
     * @param pubkdatalen Size (in bytes) of pubkdata
     * @param result RSA encrypted text, with a 2-byte header with the size of the RSA buffer in bytes
     * @param randompadding Enables padding with random bytes. Otherwise, the cleartext is 0-padded
     * @return True if the funcion succeeds, otherwise false
     */
    bool rsaEncryptKeys(const string *cleartext, const byte *pubkdata, int pubkdatalen, string *result, bool randompadding = true);

    /**
     * @brief Encrypts clear text data to an authenticated ciphertext, authenticated with an HMAC.
     * @param cleartext Clear text as byte string
     * @param pubkdata Public key in binary format (result of AsymmCipher::serializekey)
     * @param pubkdatalen Size (in bytes) of pubkdata
     * @param result Encrypted data block as byte string.
     * @param randompadding Enables padding with random bytes. Otherwise, the cleartext is 0-padded
     * @return True if the funcion succeeds, otherwise false
     */
    bool hybridEncrypt(const string *cleartext, const byte *pubkdata, int pubkdatalen, string *result, bool randompadding = true);
};

// read/write multibyte words
struct MEGA_API MemAccess
{
#ifndef ALLOW_UNALIGNED_MEMORY_ACCESS
    template<typename T> static T get(const char* ptr)
    {
        T val;
        memcpy(&val,ptr,sizeof(T));
        return val;
    }

    template<typename T> static void set(byte* ptr, T val)
    {
        memcpy(ptr,&val,sizeof val);
    }
#else
    template<typename T> static T get(const char* ptr)
    {
        return *(T*)ptr;
    }

    template<typename T> static void set(byte* ptr, T val)
    {
        *(T*)ptr = val;
    }
#endif
};

#ifdef _WIN32
int mega_snprintf(char *s, size_t n, const char *format, ...);

// get the Windows error message in UTF-8
std::string winErrorMessage(DWORD error);

#endif

struct MEGA_API TLVstore
{
private:
    TLV_map tlv;

 public:

    /**
     * @brief containerToTLVrecords Builds a TLV object with records from an encrypted container
     * @param data Binary byte array representing the encrypted container
     * @param key Master key to decrypt the container
     * @return A new TLVstore object. You take the ownership of the object.
     */
    static TLVstore * containerToTLVrecords(const string *data, SymmCipher *key);

    /**
     * @brief Builds a TLV object with records from a container
     * @param data Binary byte array representing the TLV records
     * @return A new TLVstore object. You take the ownership of the object.
     */
    static TLVstore * containerToTLVrecords(const string *data);

    /**
     * @brief Converts the TLV records into an encrypted byte array
     * @param key Master key to decrypt the container
     * @param encSetting Block encryption mode to be used by AES
     * @return A new string holding the encrypted byte array. You take the ownership of the string.
     */
    string *tlvRecordsToContainer(PrnGen &rng, SymmCipher *key, encryptionsetting_t encSetting = AES_GCM_12_16);

    /**
     * @brief Converts the TLV records into a byte array
     * @return A new string holding the byte array. You take the ownership of the string.
     */
    string *tlvRecordsToContainer();

    /**
     * @brief get Get the value for a given key
     *
     * In case the type is found, it will update value parameter and return true.
     * In case the type is not found, it will return false and value will not be changed.
     *
     * @param type Type of the value (without scope nor non-historic modifiers).
     * @param value Set to corresponding value if type was found.
     * @return True if type was found, false otherwise.
     */
    bool get(string type, string& value) const;

    /**
     * @brief Get a reference to the TLV_map associated to this TLVstore
     *
     * The TLVstore object retains the ownership of the returned object. It will be
     * valid until this TLVstore object is deleted.
     *
     * @return The TLV_map associated to this TLVstore
     */
    const TLV_map *getMap() const;

    /**
     * @brief Get a list of the keys contained in the TLV
     *
     * You take ownership of the returned value.
     *
     * @return A new vector with the keys included in the TLV
     */
    vector<string> *getKeys() const;

    /**
     * @brief add Adds a new record to the container
     * @param type Type for the new value (without scope nor non-historic modifiers).
     * @param value New value to be set.
     */
    void set(string type, string value);

    /**
     * @brief Removes a record from the container
     * @param type Type for the value to be removed (without scope nor non-historic modifiers).
     */
    void reset(string type);

    size_t size();

    static unsigned getTaglen(int mode);
    static unsigned getIvlen(int mode);
    static encryptionmode_t getMode(int mode);

    ~TLVstore();
};

class Utils {
public:
    /**
     * @brief Converts a character string from UTF-8 to Unicode
     * This method is a workaround for a legacy bug where Webclient used to encode
     * each byte of the array in UTF-8, resulting in a wider string of variable length.
     * @note The UTF-8 string should only contain characters encoded as 1 or 2 bytes.
     * @param src Characters string encoded in UTF-8
     * @param srclen Length of the string (in bytes)
     * @param result String holding the byte array of Unicode characters
     * @return True if success, false if the byte 'src' is not a valid UTF-8 string
     */
    static bool utf8toUnicode(const uint8_t *src, unsigned srclen, string *result);

    /**
     * @brief Determines size in bytes of a valid UTF-8 sequence.
     * @param c first character of UTF-8 sequence
     * @return the size of UTF-8 sequence if its valid, otherwise returns 0
     */
    static size_t utf8SequenceSize(unsigned char c);

    /**
     * @brief This function is analogous to a32_to_str in js version.
     * Converts a vector of <T> elements into a std::string
     *
     * @param data a vector of <T> elements
     * @note this function has been initially designed to work with <T> = uint32_t or <T> = int32_t
     * This is a valid example: <t> = uint32_t, data = [1952805748] => return_value = "test"
     *
     * @return returns a std::string
     */
    template<typename T> static std::string a32_to_str(std::vector<T> data)
    {
        size_t size = data.size() * sizeof(T);
        std::unique_ptr<char[]> result(new char[size]);
        for (size_t i = 0; i < size; ++i)
        {
            result[i] = static_cast<char>((data[i >> 2] >> (24 - (i & 3) * 8)) & 255);
        }
        return std::string (result.get(), size);
    }

    /**
     * @brief This function is analogous to str_to_a32 in js version.
     * Converts a std::string into a vector of <T> elements
     *
     * @param data a std::string
     * @note this function has been initially designed to work with <T> = uint32_t or <T> = int32_t
     * This is a valid example: <t> = uint32_t, data = "test"  => return_value = [1952805748]
     *
     * @return returns a vector of <T> elements
     */
    template<typename T> static std::vector<T> str_to_a32(std::string data)
    {
        std::vector<T> data32((data.size() + 3) >> 2);
        for (size_t i = 0; i < data.size(); ++i)
        {
            data32[i >> 2] |= (data[i] & 255) << (24 - (i & 3) * 8);
        }
        return data32;
    }

    static std::string stringToHex(const std::string& input);
    static std::string hexToString(const std::string& input);
    /**
     * @brief Converts a hexadecimal string to a uint64_t value. The input string may or may not have the hex prefix "0x".
     *
     * @param input The hexadecimal string to be converted (ex: "78b1bbbda5f32526", "0x10FF, "0x0001")
     * @return The uint64_t value corresponding to the input hexadecimal string.
    */
    static uint64_t hexStringToUint64(const std::string &input);
    /**
     * @brief Converts a 8-byte numeric value to a 16-character lowercase hexadecimal string, with zero-padding if necessary.
     *
     * @param input The uint64_t value to be converted to a hexadecimal string.
     * @return A 16-character lowercase hexadecimal string representation of the input value (ex: "78b1bbbda5f32526").
     *
    */
    static std::string uint64ToHexString(uint64_t input);

    static int32_t toLower(const int32_t c)
    {
        return utf8proc_tolower(c);
    }

    static int32_t toUpper(const int32_t c)
    {
        return utf8proc_toupper(c);
    }

    static string toUpperUtf8(const string& text);
    static string toLowerUtf8(const string& text);

    // Platform-independent case-insensitive comparison.
    static int icasecmp(const std::string& lhs,
                        const std::string& rhs,
                        const size_t length);

    static int icasecmp(const std::wstring& lhs,
                        const std::wstring& rhs,
                        const size_t length);

    // Same as above but only case-insensitive on Windows.
    static int pcasecmp(const std::string& lhs,
                        const std::string& rhs,
                        const size_t length);

    static int pcasecmp(const std::wstring& lhs,
                        const std::wstring& rhs,
                        const size_t length);

    static std::string replace(const std::string& str,
                               char search,
                               char replace);
    static std::string replace(const std::string& str,
                               const std::string& search,
                               const std::string& replacement);

    // join({"a", "new", "loom"}, "; ") -> "a; new; loom"
    static std::string join(const std::vector<std::string>& items, const std::string& with);
    static bool startswith(const std::string& str, const std::string& start);
    static bool startswith(const std::string& str, char chr);
    static bool endswith(const std::string& str, char chr);
    static const std::string _trimDefaultChars;
    // return string with trimchrs removed from front and back of given string str
    static string trim(const string& str, const string& trimchars = _trimDefaultChars);


    // --- environment functions that work with Unicode UTF-8 on Windows (set/unset/get) ---

    static bool hasenv(const std::string& key);
    // sets *out_found if found
    static std::string getenv(const std::string& key, bool* out_found);
    // return def if value not found
    static std::string getenv(const std::string& key, const std::string& def);
    static void setenv(const std::string& key, const std::string& value);
    static void unsetenv(const std::string& key);
};

// for pre-c++11 where this version is not defined yet.
long long abs(long long n);

extern m_time_t m_time(m_time_t* tt = NULL);
extern struct tm* m_localtime(m_time_t, struct tm *dt);
extern struct tm* m_gmtime(m_time_t, struct tm *dt);
extern m_time_t m_mktime(struct tm*);
extern int m_clock_getmonotonictime(struct timespec *t);
// Similar behaviour to mktime but it receives a struct tm with a date in UTC and return mktime in UTC
extern m_time_t m_mktime_UTC(const struct tm *src);

/**
 * Converts a datetime from string format into a Unix timestamp
 * Allowed input formats:
 *  + FORMAT_SCHEDULED_COPY  => 20221205123045   => output format: Unix timestamp in deciseconds
 *  + FORMAT_ISO8601         => 20221205T123045  => output format: Unix timestamp in seconds
*/
extern time_t stringToTimestamp(string stime, date_time_format_t format);

std::string rfc1123_datetime( time_t time );
std::string webdavurlescape(const std::string &value);
std::string escapewebdavchar(const char c);
std::string webdavnameescape(const std::string &value);

void tolower_string(std::string& str);

#ifdef __APPLE__
int macOSmajorVersion();
#endif

// file chunk macs
class chunkmac_map
{

    struct ChunkMAC
    {
        // do not change the size or layout, it is directly serialized to db from whatever the binary format is for this compiler/platform
        byte mac[SymmCipher::BLOCKSIZE];

        // For a partially completed chunk, offset is the number of bytes processed (from the start of the chunk)
        // For a finished chunk, it's 0
        // When we start consolidating from the front for macsmac calculation, it's -1 (and finished==true)
        unsigned int offset = 0;

        // True when the entire chunk has been processed.
        // For the special case of the first record being the macsmac calculation to this point,
        // finished == true and offset == -1, and mac == macsmac to the end of this block.
        bool finished = false;

        // True when the chunk is not entirely processed.
        // Offset is only increased by downloads, so (!offset) should always be true for uploads.
        bool notStarted() { return !finished && !offset; }

        // the very first record can be the macsmac calculation so far, from the start to some contiguous point
        bool isMacsmacSoFar() { return finished && offset == unsigned(-1); }
    };

    map<m_off_t, ChunkMAC> mMacMap;

    // we collapse the leading consecutive entries, for large files.
    // this is the map key for how far that collapsing has progressed
    m_off_t macsmacSoFarPos = -1;

    m_off_t progresscontiguous = 0;


public:
    int64_t macsmac(SymmCipher *cipher);
    int64_t macsmac_gaps(SymmCipher *cipher, size_t g1, size_t g2, size_t g3, size_t g4);
    void serialize(string& d) const;
    bool unserialize(const char*& ptr, const char* end);
    void calcprogress(m_off_t size, m_off_t& chunkpos, m_off_t& completedprogress, m_off_t* sumOfPartialChunks = nullptr);
    m_off_t nextUnprocessedPosFrom(m_off_t pos);
    m_off_t expandUnprocessedPiece(m_off_t pos, m_off_t npos, m_off_t fileSize, m_off_t maxReqSize);
    m_off_t hasUnfinishedGap(m_off_t fileSize);
    void finishedUploadChunks(chunkmac_map& macs);
    bool finishedAt(m_off_t pos);
    m_off_t updateContiguousProgress(m_off_t fileSize);
    void updateMacsmacProgress(SymmCipher *cipher);
    void copyEntriesTo(chunkmac_map& other);
    void copyEntryTo(m_off_t pos, chunkmac_map& other);
    void debugLogOuputMacs();

    void ctr_encrypt(m_off_t chunkid, SymmCipher *cipher, byte *chunkstart, unsigned chunksize, m_off_t startpos, int64_t ctriv, bool finishesChunk);
    void ctr_decrypt(m_off_t chunkid, SymmCipher *cipher, byte *chunkstart, unsigned chunksize, m_off_t startpos, int64_t ctriv, bool finishesChunk);

    size_t size() const
    {
        return mMacMap.size();
    }
    void clear()
    {
        mMacMap.clear();
        macsmacSoFarPos = -1;
        progresscontiguous = 0;
    }
    void swap(chunkmac_map& other) {
        mMacMap.swap(other.mMacMap);
        std::swap(macsmacSoFarPos, other.macsmacSoFarPos);
        std::swap(progresscontiguous, other.progresscontiguous);
    }
};

struct CacheableWriter
{
    CacheableWriter(string& d);
    string& dest;

    void serializebinary(byte* data, size_t len);
    void serializecstr(const char* field, bool storeNull);  // may store the '\0' also for backward compatibility. Only use for utf8!  (std::string storing double byte chars will only store 1 byte)
    void serializepstr(const string* field);  // uses string size() not strlen
    void serializestring(const string& field);
    void serializecompressedu64(uint64_t field);
    void serializecompressedi64(int64_t field) { serializecompressedu64(static_cast<uint64_t>(field)); }

    // DO NOT add size_t or other types that are different sizes in different builds, eg 32/64 bit compilation
    void serializei8(int8_t field);
    void serializei32(int32_t field);
    void serializei64(int64_t field);
    void serializeu64(uint64_t field);
    void serializeu32(uint32_t field);
    void serializeu16(uint16_t field);
    void serializeu8(uint8_t field);
    void serializehandle(handle field);
    void serializenodehandle(handle field);
    void serializeNodeHandle(NodeHandle field);
    void serializefsfp(fsfp_t field);
    void serializebool(bool field);
    void serializebyte(byte field);
    void serializedouble(double field);
    void serializechunkmacs(const chunkmac_map& m);

    // Each class that might get extended should store expansion flags at the end
    // When adding new fields to an existing class, set the next expansion flag true to indicate they are present.
    // If you turn on the last flag, then you must also add another set of expansion flags (all false) after the new fields, for further expansion later.
    void serializeexpansionflags(bool b1 = false, bool b2 = false, bool b3 = false, bool b4 = false, bool b5 = false, bool b6 = false, bool b7 = false, bool b8 = false);
};

struct CacheableReader
{
    CacheableReader(const string& d);
    const char* ptr;
    const char* end;
    unsigned fieldnum;

    bool unserializebinary(byte* data, size_t len);
    bool unserializecstr(string& s, bool removeNull); // set removeNull if this field stores the terminating '\0' at the end
    bool unserializestring(string& s);
    bool unserializecompressedu64(uint64_t& field);
    bool unserializecompressedi64(int64_t& field) { return unserializecompressedu64(reinterpret_cast<uint64_t&>(field)); }

    // DO NOT add size_t or other types that are different sizes in different builds, eg 32/64 bit compilation
    bool unserializei8(int8_t& s);
    bool unserializei32(int32_t& s);
    bool unserializei64(int64_t& s);
    bool unserializeu16(uint16_t &s);
    bool unserializeu32(uint32_t& s);
    bool unserializeu8(uint8_t& field);
    bool unserializeu64(uint64_t& s);
    bool unserializebyte(byte& s);
    bool unserializedouble(double& s);
    bool unserializehandle(handle& s);
    bool unserializenodehandle(handle& s);
    bool unserializeNodeHandle(NodeHandle& s);
    bool unserializefsfp(fsfp_t& s);
    bool unserializebool(bool& s);
    bool unserializechunkmacs(chunkmac_map& m);
    bool unserializefingerprint(FileFingerprint& fp);
    bool unserializedirection(direction_t& field);  // historic; size varies by compiler.  todo: Remove when we next roll the transfer db version

    bool unserializeexpansionflags(unsigned char field[8], unsigned usedFlagCount);

    void eraseused(string& d); // must be the same string, unchanged
    bool hasdataleft() { return end > ptr; }
};

template<typename T, typename U>
void hashCombine(T& seed, const U& v)
{
    static_assert(std::is_integral<T>::value, "T is not integral");
    // the magic number is the twos complement version of the golden ratio
    seed ^= std::hash<U>{}(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

struct FileAccess;
struct InputStreamAccess;
class SymmCipher;

std::pair<bool, int64_t> generateMetaMac(SymmCipher &cipher, FileAccess &ifAccess, const int64_t iv);

std::pair<bool, int64_t> generateMetaMac(SymmCipher &cipher, InputStreamAccess &isAccess, const int64_t iv);

bool CompareLocalFileMetaMacWithNodeKey(FileAccess* fa, const std::string& nodeKey, int type);

bool CompareLocalFileMetaMacWithNode(FileAccess* fa, Node* node);

// Helper class for MegaClient.  Suitable for expansion/templatizing for other use caes.
// Maintains a small thread pool for executing independent operations such as encrypt/decrypt a block of data
// The number of threads can be 0 (eg. for helper MegaApi that deals with public folder links) in which case something queued is
// immediately executed synchronously on the caller's thread
struct MegaClientAsyncQueue
{
    void push(std::function<void(SymmCipher&)> f, bool discardable);
    void clearDiscardable();

    MegaClientAsyncQueue(Waiter& w, unsigned threadCount);
    ~MegaClientAsyncQueue();

private:
    Waiter& mWaiter;
    std::mutex mMutex;
    std::condition_variable mConditionVariable;

    struct Entry
    {
        bool discardable = false;
        std::function<void(SymmCipher&)> f;
        Entry(bool disc, std::function<void(SymmCipher&)>&& func)
             : discardable(disc), f(func)
        {}
    };

    std::deque<Entry> mQueue;
    std::vector<std::thread> mThreads;
    SymmCipher mZeroThreadsCipher;

    void asyncThreadLoop();
};

template<class T>
struct ThreadSafeDeque
{
    // Just like a deque, but thread safe so that a separate thread can receive filesystem notifications as soon as they are available.
    // When we try to do that on the same thread, the processing of queued notifications is too slow so more notifications bulid up than
    // have been processed, so each time we get the outstanding ones from the buffer we gave to the OS, we need to give it an even
    // larger buffer to write into, otherwise it runs out of space before this thread is idle and can get the next batch from the buffer.
protected:
    std::deque<T> mNotifications;
    std::mutex m;

public:

    bool peekFront(T& t)
    {
        std::lock_guard<std::mutex> g(m);
        if (!mNotifications.empty())
        {
            t = mNotifications.front();
            return true;
        }
        return false;
    }

    bool popFront(T& t)
    {
        std::lock_guard<std::mutex> g(m);
        if (!mNotifications.empty())
        {
            t = std::move(mNotifications.front());
            mNotifications.pop_front();
            return true;
        }
        return false;
    }

    void unpopFront(const T& t)
    {
        std::lock_guard<std::mutex> g(m);
        mNotifications.push_front(t);
    }

    void pushBack(T&& t)
    {
        std::lock_guard<std::mutex> g(m);
        mNotifications.push_back(t);
    }

    bool empty()
    {
        std::lock_guard<std::mutex> g(m);
        return mNotifications.empty();
    }

    size_t size()
    {
        std::lock_guard<std::mutex> g(m);
        return mNotifications.size();
    }

};

template<typename CharT>
struct UnicodeCodepointIteratorTraits;

template<>
struct UnicodeCodepointIteratorTraits<char>
{
    static ptrdiff_t get(int32_t& codepoint, const char* m, const char* n)
    {
        assert(m && n && m < n);

        return utf8proc_iterate(reinterpret_cast<const uint8_t*>(m),
                                n - m,
                                &codepoint);
    }

    static size_t length(const char* s)
    {
        assert(s);

        return strlen(s);
    }
}; // UnicodeCodepointIteratorTraits<char>

template<>
struct UnicodeCodepointIteratorTraits<wchar_t>
{
    static ptrdiff_t get(int32_t& codepoint, const wchar_t* m, const wchar_t* n)
    {
        assert(m && n && m < n);

        // Are we looking at a high surrogate?
        if ((*m >> 10) == 0x36)
        {
            // Is it followed by a low surrogate?
            if (n - m < 2 || (m[1] >> 10) != 0x37)
            {
                // Nope, the string is malformed.
                return -1;
            }

            // Compute addend.
            const int32_t lo = m[1] & 0x3ff;
            const int32_t hi = *m & 0x3ff;
            const int32_t addend = (hi << 10) | lo;

            // Store effective code point.
            codepoint = 0x10000 + addend;

            return 2;
        }

        // Are we looking at a low surrogate?
        if ((*m >> 10) == 0x37)
        {
            // Then the string is malformed.
            return -1;
        }

        // Code point is encoded by a single code unit.
        codepoint = *m;

        return 1;
    }

    static size_t length(const wchar_t* s)
    {
        assert(s);

        return wcslen(s);
    }
}; // UnicodeCodepointIteratorTraits<wchar_t>

template<typename CharT>
class UnicodeCodepointIterator
{
public:
    using traits_type = UnicodeCodepointIteratorTraits<CharT>;

    UnicodeCodepointIterator(const CharT* s, size_t length)
      : mCurrent(s)
      , mEnd(s + length)
    {
    }

    explicit UnicodeCodepointIterator(const std::basic_string<CharT>& s)
      : UnicodeCodepointIterator(s.data(), s.size())
    {
    }

    explicit UnicodeCodepointIterator(const CharT* s)
      : UnicodeCodepointIterator(s, traits_type::length(s))
    {
    }

    UnicodeCodepointIterator(const UnicodeCodepointIterator& other)
      : mCurrent(other.mCurrent)
      , mEnd(other.mEnd)
    {
    }

    UnicodeCodepointIterator()
      : mCurrent(nullptr)
      , mEnd(nullptr)
    {
    }

    UnicodeCodepointIterator& operator=(const UnicodeCodepointIterator& rhs)
    {
        if (this != &rhs)
        {
            mCurrent = rhs.mCurrent;
            mEnd = rhs.mEnd;
        }

        return *this;
    }

    bool operator==(const UnicodeCodepointIterator& rhs) const
    {
        return mCurrent == rhs.mCurrent && mEnd == rhs.mEnd;
    }

    bool operator!=(const UnicodeCodepointIterator& rhs) const
    {
        return !(*this == rhs);
    }

    bool end() const
    {
        return mCurrent == mEnd;
    }

    int32_t get()
    {
        int32_t result = 0;

        if (mCurrent < mEnd)
        {
            ptrdiff_t nConsumed = traits_type::get(result, mCurrent, mEnd);
            assert(nConsumed > 0);
            mCurrent += nConsumed;
        }

        return result;
    }

    bool match(const int32_t character)
    {
        if (peek() != character)
        {
            return false;
        }

        (void)get();

        return true;
    }

    int32_t peek() const
    {
        int32_t result = 0;

        if (mCurrent < mEnd)
        {
            #ifndef NDEBUG
            ptrdiff_t nConsumed =
            #endif
                traits_type::get(result, mCurrent, mEnd);
            assert(nConsumed > 0);
        }

        return result;
    }

private:
    const CharT* mCurrent;
    const CharT* mEnd;
}; // UnicodeCodepointIterator<CharT>

template<typename CharT>
UnicodeCodepointIterator<CharT> unicodeCodepointIterator(const std::basic_string<CharT>& s)
{
    return UnicodeCodepointIterator<CharT>(s);
}

template<typename CharT>
UnicodeCodepointIterator<CharT> unicodeCodepointIterator(const CharT* s, size_t length)
{
    return UnicodeCodepointIterator<CharT>(s, length);
}

template<typename CharT>
UnicodeCodepointIterator<CharT> unicodeCodepointIterator(const CharT* s)
{
    return UnicodeCodepointIterator<CharT>(s);
}

inline int hexval(const int c)
{
    return ((c & 0xf) + (c >> 6)) | ((c >> 3) & 0x8);
}

bool islchex_high(const int c);
bool islchex_low(const int c);

// gets a safe url by replacing private parts to be used in logs
std::string getSafeUrl(const std::string &posturl);

bool readLines(FileAccess& ifAccess, string_vector& destination);
bool readLines(InputStreamAccess& isAccess, string_vector& destination);
bool readLines(const std::string& input, string_vector& destination);

bool wildcardMatch(const string& text, const string& pattern);
bool wildcardMatch(const char* text, const char* pattern);

struct MEGA_API FileSystemAccess;

// generate a new drive id
handle generateDriveId(PrnGen& rng);

// return API_OK if success and set driveID handle to the drive id read from the drive,
// otherwise return error code and set driveId to UNDEF
error readDriveId(FileSystemAccess& fsAccess, const char* pathToDrive, handle& driveId);
error readDriveId(FileSystemAccess& fsAccess, const LocalPath& pathToDrive, handle& driveId);

// return API_OK if success, otherwise error code
error writeDriveId(FileSystemAccess& fsAccess, const char* pathToDrive, handle driveId);

int platformGetRLimitNumFile();

bool platformSetRLimitNumFile(int newNumFileLimit = -1);

void debugLogHeapUsage();

bool haveDuplicatedValues(const string_map& readableVals, const string_map& b64Vals);

struct SyncTransferCount
{
    bool operator==(const SyncTransferCount& rhs) const;
    bool operator!=(const SyncTransferCount& rhs) const;
    void operator-=(const SyncTransferCount& rhs);

    uint32_t mCompleted = 0;
    uint32_t mPending = 0;
    uint64_t mCompletedBytes = 0;
    uint64_t mPendingBytes = 0;
};

struct SyncTransferCounts
{
    bool operator==(const SyncTransferCounts& rhs) const;
    bool operator!=(const SyncTransferCounts& rhs) const;
    void operator-=(const SyncTransferCounts& rhs);

    // returns progress 0.0 to 1.0
    double progress(m_off_t inflightProgress) const;

    SyncTransferCount mDownloads;
    SyncTransferCount mUploads;
};

// creates a new id filling `id` with random bytes, up to `length`
void resetId(char* id, size_t length, PrnGen& rng);

// write messsage and strerror(aerrno) to log as an error
void reportError(const std::string& message, int aerrno = -1);

#ifdef WIN32

// as per (non C library standard) unix API
inline void sleep(int sec) {
    Sleep(sec * 1000);
}

// as per (non C library standard) unix API
// sleep for given number of microseconds
inline void usleep(int microsec) {
    Sleep(microsec / 1000);
}

// print messgae: error-num: error-description
void reportWindowsError(const std::string& message, DWORD error = 0xFFFFFFFF);

#endif // WIN32

// returns the direction type of a connection
string connDirectionToStr(direction_t directionType);

} // namespace mega

#endif // MEGA_UTILS_H
