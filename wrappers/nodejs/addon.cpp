#include <iostream>
#include <vector>
#include <napi.h>

class NativeClass : public Napi::ObjectWrap<NativeClass> {
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  NativeClass(const Napi::CallbackInfo& info);
private:
  Napi::Value doStuff(const Napi::CallbackInfo& info);
  static Napi::FunctionReference constructor;
};

Napi::FunctionReference NativeClass::constructor;

NativeClass::NativeClass(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NativeClass>(info)  {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    int length = info.Length();
    if (length != 1) {
        Napi::TypeError::New(env, "One argument expected").ThrowAsJavaScriptException();
    }
    if(!info[0].IsNumber()){
        Napi::Object object_parent = info[0].As<Napi::Object>();
        NativeClass* example_parent = Napi::ObjectWrap<NativeClass>::Unwrap(object_parent);
        return;
    }
}

Napi::Value NativeClass::doStuff(const Napi::CallbackInfo& info) {

    // std::string path = info[0].As<Napi::String>();
    // int property = info[1].As<Napi::Number>();
    // int dataSize = info[2].As<Napi::Number>();

    Napi::Object obj = info[0].As<Napi::Object>();

    Napi::Array summationMap = obj.Get("array1").As<Napi::Array>();
    int nChannels = (int) summationMap.Length();
    std::vector<std::vector<double>> sumMap;
    for(int i = 0; i < nChannels; i++) {
        Napi::Array b = summationMap.Get(std::to_string(i)).As<Napi::Array>();
        // std::cout << b.Length() << std::endl;

        std::vector<double> sumMapChannel;
        for(int j = 0; j < (int)b.Length(); j++) {
            Napi::Value v = b[i];
            if (v.IsNumber())
            {
                double value = (double) v.As<Napi::Number>();
                sumMapChannel.push_back(value);
                // std::cout << value << std::endl;
            }
        }
        sumMap.push_back(sumMapChannel);
    }
    // std::cout << "summationMap: " << sumMap.size() << " "<< sumMap[0].size() << std::endl;


    Napi::Array antennaPositions = obj.Get("array2").As<Napi::Array>();
    int nAntennas = (int) antennaPositions.Length();
    std::vector<std::vector<double>> antPos;
    for(int i = 0; i < nAntennas; i++) {
        Napi::Array b = antennaPositions.Get(std::to_string(i)).As<Napi::Array>();
        // std::cout << b.Length() << std::endl;

        std::vector<double> antCoord;
        for(int j = 0; j < (int)b.Length(); j++) {
            Napi::Value v = b[i];
            if (v.IsNumber())
            {
                double value = (double) v.As<Napi::Number>();
                antCoord.push_back(value);
                // std::cout << value << std::endl;
            }
        }
        antPos.push_back(antCoord);
    }
    // std::cout << "antennaPositions: " << antPos.size() << " "<< antPos[0].size() << std::endl;


    // auto l = b.Length();
    // std::cout << l << std::endl;

    // for(int i = 0; i < (int)l; i++)
    // {
    //   Napi::Value v = b[i];
    //   if (v.IsNumber())
    //   {
    //     double value = (double) v.As<Napi::Number>();
    //       std::cout << value << std::endl;
    //   }
    // }


    // Napi::Object obj = info[0].As<Napi::Array>();
    // // Napi::Object obj = info[0].As<Napi::Object>();
    // // Napi::Array testArray = obj.Get('test').As<Napi::Array>();

    // std::cout << obj.Length() << std::endl;

    // // for(int i = 0; i<testArray.Length(); i++)
    // // {
    // //   Napi::Value v = testArray[i];
    // //   if (v.IsNumber())
    // //   {
    // //     int value = (int)v.As<Napi::Number>();
    // //     std::cout << value << std::endl;
    // //   }
    // // }

    // int i = 0;
    // double dum = testArray[i].As<Number>().DoubleValue();

    // std::cout << dum << std::endl;
    // std::cout << testArray[1] << std::endl;
    // std::cout << testArray[2] << std::endl;

    // Napi::Float64Array buf = info[0].As<Napi::Float64Array>();

    Napi::Array outputArray = Napi::Array::New(info.Env(), 3);
    int i = 0; outputArray[i] = 1;
    i = 1; outputArray[i] = 2;
    i = 2; outputArray[i] = 3;

    return outputArray;
}

Napi::Object NativeClass::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(env, "NativeClass", {
            InstanceMethod("doStuff", &NativeClass::doStuff),
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("NativeClass", func);
    return exports;
}


// class AsyncWorkerClass : public Napi::AsyncWorker {
// public:
//     AsyncWorkerClass(Napi::Function& callback, int runTime, double frequency, Music music);
//     virtual ~AsyncWorkerClass() {};
//     void Execute();
//     void OnOK();
//     // unsigned char image[90 * 360];
//     // double frequency;
// private:
//     // Music music;
//     int runTime;
// };


Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  return NativeClass::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll);


// #include <node.h>

// using v8::FunctionCallbackInfo;
// using v8::Isolate;
// using v8::Local;
// using v8::NewStringType;
// using v8::Object;
// using v8::String;
// using v8::Value;

// void Method(const FunctionCallbackInfo<Value>& args) {
//   Isolate* isolate = args.GetIsolate();
//   v8::MaybeLocal<v8::String> str = String::NewFromUtf8(isolate, "world", NewStringType::kNormal);
//   v8::Local<v8::String> checkedString = str.ToLocalChecked();
//   v8::ReturnValue<v8::Value> retVal = args.GetReturnValue();
//   retVal.Set(checkedString);
// }

// void Initialize(Local<Object> exports) {
//   NODE_SET_METHOD(exports, "hello", Method);
// }

// NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)