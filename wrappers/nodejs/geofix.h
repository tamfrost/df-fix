#include <napi.h>
#include <node_api.h>
#include <chrono>

Napi::Object Init(Napi::Env env, Napi::Object exports);

class RFDFGeo : public Napi::ObjectWrap<RFDFGeo>
{
public:
  RFDFGeo(const Napi::CallbackInfo &info);
  static Napi::Function GetClass(Napi::Env env);

private:
  static Napi::FunctionReference constructor;
  static Napi::FunctionReference constructor2;
  Napi::Value addData(const Napi::CallbackInfo &info);
  // Napi::Value makeAsync(const Napi::CallbackInfo &info);
};

// class AsyncMaker : public Napi::AsyncWorker
// {
// public:
// public:
//   AsyncMaker(Napi::Function &callback, double arg);
//   virtual ~AsyncMaker(){};
//   void Execute();
//   void OnOK();
//   double arg;
// };
