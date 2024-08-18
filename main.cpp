#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <windows.h>
#include <format>
#include <vector>
#include <random>
#include "sha256.h"

int filter_exception(unsigned int code, struct _EXCEPTION_POINTERS* ep)
{
    std::cout << std::format("Exception code: 0x{:x}", code);
    if (ep != nullptr && ep->ExceptionRecord != nullptr)
        std::cout << ", Address=0x" << (void*)ep->ExceptionRecord->ExceptionAddress;
    std::cout << " : ";
    switch (code)
    {
    case  EXCEPTION_ACCESS_VIOLATION:
        std::cout << "Acess Violation" << std::endl;
        break;
    case  EXCEPTION_DATATYPE_MISALIGNMENT:
        std::cout << "Data Type MisAlignement" << std::endl;
        break;
    case  EXCEPTION_BREAKPOINT:
        std::cout << "Breakpoint" << std::endl;
        break;
    case  EXCEPTION_SINGLE_STEP:
        std::cout << "Single Step" << std::endl;
        break;
    case  EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        std::cout << "Array Bounds Exceeded" << std::endl;
        break;
    case  EXCEPTION_FLT_DENORMAL_OPERAND:
        std::cout << "Floating Point Denormal Operand" << std::endl;
        break;
    case  EXCEPTION_FLT_DIVIDE_BY_ZERO:
        std::cout << "Floating Point Divide By Zero" << std::endl;
        break;
    case  EXCEPTION_FLT_INEXACT_RESULT:
        std::cout << "Floating Point Inexact Result" << std::endl;
        break;
    case  EXCEPTION_FLT_INVALID_OPERATION:
        std::cout << "Floating Point Invalid Operation" << std::endl;
        break;
    case  EXCEPTION_FLT_OVERFLOW:
        std::cout << "Floating Point Overflow" << std::endl;
        break;
    case  EXCEPTION_FLT_STACK_CHECK:
        std::cout << "Floating Point Stack Check" << std::endl;
        break;
    case  EXCEPTION_FLT_UNDERFLOW:
        std::cout << "Floating Point Underflow" << std::endl;
        break;
    case  EXCEPTION_INT_DIVIDE_BY_ZERO:
        std::cout << "Integer Divide By Zero" << std::endl;
        break;
    case  EXCEPTION_INT_OVERFLOW:
        std::cout << "Integer Overflow" << std::endl;
        break;
    case  EXCEPTION_PRIV_INSTRUCTION:
        std::cout << "Privileged Instruction" << std::endl;
        break;
    case  EXCEPTION_IN_PAGE_ERROR:
        std::cout << "Page Error" << std::endl;
        break;
    case  EXCEPTION_ILLEGAL_INSTRUCTION:
        std::cout << "Illegal Instruction" << std::endl;
        break;
    case  EXCEPTION_NONCONTINUABLE_EXCEPTION:
        std::cout << "Non Continuable Exception" << std::endl;
        break;
    case  EXCEPTION_STACK_OVERFLOW:
        std::cout << "Stack Overflow" << std::endl;
        break;
    case  EXCEPTION_INVALID_DISPOSITION:
        std::cout << "Invalid Disposition" << std::endl;
        break;
    case  EXCEPTION_GUARD_PAGE:
        std::cout << "Guard Page" << std::endl;
        break;
    case  EXCEPTION_INVALID_HANDLE:
        std::cout << "Invalid Handle" << std::endl;
        break;
    case  CONTROL_C_EXIT:
        std::cout << "Control-C Exit" << std::endl;
        break;
    default:
        std::cout << "Unknown Exception" << std::endl;
        break;
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

void parseOption(int argc, char** argv, bool& throwfpe, size_t& N)
{
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-throwfpe") == 0)
        {
            throwfpe = true;
        }
        if (strcmp(argv[i], "-N") == 0)
        {
            N = atoll(argv[i + 1]);
        }
    }
}

unsigned int
flagsValues()
{
    return   _EM_ZERODIVIDE | _EM_INVALID | _EM_OVERFLOW;
}

static bool G_throwFPE = false;
void setThrowFPE()
{
    unsigned int cw;
    _controlfp_s(&cw, 0, 0);
    unsigned int new_value = cw & ~flagsValues();
    _controlfp_s(&cw, new_value, _MCW_EM);
    std::cout << "Thow FPE activated" << std::endl;
    G_throwFPE = true;
}

void unsetThrowFPE()
{
    unsigned int cw;
    _controlfp_s(&cw, 0, 0);
    unsigned int new_value = cw | flagsValues();
    _controlfp_s(&cw, new_value, _MCW_EM);
    std::cout << "Thow FPE desactivated" << std::endl;
    G_throwFPE = false;
}


double
toDouble(unsigned long long value)
{
    return *reinterpret_cast<double*>(&value);
}

unsigned long long 
toLongLong(double value)
{
    return *reinterpret_cast<unsigned long long*>(&value);
}

std::string
hash_result(const char* data, size_t size)
{
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (const BYTE*)data, size);
    unsigned char out[32];
    sha256_final(&ctx, out);

    std::string result;
    for (int i = 0; i < 32; i++)
        result += std::format("{:02x}", (int)out[i]);
    return result;
}

void check_initial_vector(size_t N, std::string hash)
{
    if (N == 10000000)
    {
        if (hash != "e9384d8448ece924f31f4d500017baf6583c37d1eb59a8ba98b78d5b84dd5449")
        {
            throw std::runtime_error("Invalid initial vector!");
        }
        std::cout << "Inital vector checked" << std::endl;
    }
}

