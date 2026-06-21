#include "geofix.h"
#include <chrono>
#include <thread>
#include <vector>
#include <array>

// #define TWO_PI 6.28318530718;

Napi::Array stdVectorDoubleToNapiArray(const std::vector<double> &vec, Napi::Env env)
{
    Napi::Array array = Napi::Array::New(env, vec.size());
    for (size_t i = 0; i < vec.size(); ++i)
    {
        array.Set(static_cast<uint32_t>(i), Napi::Number::New(env, vec[i]));
    }
    return array;
};


Napi::FunctionReference RFDFGeo::constructor;
/**
 *
 * @param info
 */
RFDFGeo::RFDFGeo(const Napi::CallbackInfo &info) : Napi::ObjectWrap<RFDFGeo>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    // int length = info.Length();
    // if (length != 4)
    // {
    //     Napi::TypeError::New(env, "Four arguments expected").ThrowAsJavaScriptException();
    // }
    // if (!info[0].IsNumber())
    // {
    //     Napi::Object object_parent = info[0].As<Napi::Object>();
    //     RFDFGeo *example_parent = Napi::ObjectWrap<RFDFGeo>::Unwrap(object_parent);
    //     return;
    // }
}

Napi::String getTimestamp(const Napi::CallbackInfo &info)
// {
//     Napi::Env env = info.Env();
//     return Napi::String::New(env, std::string(__DATE__) + " " + std::string(__TIME__));
// }

// Napi::String getBuildHash(const Napi::CallbackInfo &info)
// {
//     Napi::Env env = info.Env();
//     return Napi::String::New(env, std::string(BUILD_HASH));
// }

Napi::Function RFDFGeo::GetClass(Napi::Env env)
{

    Napi::Function func = DefineClass(env, "RFDFGeo", {
        InstanceMethod("addData", &RFDFGeo::addData), 
        InstanceMethod("make", &RFDFGeo::make), 
        InstanceMethod("makeAsync", &RFDFGeo::makeAsync)
        });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    return func;
}

/**
 *
 * @param env
 * @param exports
 * @return
 */
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);
    exports.Set("RFDFGeo", RFDFGeo::GetClass(env));
    exports.Set("getTimestamp", Napi::Function::New(env, getTimestamp));
    // exports.Set("getBuildHash", Napi::Function::New(env, getBuildHash));
    return exports;
}

/**
 *
 * @param info
 * @return
 */
Napi::Value RFDFGeo::addData(const Napi::CallbackInfo &info)
{
    // if (info.Length() != 1)
    // {
    //     Napi::Error::New(info.Env(), "Expected exactly one argument").ThrowAsJavaScriptException();
    // }
    // if (!info[0].IsTypedArray())
    // {
    //     Napi::Error::New(info.Env(), "Expected an Array").ThrowAsJavaScriptException();
    // }

    Napi::Array outputArray = Napi::Array::New(info.Env(), 6);

    return outputArray;
}

/**
 *
 * @param info
 * @return
 */
Napi::Value RFDFGeo::make(const Napi::CallbackInfo &info)
{
    double arg = info[0].As<Napi::Number>();
    Napi::TypedArray outputArray = Napi::Uint8Array::New(info.Env(), 90 * 360);
    return outputArray;
}

// /**
//  *
//  * @param info
//  * @return
//  */
// Napi::Value RFDFGeo::makeAsync(const Napi::CallbackInfo &info)
// {
//     double arg = info[0].As<Napi::Number>();
//     Napi::Function callback = info[1].As<Napi::Function>();
//     AsyncMaker *asyncWorker = new AsyncMaker(callback, arg);
//     asyncWorker->Queue();
//     std::string msg = "making async";
//     return Napi::String::New(info.Env(), msg.c_str());
// };

// /**
//  *
//  * @param callback
//  * @param arg
//  */
// AsyncMaker::AsyncMaker(Napi::Function &callback, double arg) : AsyncWorker(callback), arg(arg){};

// /**
//  *
//  */
// void AsyncMaker::Execute()
// {
//     // double spectrum[90 * 360];
//     // std::map<std::string, double> meteData = music.makeMusic(frequency, elevationFactor, spectrum, image);
// };

// /**
//  *
//  */
// void AsyncMaker::OnOK()
// {
//     Napi::TypedArray outputArray2 = Napi::Uint8Array::New(Env(), 90 * 360);
//     // auto start = std::chrono::steady_clock::now();
//     for (int i = 0; i < 90 * 360; i++)
//     {
//         outputArray2[i] = Napi::Number::New(Env(), image[i]);
//     }
//     Callback().Call({Env().Null(), outputArray2});
// };