std::vector<unsigned long long> 
generateTestVector(size_t N)
{
    std::vector<unsigned long long> data(N);
    std::mt19937_64 gen64;
    for (size_t i = 0; i < N; i++)
    {
        unsigned long long value;
        double d;
        do
        {
            do
            {
                value = gen64();
            } while ((value >= 0x7FF0000000000001 && value <= 0x7FF7FFFFFFFFFFFF) ||
                (value >= 0xFFF0000000000001 && value <= 0xFFF7FFFFFFFFFFFF));
            d = toDouble(value);
        } while (!std::isfinite(d));
        data[i] = value;
    }
    auto hashstr = hash_result((const char*)data.data(), N * sizeof(unsigned long long));
    std::cout << "hash test  : " << hashstr << std::endl;
    check_initial_vector(N, hashstr);
    return data;
}

double test_function(double x)
{
    while (std::abs(x) > 1e+100)
        x /= 1e+100;
    while (std::abs(x) < 1e-100)
        x *= 1e+100;
    return  1 / x + 1 / std::sqrt((x * x + 1.235)) + std::log(x * x + 0.12546);
}

std::string 
save_result(const char* data, size_t size, const std::string& config)
{
    std::string filename = std::format("result_{}.bin", config);
    FILE* f = fopen(filename.c_str(), "wb");
    if (f != nullptr)
    {
        fwrite(data, 1, size, f);
        fclose(f);
    }
    return filename;
}

std::string
getConfigString(size_t N)
{
    std::string config;
    config += std::format("{}", N);
    if (G_throwFPE)
        config += "_FPE";
#ifdef _WIN64
    config += "_64bits";
#else
    config += "_32bits";
#endif

#ifdef _DEBUG
    config += "_Debug";
#else
    config += "_Release";
#endif
    return config;
}

size_t G_last_index = 0;
std::string test_calculation(size_t N, int counter)
{
    auto config = getConfigString(N) + "_" + std::format("_#{}", counter);
    std::cout << "Config: " << config << std::endl;
    auto data = generateTestVector(N);
    std::vector<unsigned long long> result(N);

    for (size_t i = 0; i < N; i++)
    {
        G_last_index = i;
        result[i] = toLongLong(test_function(toDouble(data[i])));
    }
    auto hashstr = hash_result((const char*)result.data(), N * sizeof(unsigned long long));
    std::cout << "hash result: " << hashstr << std::endl;
    return save_result((const char*)result.data(), N * sizeof(unsigned long long), hashstr + "_" + config);
}

std::vector<unsigned long long> 
load_result(const std::string& filename)
{
    std::vector<unsigned long long> data;
    FILE* f = fopen(filename.c_str(), "rb");
    if (f != nullptr)
    {
        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);
        fseek(f, 0, SEEK_SET);
        if (size % sizeof(unsigned long long) != 0)
            throw std::runtime_error("Invalid file size");
        data.resize(size / sizeof(unsigned long long));
        fread(data.data(), 1, size, f);
        fclose(f);
    }
    else
    {
        throw std::runtime_error("File not found: " + filename);
    }
    return data;
}

void 
compare_results(const std::string& file1, const std::string& file2)
{
    std::cout << "Comparing files: " << file1 << " and " << file2 << std::endl;
    //std::string file1 = "result_ba18c0cbd7220b4645bd832e16a4f7b453b79340c1a6b35bf09363ce9e4f9203_10000000_32bits_Release__#0.bin";
    //std::string file2 = "result_7c448f88e2a76a4c05b133c84d2f487e54a7a16410bb6e1109067b6813a535ac_10000000_FPE_32bits_Release__#1.bin";

    auto data1 = load_result(file1);
    auto data2 = load_result(file2);

    auto hash1= hash_result((const char*)data1.data(), data1.size() * sizeof(unsigned long long));
    auto hash2 = hash_result((const char*)data2.data(), data2.size() * sizeof(unsigned long long));

    std::cout << file1  << " : " << hash1 << std::endl;
    std::cout << file2  << " : " << hash2 << std::endl;

    if (data1.size() != data2.size())
        throw std::runtime_error("Invalid size");

    int diffs = 0;
    for (size_t i = 0; i < data1.size(); i++)
    {
        if (data1[i] != data2[i])
        {
            std::cout << "Difference at index: " << i << std::endl;
            std::cout << "Value1: " << std::format("{}\t0x{:x}", toDouble(data1[i]), data1[i]) << std::endl;
            std::cout << "Value2: " << std::format("{}\t0x{:x}", toDouble(data2[i]), data2[i]) << std::endl;
            ++diffs;
        }
    }
    std::cout << "Diffs: " << diffs << std::endl;
}

int effective_main(int argc, char** argv)
{
    try
    {
        bool throwfpe = false;
        size_t N = 100;
        parseOption(argc, argv, throwfpe, N);

        std::string file0 = test_calculation(N, 0);
        if (throwfpe)
            setThrowFPE();
        std::string file1 = test_calculation(N, 1);
        if (throwfpe)
            unsetThrowFPE();
        std::string file2 = test_calculation(N, 2);

        compare_results(file1, file2);
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}


int main(int argc, char **argv) 
{
    __try
    {
        return effective_main(argc, argv);
    }
    __except (filter_exception(GetExceptionCode(), GetExceptionInformation()))
    {
        std::cout << "Last index: " << G_last_index << std::endl;
        exit(GetExceptionCode());
    }
    return 0;
}
